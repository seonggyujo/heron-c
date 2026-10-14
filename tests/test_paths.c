// Tests for folder names, session groups, start folders and time texts.
#include <string.h>

#include "agent/session_groups.h"
#include "app/paths.h"
#include "app/strings.h"
#include "check.h"
#include "claude/claude_commands.h"

static const char *label(const char *path) {
  static char out[256];
  folder_label(path, out, sizeof out);
  return out;
}

void test_paths(void) {
  char name[64];
  folder_name("C:\\Users\\me\\app\\", name, sizeof name);
  CHECK_STR(name, "app");
  folder_name("C:\\", name, sizeof name);
  CHECK_STR(name, "C:");
  CHECK_STR(label("C:\\code\\heron"), "heron");
  CHECK_STR(label("C:\\code\\heron\\.claude\\worktrees\\fix-login\\src"), "heron \xc2\xb7 fix-login");
  CHECK(in_worktree("C:/code/heron/.claude/worktrees/x") && !in_worktree("C:\\code\\.claude\\settings"));
  char cmd[128];
  CHECK(claude_resume_command("158ec1a7-965f-43a3-99cb-8f9e257cb741", cmd, sizeof cmd));
  CHECK_STR(cmd, "claude --resume 158ec1a7-965f-43a3-99cb-8f9e257cb741");
  CHECK(!claude_resume_command("x; rm -rf", cmd, sizeof cmd));
}

void test_session_groups(void) {
  RecentSession s[4] = {
      {"a", "C:\\work\\api", "one", 100},
      {"b", "C:\\Work\\API\\", "two", 300},
      {"c", "C:\\work\\web", "three", 200},
      {"d", "C:\\code\\heron\\.claude\\worktrees\\x", "four", 400},
  };
  SessionGroups g = session_groups(s, 4);
  CHECK(g.count == 3);  // api twice (case and slash differ), web, the worktree
  CHECK(strcmp(g.groups[0].cwd, "C:\\work\\web") == 0);  // oldest newest-session first
  CHECK(strcmp(g.groups[1].cwd, "C:\\Work\\API\\") == 0 && g.groups[1].count == 2);  // named by its newest
  CHECK(g.order[g.groups[1].first] == &s[0] && g.order[g.groups[1].first + 1] == &s[1]);
  session_groups_free(&g);
  const char *folders[START_FOLDERS_MAX];
  int n = start_folders("C:\\work\\api", s, 4, folders);
  CHECK(n == 2 && strcmp(folders[0], "C:\\work\\api") == 0 && strcmp(folders[1], "C:\\work\\web") == 0);
}

void test_time_texts(void) {
  char text[64];
  format_uptime(text, sizeof text, 134000);
  CHECK_STR(text, "2m 14s");
  format_uptime(text, sizeof text, 3900000);
  CHECK_STR(text, "1h 05m");
  format_reset_in(text, sizeof text, 7980);
  CHECK_STR(text, "2h 13m");
  format_reset_in(text, sizeof text, 720);
  CHECK_STR(text, "12m");
  format_reset_in(text, sizeof text, 273600);
  CHECK_STR(text, "3d 4h");
  format_reset_in(text, sizeof text, -30);
  CHECK_STR(text, "0m");
  unsigned long long now = 1000000000000ULL, minute = 60000;
  format_time_ago(text, sizeof text, now - 30000, now);
  CHECK_STR(text, strings()->just_now);
  format_time_ago(text, sizeof text, now - 5 * minute, now);
  CHECK(strcmp(text, "5 min. ago") == 0 || strcmp(text, "5분 전") == 0);
  format_time_ago(text, sizeof text, now - 30 * 60 * minute, now);
  CHECK(strcmp(text, "yesterday") == 0 || strcmp(text, "어제") == 0);
  format_time_ago(text, sizeof text, now - 400ULL * 24 * 60 * minute, now);
  CHECK(strcmp(text, "last yr.") == 0 || strcmp(text, "작년") == 0);
}
