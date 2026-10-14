// The window's state: drawing resources sized for the screen's DPI, the
// panes, the shell for new panes, and what the mouse, IME and dropdowns are
// doing. Every ui/ file works on this one struct.
#pragma once

#include <windows.h>

#include "layout/divider_drag.h"
#include "pane/pane_list.h"
#include "render/canvas.h"
#include "render/mono_font.h"
#include "render/ui_text.h"
#include "ui/agent_state.h"
#include "ui/buddies_state.h"
#include "ui/sidebar_state.h"

enum { WM_PANE_OUTPUT = WM_APP + 1, WM_PANE_EXIT, WM_JOB_DONE };
enum { TIMER_CLOCK = 1, TIMER_RESET_BUTTON, TIMER_ANIMATION, TIMER_AGENTS };

// Things in the title bar and the empty screen that react to the mouse.
typedef enum {
  HIT_NONE,
  HIT_NEW,
  HIT_SHELL_MENU,
  HIT_SETTINGS,
  HIT_TIDY,
  HIT_MIN,
  HIT_MAX,
  HIT_CLOSE,
  HIT_EMPTY_NEW,
} HitTarget;

typedef enum { POPOVER_NONE, POPOVER_SHELLS, POPOVER_SETTINGS, POPOVER_AGENT_FOLDERS } PopoverKind;

// Settings > Reset asks first: "Sure?", then shows "Done".
typedef enum { RESET_IDLE, RESET_CONFIRM, RESET_DONE } ResetPhase;

// A press on a pane header, which becomes a drag past a few pixels.
typedef struct {
  int pane;  // the pane pressed, 0 for none
  int start_x, start_y;
  bool moving;  // past the threshold: the pane is being dragged
  int target;   // pane it would land on, 0 for none
  Side zone;
} HeaderDrag;

// The sidebar's and the usage bar's text styles, Heron's CSS sizes: UI or
// mono face, size in px, weight.
typedef enum {
  FONT_UI_12,
  FONT_UI_12_SEMI,
  FONT_UI_13,
  FONT_UI_14,
  FONT_UI_14_SEMI,
  FONT_UI_15_SEMI,
  FONT_UI_16,
  FONT_MONO_11,
  FONT_MONO_13_BOLD,
  FONT_MONO_22_BOLD,
  FONT_COUNT,
} FontStyle;

typedef struct {
  int titlebar_h, gap, resize_border;
  PaneMetrics pane;
} UiMetrics;

typedef struct {
  HWND hwnd;
  float scale;  // DPI / 96
  UINT dpi;
  UiMetrics m;
  Canvas *canvas;
  MonoFont *font;
  int font_px;  // terminal font size before DPI scaling, 10 to 24
  UiFont *ui_font, *small_font, *header_font, *brand_font, *big_font, *huge_font;
  UiFont *fonts[FONT_COUNT];
  PaneList panes;
  AgentsState agents;
  BuddiesState buddies;  // the terminal herons (ui/buddies.c)
  SidebarState sidebar;
  bool usage_shown;  // the usage bar is up (ui/usage_bar.c)
  bool work_open;    // the first panes are open, so saving the work may start
  char *saved_work;  // the work as last saved (ui/saved_work.c)
  int chip_hover;    // pane of the usage bar chip under the mouse
  ShellKind shell;     // for new panes
  wchar_t *start_dir;  // folder from the command line, or NULL
  bool active;         // the window has the keyboard
  bool motion;         // Settings > Animations
  HitTarget hover, pressed;
  PopoverKind popover;
  int popover_hover;  // row or control under the mouse in the open dropdown, -1 for none
  ULONGLONG popover_opened_at, popover_closed_at;
  PopoverKind popover_closing;  // drawn fading out after it closed (ui/popover.c)
  int flipped_switch;           // the settings switch last pressed, sliding (ui/settings_input.c)
  ULONGLONG flipped_at;
  ResetPhase reset_phase;
  int hover_close;  // id of the pane whose close button is under the mouse
  int unfocused_pane;  // the pane that last lost focus, its glow fading (ui/animation.c)
  ULONGLONG focus_moved_at;
  bool empty_shown;  // the empty state is coming, or going since empty_changed_at
  ULONGLONG empty_changed_at;
  int drag_pane;    // id of the pane that owns the mouse drag (selection, tracking)
  HeaderDrag header_drag;
  DividerDrag divider;  // divider.split is set while one is dragged
  int divider_hover;    // index of the divider under the mouse, -1 for none
  int drop_pane;    // id of the pane files are being dragged over
  bool leave_tracked;
  HWND tooltip;           // ui/tooltip.c
  Box tip_box;            // what the tooltip explains now
  wchar_t tip_text[512];  // and its text
  int wheel_rest;  // wheel movement short of one notch
  bool link_pressed;  // Ctrl+press on a link, waiting for the release
  TermLink pressed_link;
  WCHAR high_surrogate;
  char composition[256];  // IME text being composed (UTF-8)
} App;

App *app_new(HWND hwnd, const wchar_t *start_dir);
void app_free(App *a);
// Rebuilds fonts and sizes for a new DPI or font size. Returns false when
// no terminal font is installed.
bool app_set_dpi(App *a, UINT dpi);

// A size in CSS pixels, scaled for the screen.
int app_px(const App *a, float css_px);

Box app_workspace(const App *a);
PaneLayout app_pane_layout(const App *a);
// Fits the panes to the window, with no animation.
void app_layout(App *a);
void app_redraw(App *a);

// app_panes.c: opening, closing, moving and focusing panes, animated.
void app_open_pane(App *a);
// A pane with shell in cwd (NULL: the user's home), command (NULL: none)
// typed at its first prompt. Its shell knows the pane's agent files.
Pane *app_open_pane_in(App *a, ShellKind shell, const wchar_t *cwd, const char *command);
void app_close_pane(App *a, Pane *p);
// Panes done with their exit animation give their place to the others.
void app_settle_panes(App *a);
void app_move_pane(App *a, Pane *p, Pane *target, Side zone);
void app_tidy_panes(App *a);
// Moves keyboard focus to p (or the pane step places over) and tells the
// programs that asked for focus reports.
void app_focus_pane(App *a, Pane *p);
void app_focus_step(App *a, int step);
