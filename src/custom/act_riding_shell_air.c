#include "precomp.h"

s32 act_riding_shell_air_jam(struct MarioState *m) {

    play_mario_sound(m, SOUND_ACTION_TERRAIN_JUMP, 0);
    set_mario_animation(m, MARIO_ANIM_JUMP_RIDING_SHELL);

    // transfer the timer from the previous action
    if (m->actionArg > 0) {
        m->actionTimer = m->actionArg;
        m->actionArg = 0;
    } else {
        // double jump
        // putting this in an else statement ensures that the same A press is not used for the double
        // jump
        if (m->input & INPUT_A_PRESSED && m->actionState != 2 && m->actionState != 3) {
            m->actionState = 2;
            m->vel[1] = construct_float(60.0f);
            m->particleFlags |= PARTICLE_MIST_CIRCLE;
            play_sound(SOUND_GENERAL_SWISH_WATER, m->marioObj->header.gfx.cameraToObject);
        }
    }

    //this is in place for the transition from water shell to air shell
    if (!m->riddenObj) {
        m->faceAngle[0] = 0;
        m->faceAngle[2] = 0;
        m->interactObj = spawn_object(m->marioObj, MODEL_KOOPA_SHELL, bhvKoopaShell);
            m->usedObj = m->interactObj;
            m->riddenObj = m->interactObj;

            attack_object(m->interactObj, 0x40);
    }

    m->actionTimer++;


    //ground pound
    if (m->input & INPUT_Z_PRESSED && m->actionState != 3) {
        m->actionState = 3;
        m->forwardVel *= construct_float(0.7f);
        // the timer is set to 40 so that the dash will be ready when you get back to the ground
        m->actionTimer = 40;

        play_sound(SOUND_ACTION_SPIN, m->marioObj->header.gfx.cameraToObject);
    }

    float negative_ninety = construct_float(-90.0f);

    if (m->actionState == 3) {
        // stall in the air for a bit
        if (m->actionTimer < 45) {
            m->marioObj->header.gfx.angle[1] += 0x3333;
        }
        if (m->actionTimer < 50) {
            m->vel[1] = construct_float(12.0f);
        }
        // shoot downward
        if (m->actionTimer == 50) {
            m->vel[1] = negative_ninety;
        }
    }

    float sixty_four = construct_float(64.0f);
    float one_twenty_eight = construct_float(128.0f);

    // manual deceleration in air during a dash
    if (m->forwardVel < one_twenty_eight && m->forwardVel > sixty_four) {
        m->forwardVel -= (f32) m->actionTimer / construct_float(15.0f);
    }
    if (m->forwardVel > one_twenty_eight) {
        m->forwardVel = one_twenty_eight;
    }

    float one = construct_float(1.0f);

    // accelerate downwards during pound
    if (m->actionState == 3) {
        m->vel[1] -= construct_float(10.0f);
        if (m->vel[1] < negative_ninety) {
            m->marioObj->header.gfx.scale[1] = one + (-one * m->vel[1] / construct_float(180.0f));
        }
    }

    update_air_without_turn(m);

    switch (perform_air_step(m, 0)) {
        case AIR_STEP_LANDED:
            if (m->actionState == 3) {
                
                // ground pounds go straight into water
                if (m->floor == &gWaterSurfacePseudoFloor) {
                    if (m->riddenObj != NULL) {
                        m->riddenObj->oInteractStatus = INT_STATUS_STOP_RIDING;
                        m->riddenObj = NULL;
                    }
                    play_sound(SOUND_ACTION_UNKNOWN430, m->marioObj->header.gfx.cameraToObject);
                    m->particleFlags |= PARTICLE_WATER_SPLASH;
                    //changes action but doesnt spawn a shell. the shell will spawn in the action itself.
                    set_mario_action(m, ACT_WATER_SHELL_SWIMMING, (u32)(s32)m->forwardVel);
                } else {
                    m->particleFlags |= PARTICLE_MIST_CIRCLE;
                    play_sound(SOUND_OBJ_POUNDING1, m->marioObj->header.gfx.cameraToObject);
                    set_mario_action(m, ACT_RIDING_SHELL_GROUND, m->actionTimer);
                }
            } else {
                set_mario_action(m, ACT_RIDING_SHELL_GROUND, m->actionTimer);
            }
            break;

        case AIR_STEP_HIT_WALL:
            mario_set_forward_vel(m, 0.0f);
            break;

        case AIR_STEP_HIT_LAVA_WALL:
            lava_boost_on_wall(m);
            break;

            case AIR_STEP_SHELL_ENTERED_WATER:
            if (m->riddenObj != NULL) {
                m->riddenObj->oInteractStatus = INT_STATUS_STOP_RIDING;
                m->riddenObj = NULL;
            }
            play_sound(SOUND_ACTION_UNKNOWN430, m->marioObj->header.gfx.cameraToObject);
            m->particleFlags |= PARTICLE_WATER_SPLASH;
            m->pos[1] -= construct_float(50.0f);
            //changes action but doesnt spawn a shell. the shell will spawn in the action itself.
            set_mario_action(m, ACT_WATER_SHELL_SWIMMING, (u32)(s32)m->forwardVel);
            break;
    }

    m->marioObj->header.gfx.pos[1] += construct_float(42.0f);
    return FALSE;
}
