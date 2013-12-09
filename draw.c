/*

Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005 Matthew P. Hodges
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

#define __DRAW_C__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <Xm/Xm.h>

#include "bonds.h"
#include "defs.h"
#include "draw.h"
#include "globals.h"
#include "vectors.h"
#include "view.h"

static double atom_scale  = 0.5;
static double bond_scale  = 0.5;
static double hbond_scale = 0.5;
static double smallest_cov_rad;

void
draw_atom(int si)
{

  void convert_to_canvas_coords(double *, double *, Boolean);
  double get_atom_radius(int);
  Boolean mouse_motion_p (void);
  Boolean outline_mode_p (void);

  double radius;

  double atom_coord[3], canvas_coord[2];
 
  radius = get_atom_radius(si);
  
  gcv.foreground=atoms[si].pixel;
  gcv.line_width=0;
  gcv.line_style=LineSolid;
  
  XChangeGC(display,gc,GCForeground|GCLineWidth|GCLineStyle,&gcv);
  
  /* get the canvas coords of the atom */

  atom_coord[0] = atoms[si].x;
  atom_coord[1] = atoms[si].y;
  atom_coord[2] = atoms[si].z;

  convert_to_canvas_coords(atom_coord, canvas_coord, 0);
  
  if((! mouse_motion_p ()) || (! outline_mode_p ())){

    XFillArc  
      (display,
       canvas_pm,
       gc,
       (int) (canvas_coord[0] - radius),
       (int) (canvas_coord[1] - radius),
       (int) (2.0 * radius),
       (int) (2.0 * radius),
       0,
       360*64);
    
    if(atoms[si].sel == 1)
      {
      }
  }
  
  if (atoms[si].sel == 1)
    {
      gcv.foreground = sel_color.pixel;
    }
  else
    {
      gcv.foreground = BlackPixelOfScreen (screen_ptr);
    }

  XChangeGC (display, gc, GCForeground, &gcv);
  
  XDrawArc
    (display,
     canvas_pm,
     gc,
     (int) (canvas_coord[0] - radius),
     (int) (canvas_coord[1] - radius),
     (int) (2.0 * radius),
     (int) (2.0 * radius),
     0,
     360*64);

  /* Distinguish atoms not selected for rotation/translation */
  
  if(atoms[si].edit   == 0)
    {
      XDrawLine(display,
                canvas_pm,
                gc,
                (int) (canvas_coord[0]),
                (int) (canvas_coord[1] - (radius / 2)),
                (int) (canvas_coord[0]),
                (int) (canvas_coord[1] + (radius / 2)));

      XDrawLine(display,
                canvas_pm,
                gc,
                (int) (canvas_coord[0] - (radius / 2)),
                (int) (canvas_coord[1]),
                (int) (canvas_coord[0] + (radius / 2)),
                (int) (canvas_coord[1]));
    }
}


void
draw_bond(int si, int sj)
{

  double get_atom_radius(int);
  void update_canvas_bond_points(int, int, Boolean, double);
  
  
  XPoint bond_points[5];
  
  update_canvas_bond_points(si, sj, 0, 0.0);
    
  gcv.line_width=0;
  gcv.foreground=atoms[si].pixel;
  gcv.line_style=LineSolid;

  XChangeGC(display,gc,GCLineWidth|GCForeground|GCLineStyle,&gcv);
    
  /* Draw polygon 0-1-4-5 */

  bond_points[0].x = canvas_bond_points[0].x;
  bond_points[0].y = canvas_bond_points[0].y;

  bond_points[1].x = canvas_bond_points[1].x;
  bond_points[1].y = canvas_bond_points[1].y;

  bond_points[2].x = canvas_bond_points[4].x;
  bond_points[2].y = canvas_bond_points[4].y;

  bond_points[3].x = canvas_bond_points[5].x;
  bond_points[3].y = canvas_bond_points[5].y;
    
  XFillPolygon(display,
               canvas_pm,
               gc,
               bond_points,
               4,
               Complex,CoordModeOrigin);
    
  gcv.foreground=BlackPixelOfScreen(screen_ptr);
  XChangeGC(display,gc,GCForeground,&gcv);
    
  gcv.foreground=atoms[sj].pixel;
  XChangeGC(display,gc,GCForeground,&gcv);

  /* Draw polygon 2-1-4-3 */

  bond_points[0].x = canvas_bond_points[2].x;
  bond_points[0].y = canvas_bond_points[2].y;

  bond_points[1].x = canvas_bond_points[1].x;
  bond_points[1].y = canvas_bond_points[1].y;

  bond_points[2].x = canvas_bond_points[4].x;
  bond_points[2].y = canvas_bond_points[4].y;

  bond_points[3].x = canvas_bond_points[3].x;
  bond_points[3].y = canvas_bond_points[3].y;

  XFillPolygon(display,
               canvas_pm,
               gc,
               bond_points,
               4,
               Complex,
               CoordModeOrigin);
    
  gcv.foreground=BlackPixelOfScreen(screen_ptr);
  XChangeGC(display,gc,GCForeground,&gcv);
    
  /* Draw the lines 0-2-3-5-0 */

  bond_points[0].x = canvas_bond_points[0].x;
  bond_points[0].y = canvas_bond_points[0].y;

  bond_points[1].x = canvas_bond_points[2].x;
  bond_points[1].y = canvas_bond_points[2].y;

  bond_points[2].x = canvas_bond_points[3].x;
  bond_points[2].y = canvas_bond_points[3].y;

  bond_points[3].x = canvas_bond_points[5].x;
  bond_points[3].y = canvas_bond_points[5].y;

  bond_points[4].x = canvas_bond_points[0].x;
  bond_points[4].y = canvas_bond_points[0].y;

  XDrawLines(display,
             canvas_pm,
             gc,
             bond_points,
             5,
             CoordModeOrigin);

  /* Draw the line 1-4 */

  bond_points[0].x = canvas_bond_points[1].x;
  bond_points[0].y = canvas_bond_points[1].y;

  bond_points[1].x = canvas_bond_points[4].x;
  bond_points[1].y = canvas_bond_points[4].y;

  XDrawLines(display,
             canvas_pm,
             gc,
             bond_points,
             2,
             CoordModeOrigin);
  
}


static int no_dashes = 5;


int get_no_dashes(void)
{
  
  return(no_dashes);

}


void
draw_hbond(int si, int sj)
{
  
  double get_h_bond_width(int, int);
  int get_no_dashes(void);
  void update_canvas_hbond_points(int, int, Boolean);
  int xm_nint(double);
  
  int offset, no_slots, ihbond_length, dash_length;
  
  char dash_list[2];            /* So range is 0-255 */
  
  double x, y, hbond_length;
  
  update_canvas_hbond_points(si, sj, 0);

  /* Get the H-bond length (on the canvas) */

  x = canvas_hbond_points[0].x - canvas_hbond_points[1].x;
  y = canvas_hbond_points[0].y - canvas_hbond_points[1].y;
    
  hbond_length = sqrt ((x * x) + (y * y));
  
  /* Integer value of hbond_length */

  ihbond_length = xm_nint(hbond_length);
  
  /* for no_dashes dashes, there are ((2 * no_dashes) - 1) slots:

    \                           /
     |||   |||   |||   |||   |||
  A  |||   |||   |||   |||   |||  B
     |||   |||   |||   |||   ||| 
    /                           \

  */
  
  no_slots = (2 * get_no_dashes()) - 1;
  
  dash_length = xm_nint(hbond_length/no_slots);

  offset = (2 * dash_length) - 
    ((ihbond_length - (no_slots * dash_length)) / 2);
    
  if(dash_length == 0)
    {
      /* Must be non-zero */
      return;
    }
  else if(dash_length > 255)
    {
      /* stored in type char */
      return;
    }
  
  dash_list[0] = dash_length;
  dash_list[1] = dash_list[0];
    
  /* Set the dashes */

  XSetDashes(display,
             gc,
             offset,
             dash_list,
             2);
  
  /* Change the GC */

  gcv.line_width = (int) get_h_bond_width(si, sj);
  gcv.foreground=BlackPixelOfScreen(screen_ptr);
  gcv.line_style=LineOnOffDash;

  XChangeGC(display,gc,GCForeground | GCLineWidth | GCLineStyle, &gcv);
  
  /* Draw the lines 0-1 */

  XDrawLine(display,
            canvas_pm,
            gc,
            canvas_hbond_points[0].x,
            canvas_hbond_points[0].y,
            canvas_hbond_points[1].x,
            canvas_hbond_points[1].y);
  
}


void
draw_atom_numbers_and_symbols(int si)
{
          
  void convert_to_canvas_coords(double *, double *, Boolean);

  int i;
  
  char *labels = "ABCD", label_string[10], temp_string[10];
          
  double atom_coord[3], label_coord[2];
          
  atom_coord[0] = atoms[si].x;
  atom_coord[1] = atoms[si].y;
  atom_coord[2] = atoms[si].z;

  convert_to_canvas_coords(atom_coord, label_coord, 0);

  /* Add atom number if appropriate */

  strcpy(label_string, "");     /* Initialize */

  if(at_nos_flag == 1)
    {
      sprintf(label_string, "%d ", si + 1);
    }

  /* Add atom symbol if appropriate */

  if(at_sym_flag == 1)
    {
      sprintf(temp_string, "%s ", atoms[si].label);
      strcat(label_string, temp_string);
    }

  /* Add the measure label if appropriate */

  for(i = 0; i < 4; i++)
    {
      if(si == selected[i])
        {
          sprintf(temp_string, "%c", labels[i]);
          strcat(label_string, temp_string);
        }
    }

  XDrawString(display,
              canvas_pm,
              gc,
              (int) label_coord[0],
              (int) label_coord[1],
              label_string,
              strlen(label_string));
}


void
draw_axes(void)
{

  char axis_lab[2];

  int x_pos,y_pos;
    
  gcv.foreground=BlackPixelOfScreen(screen_ptr);
  gcv.line_style=LineSolid;
  gcv.line_width=0;
  
  x_pos = (int)(canvas_width-50);
  y_pos = 50;

  XChangeGC(display,gc,GCForeground|GCLineStyle|GCLineWidth, &gcv);
  
  XDrawLine(display,canvas_pm,gc,
            x_pos,
            y_pos,
            (int)(x_pos+global_matrix[0][0]*30),
            (int)(y_pos-global_matrix[0][1]*30));
  
  strcpy(axis_lab,"X");
  
  XDrawString(display,canvas_pm,gc,
              (int)(x_pos+global_matrix[0][0]*40),  
              (int)(y_pos-global_matrix[0][1]*40),
              axis_lab,
              strlen(axis_lab));
  
  XDrawLine(display,canvas_pm,gc,
            x_pos,
            y_pos,
            (int)(x_pos+global_matrix[1][0]*30),
            (int)(y_pos-global_matrix[1][1]*30));
  
  strcpy(axis_lab,"Y");
  
  XDrawString(display,canvas_pm,gc,
              (int)(x_pos+global_matrix[1][0]*40),
              (int)(y_pos-global_matrix[1][1]*40),
              axis_lab,
              strlen(axis_lab));
  
  XDrawLine(display,canvas_pm,gc,
            x_pos,
            y_pos,
            (int)(x_pos+global_matrix[2][0]*30),
            (int)(y_pos-global_matrix[2][1]*30));
     
  strcpy(axis_lab,"Z");
  
  XDrawString(display,canvas_pm,gc,
              (int)(x_pos+global_matrix[2][0]*40),
              (int)(y_pos-global_matrix[2][1]*40),
              axis_lab,
              strlen(axis_lab));
  
}


void
draw_vector(int si, int j)
{

  void update_canvas_vector_points (int, int);

  update_canvas_vector_points (si, j);

  gcv.line_width = 2;
  gcv.line_style=LineSolid;
  gcv.foreground=BlackPixelOfScreen(screen_ptr);

  XChangeGC(display, gc, GCLineWidth | GCLineStyle | GCForeground, &gcv);

  switch (arrow_type)
    {
    case ARROW_OPEN:

      XDrawLine(display,
                canvas_pm,
                gc,
                canvas_vector_points[0].x,
                canvas_vector_points[0].y,
                canvas_vector_points[1].x,
                canvas_vector_points[1].y);

      XDrawLine(display,
                canvas_pm,
                gc,
                canvas_vector_points[1].x,
                canvas_vector_points[1].y,
                canvas_vector_points[3].x,
                canvas_vector_points[3].y);

      XDrawLine(display,
                canvas_pm,
                gc,
                canvas_vector_points[1].x,
                canvas_vector_points[1].y,
                canvas_vector_points[4].x,
                canvas_vector_points[4].y);

      break;
    case ARROW_CLOSED:

      XDrawLine(display,
                canvas_pm,
                gc,
                canvas_vector_points[0].x,
                canvas_vector_points[0].y,
                canvas_vector_points[2].x,
                canvas_vector_points[2].y);

      XDrawLine(display,
                canvas_pm,
                gc,
                canvas_vector_points[1].x,
                canvas_vector_points[1].y,
                canvas_vector_points[3].x,
                canvas_vector_points[3].y);

      XDrawLine(display,
                canvas_pm,
                gc,
                canvas_vector_points[3].x,
                canvas_vector_points[3].y,
                canvas_vector_points[4].x,
                canvas_vector_points[4].y);

      XDrawLine(display,
                canvas_pm,
                gc,
                canvas_vector_points[4].x,
                canvas_vector_points[4].y,
                canvas_vector_points[1].x,
                canvas_vector_points[1].y);

      break;
    case ARROW_FILLED:

      XDrawLine(display,
                canvas_pm,
                gc,
                canvas_vector_points[0].x,
                canvas_vector_points[0].y,
                canvas_vector_points[2].x,
                canvas_vector_points[2].y);

      {
        XPoint xpoints[3];

        xpoints[0].x = canvas_vector_points[1].x;
        xpoints[0].y = canvas_vector_points[1].y;

        xpoints[1].x = canvas_vector_points[3].x;
        xpoints[1].y = canvas_vector_points[3].y;

        xpoints[2].x = canvas_vector_points[4].x;
        xpoints[2].y = canvas_vector_points[4].y;

        XFillPolygon (display,
                      canvas_pm,
                      gc,
                      xpoints,
                      3,
                      Complex,
                      CoordModeOrigin);
                      
      }
      break;
    }
  
}


void
draw_region(void)
{

  XPoint region_get_start(void);
  XPoint region_get_end(void);

  int x1, y1, x2, y2, temp, width, height;

  XPoint start, end;

  start = region_get_start();
  end   = region_get_end();

  x1 = start.x;
  y1 = start.y;

  x2 = end.x;
  y2 = end.y;

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

  width  = x2 - x1;
  height = y2 - y1;

  gcv.line_width = 2;

  XChangeGC(display,
            gc,
            GCLineWidth,
            &gcv);

  XDrawRectangle(display,
                 XtWindow(canvas),
                  gc,
                 x1, y1,
                 width, height);

}


double
get_atom_radius(int i)
{

  double get_atom_scale(void);
  double get_canvas_scale(void);
  double get_depth_scale(double);
  
  double radius;

  radius = 
    get_atom_scale() *           /* which is interactively scalable */
    get_canvas_scale() *         /* scale factor due to size of canvas */
    get_depth_scale(atoms[i].z); /* scale factor due to depth of atom */

  /* Scale by covalent of van der Waals radius, depending on whether
     the atom is bonded to anything else */

  if ((bond_adjacency_list[i] == NULL) && (never_use_vdw == 0))
    {
      radius *= atoms[i].vdw_rad;
    }
  else
    {
      radius *= atoms[i].cov_rad;
    }

  return (radius);
  
}


/*
  
  What a bond looks like:

       \                                           /
        5--------------------4--------------------3
        |                    |                    |
  A     l                    m                    n     B
        |                    |                    |
        0--------------------1--------------------2
       /                                           \

  A and B are the atom centres. The bond length is the distance
  between A and B. The bond is separated at the midpoint (vertical
  line above). This is defined as follows. If the radii of a and b are
  r_a and r_b respectively, the point m is at a + r_a/(r_a + r_b) * r
  - this is the proportion of the bond that "belongs" to atom a.
  Likewise, the proportion of the bond that "belongs" to atom b is
  given by r_b/(r_a + r_b).

     a = 0 -> A
     b = 0 -> b

     r = b - a

     p = z x r (bond perpendicular, parallel 23 and 05)
     
     l = a + radius(a) * r
     m = a + r_a/(r_a + r_b) * r
     n = b - radius(b) * r

 */

/* Note that for PostScript output, the Y axis is in the opposite
   direction, so that y must be replaced with (canvas_width - y). This
   is covered using the Boolean variable ps_corr */

void
update_canvas_bond_points(int si, int sj, Boolean ps_corr, double offset)
{

  void convert_to_canvas_coords(double *, double *, Boolean);
  double get_atom_scale(void);
  double get_bond_width(void);
  double mod_of_vec(double *);
  void normalize_vec(double *);

  int i;

  double mod_r, a_prop, sb_wid, local_at_scale, half_bond_width;
  double a[3], b[3], r[3], p[3], l[3], m[3], n[3];
  
  a[0] = atoms[si].x;
  a[1] = atoms[si].y;
  a[2] = atoms[si].z;

  b[0] = atoms[sj].x;
  b[1] = atoms[sj].y;
  b[2] = atoms[sj].z;

  for(i = 0; i < 3; i++)
    {
      r[i] = b[i] - a[i];
    }

  /* We need to keep the modulus of r for later */

  mod_r = mod_of_vec(r);

  normalize_vec(r);
      
  /* This satisfies the condition that p is perpendicular to both
     the vector r and the z direction */

  p[0] = -1.0 * r[1];
  p[1] =  1.0 * r[0];
  p[2] =  0.0;
      
  normalize_vec(p);
      
  /* Calculate intermediate quantities */
      
  a_prop = atoms[si].cov_rad/(atoms[si].cov_rad + atoms[sj].cov_rad);

  /* Fudge: switching off atoms is equivalent to having atoms of zero
     radius */
      
  if(atom_flag == 0)
    {
      local_at_scale = 0.0;
    }
  else
    {
      local_at_scale = get_atom_scale(); /* The "canonical" value */
    }
  
  for(i=0; i < 3; i++)
    {
      if (local_at_scale != 0)
        {
          l[i] = a[i] +
            (atoms[si].cov_rad * local_at_scale + offset) * r[i];
        }
      else
        {
          l[i] = a[i] + (offset * r[i]);
        }

      m[i] = a[i] + a_prop * (mod_r - offset) * r[i];
      n[i] = b[i] - (atoms[sj].cov_rad * local_at_scale * r[i]);
    }

  /* Calculate bond coordinates (cartesian) */
      
  /* The maximum bond width is the covalent radius of the smallest atom */

  sb_wid = get_bond_width ();
  
  for(i = 0; i < 3; i++)
    {
      if ((i == 0) || (i == 1))
        {
          half_bond_width = ((sb_wid - (2.0 * offset)) * p[i] / 2.0);
        }
      else
        {
          half_bond_width = (sb_wid * p[i] / 2.0);
        }

      cartesian_bond_points[0][i] = l[i] - half_bond_width;
      cartesian_bond_points[1][i] = m[i] - half_bond_width;
      cartesian_bond_points[2][i] = n[i] - half_bond_width;

      cartesian_bond_points[3][i] = n[i] + half_bond_width;
      cartesian_bond_points[4][i] = m[i] + half_bond_width;
      cartesian_bond_points[5][i] = l[i] + half_bond_width;
      
    }
  
  /* Loop over the bond coordinates and get canvas coordinates */
  
  for(i = 0; i < 6; i++)
    {
      double xy_temp[2];
      
      convert_to_canvas_coords(cartesian_bond_points[i], xy_temp, ps_corr);
          
      canvas_bond_points[i].x = xy_temp[0];
      canvas_bond_points[i].y = xy_temp[1];
          
    }
}


void
update_canvas_hbond_points(int si, int sj, Boolean ps_corr)
{

  double get_atom_scale(void);
  void convert_to_canvas_coords(double *, double *, Boolean);
  double mod_of_vec(double *);
  void normalize_vec(double *);

  int i;

  double mod_r, local_at_scale;
  double a[3], b[3], r[3];
  
  a[0] = atoms[si].x;
  a[1] = atoms[si].y;
  a[2] = atoms[si].z;

  b[0] = atoms[sj].x;
  b[1] = atoms[sj].y;
  b[2] = atoms[sj].z;

  for(i = 0; i < 3; i++)
    {
      r[i] = b[i] - a[i];
    }

  /* We need to keep the modulus of r for later */

  mod_r = mod_of_vec(r);

  normalize_vec(r);
      
  /* Fudge: switching off atoms is equivalent to having atoms of zero
     radius */
      
  if(atom_flag == 0)
    {
      local_at_scale = 0.0;
    }
  else
    {
      local_at_scale = get_atom_scale(); /* The "global" value */
    }
  
  for(i=0; i < 3; i++)
    {
      double radius[2];

      /* If bonded use covalent radius, otherwise van der Waals */

      if ((bond_adjacency_list[si] == NULL) && (never_use_vdw == 0))
        {
          radius[0] = atoms[si].vdw_rad;
        }
      else
        {
          radius[0] = atoms[si].cov_rad;
        }

      if ((bond_adjacency_list[sj] == NULL) && (never_use_vdw == 0))
        {
          radius[1] = atoms[sj].vdw_rad;
        }
      else
        {
          radius[1] = atoms[sj].cov_rad;
        }


      cartesian_hbond_points[0][i] = 
        a[i] + (radius[0] * local_at_scale * r[i]);
      cartesian_hbond_points[1][i] = 
        b[i] - (radius[1] * local_at_scale * r[i]);
    }

  /* Loop over the bond coordinates and get canvas coordinates */
  
  for(i = 0; i < 2; i++)
    {
      double xy_temp[2];
      
      convert_to_canvas_coords(cartesian_hbond_points[i], xy_temp, ps_corr);
          
      canvas_hbond_points[i].x = xy_temp[0];
      canvas_hbond_points[i].y = xy_temp[1];

    }
      
}


double
get_h_bond_width(int si, int sj)
{
    
  double get_atom_radius(int);
  double get_canvas_scale(void);
  double get_depth_scale(double);
  double get_hbond_scale(void);
          
  int hi;                     /* hydrogen index */
    
  double h_bond_width;
    
  /* Determine which atom is hydrogen */

  if(atoms[si].is_hydrogen == 1)
    {
      hi = si;
    }
  else
    {
      hi = sj;
    }
    
  /* This is set to the radius of a hydrogen atom. This is dependent
     on the depth - cf get_atom_radius (we don't want this to be
     dependent on atom_scale */
    
  h_bond_width = atoms[hi].cov_rad *
    get_canvas_scale() *
    get_depth_scale(atoms[hi].z);
    
  /* Scale according the adjustable H-bond factor (0->1) */

  h_bond_width *= get_hbond_scale();
   
  return(h_bond_width);
    
}


double
get_atom_scale(void)
{
  
  return(atom_scale);
  
}

void
set_atom_scale(double new_scale)
{

  atom_scale = new_scale;
  
}


double
get_hbond_scale(void)
{
  
  return(hbond_scale);
  
}

void
set_hbond_scale(double new_scale)
{

  hbond_scale = new_scale;
  
}


double
get_bond_scale(void)
{
  
  return(bond_scale);
  
}

void
set_bond_scale(double new_scale)
{

  bond_scale = new_scale;
  
}

double
get_bond_width (void)
{

  return (bond_scale * smallest_cov_rad);
  
}


void
set_smallest_cov_rad(double new)
{
  
  smallest_cov_rad = new;
  
}

double
get_smallest_cov_rad(void)
{

   return(smallest_cov_rad);

}
