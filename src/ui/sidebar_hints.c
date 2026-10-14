// Laying out, drawing and hit testing the sidebar note.
#include "ui/sidebar_hints.h"

#include "app/strings.h"
#include "app/theme.h"

#define LINE_H 18.2f  // 13 px at line-height 1.4

static SidebarPart note_shown(const App *a) {
  const SidebarState *s = &a->sidebar;
  if (!a->agents.hooks_on && !s->offer_hidden) return SIDEBAR_OFFER_CLOSE;
  if (!a->agents.board.status_seen && !s->plugin_hidden) return SIDEBAR_PLUGIN_CLOSE;
  return SIDEBAR_NONE;
}

static const char *note_text(SidebarPart note) {
  return note == SIDEBAR_OFFER_CLOSE ? strings()->hooks_offer : strings()->agent_plugin_hint;
}

static const char *button_text(const App *a) {
  return a->agents.hooks_busy ? strings()->hooks_turning_on : strings()->hooks_turn_on;
}

void sidebar_hints_layout(App *a, int x, int y, int w, HintsLayout *l) {
  *l = (HintsLayout){.note = note_shown(a)};
  if (l->note == SIDEBAR_NONE) return;
  UiFont *f = a->fonts[FONT_UI_13];
  float line = LINE_H * a->scale;
  int close_w = (int)ui_text_width(f, "×") + app_px(a, 8), line_h = (int)(line + 0.5f);
  int text_w = w - close_w - app_px(a, 4);
  int lines = ui_text_wrap(NULL, f, note_text(l->note), 0, 0, (float)text_w, line, 0);
  l->text = (Box){x, y, text_w, (int)(lines * line + 0.5f)};
  l->close = (Box){x + w - close_w, y, close_w, line_h};
  l->h = l->text.h;
  if (l->note != SIDEBAR_OFFER_CLOSE) return;
  int button_w = (int)ui_text_width(f, button_text(a)) + app_px(a, 22);
  l->turn_on = (Box){x, y + l->h + app_px(a, 4), button_w, line_h + app_px(a, 6)};
  l->h += app_px(a, 4) + l->turn_on.h;
  if (!a->agents.hooks_error[0]) return;
  int error_lines = ui_text_wrap(NULL, f, a->agents.hooks_error, 0, 0, (float)w, line, 0);
  l->error = (Box){x, y + l->h + app_px(a, 4), w, (int)(error_lines * line + 0.5f)};
  l->h += app_px(a, 4) + l->error.h;
}

void sidebar_hints_paint(App *a, const HintsLayout *l) {
  if (l->note == SIDEBAR_NONE) return;
  UiFont *f = a->fonts[FONT_UI_13];
  float line = LINE_H * a->scale;
  SidebarHit hover = a->sidebar.hover;
  ui_text_wrap(a->canvas, f, note_text(l->note), l->text.x, l->text.y, l->text.w, line, THEME_FG_MUTED);
  bool close_hover = hover.part == l->note;
  ui_text_draw(a->canvas, f, "×", l->close.x, l->close.y, l->close.w, l->close.h,
               close_hover ? THEME_FG : THEME_FG_MUTED, ALIGN_CENTER);
  if (l->turn_on.h) {
    Box b = l->turn_on;
    bool lit = hover.part == SIDEBAR_HOOKS_ON && !a->agents.hooks_busy;
    float r = 6 * a->scale;
    canvas_fill_round(a->canvas, b.x, b.y, b.w, b.h, r, lit ? THEME_HOVER : THEME_CONTROL);
    canvas_stroke_round(a->canvas, b.x, b.y, b.w, b.h, r, 1, THEME_BORDER_STRONG);
    ui_text_draw(a->canvas, f, button_text(a), b.x, b.y, b.w, b.h, THEME_FG, ALIGN_CENTER);
  }
  if (l->error.h) {
    ui_text_wrap(a->canvas, f, a->agents.hooks_error, l->error.x, l->error.y, l->error.w, line, THEME_AGENT_BAD);
  }
}

SidebarPart sidebar_hints_hit(const HintsLayout *l, int x, int y) {
  if (l->note == SIDEBAR_NONE) return SIDEBAR_NONE;
  if (box_contains(l->close, x, y)) return l->note;
  if (l->turn_on.h && box_contains(l->turn_on, x, y)) return SIDEBAR_HOOKS_ON;
  return SIDEBAR_NONE;
}
