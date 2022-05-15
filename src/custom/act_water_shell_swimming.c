#include "precomp.h"

s32 act_water_shell_swimming_jam(struct MarioState *m) {

    //transfer forward velocity when entering water
    if (((s32)m->actionArg > 0)) {
        m->forwardVel = ((s32)m->actionArg);
        m->actionArg = 0;
    }

    if (!m->heldObj) {
        m->usedObj = spawn_object(m->marioObj, MODEL_KOOPA_SHELL, bhvKoopaShellUnderwater);
        m->usedObj->oFlags |= OBJ_FLAG_HOLDABLE;
        mario_grab_used_object(m);
        m->marioBodyState->grabPos = GRAB_POS_LIGHT_OBJ;
    }

    m->actionTimer += 1;


    if (m->marioObj->oInteractStatus & INT_STATUS_MARIO_DROP_OBJECT) {
        return drop_and_set_mario_action(m, ACT_WATER_IDLE, 0);
    }

    //underwater shell dash
    if (m->input & INPUT_B_PRESSED && m->actionState != 5) {
        //return set_mario_action(m, ACT_WATER_THROW, 0);
        m->particleFlags |= PARTICLE_VERTICAL_STAR;
        play_sound(SOUND_OBJ_CANNON4, m->marioObj->header.gfx.cameraToObject);
        m->actionState = 5;
        m->forwardVel *= 2;
    }

    //water shell timer is really stupid tbh
/*
    if (m->actionTimer++ == 240) {
        m->heldObj->oInteractStatus = INT_STATUS_STOP_RIDING;
        m->heldObj = NULL;
        stop_shell_music();
        set_mario_action(m, ACT_FLUTTER_KICK, 0);
    }
    */

   const float forty = construct_float(40.0f);
   const float one = construct_float(1.0f);
   const float two = construct_float(2.0f);
   const float zero_point_two = construct_float(0.2f);

   if (m->forwardVel <= forty) {
        if (m->actionState == 5) {
            m->actionState = 0;
        }
        //braking
        if (m->input & INPUT_Z_DOWN) {
            m->forwardVel = approach_f32(m->forwardVel, 0.0f, two, zero_point_two);
        }
        //normal speed
        else {
            m->forwardVel = approach_f32(m->forwardVel, forty, two, one);
        }
    }
    //if dashing, approach the normal speed faster
    else {
        m->forwardVel = approach_f32(m->forwardVel, forty, two, one);
        //m->marioObj->header.gfx.angle[2] += 0x2000;
    }

    play_swimming_noise(m);
    set_mario_animation(m, MARIO_ANIM_FLUTTERKICK_WITH_OBJ);
    //check to see if mario wants to do the spin jump out of the water
    if (m->actionTimer > 5) {
        check_water_jump(m);  
    }
    common_swimming_step(m, 0x012C);

    return FALSE;
}
