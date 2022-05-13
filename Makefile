TARGET := sm64

# Location of the code in ROM manager in ROM (0x1200000 + X) and RAM (0x80400000 + X)
RM_ASM_OFFSET := 0x8000

# Directories
SRC_DIR := src
BUILD_DIR := build
PATCH_DIR := $(BUILD_DIR)/patch

SRC_DIRS := $(shell find $(SRC_DIR)/ -type d)
BUILD_SRC_DIRS := $(addprefix $(BUILD_DIR)/,$(SRC_DIRS))

# Tools
CROSS := mips-n64-

CC      := $(CROSS)gcc
AS      := $(CROSS)gcc
LD      := $(CROSS)ld
CPP     := $(CROSS)cpp
OBJCOPY := $(CROSS)objcopy
NM      := $(CROSS)nm
MKDIR   := mkdir -p
RMDIR   := rm -rf
CKSUM   := $(PYTHON) tools/n64cksum.py

# Inputs/outputs
ELF := $(BUILD_DIR)/$(TARGET).elf
Z64 := $(ELF:.elf=.z64)
ELF_IN := elf/sm64.us.elf
Z64_IN := rm/sm64.z64
Z64_IN_OBJ := $(BUILD_DIR)/sm64.z64.o
PATCH_BIN := $(PATCH_DIR)/jam.bin

C_SRCS := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
C_OBJS := $(addprefix $(BUILD_DIR)/, $(C_SRCS:.c=.o))
A_SRCS := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.s))
A_OBJS := $(addprefix $(BUILD_DIR)/, $(A_SRCS:.s=.o))

OBJS := $(C_OBJS) $(A_OBJS) $(Z64_IN_OBJ)

# Flags
CFLAGS      := -c -march=vr4300 -mabi=32 -ffreestanding -mfix4300 -G 0 -fno-zero-initialized-in-bss -Wall -Wextra -Wpedantic -Wno-strict-aliasing -mno-abicalls
CPPFLAGS    := -Iinclude -I../r15 -I../r15/include -I../r15/src/ -I../r15/src/game -DVERSION_US=1 -D_LANGUAGE_C -DNON_MATCHING=1 -DAVOID_UB
OPTFLAGS    := -Os -ffast-math
ASFLAGS     := -c -x assembler-with-cpp -march=vr4300 -mabi=32 -ffreestanding -mfix4300 -G 0 -O -Iinclude -mno-abicalls
LD_SCRIPT   := sm64.ld
LDFLAGS     := -T $(BUILD_DIR)/$(LD_SCRIPT) -mips3 --accept-unknown-input-arch --no-check-sections
CPP_LDFLAGS := -P -Wno-trigraphs -DBUILD_DIR=$(BUILD_DIR) -Umips -DBASEROM=$(Z64_IN_OBJ) -DRM_ASM_OFFSET=$(RM_ASM_OFFSET)
BINOFLAGS   := -I binary -O elf32-big
Z64OFLAGS   := -O binary

# Rules
all: $(PATCH_DIR)

$(BUILD_DIR) $(BUILD_SRC_DIRS) :
	$(MKDIR) $@

$(BUILD_DIR)/%.o : %.c | $(BUILD_SRC_DIRS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(OPTFLAGS) $< -o $@

$(BUILD_DIR)/%.o : %.s | $(BUILD_SRC_DIRS)
	$(AS) $(ASFLAGS) $< -o $@

$(BUILD_DIR)/$(LD_SCRIPT) :  $(LD_SCRIPT)
	$(CPP) $(CPP_LDFLAGS) $< -o $@

$(ELF) : $(OBJS) $(BUILD_DIR)/$(LD_SCRIPT) $(ELF_IN)
	$(LD) -R $(ELF_IN) $(LDFLAGS) -Map $(@:.elf=.map) -o $@

$(Z64_IN_OBJ) : $(Z64_IN) | $(BUILD_DIR)
	$(OBJCOPY) $(BINOFLAGS) $< $@
	
$(Z64) : $(ELF)
	$(OBJCOPY) $(Z64OFLAGS) $< $@
	$(CKSUM) $@

clean:
	$(RMDIR) $(BUILD_DIR)

$(PATCH_DIR): $(Z64)
	$(PYTHON) tools/extract.py $(ELF_IN) $(ELF) $(Z64) $(PATCH_DIR)


.PHONY: all clean patch

print-% : ; $(info $* is a $(flavor $*) variable set to [$($*)]) @true

