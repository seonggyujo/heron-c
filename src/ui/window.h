// The main window: registering its class, creating it, and passing each
// message to the ui/ file that handles it.
#pragma once

#include <windows.h>

// start_dir (optional) is where panes start when no pane has a folder yet.
// Returns NULL when the window could not be made.
HWND window_create(HINSTANCE instance, int show, const wchar_t *start_dir);
