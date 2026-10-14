// The Pty struct and the thread entry points. Only pty/pty*.c include this.
#pragma once

#include "pty/pty.h"

struct Pty {
  CRITICAL_SECTION lock;  // guards console, pending, notified
  HPCON console;          // NULL once closed
  HANDLE input, output;   // our ends of the pipes
  HANDLE process;
  DWORD pid;
  HANDLE reader, waiter;
  HANDLE drained;      // set when the window thread takes output
  HANDLE kill_event;   // set by pty_kill
  Buf pending;         // output read but not yet taken
  bool notified;       // a msg_output is on its way
  volatile LONG discard;  // drop output: nobody will read it
  HWND notify;
  UINT msg_output, msg_exit;
  WPARAM tag;
};

// Closes the console once, from any thread. ClosePseudoConsole can block
// until the console's last output is read, so the reader must still run.
void pty_close_console(Pty *p);

DWORD WINAPI pty_reader_main(void *arg);
DWORD WINAPI pty_waiter_main(void *arg);
