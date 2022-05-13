#include "precomp.h"

s32 check_water_jump_jam(struct MarioState *m) {
    s32 probe = (s32)(m->pos[1] + construct_float(1.5f));
    
    if (m->input & INPUT_A_PRESSED || m->action == ACT_WATER_SHELL_SWIMMING) {
        if (probe >= m->waterLevel - 80 && m->faceAngle[0] >= 0 && m->controller->stickY < construct_float(-60.0f)) {
            vec3s_set(m->angleVel, 0, 0, 0);

            m->vel[1] = construct_float(62.0f);

            if (m->heldObj == NULL) {
                return set_mario_action(m, ACT_WATER_JUMP, 0);
            } else {
                //mario is doing a water dash and going WAY too fast. cut his speed a bit
                if (m->actionState == 5) {
                    m->forwardVel *= construct_float(0.6f);
                    const float forty = construct_float(40.0f);
                    if (m->forwardVel < forty) {
                        m->forwardVel = forty;
                    }
                }
                const float fifty = construct_float(50.0f);
                if (m->forwardVel > fifty) {
                    m->forwardVel = fifty;
                }
                m->actionState = 4;
                play_sound(SOUND_ACTION_UNKNOWN430, m->marioObj->header.gfx.cameraToObject);
                m->particleFlags |= PARTICLE_WATER_SPLASH;
                return set_mario_action(m, ACT_HOLD_WATER_JUMP, 4);
            }
        }
    }

    return FALSE;
}
