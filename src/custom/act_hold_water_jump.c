#include "precomp.h"

s32 act_hold_water_jump_jam(struct MarioState *m) {
    // transfer actionArg to actionState if holding a shell
    if (m->actionArg == 4) {
        m->vel[1] = m->forwardVel * construct_float(1.9f);
        m->forwardVel *= construct_float(1.6f);
        m->actionArg = 0;
        m->actionState = 4;

        // the water cam is awful for this, just use the normal one instead
        set_camera_mode(m->area->camera, m->area->camera->defMode, 1);
    }
    m->actionTimer += 1;

    const float fifteen = construct_float(15.0f);

    if (m->forwardVel < fifteen) {
        mario_set_forward_vel(m, fifteen);
    }

    //if Z is pressed midair, mario will do a flip and then go back to riding his shell
    if (m->input & INPUT_Z_PRESSED && m->actionState != 6) {
        m->actionState = 6;
        m->actionTimer = 0;
    }

    // apparently air step doesnt use fvel???
    m->vel[0] = m->forwardVel * coss(m->faceAngle[0]) * sins(m->faceAngle[1]);
    m->vel[2] = m->forwardVel * coss(m->faceAngle[0]) * coss(m->faceAngle[1]);

    play_mario_sound(m, SOUND_ACTION_UNKNOWN432, 0);

    // if holding the shell, do a midair spin instead of a normal jump
    if (m->actionState == 4) {
        set_mario_animation(m, MARIO_ANIM_SLIDE_DIVE);
        m->marioObj->header.gfx.angle[2] += 0x2000;
        m->marioObj->header.gfx.angle[0] = construct_float(-0x80) * m->vel[1];
    } else {
        set_mario_animation(m, MARIO_ANIM_JUMP_WITH_LIGHT_OBJ);
    }

    //frontflip a few times and then go into the air shell action
    if (m->actionState == 6) {
        if (m->actionTimer < 10) {
            m->marioObj->header.gfx.angle[0] += 0x3333;
            m->marioObj->header.gfx.angle[2] = 0;
        }
        else {
            m->heldObj = NULL;
                set_camera_mode(m->area->camera, m->area->camera->defMode, 1);
            return set_mario_action(m, ACT_RIDING_SHELL_JUMP, m->actionTimer);
        }
    }

    switch (perform_air_step(m, 0)) {
        case AIR_STEP_LANDED:
            // only under extreme circumstances would you be able to hold something other than a shell
            // underwater. still, better safe than sorry
            if (m->actionState == 4 || m->actionState == 6) {
                set_mario_action(m, ACT_RIDING_SHELL_GROUND, m->actionState);
                m->heldObj = NULL;

                set_camera_mode(m->area->camera, m->area->camera->defMode, 1);
            } else {
                set_mario_action(m, ACT_HOLD_JUMP_LAND, m->actionState);
                set_camera_mode(m->area->camera, m->area->camera->defMode, 1);
            }
            break;

        case AIR_STEP_HIT_WALL:
            mario_set_forward_vel(m, construct_float(15.0f));
            break;

        case AIR_STEP_HIT_LAVA_WALL:
            lava_boost_on_wall(m);
            break;
    }

    return FALSE;
}
