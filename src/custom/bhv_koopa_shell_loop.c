#include "precomp.h"

void koopa_shell_spawn_sparkles(f32 a);
void koopa_shell_spawn_water_drop(void);
void bhv_koopa_shell_flame_spawn(void);

extern struct ObjectHitbox sKoopaShellHitbox;

#define o gCurrentObject

void bhv_koopa_shell_loop_jam(void) {
    struct Surface *sp34;

    obj_set_hitbox(o, &sKoopaShellHitbox);
    cur_obj_scale(construct_float(1.0f));

    float ten = construct_float(10.0f);

    switch (o->oAction) {
        case 0:
            cur_obj_update_floor_and_walls();
            cur_obj_if_hit_wall_bounce_away();

            if (o->oInteractStatus & INT_STATUS_INTERACTED) {
                o->oAction++;
            }

            o->oFaceAngleYaw += 0x1000;
            cur_obj_move_standard(-20);
            koopa_shell_spawn_sparkles(ten);
            break;

        case 1:
            obj_copy_pos(o, gMarioObject);
            sp34 = cur_obj_update_floor_height_and_get_floor();

            if (absf(find_water_level(o->oPosX, o->oPosZ) - o->oPosY) < ten) {
                koopa_shell_spawn_water_drop();
            } else if (absf(o->oPosY - o->oFloorHeight) < construct_float(5.0f)) {
                if (sp34 != NULL && sp34->type == 1) {
                    bhv_koopa_shell_flame_spawn();
                } else {
                    koopa_shell_spawn_sparkles(ten);
                }
            } else {
                koopa_shell_spawn_sparkles(ten);
            }
            o->oFaceAngleRoll = 0;
            o->oFaceAnglePitch = 0;
            o->oFaceAngleYaw = gMarioObject->oMoveAngleYaw;

            if (o->oInteractStatus & INT_STATUS_STOP_RIDING) {
                obj_mark_for_deletion(o);
                spawn_mist_particles();
                o->oAction = 0;
            }
            break;
    }

    o->oInteractStatus = 0;
}
