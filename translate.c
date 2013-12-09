/*

Copyright (C) 1998, 1999, 2001, 2002, 2004, 2005 Matthew P. Hodges
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
translate_atoms(double *vec)
{
  
  void adjust_current_frame(void);
  void canvas_cb(Widget, XtPointer, XtPointer);
  void clear_message_area (void);
  void deactivate_region(void);
  void update_bbox(void);
  void update_bond_matrix(Boolean);
  void update_lengths_dialog(Boolean);

  int i,all_moved;

  all_moved=1;

  for(i = 0; i < no_atoms; i++)
    {
      if((atoms[i].edit == 1))
        {
          atoms[i].x += vec[0];
          atoms[i].y += vec[1];
          atoms[i].z += vec[2];
        }
      else
        {
          all_moved = 0;
          atoms_sorted = 0; /* Depths have changed */
        }
    }

  if (bbox_available)
    {
      for (i = 0; i < 8; i++)
        {
          file_bbox.v[i][0] += vec[0];
          file_bbox.v[i][1] += vec[1];
          file_bbox.v[i][2] += vec[2];
        }
    }

  if(all_moved){
    for(i=0;i<3;i++){
      global_vector[i]+=vec[i];
    }
  }

  update_bbox ();

  redraw=1;

  if(!all_moved){
    update_bond_matrix(True);
  }

  deactivate_region();

  clear_message_area();

  canvas_cb(canvas,NULL,NULL);

  update_lengths_dialog(False);
}
