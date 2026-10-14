// The tooltip control and what it says.
#include "ui/tooltip.h"

#include <commctrl.h>
#include <string.h>

#include "app/strings.h"
#include "base/log.h"
#include "ui/folder_menu.h"
#include "ui/paint.h"
#include "ui/popover.h"
#include "ui/settings_panel.h"
#include "ui/sidebar.h"
#include "ui/titlebar.h"
#include "ui/usage_bar.h"

#define TOOL_ID 1

void tooltip_create(App *a) {
  INITCOMMONCONTROLSEX init = {sizeof init, ICC_BAR_CLASSES};
  InitCommonControlsEx(&init);
  a->tooltip = CreateWindowExW(WS_EX_TOPMOST, TOOLTIPS_CLASSW, NULL, WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
                               CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, a->hwnd, NULL, NULL, NULL);
  if (!a->tooltip) {
    log_warn("tooltip", "no tooltip control: %lu", GetLastError());
    return;
  }
  TTTOOLINFOW tool = {.cbSize = sizeof tool, .uFlags = TTF_SUBCLASS, .hwnd = a->hwnd, .uId = TOOL_ID,
                      .lpszText = LPSTR_TEXTCALLBACKW};
  SendMessageW(a->tooltip, TTM_ADDTOOLW, 0, (LPARAM)&tool);
  SendMessageW(a->tooltip, TTM_SETMAXTIPWIDTH, 0, (LPARAM)(int)(480 * a->scale));
}

// What the thing under (x, y) explains, and where it is.
static const char *tip_at(App *a, int x, int y, Box *box, char *buf, size_t cap) {
  if (a->popover != POPOVER_NONE) {
    if (!popover_contains(a, x, y)) return NULL;
    if (a->popover == POPOVER_AGENT_FOLDERS) return folder_menu_tip_at(a, x, y, box, buf, cap);
    return a->popover == POPOVER_SETTINGS ? settings_tip_at(a, x, y, box) : NULL;
  }
  const char *text = sidebar_tip_at(a, x, y, box, buf, cap);
  if (!text) text = usage_bar_tip_at(a, x, y, box, buf, cap);
  if (text) return text;
  TitleLayout t;
  titlebar_layout(a, &t);
  switch (titlebar_hit(a, x, y)) {
    case HIT_TIDY: *box = t.tidy; return strings()->tidy;
    case HIT_SETTINGS: *box = t.settings; return strings()->settings;
    default: break;
  }
  Pane *p = panes_at(&a->panes, x, y);
  PaneStyle style = paint_pane_style(a, GetTickCount64());
  return p && pane_header_tip(p, &style, x, y, box, &text) ? text : NULL;
}

void tooltip_mouse_move(App *a, int x, int y) {
  if (!a->tooltip) return;
  Box box = {0};
  char buf[1024];
  const char *text = tip_at(a, x, y, &box, buf, sizeof buf);
  wchar_t wide[512] = L"";
  if (text) MultiByteToWideChar(CP_UTF8, 0, text, -1, wide, 512);
  bool same = memcmp(&box, &a->tip_box, sizeof box) == 0 && wcscmp(wide, a->tip_text) == 0;
  if (same) return;
  a->tip_box = box;
  wcscpy(a->tip_text, wide);
  if (text) log_debug("tooltip", "over: %s", text);
  SendMessageW(a->tooltip, TTM_POP, 0, 0);
  // An empty rectangle when there is nothing to explain.
  TTTOOLINFOW tool = {.cbSize = sizeof tool, .hwnd = a->hwnd, .uId = TOOL_ID,
                      .rect = {box.x, box.y, box.x + box.w, box.y + box.h}};
  SendMessageW(a->tooltip, TTM_NEWTOOLRECTW, 0, (LPARAM)&tool);
}

bool tooltip_notify(App *a, LPARAM lp) {
  NMHDR *n = (NMHDR *)lp;
  if (!a->tooltip || n->hwndFrom != a->tooltip) return false;
  if (n->code == TTN_GETDISPINFOW) ((NMTTDISPINFOW *)lp)->lpszText = a->tip_text;
  if (n->code == TTN_SHOW && log_enabled(LOG_DEBUG)) {
    char text[512];
    WideCharToMultiByte(CP_UTF8, 0, a->tip_text, -1, text, sizeof text, NULL, NULL);
    log_debug("tooltip", "%s", text);
  }
  return true;
}
