// The language choice and the texts with numbers or names in them. Source
// is UTF-8 (see strings_ko.c).
#include "app/strings.h"

#include <stdio.h>
#include <string.h>
#include <windows.h>

#include "app/prefs.h"
#include "app/strings_tables.h"

static int g_lang = -1;

static Lang system_lang(void) {
  return PRIMARYLANGID(GetUserDefaultUILanguage()) == LANG_KOREAN ? LANG_KO : LANG_EN;
}

Lang strings_lang(void) {
  if (g_lang < 0) {
    char saved[8];
    g_lang = prefs_get("lang", saved, sizeof saved) ? (strcmp(saved, "ko") == 0 ? LANG_KO : LANG_EN)
                                                    : system_lang();
  }
  return (Lang)g_lang;
}

const Strings *strings(void) { return strings_lang() == LANG_KO ? &STRINGS_KO : &STRINGS_EN; }

void strings_set_lang(Lang lang) {
  g_lang = lang;
  prefs_set("lang", lang == LANG_KO ? "ko" : "en");
}

void strings_follow_system(void) {
  prefs_delete("lang");
  g_lang = system_lang();
}

static bool ko(void) { return strings_lang() == LANG_KO; }

void format_running(char *out, size_t cap, int running, int total) {
  if (running == total) snprintf(out, cap, ko() ? "%d개 실행 중" : "%d running", running);
  else if (ko()) snprintf(out, cap, "%d개 중 %d개 실행 중", total, running);
  else snprintf(out, cap, "%d / %d running", running, total);
}

void format_exit_code(char *out, size_t cap, unsigned long code) { snprintf(out, cap, "exit %lu", code); }

void format_agent_mismatch(char *out, size_t cap, const char *selected, const char *actual) {
  snprintf(out, cap, ko() ? "선택 %s, 실제 응답 %s" : "Selected %s, answered by %s", selected, actual);
}

void format_hooks_old_claude(char *out, size_t cap, const char *version) {
  snprintf(out, cap, ko() ? "Claude Code 2.1.139 이상이 필요해요(지금 %s)."
                          : "Needs Claude Code 2.1.139 or later (found %s).", version);
}

void format_new_agent_in(char *out, size_t cap, const char *folder) {
  snprintf(out, cap, ko() ? "새 pane의 %s 폴더에서 Claude Code 시작"
                          : "Start Claude Code in a new pane, in %s", folder);
}

void format_worktree_hint(char *out, size_t cap, const char *folder) {
  snprintf(out, cap, ko() ? "%s의 새 git worktree에서 시작(폴더와 브랜치를 따로 받아요)"
                          : "Start in a new git worktree of %s: a folder and a branch of its own", folder);
}

void format_uncommitted(char *out, size_t cap, int files) {
  if (ko()) snprintf(out, cap, "커밋하지 않은 파일 %d개", files);
  else if (files == 1) snprintf(out, cap, "1 file with uncommitted changes");
  else snprintf(out, cap, "%d files with uncommitted changes", files);
}

void format_more_subagents(char *out, size_t cap, int n) {
  snprintf(out, cap, ko() ? "외 %d개" : "+%d more", n);
}

void format_more(char *out, size_t cap, int n, bool above) {
  if (ko()) snprintf(out, cap, above ? "↑ %d개 더" : "↓ %d개 더", n);
  else snprintf(out, cap, above ? "↑ %d more" : "↓ %d more", n);
}

void format_limit_resets(char *out, size_t cap, const char *left) {
  snprintf(out, cap, ko() ? "%s 뒤 초기화" : "resets in %s", left);
}

void format_context_used(char *out, size_t cap, const char *folder, int percent) {
  snprintf(out, cap, ko() ? "%s: 컨텍스트 창의 %d%% 사용" : "%s: %d%% of the context window used", folder,
           percent);
}
