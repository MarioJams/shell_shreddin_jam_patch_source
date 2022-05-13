#include "precomp.h"

s32 perform_ground_step_jam(struct MarioState *m) {
    s32 i;
    u32 stepResult;
    Vec3f intendedPos;

    for (i = 0; i < 4; i++) {
        float four = construct_float(4.0f);
        intendedPos[0] = m->pos[0] + m->floor->normal.y * (m->vel[0] / four);
        intendedPos[2] = m->pos[2] + m->floor->normal.y * (m->vel[2] / four);
        intendedPos[1] = m->pos[1];

        stepResult = perform_ground_quarter_step(m, intendedPos);
        if (
            stepResult == GROUND_STEP_LEFT_GROUND
            || stepResult == GROUND_STEP_HIT_WALL_STOP_QSTEPS
            || stepResult == GROUND_STEP_ENTERED_WATER
        ) {
            break;
        }
    }

    m->terrainSoundAddend = mario_get_terrain_sound_addend(m);
    vec3f_copy(m->marioObj->header.gfx.pos, m->pos);
    vec3s_set(m->marioObj->header.gfx.angle, 0, m->faceAngle[1], 0);

    if (stepResult == GROUND_STEP_HIT_WALL_CONTINUE_QSTEPS) {
        stepResult = GROUND_STEP_HIT_WALL;
    }
    return stepResult;
}
