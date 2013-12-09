/*

Copyright (C) 1998, 1999, 2001, 2005 Matthew P. Hodges
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

#ifndef __DEFS_H__
#define __DEFS_H__

#define DEPTH (50)
#define BOND (1.12) /* 1.12 Bond fudge factor */
#define BOND2 (BOND*BOND)
#define HBOND (2.5)
#define MAX_ELEMENTS 150
#define PI (3.1415926536)
#define DEG2RAD (PI / 180.0)
#define Z_TOL (0.5)
/* The sum of the covalent radii is approximately the length of the
bond. This factor determines what proportion of the bond is initially
visible. A value of 0.5 is reasonable */
#define DEFAULT_AT_SCALE (0.5)

#endif /* __DEFS_H__ */
