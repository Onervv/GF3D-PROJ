#ifndef __PHYSICS_H__
#define __PHYSICS_H__

#include "entity.h"
#include <math.h>

// Tunable constants
#define PHYS_GRAVITY            -0.1f
#define PHYS_TERMINAL_VELOCITY  -10.0f
#define PHYS_GROUND_FRICTION     0.96f
#define PHYS_AIR_FRICTION        0.985f
#define PHYS_MAX_AIR_SPEED       4.0f
#define PHYS_SLOPE_SLIDE_FACTOR  0.05f

void physics_apply(Entity *self, Uint8 onGround, Uint8 ignoreGravity);
void physics_apply_slope(Entity *self, GFC_Vector3D groundNormal, Uint8 onGround);
void physics_add_force(Entity *self, GFC_Vector3D force);

#endif
