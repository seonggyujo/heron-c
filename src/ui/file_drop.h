// Dropping files on a pane types their paths into it, like Windows Terminal
// (Heron's app/file-drop.ts): quoted when needed, separated by spaces, with
// a space after. The pane under the drag gets a lit border. Implemented as
// an OLE drop target, which also sees the drag before the drop.
#pragma once

#include "ui/app.h"

// After OleInitialize and once the window exists.
void file_drop_register(App *a);
void file_drop_revoke(App *a);
