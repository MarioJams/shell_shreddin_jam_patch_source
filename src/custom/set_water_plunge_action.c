#include "precomp.h"

s32 set_water_plunge_action_jam(struct MarioState *m) {
    m->forwardVel = m->forwardVel / construct_float(4.0f);
    m->vel[1] = m->vel[1] / construct_float(2.0f);

    m->pos[1] = m->waterLevel - 100;

    m->faceAngle[2] = 0;

    vec3s_set(m->angleVel, 0, 0, 0);

    if (!(m->action & ACT_FLAG_DIVING)) {
        m->faceAngle[0] = 0;
    }

    if (m->area->camera->mode != CAMERA_MODE_WATER_SURFACE) {
        set_camera_mode(m->area->camera, CAMERA_MODE_WATER_SURFACE, 1);
    }

    if (m->actionState == 4 || (m->actionState == 6 && m->heldObj)) {
        //do the water plunge stuff without entering the water plunge action
        play_sound(SOUND_ACTION_UNKNOWN430, m->marioObj->header.gfx.cameraToObject);
        if (m->peakHeight - m->pos[1] > construct_float(1150.0f)) {
            play_sound(SOUND_MARIO_HAHA_2, m->marioObj->header.gfx.cameraToObject);
        }
    m->pos[1] -= construct_float(200.0f);
    m->vel[1] = construct_float(-30.0f);
        m->particleFlags |= PARTICLE_WATER_SPLASH;
#if ENABLE_RUMBLE
        if (m->prevAction & ACT_FLAG_AIR) {
            queue_rumble_data(5, 80);
        }
#endif
//cut straight to water shell swimming to avoid being slowed down and action transition hell
            return set_mario_action(m, ACT_WATER_SHELL_SWIMMING, (u32)(s32)m->forwardVel);
        }
    return set_mario_action(m, ACT_WATER_PLUNGE, 0);
        
}
