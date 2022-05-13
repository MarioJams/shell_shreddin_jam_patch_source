#include "precomp.h"

void update_swimming_speed_jam(struct MarioState *m, f32 decelThreshold) {
    f32 buoyancy = get_buoyancy(m);
    f32 maxSpeed = construct_float(28.0f);

    //if mario is using a shell he can go much faster
    if (m->heldObj) {
        maxSpeed = construct_float(130.0f);
    }

    if (m->action & ACT_FLAG_STATIONARY) {
        m->forwardVel -= construct_float(2.0f);
    }

    if (m->forwardVel < 0.0f) {
        m->forwardVel = 0.0f;
    }

    if (m->forwardVel > maxSpeed) {
        m->forwardVel = maxSpeed;
    }

    if (m->forwardVel > decelThreshold) {
        m->forwardVel -= construct_float(0.5f);
    }

    m->vel[0] = m->forwardVel * coss(m->faceAngle[0]) * sins(m->faceAngle[1]);
    m->vel[1] = m->forwardVel * sins(m->faceAngle[0]) + buoyancy;
    m->vel[2] = m->forwardVel * coss(m->faceAngle[0]) * coss(m->faceAngle[1]);
}
