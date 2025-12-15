/*
 * contains the implementation of all syscalls.
 */

#include <stdint.h>
#include <errno.h>

#include "util/types.h"
#include "syscall.h"
#include "string.h"
#include "process.h"
#include "util/functions.h"
#include "elf.h"

#include "spike_interface/spike_utils.h"

extern elf_symbol symbols[64];
extern char sym_names[64][32];
extern int sym_count;

//
// implement the SYS_user_print syscall
//
ssize_t sys_user_print(const char *buf, size_t n)
{
  sprint(buf);
  return 0;
}

//
// implement the SYS_user_exit syscall
//
ssize_t sys_user_exit(uint64 code)
{
  sprint("User exit with code:%d.\n", code);
  // in lab1, PKE considers only one app (one process).
  // therefore, shutdown the system when the app calls exit()
  shutdown(code);
}

ssize_t locate_function_name(uint64 frame_pointer)
{
  uint64 ret_addr = *((uint64 *)(frame_pointer - 8));

  for (int i = 0; i < sym_count; i++)
  {
    uint64 func_start = symbols[i].st_value;
    uint64 func_end = func_start + symbols[i].st_size;

    if (ret_addr < func_start || ret_addr >= func_end)
      continue;

    sprint("%s\n", sym_names[i]);

    return strcmp(sym_names[i], "main") == 0 ? 1 : 0;
  }
  return 0;
}

ssize_t sys_user_print_backtrace(uint64 num_frames)
{
  uint64 fp = current->trapframe->regs.s0;
  uint64 cur_fp = *((uint64 *)(fp - 8));

  for (int cnt = 0; cnt < num_frames; cnt++)
  {
    if (locate_function_name(cur_fp))
      return 0;

    cur_fp = *((uint64 *)(cur_fp - 16));
  }
  return 0;
}

//
// [a0]: the syscall number; [a1] ... [a7]: arguments to the syscalls.
// returns the code of success, (e.g., 0 means success, fail for otherwise)
//
long do_syscall(long a0, long a1, long a2, long a3, long a4, long a5, long a6, long a7)
{
  switch (a0)
  {
  case SYS_user_print:
    return sys_user_print((const char *)a1, a2);
  case SYS_user_exit:
    return sys_user_exit(a1);
  case SYS_user_print_backtrace:
    return sys_user_print_backtrace(a1);
  default:
    panic("Unknown syscall %ld \n", a0);
  }
}