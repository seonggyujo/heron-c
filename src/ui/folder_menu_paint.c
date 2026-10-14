// Drawing the folder menu (Heron's agent-folder-menu.css): a raised panel
// with a 12 px heading, folder rows of a 14 px semibold name over an 11 px
// mono path, "worktree" chips, and the 13 px picker under a soft line.
#include "app/paths.h"
#include "app/strings.h"
#include "app/theme.h"
#include "ui/folder_menu.h"
#include "ui/folder_menu_layout.h"
#include "ui/widgets.h"

static void paint_row(App *a, const FolderMenuLayout *l, int i) {
  const char *folder = a->sidebar.menu.folders[i];
  Box b = l->rows[i];
  float k = a->scale, x = b.x + 10 * k, w = b.w - 20 * k;
  bool hover = a->popover_hover == i;
  if (hover) canvas_fill_round(a->canvas, b.x, b.y, b.w, b.h, 6 * k, THEME_HOVER);
  char name[256];
  folder_name(folder, name, sizeof name);
  ui_text_draw(a->canvas, a->fonts[FONT_UI_14_SEMI], name[0] ? name : folder, x, b.y + 6 * k, w, 19 * k,
               hover ? THEME_FG : THEME_FG_SOFT, ALIGN_LEFT);
  ui_text_draw(a->canvas, a->fonts[FONT_MONO_11], folder, x, b.y + 26 * k, w, 15 * k, THEME_FG_MUTED, ALIGN_LEFT);
  Box c = l->worktrees[i];
  if (!c.w) return;
  bool lit = a->popover_hover == MENU_WORKTREE + i;
  canvas_fill_round(a->canvas, c.x, c.y, c.w, c.h, 6 * k, lit ? THEME_HOVER : THEME_INSET);
  canvas_stroke_round(a->canvas, c.x, c.y, c.w, c.h, 6 * k, 1, THEME_BORDER_SOFT);
  ui_text_draw(a->canvas, a->fonts[FONT_MONO_11], "worktree", c.x, c.y, c.w, c.h, lit ? THEME_FG : THEME_FG_DIM,
               ALIGN_CENTER);
}

void folder_menu_paint(App *a) {
  FolderMenuLayout l;
  folder_menu_layout(a, &l);
  float k = a->scale;
  widget_panel(a, l.panel, 9 * k);
  ui_text_draw(a->canvas, a->fonts[FONT_UI_12], strings()->agent_folders, l.head.x + 10 * k, l.head.y + 6 * k,
               l.head.w - 20 * k, 16 * k, THEME_FG_MUTED, ALIGN_LEFT);
  for (int i = 0; i < l.count; i++) paint_row(a, &l, i);
  Box p = l.pick;
  bool hover = a->popover_hover == MENU_PICK;
  if (hover) {
    canvas_push_clip(a->canvas, p.x, p.y, p.w, p.h);
    canvas_fill_round(a->canvas, p.x, p.y - 6 * k, p.w, p.h + 6 * k, 6 * k, THEME_HOVER);
    canvas_pop_clip(a->canvas);
  }
  canvas_fill_rect(a->canvas, p.x, p.y, p.w, 1, THEME_BORDER_SOFT);
  ui_text_draw(a->canvas, a->fonts[FONT_UI_13], strings()->pick_folder, p.x + 10 * k, p.y + 1, p.w - 20 * k, p.h - 1,
               hover ? THEME_FG : THEME_FG_DIM, ALIGN_LEFT);
}
