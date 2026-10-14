// Collecting intermediate bytes, parameters and OSC text for the parser.
#include "vt/parser_collect.h"

void collect_begin(VtParser *p) {
  p->ninter = 0;
  p->inter[0] = 0;
  p->prefix = 0;
  p->params.n = 0;
}

void collect_inter(VtParser *p, uint8_t b) {
  if (p->ninter == VT_MAX_INTER) return;
  p->inter[p->ninter++] = (char)b;
  p->inter[p->ninter] = 0;
}

static void ensure_first_param(VtParams *ps) {
  if (ps->n > 0) return;
  ps->n = 1;
  ps->v[0] = -1;
  ps->colon[0] = false;
}

void collect_digit(VtParser *p, uint8_t b) {
  ensure_first_param(&p->params);
  int *v = &p->params.v[p->params.n - 1];
  if (*v < 0) *v = 0;
  if (*v < 100000) *v = *v * 10 + (b - '0');
}

void collect_separator(VtParser *p, uint8_t b) {
  VtParams *ps = &p->params;
  ensure_first_param(ps);
  if (ps->n == VT_MAX_PARAMS) return;
  ps->v[ps->n] = -1;
  ps->colon[ps->n] = (b == ':');
  ps->n++;
}

void collect_osc_begin(VtParser *p) {
  p->osc_len = 0;
  p->osc_overflow = false;
}

void collect_osc(VtParser *p, uint8_t b) {
  if (p->osc_len < VT_OSC_MAX) {
    p->osc[p->osc_len++] = (char)b;
  } else {
    p->osc_overflow = true;
  }
}
