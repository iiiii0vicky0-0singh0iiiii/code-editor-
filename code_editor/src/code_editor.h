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
