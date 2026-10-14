// Copying text to and from the Windows clipboard, as UTF-8.
#pragma once

#include <stdbool.h>
#include <windows.h>

bool clipboard_set_text(HWND owner, const char *utf8);
// malloc'd text, or NULL when the clipboard holds no text.
char *clipboard_get_text(HWND owner);
