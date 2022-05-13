#include "precomp.h"

extern u8 sSquishScaleOverTime[16];

void squish_mario_model_jam(struct MarioState *m) {
    if (m->squishTimer != 0xFF) {
        const float one = construct_float(1.0f);
        // If no longer squished, scale back to default.
        if (m->squishTimer == 0) {
            if (m->action != ACT_RIDING_SHELL_FALL && m->action != ACT_RIDING_SHELL_JUMP) {
                vec3f_set(m->marioObj->header.gfx.scale, one, one, one);
            }
        }
        // If timer is less than 16, rubber-band Mario's size scale up and down.
        else if (m->squishTimer <= 16) {
            m->squishTimer -= 1;

            m->marioObj->header.gfx.scale[1] =
                one - (sSquishScaleOverTime[15 - m->squishTimer] * construct_float(0.6f / 100.0f));
            m->marioObj->header.gfx.scale[0] =
                (sSquishScaleOverTime[15 - m->squishTimer] * construct_float(0.4f / 100.0f)) + one;

            m->marioObj->header.gfx.scale[2] = m->marioObj->header.gfx.scale[0];
        } else {
            m->squishTimer -= 1;
            const float one_point_four = construct_float(1.4f);
            vec3f_set(m->marioObj->header.gfx.scale, one_point_four, construct_float(0.4f), one_point_four);
        }
    }
}
