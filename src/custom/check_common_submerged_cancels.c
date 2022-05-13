#include "precomp.h"

s32 check_common_submerged_cancels_jam(struct MarioState *m) {
    float eighty = construct_float(80.0f);
    s16 waterHeight = m->waterLevel - eighty;
    if (m->pos[1] > waterHeight) {
        float fifty = construct_float(50.0f);
        if (waterHeight > m->floorHeight) {
            if (m->pos[1] - waterHeight < fifty) {
                m->pos[1] = waterHeight; // lock mario to top if the falloff isn't big enough
            } 
            else if (m->action == ACT_WATER_SHELL_SWIMMING && m->heldObj != NULL) {
                if (m->forwardVel > fifty) {
                    m->forwardVel = fifty;
                }
                m->actionState = 4;
                play_sound(SOUND_ACTION_UNKNOWN430, m->marioObj->header.gfx.cameraToObject);
                m->particleFlags |= PARTICLE_WATER_SPLASH;
                return set_mario_action(m, ACT_HOLD_WATER_JUMP, 40);
            }
            else {
                m->pos[1] = m->waterLevel - eighty; // Vanilla bug: Downwarp swimming out of waterfalls
            }
        } else {
            if (m->action == ACT_WATER_SHELL_SWIMMING && m->heldObj != NULL) {
                if (m->pos[1] - m->floorHeight > construct_float(100.0f)) {
                //exit the water in the shell spin state
                if (m->forwardVel > fifty) {
                    m->forwardVel = fifty;
                }
                m->actionState = 4;
                play_sound(SOUND_ACTION_UNKNOWN430, m->marioObj->header.gfx.cameraToObject);
                m->particleFlags |= PARTICLE_WATER_SPLASH;
                return set_mario_action(m, ACT_HOLD_WATER_JUMP, 40);
            }
            else {
                //exit the water in a generic air shell state
                m->heldObj = NULL;
                set_camera_mode(m->area->camera, m->area->camera->defMode, 1);
                return set_mario_action(m, ACT_RIDING_SHELL_JUMP, m->actionTimer);
            }
            }

            return transition_submerged_to_walking(m);
        }
    }

    if (m->health < 0x100 && !(m->action & (ACT_FLAG_INTANGIBLE | ACT_FLAG_INVULNERABLE))) {
        set_mario_action(m, ACT_DROWNING, 0);
    }

    return FALSE;
}
