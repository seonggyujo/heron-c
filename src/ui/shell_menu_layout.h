// Where the shell dropdown and its rows are. Only ui/shell_menu*.c include
// this.
#pragma once

#include "ui/app.h"

typedef struct {
  Box panel;
  int count;
  ShellKind kinds[SHELL_COUNT];
  Box rows[SHELL_COUNT];
} MenuLayout;

// Right-aligned under the arrow button, as wide as the longest name needs
// (at least 170 px), one 30 px row per installed shell.
void shell_menu_layout(App *a, MenuLayout *m);
