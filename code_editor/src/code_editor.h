#ifndef CODE_EDITOR_H
# define CODE_EDITOR_H

#include "prototype.h"

// _WIN32 is defined as 1 when the compilation target is 32-bit or 64-bit.
// Note: If you want to check for 64-bit use the _WIN64 macro.
#if defined(WIN32) || defined(_WIN32)
# define MSWIN
#endif

#if defined(MSWIN) && !defined(PROTO)
# include <io.h>
#endif


// ============ the header file puzzle: order matters =========

#ifdef HAVE_CONFIG_H	// GNU autoconf (or something else) was here
# include "auto/config.h"
# define HAVE_PATHDEF

/*
 * Check if configure correctly managed to find sizeof(int).  If this failed,
 * it becomes zero.  This is likely a problem of not being able to run the
 * test program.  Other items from configure may also be wrong then!
 */
# if (VIM_SIZEOF_INT == 0)
#  error configure did not run properly.  Check auto/config.log.
# endif

# if (defined(__linux__) && !defined(__ANDROID__)) || defined(__CYGWIN__) || defined(__GNU__)
// Needed for strptime().  Needs to be done early, since header files can
// include other header files and end up including time.h, where these symbols
// matter for Vim.
// 700 is needed for mkdtemp().
#  ifndef _XOPEN_SOURCE
#   define _XOPEN_SOURCE    700
#   ifndef _BSD_SOURCE
#    define _BSD_SOURCE 1
#   endif
#   ifndef _SVID_SOURCE
#    define _SVID_SOURCE 1
#   endif
#   ifndef _DEFAULT_SOURCE
#    define _DEFAULT_SOURCE 1
#   endif
#  endif
# endif
/*
 * Cygwin may have fchdir() in a newer release, but in most versions it
 * doesn't work well and avoiding it keeps the binary backward compatible.
 */
# if defined(__CYGWIN32__) && defined(HAVE_FCHDIR)
#  undef HAVE_FCHDIR
# endif
// We may need to define the uint32_t on non-Unix system, but using the same
// identifier causes conflicts.  Therefore use UINT32_T.
# define UINT32_TYPEDEF uint32_t
#endif


// for INT_MAX, LONG_MAX et al.
#include <limits.h>

#if !defined(UINT32_TYPEDEF)
# if defined(uint32_t)  // this doesn't catch typedefs, unfortunately
#  define UINT32_TYPEDEF uint32_t
# else

  // Fall back to assuming unsigned int is 32 bit.  If this is wrong then the
  // test in blowfish.c will fail.
#  define UINT32_TYPEDEF unsigned int
# endif
#endif
// user ID of root is usually zero, but not for everybody
#ifdef __TANDEM
# ifndef _TANDEM_SOURCE
#  define _TANDEM_SOURCE
# endif
# include <floss.h>
# define ROOT_UID 65535
# define OLDXAW
# if (_TANDEM_ARCH_ == 2 && __H_Series_RVU >= 621)
#  define SA_ONSTACK_COMPATIBILITY
# endif
#else
# define ROOT_UID 0
#endif
/* Include MAC_OS_X_VERSION_* macros */
#ifdef HAVE_AVAILABILITYMACROS_H
# include <AvailabilityMacros.h>
#endif
/*
 * MACOS_X	    compiling for Mac OS X
 * MACOS_X_DARWIN   integrating the darwin feature into MACOS_X
 */
#if defined(MACOS_X_DARWIN) && !defined(MACOS_X)
# define MACOS_X
#endif
// Unless made through the Makefile enforce GUI on Mac
#if defined(MACOS_X) && !defined(HAVE_CONFIG_H)
# define UNIX
#endif
#if defined(FEAT_GUI_MOTIF) \
    || defined(FEAT_GUI_GTK) \
    || defined(FEAT_GUI_HAIKU) \
    || defined(FEAT_GUI_MSWIN) \
    || defined(FEAT_GUI_PHOTON)
# if !defined(FEAT_GUI) && !defined(NO_X11_INCLUDES)
#  define FEAT_GUI
# endif
#endif
#ifdef FEAT_GUI
# if defined(FEAT_DIRECTX)
#  define FEAT_RENDER_OPTIONS
# endif
#endif

/*
 * VIM_SIZEOF_INT is used in feature.h, and the system-specific included files
 * need items from feature.h.  Therefore define VIM_SIZEOF_INT here.
 */
#ifdef MSWIN
# define VIM_SIZEOF_INT 4
#endif

#ifdef AMIGA
  // Be conservative about sizeof(int). It could be 4 too.
# ifndef FEAT_GUI_GTK	// avoid problems when generating prototypes
#  ifdef __GNUC__
#   define VIM_SIZEOF_INT	4
#  else
#   define VIM_SIZEOF_INT	2
#  endif
# endif
#endif
#if defined(MACOS_X) && !defined(HAVE_CONFIG_H)
#  define VIM_SIZEOF_INT __SIZEOF_INT__
#endif

#if VIM_SIZEOF_INT < 4 && !defined(PROTO)
# error Vim only works with 32 bit int or larger
#endif
# include "auto/osdef.h"	// bring missing declarations in
#endif

#ifdef AMIGA
# include "os_amiga.h"
#endif

#ifdef MSWIN
# include "os_win32.h"
#endif

#if defined(MACOS_X)
# include "os_mac.h"
#endif

#ifdef __QNX__
# include "os_qnx.h"
#endif

#ifdef X_LOCALE
# include <X11/Xlocale.h>
#else
# ifdef HAVE_LOCALE_H
#  include <locale.h>
# endif
#endif

/*
 * Maximum length of a path (for non-unix systems) Make it a bit long, to stay
 * on the safe side.  But not too long to put on the stack.
 */
#ifndef MAXPATHL
# ifdef MAXPATHLEN
#  define MAXPATHL  MAXPATHLEN
# else
#  define MAXPATHL  256
# endif
#endif
#ifdef BACKSLASH_IN_FILENAME
# define PATH_ESC_CHARS ((char_u *)" \t\n*?[{`%#'\"|!<")
# define BUFFER_ESC_CHARS ((char_u *)" \t\n*?[`%#'\"|!<")
#else
#ifndef UNIX
// Note: Some systems need both string.h and strings.h (Savage).  If the
// system can't handle this, define NO_STRINGS_WITH_STRING_H.
# ifdef HAVE_STRING_H
#  include <string.h>
# endif
# if defined(HAVE_STRINGS_H) && !defined(NO_STRINGS_WITH_STRING_H)
#  include <strings.h>
# endif
# ifdef HAVE_STAT_H
#  include <stat.h>
# endif
# ifdef HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif // NON-UNIX
