/*

Copyright (C) 1998, 1999, 2000, 2001, 2002, 2004, 2005 Matthew P. Hodges
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
#include "bbox.h"
#include "draw.h"

/* Code to actually alter the atomic x,y,z values. This routine accepts
   first the axis to rotate about, second the angle, third a logical
   variable to state whether we want to store the geometry after the 
   rotation and fourth a logical variable to determine whether to honour
   the atoms[i].edit flags */

void
rotate_atoms(double *axis, double phi, Boolean store_g, Boolean rot_subset)
{
  void adjust_current_frame(void);
  void canvas_cb(Widget, XtPointer, XtPointer);
  void clear_message_area (void);
  void deactivate_region(void);
  void normalize_vec(double *);
  void rotate(double *);
  void rotate_vectors(double *, double);
  void set_rotate_axis(double *);
  void set_rotate_angle(double);
  void set_rotate_origin(double *);
  void store_coords(void);
  void update_bbox(void);
  void update_bond_matrix(Boolean);
  void update_lengths_dialog(Boolean);
  void vector_product(double *, double *, double *);

  int i,all_rotated;

  double local_com[3], local_mass;
  double atom_coords[3];

  all_rotated = 1;
  
  local_com[0] = local_com[1] = local_com[2] = 0;
  local_mass = 0;

  changing_frame = False;
  if((rotate_about == ROTATE_ABOUT_ORIGIN) || changing_frame)
    {
      set_rotate_origin(NULL);
      for(i = 0; i < no_atoms; i++)
        {
          if(atoms[i].edit == 0)
            {
              all_rotated = 0;
              atoms_sorted = 0; /* Depths have changed */
            }
        }
    }
  else if(rotate_about == ROTATE_ABOUT_LOCAL_COM)
    {
      for(i = 0; i < no_atoms; i++)
        {
          if(atoms[i].edit == 1)
            {
              local_com[0] += atoms[i].mass * atoms[i].x;
              local_com[1] += atoms[i].mass * atoms[i].y;
              local_com[2] += atoms[i].mass * atoms[i].z;
              
              local_mass += atoms[i].mass;
            }
          else
            {
              all_rotated = 0;
              atoms_sorted = 0; /* Depths have changed */
            }
        }
      
      for(i = 0; i < 3; i++)
        {
          if (local_mass == 0)
            {
              /* All dummy atoms? */
              local_com[i] = 0;
            }
          else
            {
              local_com[i] /= local_mass;
            }
        }
      set_rotate_origin(local_com);
    }
  
  set_rotate_axis(axis);
  set_rotate_angle(phi);

  for(i = 0; i < no_atoms; i++)
    {
      if((atoms[i].edit == 1) || (rot_subset == 0))
        {
          atom_coords[0] = atoms[i].x;
          atom_coords[1] = atoms[i].y;
          atom_coords[2] = atoms[i].z;

          rotate(atom_coords);

          atoms[i].x = atom_coords[0];
          atoms[i].y = atom_coords[1];
          atoms[i].z = atom_coords[2];
        }
    }

  if (bbox_available)
    {
      for (i = 0; i < 8; i++)
        {
          rotate (file_bbox.v[i]);
        }
    }

  if(store_g && all_rotated){ 

    /* modify global vectors describing current orientation of frame */

    set_rotate_origin(NULL);

    for(i=0;i<3;i++){
      rotate(global_matrix[i]);
    }
  }

  if(!all_rotated){
    update_bond_matrix(True);
  }

  rotate_vectors(axis, phi);

  update_bbox ();

  deactivate_region();

  clear_message_area ();

  canvas_cb(canvas,NULL,NULL);

  update_lengths_dialog(False);
}


void
rotate_vectors(double *axis, double phi)
{
  
  void rotate(double *);
  void set_rotate_axis(double *);
  void set_rotate_angle(double);
  void set_rotate_origin(double *);

  int i, j;

  set_rotate_axis(axis);
  set_rotate_angle(phi);
  set_rotate_origin(NULL);

  for (i = 0; i < no_atoms; i++)
    {
      for (j = 0; j < MAX_VECTORS_PER_ATOM; j++)
        {
          if ((atoms[i].has_vector > j) && (atoms[i].edit == 1))
            {
              rotate (atoms[i].v[j]);
            }
        }
    }
}
  

/* General purpose routines which rotate a vector relative to some
   origin about an axis by a certain angle */

/* The relative origin */

static Boolean ignore_rotate_origin = 0;
static double rotate_origin[3];

/* Local axis variables */

static double rotate_axis_l[3], rotate_axis_m[3], rotate_axis_n[3];

/* The angle by which the vector will be rotated */

static double rotate_angle, sina, cosa;

/* Public routines to set the above static variables */

void
set_rotate_origin(double *origin)
{

  if(origin != NULL)
    {
      ignore_rotate_origin = 0;
      rotate_origin[0] = origin[0];
      rotate_origin[1] = origin[1];
      rotate_origin[2] = origin[2];
    }
  else
    {
      ignore_rotate_origin = 1;
    }

}


void
set_rotate_axis(double *axis)
{

  void normalize_vec(double *);
  void vector_product(double *, double *, double *);

  int i;

  /* Set up local axis frame */

  /* rotate_axis_n is parallel to rotate_axis */

  for(i = 0; i < 3; i++)
    {
      rotate_axis_n[i] = axis[i];
    }

  normalize_vec(rotate_axis_n);

  /* rotate_axis_l is perpendicular to rotate_axis_n and lies in the
     001 plane */

  if(fabs(rotate_axis_n[2]) == 1.0)
    {
      rotate_axis_l[0] = 1.0;               /* Arbitrary */
      rotate_axis_l[1] = 0.0;
      rotate_axis_l[2] = 0.0;
    }
  else
    {
      rotate_axis_l[0] =  rotate_axis_n[1];
      rotate_axis_l[1] = -rotate_axis_n[0];
      rotate_axis_l[2] =  0.0;
    }

  normalize_vec(rotate_axis_l);

  /* m is perpendicular to l and n */

  vector_product(rotate_axis_m, rotate_axis_n, rotate_axis_l);


}


void
set_rotate_angle(double angle)
{

  rotate_angle = angle;

  sina = sin(rotate_angle);     /* sin of angle */
  cosa = cos(rotate_angle);     /* cos of angle */

}


/* The actually function to do the rotation -- pass pointer to a
   double foo[3] type variable */


void
rotate(double *v)
{

  double dot_product(double *, double *);

  int i;

  double a, b, c, ap, bp;


  /* Consider a rotation of vector v about an axis (l, m, n):

     v = a l + b m + c n   ---   ie vector in local axis frame

     rotate about n (local z) by angle p => rotation matrix given by:

         /  cos(phi) sin(phi)  0 \
     M = | -sin(phi) cos(phi)  0 |
         \     0        0      1 /

  */

  /* If rotate_origin is not NULL move the vector */

  if(ignore_rotate_origin == 0)
    {
      for(i = 0; i < 3; i++)
        {
          v[i] -= rotate_origin[i];
        }
    }

  /* decompose vector into components parallel to l, m and n which we
     label a, b and c */

  a = dot_product(v, rotate_axis_l);
  b = dot_product(v, rotate_axis_m);
  c = dot_product(v, rotate_axis_n);

  /* Now the rotation about n */
  
  ap = ((a * cosa) + (b * sina));
  bp = ((b * cosa) - (a * sina));

  /* Work out what new v is */

  a = ap; b = bp;

  for(i = 0; i < 3; i++)
    {
      v[i] = ((a * rotate_axis_l[i]) +
              (b * rotate_axis_m[i]) +
              (c * rotate_axis_n[i]));
    }

  /* If rotate_origin is not NULL move the vector */

  if(ignore_rotate_origin == 0)
    {
      for(i = 0; i < 3; i++)
        {
          v[i] += rotate_origin[i];
        }
    }
}
