// Hit testing the sidebar, shared by its mouse handling and its tooltips.
// Only ui/sidebar*.c include this.
#pragma once

#include "ui/sidebar_layout.h"

// The part under (x, y) and its box (zero for none).
SidebarHit sidebar_hit(App *a, const SidebarLayout *l, int x, int y, Box *box);
