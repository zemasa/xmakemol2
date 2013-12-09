/*

Copyright (C) 1999, 2000, 2005 Matthew P. Hodges
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

#ifndef __VECTORS_H__
#define __VECTORS_H__

enum arrow_types {ARROW_OPEN, ARROW_CLOSED, ARROW_FILLED};

#ifdef __VECTORS_C__

enum arrow_types arrow_type = ARROW_OPEN;

XPoint canvas_vector_points[5];

#else  /* __VECTORS_C__ */

extern enum arrow_types arrow_type;

extern XPoint canvas_vector_points[5];

#endif /* __VECTORS_C__ */

#endif /* __VECTORS_H__ */
