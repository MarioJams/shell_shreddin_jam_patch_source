#include "precomp.h"

s32 perform_ground_quarter_step_jam(struct MarioState *m, Vec3f nextPos) {
    UNUSED struct Surface *lowerWall;
    struct Surface *upperWall;
    struct Surface *ceil;
    struct Surface *floor;
    f32 ceilHeight;
    f32 floorHeight;
    f32 waterLevel;

    lowerWall = resolve_and_return_wall_collisions(nextPos, construct_float(30.0f), construct_float(24.0f));
    upperWall = resolve_and_return_wall_collisions(nextPos, construct_float(60.0f), construct_float(50.0f));

    floorHeight = find_floor(nextPos[0], nextPos[1], nextPos[2], &floor);
    ceilHeight = vec3f_find_ceil(nextPos, floorHeight, &ceil);

    waterLevel = find_water_level(nextPos[0], nextPos[2]);

    m->wall = upperWall;

    if (floor == NULL) {
        return GROUND_STEP_HIT_WALL_STOP_QSTEPS;
    }

    float one_hundred = construct_float(100.0f);

    if (m->action & ACT_FLAG_RIDING_SHELL) {
        if (m->pos[1] < waterLevel - one_hundred) {
            if (m->riddenObj != NULL) {
                m->riddenObj->oInteractStatus = INT_STATUS_STOP_RIDING;
                m->riddenObj = NULL;
            }

            m->usedObj = spawn_object(m->marioObj, MODEL_KOOPA_SHELL, bhvKoopaShellUnderwater);
            m->usedObj->oFlags |= OBJ_FLAG_HOLDABLE;
            mario_grab_used_object(m);
            m->marioBodyState->grabPos = GRAB_POS_LIGHT_OBJ;
            set_mario_action(m, ACT_WATER_SHELL_SWIMMING, (u32)(s32)m->forwardVel);
            m->pos[0] = nextPos[0];
            m->pos[1] = MIN(m->pos[1], waterLevel - construct_float(120.0f));
            m->pos[2] = nextPos[2];
            return GROUND_STEP_ENTERED_WATER;
        }
        else if (floorHeight < waterLevel) {
            floorHeight = waterLevel;
            floor = &gWaterSurfacePseudoFloor;
            floor->originOffset = -floorHeight;
        }
    }

    float one_sixty = construct_float(160.0f);

    if (nextPos[1] > floorHeight + one_hundred) {
        if (nextPos[1] + one_sixty >= ceilHeight) {
            return GROUND_STEP_HIT_WALL_STOP_QSTEPS;
        }

        vec3f_copy(m->pos, nextPos);
        m->floor = floor;
        m->floorHeight = floorHeight;
        return GROUND_STEP_LEFT_GROUND;
    }

    if (floorHeight + one_sixty >= ceilHeight) {
        return GROUND_STEP_HIT_WALL_STOP_QSTEPS;
    }

    vec3f_set(m->pos, nextPos[0], floorHeight, nextPos[2]);
    m->floor = floor;
    m->floorHeight = floorHeight;

    if (upperWall != NULL) {
        s16 wallDYaw = atan2s(upperWall->normal.z, upperWall->normal.x) - m->faceAngle[1];

        if (wallDYaw >= 0x2AAA && wallDYaw <= 0x5555) {
            return GROUND_STEP_NONE;
        }
        if (wallDYaw <= -0x2AAA && wallDYaw >= -0x5555) {
            return GROUND_STEP_NONE;
        }

        return GROUND_STEP_HIT_WALL_CONTINUE_QSTEPS;
    }

    return GROUND_STEP_NONE;
}
