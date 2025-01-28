#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifndef UNIX_LINT
# include <io.h>
# include <ctype.h>

# include <direct.h>

# include <windows.h>
# include <pm.h>
#endif

#ifdef UNIX_LINT

char *searchpath(char *name);
#endif

#if defined(UNIX_LINT)
# include <unistd.h>
# include <errno.h>
#endif

#include "version.h"

#if defined(UNIX_LINT)
# define vim_mkdir(x, y) mkdir((char *)(x), y)
#else
# define vim_mkdir(x, y) _mkdir((char *)(x))
#endif

#define sleep(n) Sleep((n) * 1000)



#define BUFSIZE (MAX_PATH*2)		// long enough to hold a file name path
#define NUL 0

#define FAIL 0
#define OK 1

#ifndef FALSE
# define FALSE 0
#endif
#ifndef TRUE
# define TRUE 1
#endif

  /*
 * Modern way of creating registry entries, also works on 64 bit windows when
 * compiled as a 32 bit program.
 */
# ifndef KEY_WOW64_64KEY
#  define KEY_WOW64_64KEY 0x0100
# endif
# ifndef KEY_WOW64_32KEY
#  define KEY_WOW64_32KEY 0x0200
# endif

#ifdef __MINGW32__
# define UNUSED __attribute__((unused))
#else
# define UNUSED
#endif

#define VIM_STARTMENU "Programs\\Vim " VIM_VERSION_SHORT

int	interactive;		// non-zero when running interactively

/*
 * Call malloc() and exit when out of memory.
 */
    static void *
alloc(int len)
{
    void *p;

    p = malloc(len);
    if (p == NULL)
    {
	printf("ERROR: out of memory\n");
	exit(1);
    }
    return p;
}


