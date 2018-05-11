#include <sys/stat.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/times.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <stdio.h>
#include "../kdebug.hpp"
#include <portable.h>

extern "C"
{
	void _exit(int)
	{
		//kassert(!"Kernel exit");
		PortHaltProcessor();
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
