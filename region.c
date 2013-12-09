/*

Copyright (C) 1999, 2000, 2001, 2005 Matthew P. Hodges
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

#define __REGION_C__

/* Functions to define a region on the canvas and to determine which
   atoms lie inside it */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>

#include <Xm/Xm.h>

#include "globals.h"

/* Function prototypes */

void canvas_cb (Widget, XtPointer, XtPointer);

static int *atoms_in_region, atoms_in_region_size;

static Boolean region_active;
static XPoint region_start, region_end;

XPoint
region_get_start(void)
{
  return (region_start);
}


void
region_set_start (XButtonEvent *bevent)
{

  void canvas_cb(Widget, XtPointer, XtPointer);
  void deactivate_region(void);

  region_start.x = bevent->x;
  region_start.y = bevent->y;

  deactivate_region ();

  canvas_cb (canvas, NULL, NULL);

}


XPoint
region_get_end(void)
{
 
  return(region_end);
  
}


void
region_set_end (XButtonEvent *bevent)
{

  void activate_region (void);

  region_end.x = bevent->x;
  region_end.y = bevent->y;

  if (region_active == 0)
    {
      activate_region ();
    }

  redraw = 1;
  canvas_cb (canvas, NULL, NULL);
}


void
activate_region (void)
{

  region_active = 1;

}


void
deactivate_region (void)
{

  region_active = 0;

}


Boolean
region_active_p (void)
{
  return(region_active);
}


int *
get_atoms_in_region (void)
{

  return (atoms_in_region);

}


void
update_atoms_in_region (Boolean inside_p)
{

  /* Work out which atoms are inside/outside the region depending on
     the value of inside_p */

  void allocate_atoms_in_region (void);
  void convert_to_canvas_coords (double *, double *, Boolean);

  int i;

  double atom_in[3], atom_out[2];
  double x1, y1, x2, y2, temp;

  /* Start coords of rectangle */

  x1 = region_start.x;
  y1 = region_start.y;

  /* End coords of rectangle */

  x2 = region_end.x;
  y2 = region_end.y;

  /* Ensure that top left of rectangle is the start */

  if(x1 > x2)
    {
      temp = x1;
      x1   = x2;
      x2   = temp;
    }

  if(y1 > y2)
    {
      temp = y1;
      y1   = y2;
      y2   = temp;
    }


  /* Initialize atoms_in_region if necessary */

  if(atoms_in_region == NULL)
    {
      allocate_atoms_in_region();
    }
  else
    {
      /* Check the size of atoms_in_region */
      
      if(atoms_in_region_size != no_atoms)
        {
          free(atoms_in_region);
          allocate_atoms_in_region();
        }
    }

  for(i = 0; i < no_atoms; i++)
    {

      atom_in[0] = atoms[i].x;
      atom_in[1] = atoms[i].y;
      atom_in[2] = atoms[i].z;
      
      convert_to_canvas_coords(atom_in, atom_out, 0);

      if((atom_out[0] > x1) &&
         (atom_out[0] < x2) &&
         (atom_out[1] > y1) &&
         (atom_out[1] < y2))
        {
          if(inside_p == 1)
            {
              atoms_in_region[i] = 1;
            }
          else
            {
              atoms_in_region[i] = 0;
            }
        }
      else
        {
          if(inside_p == 1)
            {
              atoms_in_region[i] = 0;
            }
          else
            {
              atoms_in_region[i] = 1;
            }
        }
      
    }

}


void
allocate_atoms_in_region(void)
{

  atoms_in_region = malloc((no_atoms * sizeof(int)));

  if(atoms_in_region == NULL)
    {
      fprintf(stderr, "Malloc failed for atoms_in_region (region.c)\n");
    }

  /* Keep track of the size */
  
  atoms_in_region_size = no_atoms;

}
