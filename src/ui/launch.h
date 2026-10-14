// One window per user (Heron's launch.rs and its single-instance plugin):
// a second launch, such as Explorer's "Open in heron-c" on another folder,
// hands its folder to the running window, which opens it as a new pane and
// comes to the front, and then exits. The first launch keeps its window
// handle in a small named shared memory; finding it there, not by window
// class, means a heron-c of another build is never disturbed. Release
// builds only, so a dev run never attaches to an installed heron-c.
#pragma once

#include <stdbool.h>

#include "ui/app.h"

// Before the window: true when a running heron-c took over (exit then).
bool launch_hand_over(const wchar_t *dir);
// After the window exists: later launches find it.
void launch_set_window(HWND hwnd);
// WM_COPYDATA: true when it came from a second launch.
bool launch_received(App *a, const COPYDATASTRUCT *data);
