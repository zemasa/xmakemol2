/*

Copyright (C) 2000, 2003, 2005 Matthew P. Hodges
This file is part of XMakemol.

XMakemol is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

XMakemol is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with XMakemol; see the file COPYING.  If not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

#include <GL/glx.h>
#include <GL/glu.h>
#include <GL/glut.h>

#ifndef __GL_FUNCS_H__
#define __GL_FUNCS_H__

#ifdef __GL_FUNCS_C__

int gl_attrib[] = {GLX_RGBA,
                   GLX_RED_SIZE, 1,
                   GLX_GREEN_SIZE, 1,
                   GLX_BLUE_SIZE, 1,
                   GLX_DOUBLEBUFFER,
                   GLX_DEPTH_SIZE, 16,
                   None};

GLXContext glx_context;

XVisualInfo *visinfo;

#else  /* __GL_FUNCS_C__ */

extern int gl_attrib[];

extern GLXContext glx_context;

extern XVisualInfo *visinfo;

#endif /* __GL_FUNCS_C__ */

#endif /* __GL_FUNCS_H__ */
