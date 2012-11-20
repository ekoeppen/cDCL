/*
 * Copyright (c) 1997 Todd C. Miller <Todd.Miller@courtesan.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _COMPAT_LIBGEN_H
#define _COMPAT_LIBGEN_H	1

#ifdef __cplusplus
extern "C"
{
#endif

/*
NAME
	basename - extract the base portion of a pathname

SYNOPSIS
	#include <libgen.h>

	char *
	basename(const char *path);

DESCRIPTION
	The basename() function returns the last component from the pathname
	pointed to by path, deleting any trailing `/' characters.  If path con-
	sists entirely of `/' characters, a pointer to the string "/" is
	returned.  If path is a null pointer or the empty string, a pointer to
	the string "." is returned.

RETURN VALUES
	On successful completion, basename() returns a pointer to the last compo-
	nent of path.

	If basename() fails, a null pointer is returned and the global variable
	errno is set to indicate the error.

ERRORS
	The following error codes may be set in errno:

	[ENAMETOOLONG]	The path component to be returned was larger than
				    MAXPATHLEN.

WARNINGS
	The basename() function returns a pointer to internal static storage
	space that will be overwritten by subsequent calls.
*/

char* basename( const char* path );

/*
NAME
	dirname - extract the directory part of a pathname

SYNOPSIS
	#include "compat_libgen.h"

	char *
	dirname(const char *path);

DESCRIPTION
	The dirname() function is the converse of basename(3); it returns a
	pointer to the parent directory of the pathname pointed to by path.  Any
	trailing `/' characters are not counted as part of the directory name.
	If path is a null pointer, the empty string, or contains no `/' charac-
	ters, dirname() returns a pointer to the string ".", signifying the cur-
	rent directory.

RETURN VALUES
	On successful completion, dirname() returns a pointer to the parent
	directory of path.

	If dirname() fails, a null pointer is returned and the global variable
	errno is set to indicate the error.

ERRORS
	The following error codes may be set in errno:

	[ENAMETOOLONG]	The path component to be returned was larger than
				    MAXPATHLEN.

WARNINGS
	The dirname() function returns a pointer to internal static storage space
	that will be overwritten by subsequent calls (each function has its own
	separate storage).

	Other vendor implementations of dirname() may modify the contents of the
	string passed to dirname(); this should be taken into account when writ-
	ing code which calls this function if portability is desired.
*/

char* dirname( const char* path );

#ifdef __cplusplus
}
#endif

#endif /* _COMPAT_LIBGEN_H */
