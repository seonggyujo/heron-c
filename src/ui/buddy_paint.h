// Drawing a crew's pose (buddy/crew.h) in the theme's colors: every sprite
// pixel is a square of the given size with its edges on whole device
// pixels, so neighbors meet without seams, and a row's run of one color is
// one rectangle.
#pragma once

#include "buddy/crew.h"
#include "render/canvas.h"

// (x, y) is the terminal's top-left corner; px the size of a sprite pixel.
void buddy_paint_pose(Canvas *c, const Pose *pose, float x, float y, float px);
