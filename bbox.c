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

#define __BBOX_C__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <Xm/Xm.h>

#include "globals.h"
#include "bbox.h"
#include "defs.h"
#ifdef GL
#include "gl_funcs.h" /* aro - for visinfo */
#endif /* GL */
#include "view.h"

static int top_point_index;
static Boolean bbox_above_flag;

void
draw_bbox(Boolean dbb_arg)
{

  void draw_bbox_line(int, int);

  int i;
  
  double top_point;

  bbox_above_flag = dbb_arg;

  /* Find the point of the bounding box with the greatest value of z */

  top_point = 0;

  for(i = 0; i < 8 ; i++)
    {
      if(bbox.v[i][2] > top_point)
        {
          top_point = bbox.v[i][2];
          top_point_index = i;
        }
    }
  
  /* Look at the comment in draw_bbox_faces to see what the lines
     refer to */

  draw_bbox_line(0,1);
  draw_bbox_line(0,2);
  draw_bbox_line(0,4);

  draw_bbox_line(1,3);
  draw_bbox_line(1,5);

  draw_bbox_line(2,3);
  draw_bbox_line(2,6);

  draw_bbox_line(3,7);

  draw_bbox_line(4,5);
  draw_bbox_line(4,6);

  draw_bbox_line(5,7);
  draw_bbox_line(6,7);
}


void
draw_bbox_line(int i, int j)
{

  void convert_to_canvas_coords(double *, double *, Boolean);
  double get_z_depth(void);

  int bbox_x1, bbox_x2, bbox_y1, bbox_y2;

  double local_z_depth;
  
  local_z_depth = get_z_depth();

  /* Check that we can draw the points */

  if(depth_is_on == 1)
    {
      if(((local_z_depth - bbox.v[i][2]) < Z_TOL) ||
         ((local_z_depth - bbox.v[j][2]) < Z_TOL))
        {
          return;
        }
    }

  if(bbox_above_flag == 1)
    {
      /* Draw any lines to the topmost point -- these are drawn
         _after_ atoms and bonds */
      if((i != top_point_index) && (j != top_point_index))
        {
          /* Do nothing */
          return;
        }
    }
  else
    {
      /* Draw any lines not to the topmost point -- these are drawn
         _before_ atoms and bonds */
      if((i == top_point_index) || (j == top_point_index))
        {
          return;
        }
    }

  {
    int k;
    
    double xyz_in[3];
    double xy_out[2];

    /* Convert the Cartesian coordinates from the ith point to canvas
       coordinates */

    for(k = 0; k < 3; k++)
      {
        xyz_in[k] = bbox.v[i][k];
      }
    
    convert_to_canvas_coords(xyz_in, xy_out, 0);
    
    bbox_x1 = (int) xy_out[0];
    bbox_y1 = (int) xy_out[1];

    /* Convert the Cartesian coordinates from the jth point to canvas
       coordinates */

    for(k = 0; k < 3; k++)
      {
        xyz_in[k] = bbox.v[j][k];
      }
    
    convert_to_canvas_coords(xyz_in, xy_out, 0);
    
    bbox_x2 = (int) xy_out[0];
    bbox_y2 = (int) xy_out[1];
  }
  
  gcv.foreground=8;
  gcv.line_width=2;
  gcv.line_style=LineSolid;

  XChangeGC(display,gc,GCForeground|GCLineWidth|GCLineStyle,&gcv);

  XDrawLine(display,canvas_pm,gc,bbox_x1,bbox_y1,bbox_x2,bbox_y2);

}


void
draw_bbox_faces(void)
{

  void draw_bbox_face(int, int, int, int);

  /* 
   * The points on the square refer to a numbering scheme as
   * illustrated below --- this should make it easier to see which
   * face is being drawn by which call to draw_bbox_face.
   *
   *                   4------------------5
   *                  /|                 /|
   *                 / |                / |
   *                0------------------1  |
   *                |  |               |  |
   *                |  |               |  |
   *                |  |               |  |
   *                |  |               |  |
   *                |  6---------------|--7
   *                | /                | / 
   *                |/                 |/  
   *                2------------------3   
   */

  draw_bbox_face(0,1,3,2);
  draw_bbox_face(4,5,7,6);
  draw_bbox_face(0,2,6,4);
  draw_bbox_face(1,3,7,5);
  draw_bbox_face(0,1,5,4);
  draw_bbox_face(2,3,7,6);

}


void
draw_bbox_face(int i, int j, int k, int l)
{

  void convert_to_canvas_coords(double *, double *, Boolean);
  double get_z_depth(void);

  static int color_parsed;

  double local_z_depth, xyz_out[2];
  
  Colormap cmap;

  static XColor xcolor;

  XPoint box_points[4];

  local_z_depth = get_z_depth();
  
  /* Check that we can draw the points */

  if(depth_is_on == 1)
    {
      if(((local_z_depth - bbox.v[i][2]) < Z_TOL) ||
         ((local_z_depth - bbox.v[j][2]) < Z_TOL) ||
         ((local_z_depth - bbox.v[k][2]) < Z_TOL) ||
         ((local_z_depth - bbox.v[l][2]) < Z_TOL))
        {
          return;
        }
    }
  
  /* aro - use colormap specific to canvas and visual, default
     colormap may give inaccurate colors on some displays */
#ifdef GL
  cmap = XCreateColormap(display, XtWindow(canvas), 
                         visinfo->visual, AllocNone);
#else  /* GL */
  cmap = DefaultColormap(display,screen_num);
#endif /* GL */

  if((color_parsed==0)&&(strcmp(bb_color,"")!=0)){
    XParseColor(display,cmap,bb_color,&xcolor);
    
    if(!XAllocColor(display,cmap,&xcolor)){
      printf("Could not allocate colour %20s (draw_bbox_face)\n",
             bb_color);
    }
    
    color_parsed=1;
  }

  gcv.foreground=xcolor.pixel;
  XChangeGC(display,gc,GCForeground,&gcv);
      
  convert_to_canvas_coords(bbox.v[i], xyz_out, 0);      
  box_points[0].x = (int) xyz_out[0];
  box_points[0].y = (int) xyz_out[1];

  convert_to_canvas_coords(bbox.v[j], xyz_out, 0);      
  box_points[1].x = (int) xyz_out[0];
  box_points[1].y = (int) xyz_out[1];

  convert_to_canvas_coords(bbox.v[k], xyz_out, 0);      
  box_points[2].x = (int) xyz_out[0];
  box_points[2].y = (int) xyz_out[1];

  convert_to_canvas_coords(bbox.v[l], xyz_out, 0);      
  box_points[3].x = (int) xyz_out[0];
  box_points[3].y = (int) xyz_out[1];
    
  XFillPolygon(display,canvas_pm,gc,box_points,4,Complex,CoordModeOrigin);
  
#ifdef GL
  XFreeColormap(display, cmap);
#endif /* GL */

}


void
rotate_bbox(double *axis, double phi)
{

  void rotate(double *);
  void set_rotate_axis(double *);
  void set_rotate_angle(double);
  void set_rotate_origin(double *);
  struct frame * get_selected_frame ();
  int count_visible_atoms_in_frame (struct frame *);

  int i, frame_visible_atoms;

  struct frame *this_frame;

  this_frame = get_selected_frame ();
  frame_visible_atoms = count_visible_atoms_in_frame (this_frame);

  if((frame_visible_atoms == 0) || (bbox_flag == 0))
    {
      return;
    }

  set_rotate_axis (axis);
  set_rotate_angle (phi);
  set_rotate_origin (NULL);

  for(i = 0; i < 8; i++)
    {
      rotate (bbox.v[i]);
    }
}


void update_bbox (void)
{
  void update_normal_bbox (void);
#ifdef GL
  int crystal_p (void);
  int render_using_gl_p (void);
  void update_crystal_bbox (bounding_box *bbox);
#endif

  if (bbox_flag == 0)
    {
      return;
    }

#ifdef GL
  if ((! render_using_gl_p ()) || (! crystal_p ()))
    {
      update_normal_bbox ();
    }
  else
    {
      /* We only render crystal cell bbox when rendering w/ OpenGL */
      update_crystal_bbox (&bbox);
    }
#else  /* GL */
  update_normal_bbox ();
#endif /* GL */
}


/* wjq --> */ 

void get_automatic_bbox (void)
{
  double get_atom_scale (void);
#ifdef GL
  double gl_get_rendered_atom_size (int);
  int render_using_gl_p (void);
#endif  /* GL */
  void get_gx_coords (void);
  struct frame * get_selected_frame (void);
  int count_visible_atoms_in_frame (struct frame *);

  int i, frame_visible_atoms;

  struct frame *this_frame;

  Boolean first_values_set;

  /* Get the gx coordinates */

  get_gx_coords ();

  /* Get the bbox coords in the global axis frame */

  this_frame = get_selected_frame ();
  frame_visible_atoms = count_visible_atoms_in_frame (this_frame);

  if (frame_visible_atoms == 0)
    {
      bbox.x_max = 0;
      bbox.x_min = 0;
      bbox.y_max = 0;
      bbox.y_min = 0;
      bbox.z_max = 0;
      bbox.z_min = 0;
    }

  first_values_set = 0;

  for (i = 0; i < no_atoms; i++)
    {
      double atom_size = 0;

      if (atoms[i].visi == 1)
        { /* Bound only *visible* atoms */

#ifdef GL
          if (render_using_gl_p ())
            {
              atom_size = gl_get_rendered_atom_size (i);
            }
          else
            {
              if (atom_flag == 1)
                {
                  atom_size = atoms[i].cov_rad * get_atom_scale ();
                }
              else
                {
                  atom_size = 0;
                }
            }
#else
  	  if (atom_flag == 1)
    	    {
      	      atom_size = atoms[i].cov_rad * get_atom_scale ();
            }
          else
            {
              atom_size = 0;
            }
#endif  /* GL */

          if (first_values_set == 0)
            {
              bbox.x_max = atoms[i].g[0] + atom_size;
              bbox.y_max = atoms[i].g[1] + atom_size;
              bbox.z_max = atoms[i].g[2] + atom_size;

              bbox.x_min = atoms[i].g[0] - atom_size;
              bbox.y_min = atoms[i].g[1] - atom_size;
              bbox.z_min = atoms[i].g[2] - atom_size;

              first_values_set = 1;
            }

          if ((atoms[i].g[0] - atom_size) < bbox.x_min)
            {
              bbox.x_min = (atoms[i].g[0] - atom_size);
            }
          else if ((atoms[i].g[0] + atom_size) > bbox.x_max)
            {
              bbox.x_max = (atoms[i].g[0] + atom_size);
            }

          if ((atoms[i].g[1] - atom_size) < bbox.y_min)
            {
              bbox.y_min = (atoms[i].g[1] - atom_size);
            }
          else if ((atoms[i].g[1] + atom_size) > bbox.y_max)
            {
              bbox.y_max = (atoms[i].g[1] + atom_size);
            }
    
          if ((atoms[i].g[2] - atom_size) < bbox.z_min)
            {
              bbox.z_min = (atoms[i].g[2] - atom_size);
            }
          else if ((atoms[i].g[2] + atom_size) > bbox.z_max)
            {
              bbox.z_max = (atoms[i].g[2] + atom_size);
            }
        }
    }

  bbox.x_min += bbox_by_hand.x_min;
  bbox.x_max += bbox_by_hand.x_max;
  bbox.y_min += bbox_by_hand.y_min;
  bbox.y_max += bbox_by_hand.y_max;
  bbox.z_min += bbox_by_hand.z_min;
  bbox.z_max += bbox_by_hand.z_max;

  bbox.v[0][0] = bbox.x_max; bbox.v[0][1] = bbox.y_min; bbox.v[0][2] = bbox.z_max;
  bbox.v[1][0] = bbox.x_max; bbox.v[1][1] = bbox.y_max; bbox.v[1][2] = bbox.z_max;
  bbox.v[2][0] = bbox.x_max; bbox.v[2][1] = bbox.y_min; bbox.v[2][2] = bbox.z_min;
  bbox.v[3][0] = bbox.x_max; bbox.v[3][1] = bbox.y_max; bbox.v[3][2] = bbox.z_min;
  bbox.v[4][0] = bbox.x_min; bbox.v[4][1] = bbox.y_min; bbox.v[4][2] = bbox.z_max;
  bbox.v[5][0] = bbox.x_min; bbox.v[5][1] = bbox.y_max; bbox.v[5][2] = bbox.z_max;
  bbox.v[6][0] = bbox.x_min; bbox.v[6][1] = bbox.y_min; bbox.v[6][2] = bbox.z_min;
  bbox.v[7][0] = bbox.x_min; bbox.v[7][1] = bbox.y_max; bbox.v[7][2] = bbox.z_min;
}

void
get_file_bbox (void)
{
  int i;

  for (i = 0; i < 8; i++)
    {
      bbox.v[i][0] = file_bbox.v[i][0];
      bbox.v[i][1] = file_bbox.v[i][1];
      bbox.v[i][2] = file_bbox.v[i][2];
    }
}


void
update_normal_bbox (void)
{
  /* If this function has been called it means that the relative
     positions of atoms have changed or that atom visibilities have
     been toggled or the size of the atoms have changed.

     We need to convert the current atomic positions back into the
     fixed axis frame then calculate the coordinate (in this frame) of
     the bounding box.
     
     Then we need to rotate the bbox back into the current axis frame */
  
  double get_angle_axis (double *);
  void get_automatic_bbox (void);
  void get_file_bbox (void);
  struct frame * get_selected_frame (void);
  void rotate_bbox (double *,double);
  void set_rotate_origin(double *);

  int i, j;
  double axis[3], phi;
  struct frame *this_frame;

  this_frame = get_selected_frame ();

  if ((bbox_type == AUTOMATIC) || (! this_frame->bbox_available))
    {
      get_automatic_bbox ();

      /* Rotate back into the local axis frame */
  
      for (i = 0; i < 3; i++)
        {
          for (j = 0; j < 3; j++)
            {
              angle_axis_matrix[i][j] = global_matrix[i][j];
            }
        }
  
      phi = get_angle_axis (axis);

      rotate_bbox (axis, phi);
    }
  else
    {
      get_file_bbox ();
    }
}


void
get_gx_coords(void)
{

  double get_angle_axis(double *);
  void rotate(double *);
  void set_rotate_axis(double *);
  void set_rotate_angle(double);
  void set_rotate_origin(double *);

  int i,j;

  double phi,axis[3];

  /* We put the *transpose* of the global_matrix matrix into
     angle_axis_matrix. Notice that the indices are [j][i], not [i][j] */

  for(i = 0; i < 3; i++)
    {
      for(j = 0; j < 3; j++)
        {
          angle_axis_matrix[j][i] = global_matrix[i][j];
        }
    }
  
  phi = get_angle_axis(axis);

  set_rotate_axis(axis);
  set_rotate_angle(phi);
  set_rotate_origin(NULL);

  /* Put the local axis coords into the global axis variables */

  for(i = 0; i < no_atoms; i++)
    {
      atoms[i].g[0] = atoms[i].x;
      atoms[i].g[1] = atoms[i].y;
      atoms[i].g[2] = atoms[i].z;
    }

  /* Now rotate the global coords into the global axis frame */

  for(i = 0; i < no_atoms; i++)
    {
      rotate(atoms[i].g);
    }

}
  
