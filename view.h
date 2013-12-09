/*

Copyright (C) 1998, 1999, 2003, 2005 Matthew P. Hodges
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

#ifndef __VIEW_H__
#define __VIEW_H__


#ifdef __VIEW_C__

Boolean atom_flag=1;
Boolean bond_flag=1;
Boolean hbond_flag=1;
Boolean vector_flag=1;
Boolean at_nos_flag=0;
Boolean at_sym_flag=0;
Boolean axes_flag=0;
Boolean bbox_flag=0;

#else  /* __VIEW_C__ */

extern Boolean atom_flag;
extern Boolean bond_flag;
extern Boolean hbond_flag;
extern Boolean vector_flag;
extern Boolean at_nos_flag;
extern Boolean at_sym_flag;
extern Boolean axes_flag;
extern Boolean bbox_flag;

#endif /* __VIEW_C__ */


#endif /* __VIEW_H__ */
