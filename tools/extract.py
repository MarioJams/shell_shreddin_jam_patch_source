#!/usr/bin/env python3
import sys
import subprocess
from pathlib import Path

if len(sys.argv) != 5:
    print(f"Usage: {sys.argv[0]} [base elf file] [input elf file] [input z64 file] [patch output folder]")

nm = "mips-n64-nm"
objdump = "mips-n64-objdump"
dd = "dd"
input_elf_file = sys.argv[1]
elf_file = sys.argv[2]
z64_file = sys.argv[3]
output_folder = Path(sys.argv[4])

nm_output = subprocess.run([nm, elf_file], stdout=subprocess.PIPE)
objdump_output = subprocess.run([objdump, "-t", input_elf_file], stdout=subprocess.PIPE)

nm_lines = str(nm_output.stdout.decode('utf-8')).splitlines()
objdump_lines = str(objdump_output.stdout.decode('utf-8')).splitlines()

func_ends = dict()
hook_addrs = dict()
addr_to_funcs = dict()
func_sizes = dict()
func_names = dict()
num_funcs = 0

func_ends[-1] = 0

for line in nm_lines:
    tokens = line.split()
    if len(tokens) == 3:
        value = int(tokens[0], 16)
        name = tokens[2]
        # print(f"Name: {name} Value: {value}")
        if name.startswith('func_end_'):
            func_index = int(name[len('func_end_'):])
            func_ends[func_index] = value
            num_funcs += 1
        if name.startswith('hook_'):
            func_index = int(name[len('hook_'):])
            hook_addrs[func_index] = value & 0xFFFFFFFF
            addr_to_funcs[value & 0xFFFFFFFF] = [name, func_index]

for line in objdump_lines:
    tokens = line.split()
    if len(tokens) == 6:
        addr = int(tokens[0], 16)
        size = int(tokens[4], 16)
        func = tokens[5]
        if addr in addr_to_funcs:
            func_index = addr_to_funcs[addr][1]
            func_sizes[func_index] = size
            func_names[func_index] = func
            # print(f'func {func} index {func_index} replaced')

jump_hooks = "\n.headersize 0x80246000 - 0x1000\n"
incbins = ".headersize 0x80400000 - 0x1200000\n.org 0x80408000\n"
bbp_global_asms = ""
bbp_injected = ""

(output_folder / "bbp").mkdir(parents=True, exist_ok=True)
(output_folder / "rm").mkdir(parents=True, exist_ok=True)

for idx in range(num_funcs):
    start = func_ends[idx - 1]
    end = func_ends[idx]
    size = end - start
    func_name = func_names[idx]
    cur_rm_func_bin = output_folder / f"rm/{func_name}_jam.bin"
    cur_bbp_func_bin = output_folder / f"bbp/{func_name}_jam.bin"

    subprocess.run([dd, f"if={z64_file}", f"of={str(cur_rm_func_bin)}", "bs=1", f"skip={start + 0x1208000}", f"count={size}", "status=none"])
    subprocess.run(["cp", str(cur_rm_func_bin), str(cur_bbp_func_bin)])
    
    jump_hooks += f".org 0x{hook_addrs[idx]:x}\nj {func_name}_jam\nnop\n"
    incbins += f"{func_name}_jam:\n.incbin \"{func_name}_jam.bin\"\n"
    
    bbp_global_asms += f"        {{\n            \"file\": \"{func_name}_hook.asm\",\n            \"start\": \"0x{hook_addrs[idx]:x}\",\n            \"end\": \"0x{hook_addrs[idx] + func_sizes[idx]:x}\"\n        }},\n"
    bbp_injected += f"${func_name}_jam:\n.incbin \"{func_name}_jam.bin\"\n"

    cur_func_file = open(output_folder / f"bbp/{func_name}_hook.asm", "w")
    cur_func_file.write(f".headersize 0x80246000 - 0x1000\n.org 0x{hook_addrs[idx]:x}\nj ${func_name}_jam\nnop\n")
    cur_func_file.close()

patch_file = open(output_folder / "rm/jam.s", "w")
patch_file.write(incbins + jump_hooks + """
.orga 0xFD428
lui $t0, 0x4080
mtc1 $t0, $f20
""")
patch_file.close()

bbp_json = """{
    "schema_version": "1.1.0",
    "name": "Forced Mechanic Jam",
    "uuid": "a090e5fb-2e80-427a-8559-68c3c046ca10",
    "version": "1.0.0",
    "author": "Wiseguy & CowQuack",
    "description": "Implementation of the forced mechanic for Mario Jams #4",
    "entry_injected": "injected.asm",
    "entry_global": [
"""

bbp_json_end = """
    ]
}
"""

bbp_json_file = open(output_folder / "bbp/module.json", "w")
bbp_json_file.write(bbp_json + bbp_global_asms[:-2] + bbp_json_end)
bbp_json_file.close()

bbp_injected_file = open(output_folder / "bbp/injected.asm", "w")
bbp_injected_file.write(bbp_injected)
bbp_injected_file.close()


