// Laying out the folder menu: 5 px padding inside a 1 px border, a 26 px
// heading, 47 px folder rows 1 px apart, and the 34 px picker 4 px below.
#include "ui/folder_menu_layout.h"

#include "ui/sidebar.h"

void folder_menu_layout(App *a, FolderMenuLayout *l) {
  const FolderMenu *m = &a->sidebar.menu;
  RECT rc;
  GetClientRect(a->hwnd, &rc);
  Box anchor = sidebar_new_agent_box(a);
  int w = app_px(a, 300), most = rc.right - app_px(a, 16);
  if (w > most) w = most;
  int inset = app_px(a, 6), x = anchor.x + inset, inner = w - 2 * inset;
  int y = anchor.y + anchor.h + app_px(a, 6) + inset;
  l->head = (Box){x, y, inner, app_px(a, 26)};
  y += l->head.h;
  l->count = m->count;
  UiFont *mono = a->fonts[FONT_MONO_11];
  int chip_w = (int)ui_text_width(mono, "worktree") + app_px(a, 18), chip_h = app_px(a, 23);
  for (int i = 0; i < m->count; i++) {
    int row_h = app_px(a, 47);
    bool chip = m->repo_known && m->repo[i];
    l->worktrees[i] = chip ? (Box){x + inner - chip_w, y + (row_h - chip_h) / 2, chip_w, chip_h} : (Box){0};
    l->rows[i] = (Box){x, y, chip ? inner - chip_w - app_px(a, 4) : inner, row_h};
    y += row_h + app_px(a, 1);
  }
  y += app_px(a, 4) - (m->count ? app_px(a, 1) : 0);
  l->pick = (Box){x, y, inner, app_px(a, 34)};
  y += l->pick.h + inset;
  l->panel = (Box){anchor.x, anchor.y + anchor.h + app_px(a, 6), w, y - (anchor.y + anchor.h + app_px(a, 6))};
}

int folder_menu_item_at(const FolderMenuLayout *l, int x, int y) {
  for (int i = 0; i < l->count; i++) {
    if (box_contains(l->rows[i], x, y)) return i;
    if (l->worktrees[i].w && box_contains(l->worktrees[i], x, y)) return MENU_WORKTREE + i;
  }
  return box_contains(l->pick, x, y) ? MENU_PICK : -1;
}
