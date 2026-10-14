// What the end of one Claude Code session file says about the session
// (Heron's sessions/summary.rs), pure: the folder it last ran in and a
// title. Also the shape a session id must have before heron-c types it
// into a shell.
#pragma once

#include <stdbool.h>
#include <stddef.h>

typedef struct {
  char cwd[1024];
  char title[512];
} SessionSummary;

// tail is the end of the file (NUL-terminated), its first line possibly
// cut. The last record of each kind wins, as in Claude Code: a title the
// user gave (/rename), else the generated one, else the last prompt. The
// title is its first non-empty line, cut at 80 characters. False when the
// tail names no folder or nothing to call the session.
bool session_summarize(const char *tail, SessionSummary *out);

// A UUID ("158ec1a7-965f-43a3-99cb-8f9e257cb741"), the name Claude Code
// gives session files. Nothing else is safe to type after --resume.
bool session_is_id(const char *id);
