#include "precomp.h"

s32 act_riding_shell_ground_jam(struct MarioState *m) {
    s16 startYaw = m->faceAngle[1];

    //transfer the timer from the previous action
    if (m->actionArg > 0) {
        m->actionTimer = m->actionArg;
        m->actionArg = 0;
    }

    //this is in place for the transition from water shell to ground shell
    if (!m->riddenObj) {
        m->faceAngle[0] = 0;
        m->faceAngle[2] = 0;
        m->interactObj = spawn_object(m->marioObj, MODEL_KOOPA_SHELL, bhvKoopaShell);
            m->usedObj = m->interactObj;
            m->riddenObj = m->interactObj;

            attack_object(m->interactObj, 0x40);
    }

    m->actionTimer++;

    if (m->input & INPUT_A_PRESSED) {
        return set_mario_action(m, ACT_RIDING_SHELL_JUMP, m->actionTimer);
    }

    //shell dash
    if (m->input & INPUT_B_PRESSED && m->actionState != 1 && m->actionTimer >= 60) {
        m->particleFlags |= PARTICLE_VERTICAL_STAR;
        play_sound(SOUND_OBJ_CANNON4, m->marioObj->header.gfx.cameraToObject);
        set_camera_shake_from_hit(SHAKE_HIT_FROM_BELOW);
        //make mario go at least max shell speed
        m->forwardVel *= 2.0f;
        float sixty_four = construct_float(64.0f);
        if (m->forwardVel < sixty_four) {
            m->forwardVel = sixty_four;
        }
        //set state to shell dash, reset timer
        m->actionState = 1;
        m->actionTimer = 0;
    }

    //get off the shell
    if (m->input & INPUT_Z_DOWN && m->input & INPUT_B_PRESSED) {
        mario_stop_riding_object(m);
        float twenty_four = construct_float(24.0f);
        if (m->forwardVel < twenty_four) {
            mario_set_forward_vel(m, twenty_four);
        }
        return set_mario_action(m, ACT_CROUCH_SLIDE, 0);
    }

    update_shell_speed(m);
    set_mario_animation(m, m->actionArg == 0 ? MARIO_ANIM_START_RIDING_SHELL : MARIO_ANIM_RIDING_SHELL);

    switch (perform_ground_step(m)) {
        case GROUND_STEP_LEFT_GROUND:
            set_mario_action(m, ACT_RIDING_SHELL_FALL, 0);
            break;

        case GROUND_STEP_HIT_WALL:
            //no more shell breaking! instead, mario will just lose all speed
            /*
            mario_stop_riding_object(m);
            play_sound(m->flags & MARIO_METAL_CAP ? SOUND_ACTION_METAL_BONK : SOUND_ACTION_BONK,
                       m->marioObj->header.gfx.cameraToObject);
            m->particleFlags |= PARTICLE_VERTICAL_STAR;
            set_mario_action(m, ACT_BACKWARD_GROUND_KB, 0);
            */
            m->forwardVel = 0;
            break;

        case GROUND_STEP_ENTERED_WATER:
            return FALSE;
    }

    //if mario presses Z over a water surface, switch to the water shell
    if (m->floor == &gWaterSurfacePseudoFloor && m->input & INPUT_Z_PRESSED) {
        if (m->riddenObj != NULL) {
            m->riddenObj->oInteractStatus = INT_STATUS_STOP_RIDING;
            m->riddenObj = NULL;
        }
        m->usedObj = spawn_object(m->marioObj, MODEL_KOOPA_SHELL, bhvKoopaShellUnderwater);
        mario_grab_used_object(m);
        m->marioBodyState->grabPos = GRAB_POS_LIGHT_OBJ;
        m->pos[1] -= construct_float(50.0f);
        set_mario_action(m, ACT_WATER_SHELL_SWIMMING, (u32)(s32)m->forwardVel);
    }

    tilt_body_ground_shell(m, startYaw);
    if (m->floor->type == SURFACE_BURNING) {
        play_sound(SOUND_MOVING_RIDING_SHELL_LAVA, m->marioObj->header.gfx.cameraToObject);
    } else {
        play_sound(SOUND_MOVING_TERRAIN_RIDING_SHELL + m->terrainSoundAddend,
                   m->marioObj->header.gfx.cameraToObject);
    }

    adjust_sound_for_speed(m);
#if ENABLE_RUMBLE
    reset_rumble_timers();
#endif
    return FALSE;
}
