// Collecting the parts of a sequence while the parser reads it: intermediate
// bytes, numeric parameters and OSC text. Only vt/parser.c uses this.
#pragma once

#include "vt/parser.h"

void collect_begin(VtParser *p);
void collect_inter(VtParser *p, uint8_t b);
void collect_digit(VtParser *p, uint8_t b);
void collect_separator(VtParser *p, uint8_t b);  // ';' or ':'
void collect_osc_begin(VtParser *p);
void collect_osc(VtParser *p, uint8_t b);
