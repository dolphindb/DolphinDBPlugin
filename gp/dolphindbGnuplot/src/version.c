/* GNUPLOT - version.c */

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

#include "version.h"

#include "syscfg.h"		/* for FAQ_LOCATION */


const char gnuplot_version[] = "5.4";
const char gnuplot_patchlevel[] = "0";
#ifdef DEVELOPMENT_VERSION
#include "timestamp.h"
#else
const char gnuplot_date[] = "2020-07-13 ";
#endif
const char gnuplot_copyright[] = "Copyright (C) 1986-1993, 1998, 2004, 2007-2020";

const char faq_location[] = FAQ_LOCATION;

char *compile_options = (void *)0;	/* Will be loaded at runtime */

const char bug_email[] = "gnuplot-beta@lists.sourceforge.net";
const char help_email[] = "gnuplot-beta@lists.sourceforge.net";
