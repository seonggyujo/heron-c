// Hovering and opening links.
#include "ui/links.h"

#include <stdlib.h>
#include <string.h>
#include <windows.h>
// After windows.h, which defines what shellapi.h uses.
#include <shellapi.h>

#include "base/log.h"
#include "base/utf.h"
#include "ui/keyboard.h"

static bool link_at(Pane *p, int x, int y, TermLink *out) {
  if (!box_contains(p->body, x, y)) return false;
  int col = (x - p->body.x) / p->cell_w, row = (y - p->body.y) / p->cell_h;
  if (col >= term_cols(p->term) || row >= term_rows(p->term)) return false;
  return term_link_at(p->term, term_view_row_id(p->term, row), col, out);
}

static bool same_link(const TermLink *a, const TermLink *b) {
  return a->row0 == b->row0 && a->col0 == b->col0 && strcmp(a->url, b->url) == 0;
}

bool links_mouse_move(App *a, Pane *p, int x, int y) {
  TermLink l;
  bool over = link_at(p, x, y, &l);
  if (over != p->has_hover_link || (over && !same_link(&l, &p->hover_link))) {
    p->has_hover_link = over;
    if (over) p->hover_link = l;
    app_redraw(a);
  }
  return over;
}

bool links_mouse_down(App *a, Pane *p, int x, int y) {
  if (!keyboard_mods().ctrl || !link_at(p, x, y, &a->pressed_link)) return false;
  a->link_pressed = true;
  return true;
}

static bool is_web(const char *url) { return _strnicmp(url, "http://", 7) == 0 || _strnicmp(url, "https://", 8) == 0; }

static void open_link(const char *url) {
  for (const char *c = url; *c; c++) {
    if ((unsigned char)*c < 0x20) return;  // no control characters in what goes to the shell
  }
  if (!is_web(url)) {
    log_warn("links", "not opening a non-web link");
    return;
  }
  wchar_t *w = utf8_to_wide(url, -1);
  ShellExecuteW(NULL, L"open", w, NULL, NULL, SW_SHOWNORMAL);
  free(w);
  log_info("links", "opened %s", url);
}

bool links_mouse_up(App *a, Pane *p, int x, int y) {
  if (!a->link_pressed) return false;
  a->link_pressed = false;
  TermLink l;
  if (link_at(p, x, y, &l) && same_link(&l, &a->pressed_link)) open_link(l.url);
  return true;
}
