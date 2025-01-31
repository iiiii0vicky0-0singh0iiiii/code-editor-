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
