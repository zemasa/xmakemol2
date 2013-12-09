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
sort_atoms(void)
{

  int compare_sorted_atoms(const void *, const void *);

  if(atoms_sorted==1){ /* don't need to resort */
    return;
  }

  qsort(sorted_atoms, no_atoms, sizeof(int), compare_sorted_atoms);

}


int
compare_sorted_atoms(const void *p, const void *q)
{  

  if( (atoms[*((int *) p)].z - atoms[*((int *) q)].z) > 0.0){
    return  1;
  }else{
    return -1;
  }

}
