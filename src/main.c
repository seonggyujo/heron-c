// heron-c: a multi-pane terminal written in C. Run with --agent-hook (by
// Claude Code) or --remove-agent-hooks it does only that, without a window.
// Otherwise it sets up logging and the process, opens the window and runs
// the message loop. MinGW calls WinMain for both the console (Debug) and GUI
// (Release) builds.
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
// After windows.h, which defines what shellapi.h uses.
#include <shellapi.h>
#include <ole2.h>

#include "base/log.h"
#include "hooks/hook_entry.h"
#include "hooks/hooks.h"
#include "pty/pty.h"
#include "pty/shell.h"
#include "ui/launch.h"
#include "ui/window.h"

// A folder on the command line ("heron-c.exe C:\code") is where panes
// start. Explorer passes "%V"; for a drive root that arrives as C:" (the
// backslash escapes the closing quote, and anything after it joins), so
// trailing quotes and spaces go (no folder name ends in one) and the root
// comes back.
static wchar_t *start_folder(void) {
  int argc = 0;
  wchar_t **argv = CommandLineToArgvW(GetCommandLineW(), &argc);
  wchar_t *dir = NULL;
  if (argv && argc > 1) {
    size_t len = wcslen(argv[1]);
    dir = calloc(len + 2, sizeof *dir);
    wcscpy(dir, argv[1]);
    while (len && (dir[len - 1] == L'"' || dir[len - 1] == L' ')) dir[--len] = 0;
    if (len && dir[len - 1] == L':') dir[len] = L'\\';
    DWORD attrs = GetFileAttributesW(dir);
    if (attrs == INVALID_FILE_ATTRIBUTES || !(attrs & FILE_ATTRIBUTE_DIRECTORY)) {
      free(dir);
      dir = NULL;
    }
  }
  LocalFree(argv);
  return dir;
}

// Whether the first argument is flag.
static bool first_arg_is(const char *flag) {
  int argc = 0;
  wchar_t **argv = CommandLineToArgvW(GetCommandLineW(), &argc);
  char first[64] = "";
  if (argv && argc > 1) WideCharToMultiByte(CP_UTF8, 0, argv[1], -1, first, sizeof first, NULL, NULL);
  LocalFree(argv);
  return strcmp(first, flag) == 0;
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE previous, LPSTR command_line, int show) {
  if (first_arg_is(HOOK_FLAG)) {
    log_set_level(LOG_ERROR);
    return hook_entry_run();
  }
  log_init();
  if (first_arg_is(HOOK_REMOVE_FLAG)) return hooks_remove().error == HOOKS_OK ? 0 : 1;
  wchar_t *dir = start_folder();
  if (launch_hand_over(dir)) {
    free(dir);
    return 0;
  }
  OleInitialize(NULL);  // file drops (ui/file_drop.c), the folder picker
  log_info("app", "heron-c starting");
  pty_init_process();
  shell_detect();
  HWND hwnd = window_create(instance, show, dir);
  free(dir);
  if (!hwnd) return 1;
  MSG msg;
  while (GetMessageW(&msg, NULL, 0, 0) > 0) {
    TranslateMessage(&msg);
    DispatchMessageW(&msg);
  }
  OleUninitialize();
  return (int)msg.wParam;
}
