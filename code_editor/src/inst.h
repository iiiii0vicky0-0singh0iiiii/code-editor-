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


/*
 * Get the path to a requested Windows shell folder.
 *
 * Return FAIL on error, OK on success
 */
    int
get_shell_folder_path(
	char *shell_folder_path,
	const char *shell_folder_name)
{
    /*
     * The following code was successfully built with make_mvc.mak.
     * The resulting executable worked on Windows 95, Millennium Edition, and
     * 2000 Professional.  But it was changed after testing...
     */
    LPITEMIDLIST    pidl = 0; // Pointer to an Item ID list allocated below
    LPMALLOC	    pMalloc;  // Pointer to an IMalloc interface
    int		    csidl;
    int		    alt_csidl = -1;
    static int	    desktop_csidl = -1;
    static int	    programs_csidl = -1;
    int		    *pcsidl;
    int		    r;

    if (strcmp(shell_folder_name, "desktop") == 0)
    {
	pcsidl = &desktop_csidl;
	csidl = CSIDL_COMMON_DESKTOPDIRECTORY;
	alt_csidl = CSIDL_DESKTOP;
    }
    else if (strncmp(shell_folder_name, "Programs", 8) == 0)
    {
	pcsidl = &programs_csidl;
	csidl = CSIDL_COMMON_PROGRAMS;
	alt_csidl = CSIDL_PROGRAMS;
    }
    else
    {
	printf("\nERROR (internal) unrecognised shell_folder_name: \"%s\"\n\n",
							   shell_folder_name);
	return FAIL;
    }

    // Did this stuff before, use the same ID again.
    if (*pcsidl >= 0)
    {
	csidl = *pcsidl;
	alt_csidl = -1;
    }

retry:
    // Initialize pointer to IMalloc interface
    if (NOERROR != SHGetMalloc(&pMalloc))
    {
	printf("\nERROR getting interface for shell_folder_name: \"%s\"\n\n",
							   shell_folder_name);
	return FAIL;
    }

 // Get an ITEMIDLIST corresponding to the folder code
    if (NOERROR != SHGetSpecialFolderLocation(0, csidl, &pidl))
    {
	if (alt_csidl < 0 || NOERROR != SHGetSpecialFolderLocation(0,
							    alt_csidl, &pidl))
	{
	    printf("\nERROR getting ITEMIDLIST for shell_folder_name: \"%s\"\n\n",
							   shell_folder_name);
	    return FAIL;
	}
	csidl = alt_csidl;
	alt_csidl = -1;
    }
 // Translate that ITEMIDLIST to a string
    r = SHGetPathFromIDList(pidl, shell_folder_path);

    // Free the data associated with pidl
    pMalloc->lpVtbl->Free(pMalloc, pidl);
    // Release the IMalloc interface
    pMalloc->lpVtbl->Release(pMalloc);

    if (!r)
    {
	if (alt_csidl >= 0)
	{
	    // We probably get here for Windows 95: the "all users"
	    // desktop/start menu entry doesn't exist.
	    csidl = alt_csidl;
	    alt_csidl = -1;
	    goto retry;
	}
	printf("\nERROR translating ITEMIDLIST for shell_folder_name: \"%s\"\n\n",
							   shell_folder_name);
	return FAIL;
    }

  // If there is an alternative: verify we can write in this directory.
    // This should cause a retry when the "all users" directory exists but we
    // are a normal user and can't write there.
    if (alt_csidl >= 0)
    {
	char tbuf[BUFSIZE];
	FILE *fd;

	strcpy(tbuf, shell_folder_path);
	strcat(tbuf, " \\ Write test");
	fd = fopen(tbuf, "w");
	if (fd == NULL)
	{
	    csidl = alt_csidl;
	    alt_csidl = -1;
	    goto retry;
	}
	fclose(fd);
	unlink(tbuf);
    }
    /*
     * Keep the found csidl for next time, so that we don't have to do the
     * write test every time.
     */
    if (*pcsidl < 0)
	*pcsidl = csidl;

    if (strncmp(shell_folder_name, "Programs\\", 9) == 0)
	strcat(shell_folder_path, shell_folder_name + 8);

    return OK;
}

/*
 * List of targets.  The first one (index zero) is used for the default path
 * for the batch files.
 */
#define TARGET_COUNT  9

struct
{
    char	*name;		// Vim exe name (without .exe)
    char	*batname;	// batch file name
    char	*lnkname;	// shortcut file name
    char	*exename;	// exe file name
    char	*exenamearg;	// exe file name when using exearg
    char	*exearg;	// argument for vim.exe or gvim.exe
    char	*oldbat;	// path to existing xxx.bat or NULL
    char	*oldexe;	// path to existing xxx.exe or NULL
    char	batpath[BUFSIZE];  // path of batch file to create; not
				   // created when it's empty
} targets[TARGET_COUNT] =
{
    {"all",	"batch files", NULL, NULL, NULL, NULL, NULL, NULL, ""},
    {"vim",	"vim.bat",	"code_editor.lnk",
			"code_editor.exe",    "code_editor1.exe",  "", NULL, NULL, ""},
    {"gvim",	"code_editor1.bat",	"code_editor1.lnk",
			"code_editor1.exe",   "code_editor1.exe", "", NULL, NULL, ""},
    {"evim",	"code_editor1.bat",	"code_editor1 Easy.lnk",
			"evim.exe",   "code_editor1.exe", "-y", NULL, NULL, ""},
    {"view",	"view.bat",	"Vim Read-only.lnk",
			"view.exe",   "code_editor.exe",  "-R", NULL, NULL, ""},
    {"gview",	"code_editor1.bat",	"gVim Read-only.lnk",
			"gview.exe",  "code_editor1.exe", "-R", NULL, NULL, ""},
    {"vimdiff", "vimdiff.bat",	"Vim Diff.lnk",
			"vimdiff.exe","code_editor.exe",  "-d", NULL, NULL, ""},
    {"gvimdiff","gvimdiff.bat",	"gVim Diff.lnk",
			"gvimdiff.exe","gvim.exe", "-d", NULL, NULL, ""},
    {"vimtutor","vimtutor.bat", "Vim tutor.lnk",
			"vimtutor.bat",  "vimtutor.bat", "", NULL, NULL, ""},
};
#if defined(DOSINST)
/*
 * Run an external command and wait for it to finish.
 */
    static void
run_command(char *cmd)
{
    char	*cmd_path;
    char	cmd_buf[BUFSIZE * 2 + 35];
    char	*p;
 cmd_path = searchpath_save("cmd.exe");
    if (cmd_path != NULL)
    {
	// There is a cmd.exe, so this might be Windows NT.  If it is,
	// we need to call cmd.exe explicitly.  If it is a later OS,
	// calling cmd.exe won't hurt if it is present.
	// Also, "start" on NT expects a window title argument.
	// Replace the slashes with backslashes.
	while ((p = strchr(cmd_path, '/')) != NULL)
	    *p = '\\';
	sprintf(cmd_buf, "%s /c start \"vimcmd\" /wait %s", cmd_path, cmd);
	free(cmd_path);
    }
    else
    {
	// No cmd.exe, just make the call and let the system handle it.
	sprintf(cmd_buf, "start /w %s", cmd);
    }
    system(cmd_buf);
}
#endif

/*
 * Append a backslash to "name" if there isn't one yet.
 */
    void
add_pathsep(char *name)
{
    int		len = strlen(name);

    if (len > 0 && name[len - 1] != '\\' && name[len - 1] != '/')
	strcat(name, "\\");
}

/*
 * The normal chdir() does not change the default drive.  This one does.
 */
    int
change_drive(int drive)
{
    char temp[3] = "-:";
    temp[0] = (char)(drive + 'A' - 1);
    return !SetCurrentDirectory(temp);
}

/*
 * Change directory to "path".
 * Return 0 for success, -1 for failure.
 */
    int
mch_chdir(char *path)
{
    if (path[0] == NUL)		// just checking...
	return 0;
    if (path[1] == ':')		// has a drive name
    {
	if (change_drive(mytoupper(path[0]) - 'A' + 1))
	    return -1;		// invalid drive name
	path += 2;
    }
    if (*path == NUL)		// drive name only
	return 0;
    return chdir(path);		// let the normal chdir() do the rest
}

