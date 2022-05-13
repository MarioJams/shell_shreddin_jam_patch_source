#include <PR/ultratypes.h>

#include "sm64.h"
#include "mario.h"
#include "audio/external.h"
#include "engine/math_util.h"
#include "engine/surface_collision.h"
#include "mario_step.h"
#include "area.h"
#include "interaction.h"
#include "mario_actions_object.h"
#include "memory.h"
#include "behavior_data.h"
#include "rumble_init.h"
#include "object_helpers.h"

#define GROUND_STEP_ENTERED_WATER            4
#define AIR_STEP_SHELL_ENTERED_WATER 7

u32 attack_object(struct Object *o, s32 interaction);
void update_shell_speed(struct MarioState *m);
void tilt_body_ground_shell(struct MarioState *m, s16 startYaw);
void play_swimming_noise(struct MarioState *m);
s32 check_water_jump(struct MarioState *m);
void common_swimming_step(struct MarioState *m, s16 swimStrength);
void update_air_without_turn(struct MarioState *m);
s32 lava_boost_on_wall(struct MarioState *m);
f32 get_buoyancy(struct MarioState *m);
void apply_slope_accel(struct MarioState *m);
void apply_gravity(struct MarioState *m);
void apply_vertical_wind(struct MarioState *m);
s32 perform_air_quarter_step(struct MarioState *m, Vec3f intendedPos, u32 stepArg);
u32 check_ledge_grab(struct MarioState *m, struct Surface *wall, Vec3f intendedPos, Vec3f nextPos);
s32 perform_ground_quarter_step(struct MarioState *m, Vec3f nextPos);

extern struct Object *gCurrentObject;
extern struct Object *gMarioObject;

#define ALWAYS_INLINE inline __attribute__((always_inline))

// Constructs a float in registers, which can be faster than gcc's default of loading a float from rodata.
// Especially fast for halfword floats, which get loaded with a `lui` + `mtc1`.
static ALWAYS_INLINE float construct_float(const float f)
{
    u32 r;
    float f_out;
    u32 i = *(u32*)(&f);

    if (!__builtin_constant_p(i))
    {
        return *(float*)(&i);
    }

    u32 upper = (i >> 16);
    u32 lower = (i >>  0) & 0xFFFF;

    if ((i & 0xFFFF) == 0) {
        __asm__ ("lui %0, %1"
                                : "=r"(r)
                                : "K"(upper));
    } else if ((i & 0xFFFF0000) == 0) {
        __asm__ ("ori %0, $0, %1"
                                : "+r"(r)
                                : "K"(lower));
    } else {
        __asm__ ("lui %0, %1"
                                : "=r"(r)
                                : "K"(upper));
        __asm__ ("ori %0, %0, %1"
                                : "+r"(r)
                                : "K"(lower));
    }

    __asm__ ("mtc1 %1, %0"
                         : "=f"(f_out)
                         : "r"(r));
    return f_out;
}
