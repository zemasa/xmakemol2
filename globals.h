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

#ifndef __GLOBALS_H__
#define __GLOBALS_H__

#define MAX_VECTORS_PER_ATOM (3)

static int file_type = 0;

/* enum types */

enum gl_render_types
{
  DEFAULT,
  BALL_AND_STICK,
  TUBES
};

struct atoms {
  char label[4];
  char uppercase_label[4];
  double x,y,z;      /* real   x,y and z */
  double ox,oy,oz;   /* old    x,y and z */
  double opx,opy,opz; /* coordinates after operations */
  double g[3];   /* in global axis system */
  /* Vector */
  double v[MAX_VECTORS_PER_ATOM][3];
  /* Ellipse */
  double euler[3];
  double shape[3];
  double max_shape;
  double mass;
  double cov_rad;    /* covalent radius */
  double vdw_rad;    /* van der Waals radius */
  char color[20];
  int pixel;
  Boolean edit;     /* is to be edited (ie rotated/translated) ? */
  Boolean sel;       /* is selected ? */
  Boolean visi;      /* is visible ? */
  unsigned short red,green,blue;
  Boolean is_hydrogen; /* is the atom Hydrogen? */
  int type;          /* atom type index */
  int has_vector;
  int is_ellipse;
  enum gl_render_types gl_render_type;
  int molecule;
};

struct elements {
  char label[4];
  double mass;
  double cov_rad;
  double vdw_rad;
  char color[20];
  int pixel; 
  unsigned short red,green,blue;
  Boolean any_atoms;
  Boolean color_allocated;
  Boolean customized; /* aro */
};

typedef struct BOUNDING_BOX bounding_box;
struct BOUNDING_BOX {
  double x_min, x_max;
  double y_min, y_max;
  double z_min, z_max;
  double v[8][3];
};

 
/* Struct frame contains the number of atoms, the comment and pointers
   to struct *atom and the next frame */

struct atom
{
 /* Coordinates */
  double x;
  double y;
  double z;
  double opx,opy,opz;
  /* Vector */
  double vx[MAX_VECTORS_PER_ATOM];
  double vy[MAX_VECTORS_PER_ATOM];
  double vz[MAX_VECTORS_PER_ATOM];
  /* Ellipse */
  double euler[3];
  double shape[3];
  /* Properties */
  char label[4];
  int has_vector;
  int is_ellipse;
  Boolean visi;
  /* Override default colors: either RGB or name */
  Boolean has_rgb;
  double red, green, blue;
  Boolean has_color_name;
  char color_name[32];
  enum gl_render_types gl_render_type;
  int molecule;
};

struct frame
{
  int no_atoms;
  char comment[1024];
  struct atom *atom;
  struct frame *next;
  int perspective_depth;
  int perspective_scale;
  /* This is the bounding box from the input file, if any */
  double bbox[3][2];            /* x, y, z; min, max */
  Boolean bbox_available;
};

enum render_stereo_types
{
  NO_STEREO = 0,
  SIDE_BY_SIDE = 1,
  RED_BLUE = 2,
  SGI_HARDWARE = 3
};

#define ROTATE_ABOUT_ORIGIN     0
#define ROTATE_ABOUT_LOCAL_COM  1
#define MAX_PRINTERS 20 /* aro */

#ifdef __XMAKEMOL_C__

int no_atoms;
int no_atom_types;
int rotate_about = ROTATE_ABOUT_LOCAL_COM;
int *atom_types;

int no_frames = 0;
int frame_no;
int sel_init=0;
int selected[4];

int bg_color_parsed;/* aro */

Arg args[20]; /* should be enough! */

Dimension canvas_width=450;
Dimension canvas_height=450;

char bg_color[30]="lightgray";
char bb_color[30]="darkgray";
char sel_color_name[30]="orange";
XColor sel_color;
char current_file_name[1024] = ""; /* aro */

double depth;
double global_vector[3];
double global_matrix[3][3];
double angle_axis_matrix[3][3];
double bd_scale=1.0;
double bdfd_factor=1.0;
double hbdfd_factor=1.0;
double ibdfd_factor=1.0;
double ihbdfd_factor=1.0;
double largest_cov_rad;

Boolean redraw;
Boolean inhibit_canvas_callback = 0;
Boolean centre_each_frame=0;
Boolean depth_is_on=1;
Boolean never_use_vdw = 0;
Boolean atoms_sorted=0;
Boolean any_hydrogen;
Boolean view_ghost_atoms = False;

Widget main_w,toplevel,label,canvas,message_area;

Widget frames_dialog = NULL;
Widget anim_dialog   = NULL;
Widget meas_dialog   = NULL;
Widget pers_dialog   = NULL;
Widget edit_visi_dialog = NULL;
Widget edit_posn_dialog = NULL;
Widget edit_atbd_sizes_dialog = NULL;
Widget edit_bond_fudges_dialog = NULL;
Widget edit_element_props_dialog = NULL; /* aro */
Widget gl_render_dialog = NULL; /* aro */
Widget scale_coords_dialog = NULL;

Widget *meas_label_w=NULL;
Widget nth_menu[6];

XGCValues gcv;

GC gc;

Pixmap canvas_pm;

Display *display;
Screen  *screen_ptr;
int screen_num;
int screen_depth;
Drawable root_window;

XmString str;

struct atoms *atoms;
struct elements *element;

int *sorted_atoms;

bounding_box bbox, file_bbox, bbox_by_hand;
Boolean bbox_available;

Boolean changing_frame;

#else  /* __XMAKEMOL_C__ */

extern int no_atoms;
extern int no_atom_types;
extern int rotate_about;
extern int *atom_types;

extern int no_frames;
extern int frame_no;
extern int sel_init;
extern int selected[4];

extern int bg_color_parsed; /* aro */

extern Arg args[20];

extern Dimension canvas_width;
extern Dimension canvas_height;

extern char bg_color[30];
extern char bb_color[30];
extern char sel_color_name[30];
extern XColor sel_color;
extern char current_file_name[1024]; /* aro */

extern double depth;
extern double global_vector[3];
extern double global_matrix[3][3];
extern double angle_axis_matrix[3][3];
extern double bd_scale;
extern double bdfd_factor;
extern double hbdfd_factor;
extern double ibdfd_factor;
extern double ihbdfd_factor;
extern double largest_cov_rad;

extern Boolean redraw;
extern Boolean inhibit_canvas_callback;
extern Boolean centre_each_frame;
extern Boolean depth_is_on;
extern Boolean never_use_vdw;
extern Boolean atoms_sorted;
extern Boolean any_hydrogen;
extern Boolean view_ghost_atoms;

extern Widget main_w,toplevel,label,canvas,message_area;

extern Widget frames_dialog;
extern Widget anim_dialog;
extern Widget meas_dialog;
extern Widget pers_dialog;
extern Widget edit_visi_dialog;
extern Widget edit_posn_dialog;
extern Widget edit_atbd_sizes_dialog;
extern Widget edit_bond_fudges_dialog;
extern Widget edit_element_props_dialog;/* aro */
extern Widget gl_render_dialog; /* aro */
extern Widget scale_coords_dialog;

extern Widget *meas_label_w;
extern Widget nth_menu[6];

extern XGCValues gcv;

extern GC gc;

extern Pixmap canvas_pm;

extern Display *display;
extern Screen  *screen_ptr;
extern int screen_num;
extern int screen_depth;
extern Drawable root_window;

extern struct atoms *atoms;
extern struct elements *element;

extern int *sorted_atoms;

extern bounding_box bbox, file_bbox, bbox_by_hand;
extern Boolean bbox_available;

extern Boolean changing_frame;

#endif /* __XMAKEMOL_C__ */


#endif /* __GLOBALS_H__ */
