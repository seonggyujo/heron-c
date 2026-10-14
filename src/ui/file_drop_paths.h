// Turning dropped files into the text typed into a pane. Only
// ui/file_drop*.c include this.
#pragma once

#include <windows.h>
// After windows.h, which defines what shellapi.h uses.
#include <shellapi.h>

#include "ui/app.h"

// Types the paths in drop into pane: quoted when they hold a space or one
// of &()[]{}^=;!'+,`~, separated by spaces, with a space after.
void drop_paths_paste(App *a, Pane *pane, HDROP drop);
