#include "precomp.h"

s32 perform_air_step_jam(struct MarioState *m, u32 stepArg) {
    Vec3f intendedPos;
    s32 i;
    s32 quarterStepResult;
    s32 stepResult = AIR_STEP_NONE;

    m->wall = NULL;

    for (i = 0; i < 4; i++) {
        float four = construct_float(4.0f);
        intendedPos[0] = m->pos[0] + m->vel[0] / four;
        intendedPos[1] = m->pos[1] + m->vel[1] / four;
        intendedPos[2] = m->pos[2] + m->vel[2] / four;

        quarterStepResult = perform_air_quarter_step(m, intendedPos, stepArg);

        //! On one qf, hit OOB/ceil/wall to store the 2 return value, and continue
        // getting 0s until your last qf. Graze a wall on your last qf, and it will
        // return the stored 2 with a sharply angled reference wall. (some gwks)

        if (quarterStepResult != AIR_STEP_NONE) {
            stepResult = quarterStepResult;
        }

        if (
            quarterStepResult == AIR_STEP_LANDED
            || quarterStepResult == AIR_STEP_GRABBED_LEDGE
            || quarterStepResult == AIR_STEP_GRABBED_CEILING
            || quarterStepResult == AIR_STEP_HIT_LAVA_WALL
            || quarterStepResult == AIR_STEP_SHELL_ENTERED_WATER
        ) {
            break;
        }
    }

    if (m->vel[1] >= 0.0f) {
        m->peakHeight = m->pos[1];
    }

    m->terrainSoundAddend = mario_get_terrain_sound_addend(m);

    if (m->action != ACT_FLYING) {
        apply_gravity(m);
    }
    apply_vertical_wind(m);

    vec3f_copy(m->marioObj->header.gfx.pos, m->pos);

    //stop copying rotations if mario is doing a shell ground pound or a water shell spin.
    if (
        m->action != ACT_RIDING_SHELL_JUMP
        && m->action != ACT_RIDING_SHELL_FALL
        && m->action != ACT_HOLD_WATER_JUMP
        && m->actionState != 3
        && m->actionState != 4
    ) {
        vec3s_set(m->marioObj->header.gfx.angle, 0, m->faceAngle[1], 0);
    }

    return stepResult;
}
