#include "precomp.h"

s32 perform_air_quarter_step_jam(struct MarioState *m, Vec3f intendedPos, u32 stepArg) {
    s16 wallDYaw;
    Vec3f nextPos;
    struct Surface *upperWall;
    struct Surface *lowerWall;
    struct Surface *ceil;
    struct Surface *floor;
    f32 ceilHeight;
    f32 floorHeight;
    f32 waterLevel;

    vec3f_copy(nextPos, intendedPos);

    float fifty = construct_float(50.0f);

    upperWall = resolve_and_return_wall_collisions(nextPos, construct_float(150.0f), fifty);
    lowerWall = resolve_and_return_wall_collisions(nextPos, construct_float(30.0f), fifty);

    floorHeight = find_floor(nextPos[0], nextPos[1], nextPos[2], &floor);
    ceilHeight = vec3f_find_ceil(nextPos, floorHeight, &ceil);

    waterLevel = find_water_level(nextPos[0], nextPos[2]);

    m->wall = NULL;

    //! The water pseudo floor is not referenced when your intended qstep is
    // out of bounds, so it won't detect you as landing.

    if (floor == NULL) {
        if (nextPos[1] <= m->floorHeight) {
            m->pos[1] = m->floorHeight;
            return AIR_STEP_LANDED;
        }

        m->pos[1] = nextPos[1];
        return AIR_STEP_HIT_WALL;
    }

    if (m->action & ACT_FLAG_RIDING_SHELL) {
        if (m->pos[1] < waterLevel - construct_float(100.0f)) {
            if (m->riddenObj != NULL) {
                m->riddenObj->oInteractStatus = INT_STATUS_STOP_RIDING;
                m->riddenObj = NULL;
            }

            m->usedObj = spawn_object(m->marioObj, MODEL_KOOPA_SHELL, bhvKoopaShellUnderwater);
            mario_grab_used_object(m);
            m->marioBodyState->grabPos = GRAB_POS_LIGHT_OBJ;
            set_mario_action(m, ACT_WATER_SHELL_SWIMMING, (u32)(s32)m->forwardVel);
            m->pos[0] = nextPos[0];
            m->pos[1] = MIN(m->pos[1], waterLevel - construct_float(120.0f));
            m->pos[2] = nextPos[2];
            return AIR_STEP_SHELL_ENTERED_WATER;
        }
        else if (floorHeight < waterLevel) {
            floorHeight = waterLevel;
            floor = &gWaterSurfacePseudoFloor;
            floor->originOffset = -floorHeight;
        }
    }

    float one_sixty = construct_float(160.0f);

    //! This check uses f32, but findFloor uses short (overflow jumps)
    if (nextPos[1] <= floorHeight) {
        if (ceilHeight - floorHeight > one_sixty) {
            m->pos[0] = nextPos[0];
            m->pos[2] = nextPos[2];
            m->floor = floor;
            m->floorHeight = floorHeight;
        }

        //! When ceilHeight - floorHeight <= 160, the step result says that
        // Mario landed, but his movement is cancelled and his referenced floor
        // isn't updated (pedro spots)
        m->pos[1] = floorHeight;
        return AIR_STEP_LANDED;
    }

    if (nextPos[1] + one_sixty > ceilHeight) {
        if (m->vel[1] >= 0.0f) {
            m->vel[1] = 0.0f;

            //! Uses referenced ceiling instead of ceil (ceiling hang upwarp)
            if ((stepArg & AIR_STEP_CHECK_HANG) && m->ceil != NULL
                && m->ceil->type == SURFACE_HANGABLE) {
                return AIR_STEP_GRABBED_CEILING;
            }

            return AIR_STEP_NONE;
        }

        //! Potential subframe downwarp->upwarp?
        if (nextPos[1] <= m->floorHeight) {
            m->pos[1] = m->floorHeight;
            return AIR_STEP_LANDED;
        }

        m->pos[1] = nextPos[1];
        return AIR_STEP_HIT_WALL;
    }

    //! When the wall is not completely vertical or there is a slight wall
    // misalignment, you can activate these conditions in unexpected situations
    if ((stepArg & AIR_STEP_CHECK_LEDGE_GRAB) && upperWall == NULL && lowerWall != NULL) {
        if (check_ledge_grab(m, lowerWall, intendedPos, nextPos)) {
            return AIR_STEP_GRABBED_LEDGE;
        }

        vec3f_copy(m->pos, nextPos);
        m->floor = floor;
        m->floorHeight = floorHeight;
        return AIR_STEP_NONE;
    }

    vec3f_copy(m->pos, nextPos);
    m->floor = floor;
    m->floorHeight = floorHeight;

    if (upperWall != NULL || lowerWall != NULL) {
        m->wall = upperWall != NULL ? upperWall : lowerWall;
        wallDYaw = atan2s(m->wall->normal.z, m->wall->normal.x) - m->faceAngle[1];

        if (m->wall->type == SURFACE_BURNING) {
            return AIR_STEP_HIT_LAVA_WALL;
        }

        if (wallDYaw < -0x6000 || wallDYaw > 0x6000) {
            m->flags |= MARIO_UNKNOWN_30;
            return AIR_STEP_HIT_WALL;
        }
    }

    return AIR_STEP_NONE;
}
