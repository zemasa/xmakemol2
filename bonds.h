/*

Copyright (C) 1998, 1999, 2005 Matthew P. Hodges
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

#ifndef __BONDS_H__
#define __BONDS_H__

struct node
{
  int v;
  struct node *next;
};


#ifdef __FILE_C__

struct node **bond_adjacency_list;
struct node **hbond_adjacency_list;

#else /* __FILE_C__ */

extern struct node **bond_adjacency_list;
extern struct node **hbond_adjacency_list;

#endif /* __FILE_C__ */


#endif /* __BONDS_H__ */
