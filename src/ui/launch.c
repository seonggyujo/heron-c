// Handing a launch over to the running window.
#include "ui/launch.h"

#include <wchar.h>

#include "base/log.h"

#define SHARED_NAME L"Local\\heron-c-window"
#define COPY_TAG 0x48524E43  // "HRNC": a folder from a second launch
#define WAIT_MS 2000         // for a first launch that has no window yet

static HWND *g_window;  // in the shared memory

#ifdef HERON_DEV
bool launch_hand_over(const wchar_t *dir) {
  (void)dir;
  return false;
}
#else
static HANDLE g_shared;  // open while heron-c runs

static HWND running_window(void) {
  for (int waited = 0; waited < WAIT_MS; waited += 50) {
    HWND hwnd = *g_window;
    if (hwnd && IsWindow(hwnd)) return hwnd;
    Sleep(50);
  }
  return NULL;
}

bool launch_hand_over(const wchar_t *dir) {
  g_shared = CreateFileMappingW(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(HWND), SHARED_NAME);
  bool second = g_shared && GetLastError() == ERROR_ALREADY_EXISTS;
  if (g_shared) g_window = MapViewOfFile(g_shared, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(HWND));
  if (!second || !g_window) return false;
  HWND hwnd = running_window();
  if (!hwnd) return true;  // another one is starting: it keeps the window
  DWORD pid = 0;
  GetWindowThreadProcessId(hwnd, &pid);
  AllowSetForegroundWindow(pid);  // this launch is in front, so it may let that one come forward
  const wchar_t *folder = dir ? dir : L"";
  COPYDATASTRUCT data = {COPY_TAG, (DWORD)((wcslen(folder) + 1) * sizeof(wchar_t)), (void *)folder};
  SendMessageW(hwnd, WM_COPYDATA, 0, (LPARAM)&data);
  return true;
}
#endif

void launch_set_window(HWND hwnd) {
  if (g_window) *g_window = hwnd;
}

bool launch_received(App *a, const COPYDATASTRUCT *data) {
  if (data->dwData != COPY_TAG || data->cbData < sizeof(wchar_t)) return false;
  if (IsIconic(a->hwnd)) ShowWindow(a->hwnd, SW_RESTORE);
  SetForegroundWindow(a->hwnd);
  const wchar_t *dir = data->lpData;
  size_t chars = data->cbData / sizeof(wchar_t);
  if (dir[chars - 1] != 0 || !dir[0]) return true;  // no folder: only to the front
  log_info("launch", "open folder from second launch");
  app_open_pane_in(a, a->shell, dir, NULL);
  return true;
}
