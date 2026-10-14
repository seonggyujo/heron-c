// The window procedure: a switch that hands each message to its handler.
#include "ui/window.h"

#include <windowsx.h>

#include "base/log.h"
#include "ui/agents.h"
#include "ui/app_events.h"
#include "ui/background.h"
#include "ui/changes.h"
#include "ui/file_drop.h"
#include "ui/frame.h"
#include "ui/ime.h"
#include "ui/keyboard.h"
#include "ui/launch.h"
#include "ui/mouse.h"
#include "ui/paint.h"
#include "ui/popover.h"
#include "ui/saved_work.h"
#include "ui/settings_panel.h"
#include "ui/sidebar.h"
#include "ui/tooltip.h"

static const wchar_t CLASS_NAME[] = L"heron-c";

static LRESULT mouse_message(App *a, UINT msg, WPARAM wp, LPARAM lp) {
  int x = GET_X_LPARAM(lp), y = GET_Y_LPARAM(lp);
  switch (msg) {
    case WM_MOUSEMOVE: mouse_move(a, x, y); break;
    case WM_LBUTTONDOWN: mouse_down(a, MOUSE_LEFT, x, y, false); break;
    case WM_LBUTTONDBLCLK: mouse_down(a, MOUSE_LEFT, x, y, true); break;
    case WM_RBUTTONDOWN:
    case WM_RBUTTONDBLCLK: mouse_down(a, MOUSE_RIGHT, x, y, false); break;
    case WM_MBUTTONDOWN:
    case WM_MBUTTONDBLCLK: mouse_down(a, MOUSE_MIDDLE, x, y, false); break;
    case WM_LBUTTONUP: mouse_up(a, MOUSE_LEFT, x, y, wp); break;
    case WM_RBUTTONUP: mouse_up(a, MOUSE_RIGHT, x, y, wp); break;
    case WM_MBUTTONUP: mouse_up(a, MOUSE_MIDDLE, x, y, wp); break;
  }
  return 0;
}

static LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
  App *a = (App *)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
  if (!a) return DefWindowProcW(hwnd, msg, wp, lp);
  LRESULT result;
  if (ime_message(a, msg, wp, lp, &result)) return result;
  if (msg >= WM_MOUSEFIRST && msg <= WM_MBUTTONDBLCLK && msg != WM_MOUSEWHEEL) return mouse_message(a, msg, wp, lp);
  switch (msg) {
    case WM_NCCALCSIZE: return frame_calc_size(hwnd, wp, lp);
    case WM_NCHITTEST: return frame_hit_test(a, lp);
    case WM_GETMINMAXINFO: frame_min_size(a, (MINMAXINFO *)lp); return 0;
    case WM_SIZE: app_on_size(a, wp, LOWORD(lp), HIWORD(lp)); return 0;
    case WM_DPICHANGED: app_on_dpi(a, HIWORD(wp), (const RECT *)lp); return 0;
    case WM_PAINT: app_paint(a); return 0;
    case WM_ERASEBKGND: return 1;
    case WM_ACTIVATE: app_on_activate(a, LOWORD(wp) != WA_INACTIVE); break;
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
      if (keyboard_key_down(a, wp)) return 0;
      break;
    case WM_SYSKEYUP:  // a lone Alt or F10 would otherwise open the window menu
      if (wp == VK_MENU || wp == VK_F10) return 0;
      break;
    case WM_CHAR: keyboard_char(a, wp, false); return 0;
    case WM_SYSCHAR:
      if (wp == ' ') break;  // Alt+Space: the system menu
      keyboard_char(a, wp, true);
      return 0;
    case WM_MOUSEWHEEL: mouse_wheel(a, GET_WHEEL_DELTA_WPARAM(wp), GET_X_LPARAM(lp), GET_Y_LPARAM(lp)); return 0;
    case WM_MOUSELEAVE: mouse_leave(a); return 0;
    case WM_NOTIFY:
      if (tooltip_notify(a, lp)) return 0;
      break;
    case WM_CAPTURECHANGED:
      if ((HWND)lp != hwnd) mouse_capture_lost(a);
      return 0;
    case WM_NCLBUTTONDOWN:  // dragging or resizing the window closes the dropdown
    case WM_NCRBUTTONDOWN:
      popover_close(a);
      break;
    case WM_SETCURSOR:
      if (LOWORD(lp) != HTCLIENT) break;
      mouse_set_cursor(a);
      return TRUE;
    case WM_TIMER:
      if (wp == TIMER_RESET_BUTTON) settings_reset_timer(a);
      if (wp == TIMER_ANIMATION) app_settle_panes(a);
      if (wp == TIMER_AGENTS) agents_poll(a);
      if (wp == TIMER_CLOCK) {
        changes_tick(a);
        sidebar_tick(a);
        saved_work_tick(a);
      }
      if (wp == TIMER_ANIMATION || (wp == TIMER_CLOCK && !IsIconic(hwnd) && a->panes.shown.count)) {
        app_redraw(a);  // the next frame, or the uptimes
      }
      return 0;
    case WM_PANE_OUTPUT: app_on_output(a, (int)wp); return 0;
    case WM_PANE_EXIT: app_on_exit(a, (int)wp, (DWORD)lp); return 0;
    case WM_JOB_DONE: background_finished(a, lp); return 0;
    case WM_COPYDATA: return launch_received(a, (const COPYDATASTRUCT *)lp);
    case WM_CLOSE:
      panes_kill_all(&a->panes);
      DestroyWindow(hwnd);
      return 0;
    case WM_DESTROY:
      KillTimer(hwnd, TIMER_CLOCK);
      file_drop_revoke(a);
      SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
      app_free(a);
      PostQuitMessage(0);
      return 0;
  }
  return DefWindowProcW(hwnd, msg, wp, lp);
}

static bool register_class(HINSTANCE instance) {
  WNDCLASSEXW wc = {0};
  wc.cbSize = sizeof wc;
  wc.style = CS_DBLCLKS;
  wc.lpfnWndProc = window_proc;
  wc.hInstance = instance;
  wc.hIcon = LoadIconW(instance, MAKEINTRESOURCEW(1));
  wc.hIconSm = wc.hIcon;
  wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
  wc.lpszClassName = CLASS_NAME;
  return RegisterClassExW(&wc) != 0;
}

HWND window_create(HINSTANCE instance, int show, const wchar_t *start_dir) {
  if (!register_class(instance)) return NULL;
  float k = GetDpiForSystem() / 96.0f;
  HWND hwnd = CreateWindowExW(0, CLASS_NAME, L"heron-c", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                              (int)(1200 * k), (int)(760 * k), NULL, NULL, instance, NULL);
  if (!hwnd) return NULL;
  App *a = app_new(hwnd, start_dir);
  if (!app_set_dpi(a, GetDpiForWindow(hwnd))) {
    MessageBoxW(hwnd, L"Cascadia Mono or Consolas must be installed.", L"heron-c", MB_ICONERROR);
    app_free(a);
    DestroyWindow(hwnd);
    return NULL;
  }
  SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)a);
  frame_init(hwnd);
  file_drop_register(a);
  tooltip_create(a);
  ShowWindow(hwnd, show);
  launch_set_window(hwnd);
  saved_work_open(a);
  agents_start(a);
  SetTimer(hwnd, TIMER_CLOCK, 1000, NULL);
  log_info("app", "window ready");
  return hwnd;
}
