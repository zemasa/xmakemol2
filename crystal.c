/*

Copyright (C) 2001, 2002, 2004, 2005 Matthew P. Hodges
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

#include <Xm/Xm.h>

#include "globals.h"
#include "bbox.h"

static int crystal_origin_set = 0;
static int crystal_vector_set[3] = {0, 0, 0};
static int crystal_images_set = 0;

static double crystal_origin[3];
static double crystal_vector[9];
static int crystal_images[3];

void set_crystal_origin (double *x)
{
  int i;

  for (i = 0; i < 3; i++)
    {
      crystal_origin[i] = x[i];
    }

  crystal_origin_set = 1;
}

void set_crystal_vector (int i, double *x)
{
  int j;

  for (j = 0; j < 3; j++)
    {
      crystal_vector[((i - 1) * 3) + j] = x[j];
    }

  crystal_vector_set[i - 1] = 1;
}

double * get_crystal_vector (void)
{
  return (crystal_vector);
}

void set_crystal_images (int *p)
{
  int i;

  for (i = 0; i < 3; i++)
    {
      crystal_images[i] = p[i];
    }

  crystal_images_set = 1;
}

int * get_crystal_images (void)
{
  return (crystal_images);
}

int crystal_p (void)
{
  if (crystal_origin_set
      && crystal_images_set
      && crystal_vector_set[0]
      && crystal_vector_set[1]
      && crystal_vector_set[2])
    {
      return (1);
    }
  else
    {
      return (0);
    }
}

void reset_crystal_status (void)
{
  crystal_origin_set = 0;
  crystal_images_set = 0;
  crystal_vector_set[0] = 0;
  crystal_vector_set[1] = 0;
  crystal_vector_set[2] = 0;
}

void update_crystal_bbox (bounding_box *bbox)
{
  double get_angle_axis (double *);
  void rotate (double *);
  void set_rotate_axis (double *);
  void set_rotate_angle (double);
  void set_rotate_origin (double *);

  int i, j;
  double axis[3], phi, rotated_vector[9];

  /* Copy the vectors */

  for (i = 0; i < 9; i++)
    {
      rotated_vector[i] = crystal_vector[i];
    }

  /* Set up rotation */

  for (i = 0; i < 3; i++)
    {
      for (j = 0; j < 3; j++)
        {
          angle_axis_matrix[i][j] = global_matrix[i][j];
        }
    }

  phi = get_angle_axis (axis);

  set_rotate_axis (axis);
  set_rotate_angle (phi);
  set_rotate_origin (NULL);

  /* Rotate the vectors */

  rotate (rotated_vector);
  rotate (rotated_vector + 3);
  rotate (rotated_vector + 6);

  /* Build the bounding box */

  /* TODO: this won't work if we don't have the crystal origin
     coincident with the first atom */

  bbox->v[2][0] = atoms[0].x;
  bbox->v[2][1] = atoms[0].y;
  bbox->v[2][2] = atoms[0].z;
 
  for (i = 0; i < 3; i++)
    {
      bbox->v[3][i] = bbox->v[2][i] + rotated_vector[i];
      bbox->v[6][i] = bbox->v[2][i] + rotated_vector[3 + i];
      bbox->v[7][i] = bbox->v[6][i] + rotated_vector[i];
      bbox->v[0][i] = bbox->v[2][i] + rotated_vector[6 + i];
      bbox->v[1][i] = bbox->v[0][i] + rotated_vector[i];
      bbox->v[4][i] = bbox->v[0][i] + rotated_vector[3 + i];
      bbox->v[5][i] = bbox->v[4][i] + rotated_vector[i];
    }
}
