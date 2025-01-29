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

/*
 * The toupper() in Bcc 5.5 doesn't work, use our own implementation.
 */
    static int
mytoupper(int c)
{
    if (c >= 'a' && c <= 'z')
	return c - 'a' + 'A';
    return c;
}

    static void
myexit(int n)
{
    if (!interactive)
    {
	// Present a prompt, otherwise error messages can't be read.
	printf("Press Enter to continue\n");
	rewind(stdin);
	(void)getchar();
    }
    exit(n);
}

typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS)(HANDLE, PBOOL);
/*
 * Check if this is a 64-bit OS.
 */
    static BOOL
is_64bit_os(void)
{
#ifdef _WIN64
    return TRUE;
#else
    BOOL bIsWow64 = FALSE;
    LPFN_ISWOW64PROCESS pIsWow64Process;

    pIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(
	    GetModuleHandle("kernel32"), "IsWow64Process");
    if (pIsWow64Process != NULL)
	pIsWow64Process(GetCurrentProcess(), &bIsWow64);
    return bIsWow64;
#endif
}

    static char *
searchpath(char *name)
{
    static char widename[2 * BUFSIZE];
    static char location[2 * BUFSIZE + 2];


    // There appears to be a bug in FindExecutableA() on Windows NT.
    // Use FindExecutableW() instead...
    MultiByteToWideChar(CP_ACP, 0, (LPCTSTR)name, -1,
	    (LPWSTR)widename, BUFSIZE);
    if (FindExecutableW((LPCWSTR)widename, (LPCWSTR)"",
		(LPWSTR)location) > (HINSTANCE)32)
    {
	WideCharToMultiByte(CP_ACP, 0, (LPWSTR)location, -1,
		(LPSTR)widename, 2 * BUFSIZE, NULL, NULL);
	return widename;
    }
    return NULL;
}

/*
 * Call searchpath() and save the result in allocated memory, or return NULL.
 */
    static char *
searchpath_save(char *name)
{
    char	*p;
    char	*s;

    p = searchpath(name);
    if (p == NULL)
	return NULL;
    s = alloc(strlen(p) + 1);
    strcpy(s, p);
    return s;
}

#ifndef CSIDL_COMMON_PROGRAMS
# define CSIDL_COMMON_PROGRAMS 0x0017
#endif
#ifndef CSIDL_COMMON_DESKTOPDIRECTORY
# define CSIDL_COMMON_DESKTOPDIRECTORY 0x0019
#endif






