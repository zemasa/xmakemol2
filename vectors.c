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

#define __VECTORS_C__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <Xm/Xm.h>

#include "defs.h"
#include "globals.h"
#include "vectors.h"
#include "view.h"

static double vector_scale       =  1.0;
static double vector_arrow_angle = 30.0;
static double vector_arrow_scale =  0.1;

void set_vector_scale (double new_scale)
{

  vector_scale = new_scale;
  
}


double get_vector_scale (void)
{
  
  return (vector_scale);
  
}


void set_vector_arrow_angle (double new_angle)
{

  vector_arrow_angle = new_angle;
  
}


double get_vector_arrow_angle (void)
{
  
  return (vector_arrow_angle);
  
}


void set_vector_arrow_scale (double new_angle)
{

  vector_arrow_scale = new_angle;
  
}


double get_vector_arrow_scale (void)
{
  
  return (vector_arrow_scale);
  
}


/*

Anatomy of a vector:

                              e\
                              | \
   \                          |  \
    \                         |   \
     |                        |    \
o-----a-----------------------c-----b
     |                        |    /
    /                         |   /
  /                           |  /
                              | /
                              d/

Notes:

(1) The line d-c-e is defined to be perpendicular to both the a-c-b and
    [001] directions.

(2) The angles b-e-c and b-d-c correspond to vec_arrow_angle.

(3) The ratio |c-b| / |o-b| corresponds to vec_arrow_scale.

*/

void update_canvas_vector_points (int si, int j)
{

  void convert_to_canvas_coords(double *, double *, Boolean);
  double get_atom_scale(void);
  double get_vector_arrow_angle (void);
  double get_vector_arrow_scale (void);
  double get_vector_scale(void);
  double mod_of_vec(double *);
  void normalize_vec (double *);
  double vector_product (double *, double *, double *);

  double mod, radius, vector_scale, vec_arrow_angle, vec_arrow_scale, t;
  double vector[3], a[3], b[3], c[3], d[3], e[3], perp[3], z[3];
  double xy_temp[2];
  
  vector_scale = get_vector_scale ();

  if(vector_scale == 0.0)
    {
      return;
    }

  vector[0] = atoms[si].v[j][0] * vector_scale;
  vector[1] = atoms[si].v[j][1] * vector_scale;
  vector[2] = atoms[si].v[j][2] * vector_scale;
  
  mod = mod_of_vec (vector);

  if(atom_flag == 1)
    {
      radius = atoms[si].cov_rad * get_atom_scale();
      
      if(radius > mod)
        {
          return;
        }
    }
  else
    {
      radius = 0.0;
    }
  
  /* Base of vector */

  a[0] = atoms[si].x + (radius * vector[0] / mod);
  a[1] = atoms[si].y + (radius * vector[1] / mod);
  a[2] = atoms[si].z + (radius * vector[2] / mod);

  convert_to_canvas_coords(a, xy_temp, 0);

  canvas_vector_points[0].x = xy_temp[0];
  canvas_vector_points[0].y = xy_temp[1];

  /* Tip of vector */

  b[0] = atoms[si].x + vector[0];
  b[1] = atoms[si].y + vector[1];
  b[2] = atoms[si].z + vector[2];
  
  convert_to_canvas_coords(b, xy_temp, 0);
 
  canvas_vector_points[1].x = xy_temp[0];
  canvas_vector_points[1].y = xy_temp[1];

  /* Arrow base of vector */

  vec_arrow_scale = get_vector_arrow_scale ();

  c[0] = atoms[si].x + (1.0 - vec_arrow_scale) * vector[0];
  c[1] = atoms[si].y + (1.0 - vec_arrow_scale) * vector[1];
  c[2] = atoms[si].z + (1.0 - vec_arrow_scale) * vector[2];

  convert_to_canvas_coords(c, xy_temp, 0);

  canvas_vector_points[2].x = xy_temp[0];
  canvas_vector_points[2].y = xy_temp[1];

  z[0] = 0.0; z[1] = 0.0; z[2] = 1.0;

  vector_product (perp, vector, z);
  normalize_vec (perp);

  vec_arrow_angle = get_vector_arrow_angle ();

  t = mod * vec_arrow_scale *
    (sin (vec_arrow_angle * PI / 180.0) / cos (vec_arrow_angle * PI / 180.0));

  /* Point d */

  d[0] = c[0] + (t * perp[0]);
  d[1] = c[1] + (t * perp[1]);
  d[2] = c[2] + (t * perp[2]);

  convert_to_canvas_coords(d, xy_temp, 0);

  canvas_vector_points[3].x = xy_temp[0];
  canvas_vector_points[3].y = xy_temp[1];

  /* Point e */

  e[0] = c[0] - (t * perp[0]);
  e[1] = c[1] - (t * perp[1]);
  e[2] = c[2] - (t * perp[2]);

  convert_to_canvas_coords(e, xy_temp, 0);

  canvas_vector_points[4].x = xy_temp[0];
  canvas_vector_points[4].y = xy_temp[1];

}
