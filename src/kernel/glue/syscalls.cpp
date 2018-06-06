#include <sys/stat.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/times.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <stdio.h>
#include "../kdebug.hpp"
#include <libarch/arch.h>

extern "C"
{
	int _close(int file) __attribute__((alias("close")));
	int _fstat(int file, struct stat *st) __attribute__((alias("fstat")));
	int _getpid() __attribute__((alias("getpid")));
	int _isatty(int file) __attribute__((alias("isatty")));
	int _kill(int pid, int sig) __attribute__((alias("kill")));
	int _lseek(int file, int ptr, int dir) __attribute__((alias("lseek")));
	int _open(const char *name, int flags, ...) __attribute__((alias("open")));
	int _read(int file, char *ptr, int len) __attribute__((alias("read")));
	int _write(int file, char *ptr, int len) __attribute__((alias("write")));

	void _exit(int)
	{
		kassert(!"Kernel exit");
		//PortHaltProcessor();
	}

	int close(int file)
	{
		errno = ENOSYS;
		return -1;
	}

	char **environ; /* pointer to array of char * strings that define the current environment variables */
	int execve(const char *name, char*const* argv, char* const* env);
	int fork();

	int fstat(int file, struct stat *st)
	{
		errno = ENOSYS;
		return -1;
	}

	int getpid()
	{
		errno = ENOSYS;
		return -1;
	}

	int isatty(int file)
	{
		errno = ENOSYS;
		return 0;
	}

	int kill(int pid, int sig)
	{
		g_Logger->PutString("kill\n");
		errno = ENOSYS;
		return -1;
	}

	int link(char *old, char *newl);

	int lseek(int file, int ptr, int dir)
	{
		errno = ENOSYS;
		return -1;
	}

	int open(const char *name, int flags, ...)
	{
		errno = ENOSYS;
		return -1;
	}

	int read(int file, char *ptr, int len)
	{
		errno = ENOSYS;
		return -1;
	}

	caddr_t sbrk(int incr);
	int stat(const char *file, struct stat *st);
	clock_t times(struct tms *buf);
	int unlink(char *name);
	int wait(int *status);

	int write(int file, char *ptr, int len)
	{
		errno = ENOSYS;
		return -1;
	}

	int gettimeofday(struct timeval *__restrict p, void *__restrict tz);
}
