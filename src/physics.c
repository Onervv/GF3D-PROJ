#include "physics.h"
#include "simple_logger.h"

void physics_apply(Entity *self, Uint8 onGround, Uint8 ignoreGravity) {
    if (!self) return;

    // --- Apply gravity only if not ignored ---
    if (!ignoreGravity) {
        self->velocity.z += PHYS_GRAVITY;
        if (self->velocity.z < PHYS_TERMINAL_VELOCITY) {
            self->velocity.z = PHYS_TERMINAL_VELOCITY;
        }
    }

    // --- Apply friction ---
    if (onGround) {
        self->velocity.x *= PHYS_GROUND_FRICTION;
        self->velocity.y *= PHYS_GROUND_FRICTION;
    } else {
        self->velocity.x *= PHYS_AIR_FRICTION;
        self->velocity.y *= PHYS_AIR_FRICTION;

        if (self->velocity.x > PHYS_MAX_AIR_SPEED) {
            float scale = (PHYS_MAX_AIR_SPEED / self->velocity.x);
            self->velocity.x *= scale;
        }
        if (self->velocity.y > PHYS_MAX_AIR_SPEED) {
            float scale = (PHYS_MAX_AIR_SPEED / self->velocity.y);
            self->velocity.y *= scale;
        }
    }

    // slog("Physics apply: onGround=%d, vel.x=%f, vel.y=%f, vel.z=%f, ignoreGravity=%d", 
    // onGround, self->velocity.x, self->velocity.y, self->velocity.z, ignoreGravity);

    // --- Prevent tiny floating point values ---
    if (fabs(self->velocity.x) < 0.0001f) self->velocity.x = 0;
    if (fabs(self->velocity.y) < 0.0001f) self->velocity.y = 0;
}

void physics_apply_slope(Entity *self, GFC_Vector3D groundNormal, Uint8 onGround) {
    if (!self || !onGround) return;
    GFC_Vector3D slopeDir = { -groundNormal.x, -groundNormal.y, 0 };
    gfc_vector3d_normalize(&slopeDir);
    gfc_vector3d_scale(slopeDir, slopeDir, PHYS_SLOPE_SLIDE_FACTOR);

    self->velocity.x += slopeDir.x;
    self->velocity.y += slopeDir.y;
}

void physics_add_force(Entity *self, GFC_Vector3D force) {
    if (!self) return;
    self->velocity.x += force.x;
    self->velocity.y += force.y;
    self->velocity.z += force.z;
}
