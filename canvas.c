/*

Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2005 Matthew P. Hodges
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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <Xm/Xm.h>

#include "bonds.h"
#include "defs.h"
#include "draw.h"
#ifdef GL
#include "gl_funcs.h" /* aro */
#endif /* GL */
#include "globals.h"
#include "view.h"

#ifdef XPM
#include <X11/xpm.h>
#include "xm_logo.h"
#endif

static XColor fg_color;

#ifdef XPM
static int show_logo = 1;
static int xm_logo_read = 0;
static Pixmap xm_logo_pix;
#endif

XColor
get_bg_color (void)
{
  return (fg_color);
}

#ifdef XPM
void
disable_show_logo(void)
{
  show_logo = 0;
}
#endif


void
canvas_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{
  void draw_region (void);
#ifdef XPM
  void draw_splash_screen (void);
#endif
  Boolean region_active_p (void);
  void x_render (void);
#ifdef GL
  Boolean render_using_gl_p (void);
  void set_render_using_gl (Boolean);
  void gl_render (void);
#endif /* GL */
 
  GC gc;
  
  if ((inhibit_canvas_callback) || (! XtIsRealized(canvas)))
    {
      /* Do nothing if inhibit_canvas_callback is true or canvas isn't
         realized */
      return;
    }

  XtVaGetValues (canvas, XmNuserData, &gc, NULL);  

  if (! canvas_pm)
    {
      canvas_pm = XCreatePixmap
        (display,
         XtWindow (canvas),
         canvas_width,
         canvas_height,
#ifdef GL
         visinfo->depth
#else  /* GL */
         screen_depth
#endif
         );
      redraw = 1;
    }

  if (redraw)
    {
#ifdef GL
      if (render_using_gl_p ())
        {
          gl_render ();
        }
      else
        {
          x_render ();
        }
#else  /* GL */
      x_render ();
#endif /* GL */
      redraw = 0;
  }

  if (no_atoms > 0)
    {
#ifdef GL
      /* Under GL, we render directly onto XtWindow (canvas);
         therefore do nothing if rendering with GL */
      if (! render_using_gl_p ())
        {
          /* Copy from pixmap -> window  */
          XCopyArea (display,
                     canvas_pm,
                     XtWindow (canvas),
                     gc,
                     0,
                     0,
                     canvas_width,
                     canvas_height,
                     0,
                     0);
        }
#else  /* GL */
      /* Copy from pixmap -> window  */
      XCopyArea (display,
                 canvas_pm,
                 XtWindow (canvas),
                 gc,
                 0,
                 0,
                 canvas_width,
                 canvas_height,
                 0,
                 0);
#endif /* GL */

      /* Do any drawing which doesn't require atom/bond information */

      if (region_active_p() == 1)
        {
          draw_region ();
        }
    }
  else /* no_atoms == 0 */
    {
#ifdef XPM
      draw_splash_screen ();
#else
      XFillRectangle (display,
                      XtWindow (canvas),
                      gc,
                      0,
                      0,
                      canvas_width,
                      canvas_height);
#endif
    }
}

void x_render (void)
{

  void draw_atom(int);
  void draw_atom_numbers_and_symbols(int);
  void draw_axes(void);
  void draw_bbox(Boolean);
  void draw_bbox_faces(void);
  void draw_bbox_face(int, int, int, int);
  void draw_bond(int, int);
  void draw_hbond(int, int);
  void draw_vector(int, int);
  Boolean mouse_motion_p (void);
  Boolean outline_mode_p (void);
  double get_vector_scale(void);
  double get_z_depth(void);
  void sort_atoms(void);
  void bg_color_init(void);/* aro */

  /* aro - color_parsed moved to bg_color_parsed in globals.h so the
     background color can be changed in the program */

  int i,j,si,sj;

  double local_z_depth, vector_scale;

  struct node *ptr;

  if((! mouse_motion_p ()) || (! outline_mode_p ())){
    sort_atoms();
  }

  /* aro - moved some code to bg_color_init function so we
     can initialize the background color in gl if we are
     disabling x-rendering */
  if(strcmp(bg_color,"")!=0){
    if(bg_color_parsed == 0){
      bg_color_init();
      bg_color_parsed=1;
    }
    XSetForeground(display,gc,fg_color.pixel);
  }else{
    XSetForeground(display,gc,WhitePixelOfScreen(screen_ptr));
  }
  
  
  XFillRectangle(display,canvas_pm,gc,0,0,canvas_width,canvas_height);

  if(bbox_flag && (no_atoms > 0)){
    if((! mouse_motion_p ()) || (! outline_mode_p ())){
      draw_bbox_faces();
    }
    draw_bbox(0); /* draw stuff 'below' */
  }

  local_z_depth = get_z_depth();
  vector_scale = get_vector_scale();
    
  for(i=0;i<no_atoms;i++){

    si=sorted_atoms[i];

    if((vector_flag == 1) && (atoms[si].has_vector > 0))
      {
          
        /* Draw the vector if it points below the atom and is
           drawable */

        for (j = 0; j < MAX_VECTORS_PER_ATOM; j++)
          {
            if((atoms[si].v[j][2] < 0) &&
               (local_z_depth - (atoms[si].z + (vector_scale * atoms[si].v[j][2])))
               > Z_TOL)
              {
                draw_vector(si, j);
              }
          }
      }
      
    /* Draw the atom if infront of 'eye' ... */      
      
    if(((local_z_depth - atoms[si].z) > Z_TOL) || (depth_is_on == 0))
      {

        if(atom_flag == 1)
          {
            /* Draw if visible _or_ if view_ghost_atoms is true */

            if((atoms[si].visi == 1) || (view_ghost_atoms == 1) )
              {
                draw_atom(si);
              }
          }
        else
          {
            /* Draw if invisible _and_ view_ghost_atoms is true */
              
            if((atoms[si].visi == 0) && (view_ghost_atoms == 1) )
              {
                draw_atom(si);
              }
          }
      }
      
    if((vector_flag == 1) && (atoms[si].has_vector > 0))
      {
          
        /* Draw the vector if it points above the atom and is
           drawable */
         
        for (j = 0; j < MAX_VECTORS_PER_ATOM; j++)
          {
            if((atoms[si].v[j][2] > 0) &&
               (local_z_depth - (atoms[si].z + (vector_scale * atoms[si].v[j][2])))
               > Z_TOL)
              {
                draw_vector(si, j);
              }
          }
      }
     
    if((! mouse_motion_p ()) || (! outline_mode_p ())){

      if(bond_flag == 1)
        {
            
          /* Draw the bonds */

          ptr = bond_adjacency_list[si];
            
          while(ptr != NULL)
            {
            
              sj = (ptr->v);
                
              if((depth_is_on == 0) ||
                 (((local_z_depth - atoms[si].z) > Z_TOL) &&
                  ((local_z_depth - atoms[sj].z) > Z_TOL)))
                {
                    
                  if(atoms[si].visi && atoms[sj].visi)
                    {
                        
                      if(atoms[si].z<=atoms[sj].z)
                        {
                          draw_bond(si,sj);
                        }
                    }
                }
                
              ptr = ptr->next;
            
            }
        }

      /* Draw the H-bonds ... */
        
      if((any_hydrogen == 1) && (hbond_flag == 1))
        {
          /* ... if there are any */

          ptr = hbond_adjacency_list[si];
            
          while(ptr != NULL)
            {
              sj = (ptr->v);
                
              if((depth_is_on == 0) ||
                 (((local_z_depth - atoms[si].z) > Z_TOL) &&
                  ((local_z_depth - atoms[sj].z) > Z_TOL)))
                {
                    
                  if(atoms[si].visi && atoms[sj].visi)
                    {

                      if(atoms[si].z<=atoms[sj].z)
                        {
                          draw_hbond(si,sj);
                        }
                    }
                }

              ptr = ptr->next;

            }
        }
        
      gcv.foreground=BlackPixelOfScreen(XtScreen(canvas));
      XChangeGC(display,gc,GCForeground, &gcv);
        
      if(atoms[si].visi == 1)
        {
          draw_atom_numbers_and_symbols(si);
        }
        
    }

  }
    
  if(axes_flag){
    draw_axes();
  }

  if(bbox_flag && (no_atoms > 0)){
    draw_bbox(1); /* draw stuff 'above' */
  }

}

/* The scaling of the coordinates on the canvas depends on two
   factors:

   (1) canvas_scale - this is determined by the size of the system on
       reading in the file. Afterwards, this can be modified by
       changing the size of the canvas or via the perspective dialog
       (Choose scale). This is a transient (!) constant, ie, is the
       same for all coordinates (atoms/bond points) in a frame.
         
   (2) depth_scale - this is determined by the distance of the eye
       from the origin (Z_d). This is also set each time a file is
       read in. Unlike canvas_scale, this is different for each value
       of z and is given by Z_d/(Z_d - z).
       
*/

static double canvas_scale, depth_scale; /* See above */
static double z_depth;                   /* Referred to as Z_d above */


void
set_canvas_scale(double input)
{
  
  canvas_scale = input;
  
}


double
get_canvas_scale(void)
{
  
  return(canvas_scale);

}


void
set_depth_scale(double input)
{
  
  depth_scale = input;
  
}


double
get_depth_scale(double z_val)
{

  if(depth_is_on == 1)
    {
      depth_scale = z_depth/(z_depth - z_val);
    }
  else
    {
      depth_scale = 1.0;
    }
  
  return (depth_scale);
  
}


void update_depth_scale(double z_val)
{

  if(depth_is_on == 1)
    {
      depth_scale = z_depth/(z_depth - z_val);
    }
  else
    {
      depth_scale = 1.0;
    }
    
}


void
set_z_depth(double input)
{

  z_depth = input;
  
}


double
get_z_depth(void)
{

  return(z_depth);
  
}

/*

  The canvas coordinates (X,Y) start at the top left of the window.
  For The plotted coordinates (x,y) we want the origin to be at the
  center. In addition, Y is _antiparallel_ to y.

      ^  +---------------+---------------+-----> X
      |  |               |               |
      |  |               |               |
      |  |               |               |
      c  |               |               |
      a  |             y ^               |
      n  |               |               |
      v  |               |               |
      a  |               |               |
      s  +---------------+------>--------+
         |               |      x        |
      h  |               |               |
      e  |               |               |
      i  |               |               |
      g  |               |               |
      h  |               |               |
      t  |               |               |
      |  |               |               |
      v  +---------------+ --------------+
         |
         |<--------canvas_width---------->
         |
         v

         Y

 */

/* This takes as input a pointer to a vector xyz_in[3] representing a
   point in real space and returns a pointer to a vector xy_out[2]
   representing a point in canvas space */

void
convert_to_canvas_coords(double *xyz_in, double *xy_out, Boolean ps_corr)
{
  
  void update_depth_scale(double);
    
  double overall_scale_factor;
  
  /* This sets depth_scale; depends on z */

  update_depth_scale(xyz_in[2]);
      
  overall_scale_factor = depth_scale * canvas_scale;
  
  xy_out[0] = (canvas_width / 2.0) + (overall_scale_factor * xyz_in[0]);

  if(ps_corr == 0)
    {
      xy_out[1] = (canvas_height / 2.0) - (overall_scale_factor * xyz_in[1]);
    }
  else
    {
      xy_out[1] = (canvas_height / 2.0) + (overall_scale_factor * xyz_in[1]);
    }
  
}

/* aro--> */
void bg_color_init(void)
{
  /* aro - use colormap specific to canvas and visual, default
     colormap may give inaccurate colors on some displays */
#ifdef GL
  Colormap cmap = XCreateColormap(display, XtWindow(canvas), 
                                  visinfo->visual, AllocNone);
#else  /* GL */
  Colormap cmap = DefaultColormap(display,screen_num);
#endif /* GL */

  XParseColor(display,cmap,bg_color,&fg_color);
  if(!XAllocColor(display,cmap,&fg_color)){
    printf("Could not allocate colour %20s (canvas_cb)\n",
           bg_color);
  }

  XParseColor (display, cmap, sel_color_name, &sel_color);
  if (! XAllocColor (display,cmap,&sel_color))
    {
      printf("Could not allocate colour %20s (bg_color_init)\n",
             sel_color_name);
    }

#ifdef GL
  XFreeColormap(display, cmap);
#endif /* GL */
}
/* <--aro */

#ifdef XPM
void draw_splash_screen (void)
{
  XpmAttributes xpm_attrib;
#ifdef GL
  Colormap cmap = XCreateColormap (display, XtWindow (canvas), 
                                   visinfo->visual, AllocNone);
#else
  Colormap cmap = DefaultColormap (display, screen_num);
#endif

  xpm_attrib.valuemask = XpmDepth | XpmVisual | XpmColormap;

#ifdef GL
  xpm_attrib.depth = visinfo->depth;
  xpm_attrib.visual = visinfo->visual;
#else
  xpm_attrib.depth = screen_depth;
  xpm_attrib.visual = DefaultVisual (display, screen_num); /* Correct? */
#endif
  xpm_attrib.colormap = cmap;

  if (xm_logo_read == 0)
    {
      xm_logo_read = 1;
      XpmCreatePixmapFromData (display,
                               XtWindow (canvas),
                               xm_logo,
                               &xm_logo_pix,
                               NULL,
                               &xpm_attrib);

    if(! xm_logo_pix)
      {
        /* We'd better not try and display it */
        show_logo = 0;
      }
    }

  if (show_logo)
    {
      XCopyArea (display,
                 xm_logo_pix,
                 XtWindow (canvas),
                 gc,
                 0,
                 0,
                 canvas_width,
                 canvas_height,
                 0,
                 0);
    }
}
#endif /* XPM */
