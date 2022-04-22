#include <stdlib.h>
#include <sys/stat.h>
#include <sys/times.h>

#include <errno.h>
#undef errno
extern int errno;

#define UNUSED(x) (void)(x)

int write(int file, char *ptr, int len) {
  UNUSED(file);
  UNUSED(ptr);
  return len;
}

int close(int file) {
  UNUSED(file);
  return -1;
}

char *__env[1] = { 0 };
char **environ = __env;

int execve(char *name, char **argv, char **env) {
  UNUSED(name);
  UNUSED(argv);
  UNUSED(env);
  errno = ENOMEM;
  return -1;
}

int fork(void) {
  errno = EAGAIN;
  return -1;
}

int fstat(int file, struct stat *st) {
  UNUSED(file);
  st->st_mode = S_IFCHR;
  return 0;
}

int getpid(void) {
  return 1;
}

int isatty(int file) {
  UNUSED(file);
  return 1;
}

int kill(int pid, int sig) {
  UNUSED(pid);
  UNUSED(sig);
  errno = EINVAL;
  return -1;
}

int link(char *old, char *new) {
  UNUSED(old);
  UNUSED(new);
  errno = EMLINK;
  return -1;
}

int lseek(int file, int ptr, int dir) {
  UNUSED(file);
  UNUSED(ptr);
  UNUSED(dir);
  return 0;
}

int open(const char *name, int flags, int mode) {
  UNUSED(name);
  UNUSED(flags);
  UNUSED(mode);
  return -1;
}

int read(int file, char *ptr, int len) {
  UNUSED(file);
  UNUSED(ptr);
  UNUSED(len);
  return 0;
}

caddr_t sbrk(int incr) {
  extern char end; /* Defined by the linker */
  static char *heap_end;
  char *prev_heap_end;
  unsigned long stack_ptr;

  asm("mov r0, %0"
      : "=r" (stack_ptr)
      : /* no input */
      : /* no clobbered */
  );

  if (heap_end == 0) {
    heap_end = &end;
  }

  prev_heap_end = heap_end;
  if (heap_end + incr > (char *)stack_ptr) {
    write (1, "Heap and stack collision\n", 25);
    abort ();
  }

  heap_end += incr;
  return (caddr_t) prev_heap_end;
}

int stat(const char *file, struct stat *st) {
  UNUSED(file);
  st->st_mode = S_IFCHR;
  return 0;
}

clock_t times(struct tms *buf) {
  UNUSED(buf);
  return -1;
}

int unlink(char *name) {
  UNUSED(name);
  errno = ENOENT;
  return -1; 
}

int wait(int *status) {
  UNUSED(status);
  errno = ECHILD;
  return -1;
}

