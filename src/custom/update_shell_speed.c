#include "precomp.h"

void update_shell_speed_jam(struct MarioState *m) {
    f32 maxTargetSpeed;
    f32 targetSpeed;

    if (m->floorHeight < m->waterLevel) {
        m->floorHeight = m->waterLevel;
        m->floor = &gWaterSurfacePseudoFloor;
        m->floor->originOffset = m->waterLevel; //! Negative origin offset
    }

    if (m->floor != NULL && m->floor->type == SURFACE_SLOW) {
        maxTargetSpeed = construct_float(48.0f);
    } else {
        maxTargetSpeed = construct_float(64.0f);
    }

    targetSpeed = m->intendedMag * 2.0f;
    if (targetSpeed > maxTargetSpeed) {
        targetSpeed = maxTargetSpeed;
    }
    const float twenty_four = construct_float(24.0f);
    //make sure a minimum speed isnt being set when braking
    if (targetSpeed < twenty_four && !(m->input & INPUT_Z_DOWN)) {
        targetSpeed = twenty_four;
    }

    const float one_point_one = construct_float(1.1f);

    if (m->forwardVel <= 0.0f) {
        m->forwardVel += one_point_one;
    } else if (m->forwardVel <= targetSpeed) {
        m->forwardVel += one_point_one - m->forwardVel / construct_float(58.0f);
    } else if (m->floor->normal.y >= construct_float(0.95f)) {
        m->forwardVel -= one_point_one;
    }


    const float sixty_four = construct_float(64.0f);

    if (m->actionState == 1) {
        m->forwardVel -= (f32) m->actionTimer / construct_float(10.0f);
        const float one_twenty_eight = construct_float(128.0f);
        //increase max speed for the shell dash
        if (m->forwardVel > one_twenty_eight) { 
           m->forwardVel = one_twenty_eight;
       }
       if (m->forwardVel < sixty_four) { 
           m->actionState = 0;
        }
    }
    //! No backward speed cap (shell hyperspeed)
    else {
        //use vanilla speed
        if (m->forwardVel > sixty_four) {
            m->forwardVel = sixty_four;
        }
    }

    //braking
    if (m->input & INPUT_Z_DOWN) {
        m->forwardVel *= construct_float(0.9f);
    }

    m->faceAngle[1] =
        m->intendedYaw - approach_s32((s16)(m->intendedYaw - m->faceAngle[1]), 0, 0x800, 0x800);

    apply_slope_accel(m);
}
