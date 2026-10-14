// A frame without the Windows title bar that still resizes, snaps and keeps
// the system shadow.
#include "ui/frame.h"

#include <dwmapi.h>
#include <windowsx.h>

#include "app/theme.h"
#include "ui/titlebar.h"

// Windows 11 attributes the MinGW headers do not name yet.
#define DWMWA_DARK_MODE 20
#define DWMWA_CORNERS 33
#define DWMWA_BORDER 34
#define CORNERS_ROUND 2

void frame_apply_theme(HWND hwnd) {
  uint32_t c = THEME_BORDER;
  COLORREF border = RGB((c >> 16) & 0xFF, (c >> 8) & 0xFF, c & 0xFF);
  DwmSetWindowAttribute(hwnd, DWMWA_BORDER, &border, sizeof border);
}

void frame_init(HWND hwnd) {
  BOOL dark = TRUE;
  DWORD corners = CORNERS_ROUND;
  DwmSetWindowAttribute(hwnd, DWMWA_DARK_MODE, &dark, sizeof dark);
  DwmSetWindowAttribute(hwnd, DWMWA_CORNERS, &corners, sizeof corners);
  frame_apply_theme(hwnd);
  // A frame margin, however thin, is what keeps the drop shadow.
  MARGINS margins = {0, 0, 1, 0};
  DwmExtendFrameIntoClientArea(hwnd, &margins);
  SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
}

// The client area is the whole window. A maximized window hangs over the
// screen edge by its frame thickness, so that part is cut off.
LRESULT frame_calc_size(HWND hwnd, WPARAM wp, LPARAM lp) {
  if (!wp) return DefWindowProcW(hwnd, WM_NCCALCSIZE, wp, lp);
  if (IsZoomed(hwnd)) {
    UINT dpi = GetDpiForWindow(hwnd);
    int fx = GetSystemMetricsForDpi(SM_CXFRAME, dpi) + GetSystemMetricsForDpi(SM_CXPADDEDBORDER, dpi);
    int fy = GetSystemMetricsForDpi(SM_CYFRAME, dpi) + GetSystemMetricsForDpi(SM_CXPADDEDBORDER, dpi);
    RECT *r = &((NCCALCSIZE_PARAMS *)lp)->rgrc[0];
    r->left += fx;
    r->right -= fx;
    r->top += fy;
    r->bottom -= fy;
  }
  return 0;
}

LRESULT frame_hit_test(App *a, LPARAM lp) {
  POINT pt = {GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
  ScreenToClient(a->hwnd, &pt);
  RECT rc;
  GetClientRect(a->hwnd, &rc);
  if (!IsZoomed(a->hwnd)) {
    int b = a->m.resize_border;
    bool left = pt.x < b, right = pt.x >= rc.right - b;
    bool top = pt.y < b, bottom = pt.y >= rc.bottom - b;
    if (top && left) return HTTOPLEFT;
    if (top && right) return HTTOPRIGHT;
    if (bottom && left) return HTBOTTOMLEFT;
    if (bottom && right) return HTBOTTOMRIGHT;
    if (left) return HTLEFT;
    if (right) return HTRIGHT;
    if (top) return HTTOP;
    if (bottom) return HTBOTTOM;
  }
  if (pt.y < a->m.titlebar_h && titlebar_hit(a, pt.x, pt.y) == HIT_NONE) return HTCAPTION;
  return HTCLIENT;
}

void frame_min_size(App *a, MINMAXINFO *info) {
  info->ptMinTrackSize.x = (LONG)(480 * a->scale);
  info->ptMinTrackSize.y = (LONG)(320 * a->scale);
}
