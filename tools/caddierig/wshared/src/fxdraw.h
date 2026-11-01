// SPDX-License-Identifier: MIT
// fxdraw.h - draws what fxsim simulates, in fixed-function OpenGL.
#ifndef FXDRAW_H
#define FXDRAW_H

#include "fxsim.h"

// Sprite particles, tails and bandanas. Sprays with a *pet model are left to the caller, which
// is what knows the model behind FxPetModel. right and up are the camera's, in game space.
void FxDrawParticles(FxBox& box, const WVector& right, const WVector& up);

#endif // FXDRAW_H
