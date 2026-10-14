// A tiny test harness. CHECK records a failure and keeps going; main in
// test_main.c runs every test and fails if any CHECK did.
#pragma once

#include <stdio.h>
#include <string.h>

extern int g_failures;

#define CHECK(cond)                                                          \
  do {                                                                       \
    if (!(cond)) {                                                           \
      g_failures++;                                                          \
      fprintf(stderr, "%s:%d: CHECK failed: %s\n", __FILE__, __LINE__, #cond); \
    }                                                                        \
  } while (0)

#define CHECK_STR(actual, expected)                                                  \
  do {                                                                               \
    const char *a_ = (actual), *e_ = (expected);                                     \
    if (!a_ || strcmp(a_, e_) != 0) {                                                \
      g_failures++;                                                                  \
      fprintf(stderr, "%s:%d: got \"%s\", want \"%s\"\n", __FILE__, __LINE__,         \
              a_ ? a_ : "(null)", e_);                                               \
    }                                                                                \
  } while (0)
