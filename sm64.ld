OUTPUT_ARCH (mips)

#define BEGIN_SEG(name, addr) \
    _##name##SegmentStart = ADDR(.name); \
    _##name##SegmentRomStart = __romPos; \
    .name addr : AT(__romPos)

#define END_SEG(name) \
    _##name##SegmentEnd = ADDR(.name) + SIZEOF(.name); \
    _##name##SegmentRomEnd = __romPos + SIZEOF(.name); \
    _##name##SegmentSize = _##name##SegmentEnd - _##name##SegmentStart; \
    __romPos += SIZEOF(.name);

#define BEGIN_NOLOAD(name) \
    _##name##SegmentNoloadStart = ADDR(.name.noload); \
    .name.noload (NOLOAD) :

#define END_NOLOAD(name) \
    _##name##SegmentNoloadEnd = ADDR(.name.noload) + SIZEOF(.name.noload);

#define PATCH(segment, symbol, offset) \
    .segment##symbol##offset##_patch symbol + offset : AT(_##segment##SegmentRomStart + symbol - _##segment##SegmentStart + offset)

#define JAL_HOOK(segment, symbol, offset, helper, name) \
    .segment##symbol##offset##_patch symbol + offset : AT(_##segment##SegmentRomStart + symbol - _##segment##SegmentStart + offset) \
    { \
        name = .; \
        BYTE(0x0C); \
        BYTE((helper >> 18) & 0xFF); \
        BYTE((helper >> 10) & 0xFF); \
        BYTE((helper >> 2)  & 0xFF); \
    }

#define J_HOOK_NOP(segment, symbol, offset, helper, name) \
    .segment##symbol##offset##_patch symbol + offset : AT(_##segment##SegmentRomStart + symbol - _##segment##SegmentStart + offset) \
    { \
        hook_##name = .; \
        BYTE(0x08); \
        BYTE((helper >> 18) & 0xFF); \
        BYTE((helper >> 10) & 0xFF); \
        BYTE((helper >> 2)  & 0xFF); \
        BYTE(0x00); \
        BYTE(0x00); \
        BYTE(0x00); \
        BYTE(0x00); \
    }

#define J_HOOK_NOP_DUMMY(segment, symbol, offset, helper, name) \
    .segment##symbol##offset##_patch symbol + offset : AT(_##segment##SegmentRomStart + symbol - _##segment##SegmentStart + offset) \
    { \
        hook_##name = .; \
    }
    
#define BIN_PATCH(address) \
    .rom##address##_patch : AT(address)

#define CODE_PATCH(rom_address, ram_address, name) \
    .name##_patch ram_address : AT(rom_address)

#define FUNC_MARKER(index) \
    func_end_##index = . - custom_code_ram_address;

SECTIONS
{
    __romPos = 0;

    BEGIN_SEG(baserom, 0)
    {
        BASEROM;
    }
    END_SEG(baserom)
    baseromEnd = __romPos;

    J_HOOK_NOP(main, act_riding_shell_ground,        0, act_riding_shell_ground_jam,        0)
    J_HOOK_NOP(main, act_riding_shell_air,           0, act_riding_shell_air_jam,           1)
    J_HOOK_NOP(main, act_water_shell_swimming,       0, act_water_shell_swimming_jam,       2)
    J_HOOK_NOP(main, act_hold_water_jump,            0, act_hold_water_jump_jam,            3)
    J_HOOK_NOP(main, check_water_jump,               0, check_water_jump_jam,               4)
    J_HOOK_NOP(main, update_swimming_speed,          0, update_swimming_speed_jam,          5)
    J_HOOK_NOP(main, update_shell_speed,             0, update_shell_speed_jam,             6)
    J_HOOK_NOP(main, set_water_plunge_action,        0, set_water_plunge_action_jam,        7)
    J_HOOK_NOP(main, perform_air_step,               0, perform_air_step_jam,               8)
    J_HOOK_NOP(main, bhv_koopa_shell_loop,           0, bhv_koopa_shell_loop_jam,           9)
    J_HOOK_NOP(main, squish_mario_model,             0, squish_mario_model_jam,             10)
    J_HOOK_NOP(main, perform_ground_quarter_step,    0, perform_ground_quarter_step_jam,    11)
    J_HOOK_NOP(main, perform_air_quarter_step,       0, perform_air_quarter_step_jam,       12)
    J_HOOK_NOP(main, perform_ground_step,            0, perform_ground_step_jam,            13)
    J_HOOK_NOP(main, check_common_submerged_cancels, 0, check_common_submerged_cancels_jam, 14)

    . = 0x0;
    BIN_PATCH(0x000000)
    {
        BUILD_DIR/src/patches/header.o(.data);
    }

    custom_code_ram_address = 0xFFFFFFFF80400000 + RM_ASM_OFFSET;
    custom_code_rom_address = 0x1200000 + RM_ASM_OFFSET;
    CODE_PATCH(custom_code_rom_address, custom_code_ram_address, func_bins)
    {
        BUILD_DIR/src/custom/act_riding_shell_ground.o(.text);
        FUNC_MARKER(0);
        BUILD_DIR/src/custom/act_riding_shell_air.o(.text);
        FUNC_MARKER(1);
        BUILD_DIR/src/custom/act_water_shell_swimming.o(.text);
        FUNC_MARKER(2);
        BUILD_DIR/src/custom/act_hold_water_jump.o(.text);
        FUNC_MARKER(3);
        BUILD_DIR/src/custom/check_water_jump.o(.text);
        FUNC_MARKER(4);
        BUILD_DIR/src/custom/update_swimming_speed.o(.text);
        FUNC_MARKER(5);
        BUILD_DIR/src/custom/update_shell_speed.o(.text);
        FUNC_MARKER(6);
        BUILD_DIR/src/custom/set_water_plunge_action.o(.text);
        FUNC_MARKER(7);
        BUILD_DIR/src/custom/perform_air_step.o(.text);
        FUNC_MARKER(8);
        BUILD_DIR/src/custom/bhv_koopa_shell_loop.o(.text);
        FUNC_MARKER(9);
        BUILD_DIR/src/custom/squish_mario_model.o(.text);
        FUNC_MARKER(10);
        BUILD_DIR/src/custom/perform_ground_quarter_step.o(.text);
        FUNC_MARKER(11);
        BUILD_DIR/src/custom/perform_air_quarter_step.o(.text);
        FUNC_MARKER(12);
        BUILD_DIR/src/custom/perform_ground_step.o(.text);
        FUNC_MARKER(13);
        BUILD_DIR/src/custom/check_common_submerged_cancels.o(.text);
        FUNC_MARKER(14);
    }
    custom_code_size = . - (0xFFFFFFFF80400000 + RM_ASM_OFFSET);


    /* Discard everything not specifically mentioned above. */
    /DISCARD/ :
    {
        *(*);
    }
}
