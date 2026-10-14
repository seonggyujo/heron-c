// IME composition and result strings.
#include "ui/ime.h"

#include <imm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "base/mem.h"
#include "base/utf.h"
#include "pane/pane_input.h"

// Puts the IME's caret and candidate list at the focused pane's cursor.
static void place_windows(App *a, HIMC imc) {
  Pane *p = panes_focused(&a->panes);
  if (!p) return;
  int cx, cy;
  term_cursor(p->term, &cx, &cy);
  POINT at = {p->body.x + cx * p->cell_w, p->body.y + cy * p->cell_h};
  COMPOSITIONFORM comp = {CFS_POINT, at, {0}};
  ImmSetCompositionWindow(imc, &comp);
  CANDIDATEFORM cand = {0, CFS_EXCLUDE, at, {at.x, at.y, at.x + p->cell_w, at.y + p->cell_h}};
  ImmSetCandidateWindow(imc, &cand);
}

// The string of one kind (GCS_RESULTSTR or GCS_COMPSTR) as UTF-8.
static char *read_string(HIMC imc, DWORD kind) {
  LONG bytes = ImmGetCompositionStringW(imc, kind, NULL, 0);
  if (bytes <= 0) return NULL;
  wchar_t *w = xmalloc((size_t)bytes + sizeof(wchar_t));
  ImmGetCompositionStringW(imc, kind, w, (DWORD)bytes);
  char *s = wide_to_utf8(w, bytes / (LONG)sizeof(wchar_t), NULL);
  free(w);
  return s;
}

// One message can carry both: Korean finishes a syllable and starts the
// next with a single key.
static void composition(App *a, LPARAM lp) {
  HIMC imc = ImmGetContext(a->hwnd);
  if (!imc) return;
  Pane *p = panes_focused(&a->panes);
  if (lp & GCS_RESULTSTR) {
    char *done = read_string(imc, GCS_RESULTSTR);
    if (done && p) pane_send(p, done, strlen(done));
    free(done);
    a->composition[0] = 0;
  }
  if (lp & GCS_COMPSTR) {
    char *typing = read_string(imc, GCS_COMPSTR);
    snprintf(a->composition, sizeof a->composition, "%s", typing ? typing : "");
    free(typing);
  }
  place_windows(a, imc);
  ImmReleaseContext(a->hwnd, imc);
  app_redraw(a);
}

bool ime_message(App *a, UINT msg, WPARAM wp, LPARAM lp, LRESULT *result) {
  switch (msg) {
    case WM_IME_SETCONTEXT:  // keep the candidate list, hide the IME's composition window
      *result = DefWindowProcW(a->hwnd, msg, wp, lp & ~ISC_SHOWUICOMPOSITIONWINDOW);
      return true;
    case WM_IME_STARTCOMPOSITION: {
      HIMC imc = ImmGetContext(a->hwnd);
      if (imc) {
        place_windows(a, imc);
        ImmReleaseContext(a->hwnd, imc);
      }
      *result = 0;
      return true;
    }
    case WM_IME_COMPOSITION:
      composition(a, lp);
      *result = 0;
      return true;
    case WM_IME_ENDCOMPOSITION:
      a->composition[0] = 0;
      app_redraw(a);
      *result = 0;
      return true;
    case WM_IME_CHAR:  // already sent from the result string
      *result = 0;
      return true;
    default:
      return false;
  }
}
