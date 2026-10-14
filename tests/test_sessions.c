// Tests for reading a session file's tail (Heron's summary.rs tests).
#include <string.h>

#include "check.h"
#include "claude/session_summary.h"

static const char CWD[] = "{\"type\":\"user\",\"cwd\":\"C:\\\\work\\\\api\",\"message\":{}}";

static const char *title(const char *tail) {
  static SessionSummary s;
  return session_summarize(tail, &s) ? s.title : "(none)";
}

void test_session_summary(void) {
  char tail[2048];
  snprintf(tail, sizeof tail, "%s\n{\"type\":\"last-prompt\",\"lastPrompt\":\"run the tests\"}", CWD);
  CHECK_STR(title(tail), "run the tests");
  snprintf(tail, sizeof tail,
           "%s\n{\"type\":\"ai-title\",\"aiTitle\":\"Old\"}\n{\"type\":\"last-prompt\",\"lastPrompt\":\"x\"}\n"
           "{\"type\":\"ai-title\",\"aiTitle\":\"Fix the login\"}",
           CWD);
  CHECK_STR(title(tail), "Fix the login");
  snprintf(tail, sizeof tail,
           "%s\n{\"type\":\"custom-title\",\"customTitle\":\"Release\"}\n{\"type\":\"ai-title\",\"aiTitle\":\"Fix\"}", CWD);
  CHECK_STR(title(tail), "Release");
  SessionSummary s;
  snprintf(tail, sizeof tail,
           "%s\n{\"type\":\"assistant\",\"cwd\":\"C:\\\\work\\\\web\"}\n{\"type\":\"last-prompt\",\"lastPrompt\":\"hi\"}", CWD);
  CHECK(session_summarize(tail, &s) && strcmp(s.cwd, "C:\\work\\web") == 0);  // the last folder
  snprintf(tail, sizeof tail, "t\",\"aiTitle\":\"half\"}\n%s", CWD);
  CHECK_STR(title(tail), "(none)");  // a cut first line, no title
  CHECK_STR(title("{\"type\":\"last-prompt\",\"lastPrompt\":\"hi\"}"), "(none)");  // no folder
  snprintf(tail, sizeof tail, "%s\n{\"type\":\"last-prompt\",\"lastPrompt\":\"  \"}", CWD);
  CHECK_STR(title(tail), "(none)");
  snprintf(tail, sizeof tail, "%s\n{\"type\":\"last-prompt\",\"lastPrompt\":\"\\n  first line\\nsecond\"}", CWD);
  CHECK_STR(title(tail), "first line");
  char wide[200] = "";
  for (int i = 0; i < 85; i++) strcat(wide, "y");
  snprintf(tail, sizeof tail, "%s\n{\"type\":\"ai-title\",\"aiTitle\":\"%s\"}", CWD, wide);
  CHECK(strlen(title(tail)) == 80 + 3);  // 80 characters and an ellipsis
  CHECK(session_is_id("158ec1a7-965f-43a3-99cb-8f9e257cb741"));
  CHECK(!session_is_id("158ec1a7-965f-43a3-99cb-8f9e257cb74"));
  CHECK(!session_is_id("158ec1a7-965f-43a3-99cb-8f9e257cb7;&"));
  CHECK(!session_is_id("158ec1a7x965f-43a3-99cb-8f9e257cb741"));
}
