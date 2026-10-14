// Slow work off the UI thread: git, `claude --version`, reading session
// files. job runs on a new thread with data; then done runs on the UI
// thread (through WM_JOB_DONE) with the same data, which it frees. A job
// must not touch the App.
#pragma once

#include "ui/app.h"

typedef void (*BackgroundJob)(void *data);
typedef void (*BackgroundDone)(App *a, void *data);

void background_run(App *a, BackgroundJob job, BackgroundDone done, void *data);
// WM_JOB_DONE: runs the finished job's done.
void background_finished(App *a, LPARAM lp);
