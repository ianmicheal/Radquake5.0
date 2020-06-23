/*
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *	@(#)stdio.h	5.3 (Berkeley) 3/15/86
 */

/*
 * NB: to fit things in six character monocase externals, the
 * stdio code uses the prefix `__s' for stdio objects, typically
 * followed by a three-character attempt at a mnemonic.
 */


#ifdef __cplusplus
extern "C" {
#endif
#define	_STDIO_H_

#include "_ansi.h"

#define	_FSTDIO			/* ``function stdio'' */

#define __need_size_t
#include <stddef.h>

#define __need___va_list
#include <stdarg.h>







#define	__SLBF	0x0001		/* line buffered */
#define	__SNBF	0x0002		/* unbuffered */
#define	__SRD	0x0004		/* OK to read */
#define	__SWR	0x0008		/* OK to write */
	/* RD and WR are never simultaneously asserted */
#define	__SRW	0x0010		/* open for reading & writing */
#define	__SEOF	0x0020		/* found EOF */
#define	__SERR	0x0040		/* found error */
#define	__SMBF	0x0080		/* _buf is from malloc */
#define	__SAPP	0x0100		/* fdopen()ed in append mode - so must  write to end */
#define	__SSTR	0x0200		/* this is an sprintf/snprintf string */
#define	__SOPT	0x0400		/* do fseek() optimisation */
#define	__SNPT	0x0800		/* do not do fseek() optimisation */
#define	__SOFF	0x1000		/* set iff _offset is in fact correct */
#define	__SMOD	0x2000		/* true => fgetline modified _p text */
#if defined(__CYGWIN__) || defined(__CYGWIN__)
#define __SCLE        0x4000          /* convert line endings CR/LF <-> NL */
#endif

/*
 * The following three definitions are for ANSI C, which took them
 * from System V, which stupidly took internal interface macros and
 * made them official arguments to setvbuf(), without renaming them.
 * Hence, these ugly _IOxxx names are *supposed* to appear in user code.
 *
 * Although these happen to match their counterparts above, the
 * implementation does not rely on that (so these could be renumbered).
 */
#define	_IOFBF	0		/* setvbuf should set fully buffered */
#define	_IOLBF	1		/* setvbuf should set line buffered */
#define	_IONBF	2		/* setvbuf should set unbuffered */

#ifndef NULL
#define	NULL	0
#endif

#define	BUFSIZ	1024
#define	EOF	(-1)

#define	FOPEN_MAX	20	/* must be <= OPEN_MAX <sys/syslimits.h> */
#define	FILENAME_MAX	1024	/* must be <= PATH_MAX <sys/syslimits.h> */
#define	L_tmpnam	1024	/* XXX must be == PATH_MAX */
#ifndef __STRICT_ANSI__
#define P_tmpdir        "/tmp"
#endif

#ifndef SEEK_SET
#define	SEEK_SET	0	/* set file offset to offset */
#endif
#ifndef SEEK_CUR
#define	SEEK_CUR	1	/* set file offset to current plus offset */
#endif
#ifndef SEEK_END
#define	SEEK_END	2	/* set file offset to EOF plus offset */
#endif

#define	TMP_MAX		26





#define _stdin_r(x)	((x)->_stdin)
#define _stdout_r(x)	((x)->_stdout)
#define _stderr_r(x)	((x)->_stderr)

/*
 * Functions defined in ANSI C standard.
 */

#ifdef __GNUC__
#define __VALIST __gnuc_va_list
#else
#define __VALIST char*
#endif


char *	_EXFUN(tempnam, (const char *, const char *));
FILE *	_EXFUN(tmpfile, (void));
char *	_EXFUN(tmpnam, (char *));
int	_EXFUN(fclose, (FILE *));
int	_EXFUN(fflush, (FILE *));
FILE *	_EXFUN(freopen, (const char *, const char *, FILE *));
void	_EXFUN(setbuf, (FILE *, char *));
int	_EXFUN(setvbuf, (FILE *, char *, int, size_t));
int	_EXFUN(ungetc, (int, FILE *));
size_t	_EXFUN(fread, (_PTR, size_t _size, size_t _n, FILE *));
size_t	_EXFUN(fwrite, (const _PTR , size_t _size, size_t _n, FILE *));
int	_EXFUN(fgetpos, (FILE *, fpos_t *));
int	_EXFUN(fseek, (FILE *, long, int));
int	_EXFUN(fsetpos, (FILE *, const fpos_t *));
long	_EXFUN(ftell, ( FILE *));
void	_EXFUN(rewind, (FILE *));
void	_EXFUN(clearerr, (FILE *));
int	_EXFUN(feof, (FILE *));
int	_EXFUN(ferror, (FILE *));
void    _EXFUN(perror, (const char *));

#ifndef __STRICT_ANSI__
int	_EXFUN(vfiprintf, (FILE *, const char *, __VALIST));
int	_EXFUN(iprintf, (const char *, ...));
int	_EXFUN(fiprintf, (FILE *, const char *, ...));
int	_EXFUN(siprintf, (char *, const char *, ...));
#endif

/*
 * Routines in POSIX 1003.1.
 */

#ifndef __STRICT_ANSI__
#ifndef _REENT_ONLY
FILE *	_EXFUN(fdopen, (int, const char *));
#endif
int	_EXFUN(fileno, (FILE *));
int	_EXFUN(getw, (FILE *));
int	_EXFUN(pclose, (FILE *));
FILE *  _EXFUN(popen, (const char *, const char *));
int	_EXFUN(putw, (int, FILE *));
void    _EXFUN(setbuffer, (FILE *, char *, int));
int	_EXFUN(setlinebuf, (FILE *));
#endif

/*
 * Recursive versions of the above.
 */


/*
 * Routines internal to the implementation.
 */

int	_EXFUN(__srget, (FILE *));
int	_EXFUN(__swbuf, (int, FILE *));

/*
 * Stdio function-access interface.
 */

#ifndef __STRICT_ANSI__
FILE	*_EXFUN(funopen,(const _PTR _cookie,
		int (*readfn)(_PTR _cookie, char *_buf, int _n),
		int (*writefn)(_PTR _cookie, const char *_buf, int _n),
		fpos_t (*seekfn)(_PTR _cookie, fpos_t _off, int _whence),
		int (*closefn)(_PTR _cookie)));

#define	fropen(cookie, fn) funopen(cookie, fn, (int (*)())0, (fpos_t (*)())0, (int (*)())0)
#define	fwopen(cookie, fn) funopen(cookie, (int (*)())0, fn, (fpos_t (*)())0, (int (*)())0)
#endif





