#  Copyright (C) 2001, 2002, 2003, 2004, 2005 Matthew P. Hodges
#  This file is part of XMakemol.

#  XMakemol is free software; you can redistribute it and/or modify it
#  under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2, or (at your option)
#  any later version.

#  XMakemol is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.

#  You should have received a copy of the GNU General Public License
#  along with XMakemol; see the file COPYING. If not, write to the
#  Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#  MA 02111-1307 USA

DATA_DIR=${prefix}/share/xmakemol
ELEM_FILE=$(DATA_DIR)/elements
CFLAGS = @CFLAGS@ -DELEMENTS=\"${ELEM_FILE}\"

# List of non-source files that are included in the distribution package 
# created by the `make dist'.  Note that there is no more `VERSION' file
# as the `configure.in' already contains the current version number.
EXTRA_DIST = \
	elements \
	PROBLEMS \
	Makefile.orig \
	ToDo.txt \
	xmake_anim.pl \
	xmakemol.1 \
	xmake_anim.pl.1 \
	xmakemol.html \
	xmakemol.txt \
	examples/water.xyz \
	examples/C60.xyz \
	examples/caffeine.xyz

# Executables installed to `bin' directory.
bin_PROGRAMS = xmakemol
bin_SCRIPTS = xmake_anim.pl

# Sources making up the `xmakemol' executable.
EXTRA_xmakemol_SOURCES = \
	bbox.h \
	config.h \
	draw.h \
	globals.h  \
	view.h \
	bonds.h \
	defs.h \
	gl_funcs.h \
	vectors.h \
	xm_logo.h \
	gl2ps.h \
	operations.h

xmakemol_SOURCES = \
	animate.c \
	basename.c \
	bbox.c \
	canvas.c \
	control.c \
	crystal.c \
	draw.c \
	edit.c \
	fig.c \
	file.c \
	frames.c \
	gl_funcs.c \
	help.c \
	menus.c \
	region.c \
	rotate.c \
	sort.c \
	store.c \
	track.c \
	translate.c \
	utils.c \
	vectors.c \
	view.c \
	xmakemol.c  \
	gl2ps.c \
	operations.c

xmakemol_LDADD= @GL_LIBS@ @XM_LIBS@ @XT_LIBS@ @XPM_LIBS@ @XEXT_LIBS@ @XI_LIBS@ @X11_LIBS@

# List of files installed to data directory.
elemdir = $(DATA_DIR)
elem_DATA = elements

# List of manual pages installed.
man_MANS = xmakemol.1
