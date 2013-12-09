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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <Xm/Xm.h>

#include "globals.h"

void
store_coords(void)
{

  int i;
  
  for(i=0;i<no_atoms;i++){
    atoms[i].ox=atoms[i].x;
    atoms[i].oy=atoms[i].y;
    atoms[i].oz=atoms[i].z;
    atoms[i].opx=atoms[i].x;
    atoms[i].opy=atoms[i].y;
    atoms[i].opz=atoms[i].z;
  }

}

void
restore_geom(void)
{

  int i;

  for(i=0;i<no_atoms;i++){
    atoms[i].x=atoms[i].ox;
    atoms[i].y=atoms[i].oy;
    atoms[i].z=atoms[i].oz;
  }

}
