/* GNUPLOT - plot.h */

/*[
 * Copyright 1986 - 1993, 1998, 2004   Thomas Williams, Colin Kelley
 *
 * Permission to use, copy, and distribute this software and its
 * documentation for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.
 *
 * Permission to modify the software is granted, but not the right to
 * distribute the complete modified source code.  Modifications are to
 * be distributed as patches to the released version.  Permission to
 * distribute binaries produced by compiling modified sources is granted,
 * provided you
 *   1. distribute the corresponding source modifications from the
 *    released version in the form of a patch file along with the binaries,
 *   2. add special version identification to distinguish your version
 *    in addition to the base release version number,
 *   3. provide your name and address as the primary contact for the
 *    support of your modified version, and
 *   4. retain our contact information in regard to use of the base
 *    software.
 * Permission to distribute the released version of the source code along
 * with corresponding source modifications in the form of a patch file is
 * granted with same provisions 2 through 4 for binary distributions.
 *
 * This software is provided "as is" without express or implied warranty
 * to the extent permitted by applicable law.
]*/

#ifndef GNUPLOT_PLOT_H
# define GNUPLOT_PLOT_H

/* #if... / #include / #define collection: */

#include "syscfg.h"
#include "gp_types.h"

/* Type definitions */

/* Variables of plot.c needed by other modules: */

extern TBOOLEAN interactive;
extern TBOOLEAN noinputfiles;
extern TBOOLEAN persist_cl;
extern TBOOLEAN slow_font_startup;

extern const char *user_shell;

extern TBOOLEAN ctrlc_flag;
extern TBOOLEAN terminate_flag;

#ifdef OS2
extern TBOOLEAN CallFromRexx;
#endif

/* Prototypes of functions exported by plot.c */

#if defined(__GNUC__)
void bail_to_command_line(void) __attribute__((noreturn));
void plugin_bail_to_command_line(char *) __attribute__((noreturn));
#else
void bail_to_command_line(void);
#endif

void init_constants(void);
void init_session(void);

#if defined(_WIN32)
int gnu_main(int argc, char **argv);
#endif

void interrupt_setup(void);
void gp_expand_tilde(char **);
void get_user_env(void);

#ifdef LINUXVGA
void drop_privilege(void);
void take_privilege(void);
#endif /* LINUXVGA */

#ifdef OS2
int ExecuteMacro(char *, int);
#endif

void restrict_popen(void);

#ifdef GNUPLOT_HISTORY
void cancel_history(void);
#else
#define cancel_history()  {}
#endif

#if defined(MSDOS) || defined(OS2)
/* retrieve path relative to gnuplot executable */
char * RelativePathToGnuplot(const char * path);
#endif

#endif /* GNUPLOT_PLOT_H */
