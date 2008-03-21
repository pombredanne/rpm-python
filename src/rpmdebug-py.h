#ifndef _RPMDEBUG_PY_H
#define _RPMDEBUG_PY_H

#include <assert.h>

#define _debug(format, ...) fprintf(stderr, "*** %s: ", __func__); \
			    fprintf(stderr, format, __VA_ARGS__) 
#define _nodebug(format, ...) {}

#define debug _nodebug

#endif
