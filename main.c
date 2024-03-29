#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#ifdef WINDOWS_SLOWDOWN
#include <windows.h>
#endif
#ifdef _POSIX_C_SOURCE
#include <getopt.h>
#endif

#include "z80.h"
#include "mem.h"
#include "disk.h"
#include "console.h"
#include "panic.h"



#define DEFAULT_CPM22_LOCATION "cpm22.bin"
#define DEFAULT_CBIOS_LOCATION "cbios.bin"



static z80_t z80;
static mem_t mem;



static void crash_dump(void)
{ 
#ifndef DISABLE_Z80_TRACE
  fprintf(stderr, "\n");

  fprintf(stderr, "Stack:\n");
  mem_dump(stderr, &mem,
    (z80.sp & 0xFFF0) - 0x20,
    (z80.sp & 0xFFF0) + 0x2F);

  fprintf(stderr, "Trace:\n");
  z80_trace_dump(stderr);
  z80_dump(stderr, &z80, &mem);
  fprintf(stderr, "\n");
#endif /* DISABLE_Z80_TRACE */
}



static void sig_handler(int sig)
{
  switch (sig) {
  case SIGINT:
    fprintf(stderr, "\n");
    crash_dump();
    exit(EXIT_SUCCESS);

  default:
    break;
  }
}



void panic(const char *format, ...)
{
#ifndef IGNORE_PANIC
  va_list args;

  crash_dump();

  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);

  exit(EXIT_FAILURE);
#endif /* IGNORE_PANIC */
}



void display_help(const char *progname)
{
  fprintf(stderr, "Usage: %s <options | image>\n", progname);
  fprintf(stderr, "Options:\n"
     "  -a IMAGE   Load disk IMAGE in drive A\n"
     "  -b IMAGE   Load disk IMAGE in drive B\n"
     "  -c IMAGE   Load disk IMAGE in drive C\n"
     "  -d IMAGE   Load disk IMAGE in drive D\n"
     "  -m FILE    Load CP/M 2.2 binary from FILE instead of '%s'\n"
     "  -s FILE    Load CBIOS binary from FILE instead of '%s'\n"
     "\n"
     "Using uppercase (-A, -B, -C or -D) will cause changes to the disk\n"
     "to be written back to the file instead of just being temporary.\n"
     "Instead of options, a disk image for drive A can be specified directly.\n"
     "\n",
     DEFAULT_CPM22_LOCATION,
     DEFAULT_CBIOS_LOCATION);
}



#ifdef BUSYWAIT_SLOWDOWN
/* Based on: https://www.gnu.org/software/libc/manual/html_node/Calculating-Elapsed-Time.html */
void timeval_diff(struct timeval *r, struct timeval *a, struct timeval *b)
{
  int nsec;

  if (a->tv_usec < b->tv_usec) {
    nsec = (b->tv_usec - a->tv_usec) / 1000000 + 1;
    b->tv_usec -= 1000000 * nsec;
    b->tv_sec += nsec;
  }
  if (a->tv_usec - b->tv_usec > 1000000) {
    nsec = (a->tv_usec - b->tv_usec) / 1000000;
    b->tv_usec += 1000000 * nsec;
    b->tv_sec -= nsec;
  }

  r->tv_sec = a->tv_sec - b->tv_sec;
  r->tv_usec = a->tv_usec - b->tv_usec;
}
#endif /* BUSYWAIT_SLOWDOWN */



int main(int argc, char *argv[])
{
  int c;
  char *cpm22_location = NULL;
  char *cbios_location = NULL;

  disk_init();

  while ((c = getopt(argc, argv, "a:b:c:d:A:B:C:D:m:s:h")) != -1) {
    switch (c) {
    case 'a':
    case 'b':
    case 'c':
    case 'd':
      if (disk_image_load(c - 0x61, optarg, false) != 0) {
        fprintf(stderr, "Error: Failed to load disk image: %s\n", optarg);
        return EXIT_FAILURE;
      }
      break;

    case 'A':
    case 'B':
    case 'C':
    case 'D':
      if (disk_image_load(c - 0x41, optarg, true) != 0) {
        fprintf(stderr, "Error: Failed to load disk image: %s\n", optarg);
        return EXIT_FAILURE;
      }
      break;

    case 'm':
      cpm22_location = optarg;
      break;

    case 's':
      cbios_location = optarg;
      break;

    case 'h':
      display_help(argv[0]);
      return EXIT_SUCCESS;

    case '?':
    default:
      display_help(argv[0]);
      return EXIT_FAILURE;
    }
  }

  /* Quick direct image loading. */
  if (argc > optind) {
    if (disk_image_load(0, argv[optind], false) != 0) {
      fprintf(stderr, "Error: Failed to load disk image: %s\n", argv[1]);
      return EXIT_FAILURE;
    }
  }

  signal(SIGINT, sig_handler);
#ifndef DISABLE_Z80_TRACE
  z80_trace_init();
#endif /* DISABLE_Z80_TRACE */
  z80_init(&z80);
  mem_init(&mem);

  /* Load CP/M 2.2 and CBIOS. */
  if (mem_load_from_file(&mem, (cpm22_location) ? 
    cpm22_location : DEFAULT_CPM22_LOCATION, 0xE400) != 0) {
    fprintf(stderr, "Error: Failed to load CP/M 2.2 binary!\n");
    return EXIT_FAILURE;
  }
  if (mem_load_from_file(&mem, (cbios_location) ? 
    cbios_location : DEFAULT_CBIOS_LOCATION, 0xFA00) != 0) {
    fprintf(stderr, "Error: Failed to load CBIOS binary!\n");
    return EXIT_FAILURE;
  }

  console_init();

  /* Need to set the PC directly to the BIOS,
     since this one will initialize the data area in the zero page. */
  z80.pc = 0xFA00;

  /* Load CP/M into the disks system sectors so the BIOS can reload it later,
     which is required for other programs that may use this memory area. */
  disk_sys_write(&mem, 0xE400, 0x1600);

#ifndef DISABLE_SLOWDOWN
  int count = 0;

#ifdef BUSYWAIT_SLOWDOWN
  struct timeval now, then, diff;
  gettimeofday(&then, NULL);

#elif WINDOWS_SLOWDOWN
  HANDLE timer = NULL;
  LARGE_INTEGER due_time;

  timer = CreateWaitableTimer(NULL, FALSE, NULL);
  if (timer == NULL) {
    fprintf(stderr, "CreateWaitableTimer() failed: %lu\n", GetLastError());
    return EXIT_FAILURE;
  }

  due_time.QuadPart = -100000;
  if (SetWaitableTimer(timer, &due_time, 10, NULL, NULL, FALSE) == FALSE) {
    fprintf(stderr, "SetWaitableTimer() failed: %lu\n", GetLastError());
    return EXIT_FAILURE;
  }

#else /* !BUSYWAIT_SLOWDOWN */
  struct sigaction sa;
  memset(&sa, 0, sizeof(struct sigaction));
  sa.sa_handler = &sig_handler;

  struct itimerval new;
  new.it_value.tv_sec = 0;
  new.it_value.tv_usec = 10000;
  new.it_interval.tv_sec = 0;
  new.it_interval.tv_usec = 10000;

  sigaction(SIGALRM, &sa, NULL);
  setitimer(ITIMER_REAL, &new, NULL);

#endif /* BUSYWAIT_SLOWDOWN */
#endif /* DISABLE_SLOWDOWN */

  while (1) {
    z80_execute(&z80, &mem);

#ifndef DISABLE_SLOWDOWN
    count++;

#ifdef BUSYWAIT_SLOWDOWN
    if (count > 50000) {
      count = 0;
      /* Busy-wait using clock to slow down. */
      do {
        gettimeofday(&now, NULL);
        timeval_diff(&diff, &now, &then);
      } while (diff.tv_sec == 0 && diff.tv_usec < 100000);

      gettimeofday(&then, NULL);
    }

#elif WINDOWS_SLOWDOWN
    if (count > 10000) {
      count = 0;
      if (WaitForSingleObject(timer, INFINITE) != WAIT_OBJECT_0) {
        fprintf(stderr, "WaitForSingleObject() failed: %lu\n", GetLastError());
        return EXIT_FAILURE;
      }
    }

#else /* !BUSYWAIT_SLOWDOWN */
    if (count > 5000) {
      count = 0;
      pause(); /* Wait for SIGALRM. */
    }

#endif /* BUSYWAIT_SLOWDOWN */
#endif /* DISABLE_SLOWDOWN */
  }

  return EXIT_SUCCESS;
}



