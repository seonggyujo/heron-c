// The shell dropdown's input and choosing. Drawing is in shell_menu_paint.c.
#include "ui/shell_menu.h"

#include "app/prefs.h"
#include "ui/popover.h"
#include "ui/shell_menu_layout.h"

static void pick(App *a, ShellKind kind) {
  a->shell = kind;
  prefs_set(PREF_SHELL, shell_id(kind));
  popover_close(a);
}

static int row_at(const MenuLayout *m, int x, int y) {
  for (int i = 0; i < m->count; i++) {
    if (box_contains(m->rows[i], x, y)) return i;
  }
  return -1;
}

Box shell_menu_panel(App *a) {
  MenuLayout m;
  shell_menu_layout(a, &m);
  return m.panel;
}

bool shell_menu_mouse_move(App *a, int x, int y) {
  MenuLayout m;
  shell_menu_layout(a, &m);
  int hover = row_at(&m, x, y);
  if (hover != a->popover_hover) {
    a->popover_hover = hover;
    app_redraw(a);
  }
  return box_contains(m.panel, x, y);
}

bool shell_menu_mouse_down(App *a, int x, int y) {
  MenuLayout m;
  shell_menu_layout(a, &m);
  int row = row_at(&m, x, y);
  if (row >= 0) pick(a, m.kinds[row]);
  return box_contains(m.panel, x, y);
}

bool shell_menu_key(App *a, WPARAM vk) {
  MenuLayout m;
  shell_menu_layout(a, &m);
  if (vk == VK_UP || vk == VK_DOWN) {
    int step = vk == VK_DOWN ? 1 : -1, h = a->popover_hover;
    a->popover_hover = h < 0 ? (step > 0 ? 0 : m.count - 1) : (h + step + m.count) % m.count;
    app_redraw(a);
    return true;
  }
  if (vk == VK_RETURN) {
    if (a->popover_hover >= 0) pick(a, m.kinds[a->popover_hover]);
    return true;
  }
  return false;
}
