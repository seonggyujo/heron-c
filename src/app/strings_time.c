// Times as text: how long something has run ("2m 14s", Heron's
// formatUptime), how long until a limit resets ("2h 13m", formatResetIn)
// and how long ago it was in the UI language ("5 min. ago",
// "어제"), the words Intl.RelativeTimeFormat gives Heron's webview with
// numeric "auto" and the short style. Whole units, rounded down.
#include <math.h>
#include <stdio.h>

#include "app/strings.h"

void format_uptime(char *out, size_t cap, unsigned long long ms) {
  unsigned long long s = ms / 1000, m = s / 60;
  if (s < 60) snprintf(out, cap, "%llus", s);
  else if (m < 60) snprintf(out, cap, "%llum %02llus", m, s % 60);
  else snprintf(out, cap, "%lluh %02llum", m / 60, m % 60);
}

void format_reset_in(char *out, size_t cap, double seconds) {
  long long m = seconds > 0 ? llround(seconds / 60) : 0, h = m / 60;
  if (m < 60) snprintf(out, cap, "%lldm", m);
  else if (h < 24) snprintf(out, cap, "%lldh %lldm", h, m % 60);
  else snprintf(out, cap, "%lldd %lldh", h / 24, h % 24);
}

typedef enum { MINUTE, HOUR, DAY, MONTH, YEAR } Unit;

static void relative(char *out, size_t cap, unsigned long long n, Unit unit) {
  bool ko = strings_lang() == LANG_KO;
  // Words for one (and two days in Korean), as numeric "auto" names them.
  static const char *const LAST_EN[] = {NULL, NULL, "yesterday", "last mo.", "last yr."};
  static const char *const LAST_KO[] = {NULL, NULL, "어제", "지난달", "작년"};
  const char *word = n == 1 ? (ko ? LAST_KO : LAST_EN)[unit] : NULL;
  if (ko && unit == DAY && n == 2) word = "그저께";
  if (word) {
    snprintf(out, cap, "%s", word);
    return;
  }
  static const char *const EN[] = {"%llu min. ago", "%llu hr. ago", "%llu days ago", "%llu mo. ago", "%llu yr. ago"};
  static const char *const KO[] = {"%llu분 전", "%llu시간 전", "%llu일 전", "%llu개월 전", "%llu년 전"};
  snprintf(out, cap, (ko ? KO : EN)[unit], n);
}

void format_time_ago(char *out, size_t cap, unsigned long long ms, unsigned long long now) {
  double amount = now > ms ? (double)(now - ms) / 60000.0 : 0;
  if (amount < 1) {
    snprintf(out, cap, "%s", strings()->just_now);
    return;
  }
  static const double SIZES[] = {60, 24, 30, 12};  // minutes in an hour, hours in a day...
  for (int unit = MINUTE; unit < YEAR; unit++) {
    if (amount < SIZES[unit]) {
      relative(out, cap, (unsigned long long)amount, (Unit)unit);
      return;
    }
    amount /= SIZES[unit];
  }
  relative(out, cap, (unsigned long long)amount, YEAR);
}
