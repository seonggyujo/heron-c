// The Windows folder picker (IFileOpenDialog picking folders), modal over
// the window. Needs COM, which main.c starts with OleInitialize.
#pragma once

#include <windows.h>

// The folder chosen, UTF-8, to free; NULL when cancelled.
char *folder_picker_run(HWND owner);
