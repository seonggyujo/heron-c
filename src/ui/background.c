// A thread per job, and the message that brings its result back.
#include "ui/background.h"

#include <process.h>
#include <stdlib.h>

#include "base/log.h"
#include "base/mem.h"

typedef struct {
  HWND hwnd;
  BackgroundJob job;
  BackgroundDone done;
  void *data;
} Task;

static unsigned __stdcall run(void *arg) {
  Task *t = arg;
  t->job(t->data);
  // A window closed meanwhile cannot take it: the task is lost with the app.
  if (!PostMessageW(t->hwnd, WM_JOB_DONE, 0, (LPARAM)t)) free(t);
  return 0;
}

void background_run(App *a, BackgroundJob job, BackgroundDone done, void *data) {
  Task *t = xmalloc(sizeof *t);
  *t = (Task){a->hwnd, job, done, data};
  uintptr_t thread = _beginthreadex(NULL, 0, run, t, 0, NULL);
  if (thread) {
    CloseHandle((HANDLE)thread);
    return;
  }
  log_warn("background", "no thread for a job; running it here");
  job(data);
  done(a, data);
  free(t);
}

void background_finished(App *a, LPARAM lp) {
  Task *t = (Task *)lp;
  t->done(a, t->data);
  free(t);
}
