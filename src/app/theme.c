// The two frame themes, copied from Heron's styles/themes.css.
#include "app/theme.h"

static const ThemeColors GREEN = {
    .bg_app = 0xFF101412, .bg_pane = 0xFF151A17, .bar = 0xFF181E1A, .bar_bottom = 0xFF161B18,
    .raised = 0xFF1A201C,
    .control = 0xFF1D2420, .hover = 0xFF243029, .active = 0xFF171D19, .inset = 0xFF141915,
    .border = 0xFF2C3A32, .border_soft = 0xFF222C26, .border_strong = 0xFF3F5548,
    .fg = 0xFF8FD8A8, .fg_soft = 0xFFC2DCCB, .fg_dim = 0xFF86A693, .fg_muted = 0xFF6C7A71,
    .glow = 0x408FD8A8, .flash = 0x998FD8A8,
    .badge_bg = 0xFF222A25, .badge_fg = 0xFFA0ADA5, .light_off = 0xFF2A332D,
    .light_off_ring = 0x4086A693,
};

static const ThemeColors BLACK = {
    .bg_app = 0xFF0A0A0B, .bg_pane = 0xFF111113, .bar = 0xFF161618, .bar_bottom = 0xFF131315,
    .raised = 0xFF161618,
    .control = 0xFF1A1A1D, .hover = 0xFF242428, .active = 0xFF121214, .inset = 0xFF0E0E10,
    .border = 0xFF2B2B2F, .border_soft = 0xFF202023, .border_strong = 0xFF45454B,
    .fg = 0xFFECECEC, .fg_soft = 0xFFC9C9CF, .fg_dim = 0xFF9B9BA1, .fg_muted = 0xFF6E6E74,
    .glow = 0x29FFFFFF, .flash = 0x8CFFFFFF,
    .badge_bg = 0xFF232326, .badge_fg = 0xFFA5A5AA, .light_off = 0xFF2C2C30,
    .light_off_ring = 0x1FFFFFFF,
};

static ThemeKind g_kind = THEME_GREEN;

const ThemeColors *theme_colors(void) { return g_kind == THEME_BLACK ? &BLACK : &GREEN; }
ThemeKind theme_kind(void) { return g_kind; }
void theme_set(ThemeKind kind) { g_kind = kind; }
