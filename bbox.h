/*

Copyright (C) 1998, 1999, 2004, 2005 Matthew P. Hodges
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

#ifndef __BBOX_H__
#define __BBOX_H__

enum bbox_type {AUTOMATIC, FROM_FILE};

#ifdef __BBOX_C__

bounding_box bbox;
enum bbox_type bbox_type = AUTOMATIC;
Widget bbox_dialog = NULL;  /* wjq */
Widget bbMin[4], bbMax[4];  /* 0:     The whole entry line for Min, Max values,
                               1,2,3: Entry fields for x,y,z respectively       */

#else  /* __BBOX_C__ */

extern bounding_box bbox;
extern enum bbox_type bbox_type;
extern Widget bbox_dialog;
extern Widget bbMin[4], bbMax[4];

#endif /* __BBOX_C__ */


#endif /* __BBOX_H__ */
