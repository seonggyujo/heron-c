// Running a program without a console window and collecting what it
// prints on stdout (git, claude --version). It gets no input, and its
// stderr is dropped. Blocks the calling thread; call it off the UI thread.
#pragma once

#include <stdbool.h>

#include "base/buf.h"
#include "base/env_block.h"

typedef struct {
  const char *command_line;  // as CreateProcess reads it
  const char *cwd;           // NULL: this process's folder
  const EnvVar *env;         // added to this process's environment
  int env_count;
  unsigned timeout_ms;  // the program is killed after this
} ProcessRun;

// Returns the exit code, or -1 when it could not start or was killed.
int process_run(const ProcessRun *run, Buf *out);
