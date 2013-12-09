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

#ifndef __DRAW_H__
#define __DRAW_H__


#ifdef __DRAW_C__

double cartesian_bond_points[6][3];
double cartesian_hbond_points[2][3];

XPoint canvas_bond_points[6];
XPoint canvas_hbond_points[2];

#else  /* __DRAW_C__ */

extern double cartesian_bond_points[6][3];
extern double cartesian_hbond_points[2][3];

extern XPoint canvas_bond_points[6];
extern XPoint canvas_hbond_points[2];

#endif /* __DRAW_C__ */


#endif /* __DRAW_H */
