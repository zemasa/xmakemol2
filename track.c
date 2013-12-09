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

#include "defs.h"
#include "globals.h"

/* Function prototypes */

void canvas_cb (Widget, XtPointer, XtPointer);
void echo_to_message_area(char *);
double mod_of_vec (double *);
void rotate_atoms (double *, double, Boolean, Boolean);
void echo_to_message_area (char *);
int select_atom_internal (Boolean);
void store_mouse_coords (void);
void translate_atoms (double *);

/* When mouse_motion = 1, that means (for the purpose of redrawing)
   that the system is either being rotated or translated; this is
   used when View->Outline is selected */

static Boolean mouse_motion = 0;
static Position mouse_coord[2];

#define XY_MOUSE_SENSITIVITY 100;
#define Z_MOUSE_SENSITIVITY 10;

XButtonEvent *bevent;

enum mouse_translations
{
  ONE_DOWN,
  ONE_MOTION,
  ONE_UP,
  TWO_DOWN,
  TWO_MOTION,
  TWO_UP,
  THREE_DOWN,
  THREE_MOTION,
  THREE_UP,
  CTRL_ONE_DOWN,
  CTRL_ONE_MOTION,
  CTRL_ONE_UP,
  CTRL_TWO_DOWN,
  CTRL_TWO_MOTION,
  CTRL_TWO_UP,
  CTRL_THREE_DOWN,
  CTRL_THREE_MOTION,
  CTRL_THREE_UP,
  SHIFT_ONE_DOWN,
  SHIFT_ONE_MOTION,
  SHIFT_ONE_UP,
  SHIFT_TWO_DOWN,
  SHIFT_TWO_MOTION,
  SHIFT_TWO_UP,
  SHIFT_THREE_DOWN,
  SHIFT_THREE_MOTION,
  SHIFT_THREE_UP
} mouse_translation;

void
track_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{
  
  void change_frame(int,Boolean,Boolean);
  void invert_coords(void);
  void reset_orientation(void);
  void reset_position(void);
  void reflect_x(void);
  void reflect_y(void);
  void reflect_z(void);

  int item_no = (int) client_data;
  
  switch(item_no){
  case 0:
    echo_to_message_area("Track rotations about local COM");
    rotate_about = ROTATE_ABOUT_LOCAL_COM;
    break;
  case 1:
    echo_to_message_area("Track rotations about the origin");
    rotate_about = ROTATE_ABOUT_ORIGIN;
    break;
  case 2:
    redraw=1;
    change_frame(frame_no,False,True);
    echo_to_message_area("Centre atoms");
    break;
  case 3:
    redraw=1;
    reset_orientation();
    echo_to_message_area("Resetting orientation");
    break;
  case 4:
    redraw=1;
    reset_position();
    echo_to_message_area("Resetting position");
    break;
  case 5:
    reflect_x();
    atoms_sorted=0;             /* Depths may have changed */
    change_frame(frame_no,False,False);
    echo_to_message_area("x coordinates reflected");
    break;
  case 6:
    reflect_y();
    atoms_sorted=0;             /* Depths may have changed */
    change_frame(frame_no,False,False);
    echo_to_message_area("y coordinates reflected");
    break;
  case 7:
    reflect_z();
    atoms_sorted=0;             /* Depths may have changed */
    change_frame(frame_no,False,False);
    echo_to_message_area("z coordinates reflected");
    break;
  case 8:
    invert_coords();
    atoms_sorted=0;             /* Depths may have changed */
    change_frame(frame_no,False,False);
    echo_to_message_area("Coordinates inverted");
    break;
  }
  
}


void
track(Widget widget, XEvent *event, String *args, int *num_args)
{

  void motion_start (void);
  void motion_stop (void);

  void xy_rotation (void);      /* Rotate about vector in XY plane */
  void z_rotation (void);       /* Rotate about Z */
  void select_atom_for_measure (void);

  void xy_translation (void);   /* Translate in XY plane */
  void z_translation (void);    /* Translate along Z */
  void select_atom_for_edit (void);

  void region_set_start (XButtonEvent *);
  void region_set_end (XButtonEvent *);

  void show_invisible_atoms (void);
  void hide_invisible_atoms (void);
  void select_atom_for_invis (void);
  
  bevent = (XButtonEvent *) event;

  /* aro - don't do translations if file is not loaded yet */
  if(strcmp(current_file_name, "") != 0)
    {
        
      if(*num_args != 1)
        {
          XtError ("Wrong number of args!");
        }

      mouse_translation = (enum mouse_translations) atoi (args[0]);

      switch (mouse_translation)
        {
        case ONE_DOWN:
          motion_start ();
          break;
        case ONE_MOTION:
          xy_rotation ();
          break;
        case ONE_UP:
          motion_stop ();
          break;
        case TWO_DOWN:
          motion_start ();
          break;
        case TWO_MOTION:
          z_rotation ();
          break;
        case TWO_UP:
          motion_stop ();
          break;
        case THREE_DOWN:
          select_atom_for_measure ();
          break;
        case THREE_MOTION:
          break;
        case THREE_UP:
          break;
        case CTRL_ONE_DOWN:
          motion_start ();
          break;
        case CTRL_ONE_MOTION:
          xy_translation ();
          break;
        case CTRL_ONE_UP:
          motion_stop ();
          break;
        case CTRL_TWO_DOWN:
          motion_start ();
          break;
        case CTRL_TWO_MOTION:
          z_translation ();
          break;
        case CTRL_TWO_UP:
          motion_stop ();
          break;
        case CTRL_THREE_DOWN:
          select_atom_for_edit ();
          break;
        case CTRL_THREE_MOTION:
          break;
        case CTRL_THREE_UP:
          break;
        case SHIFT_ONE_DOWN:
          region_set_start (bevent);
          break;
        case SHIFT_ONE_MOTION:
          region_set_end (bevent);
          break;
        case SHIFT_ONE_UP:
          break;
        case SHIFT_TWO_DOWN:
          show_invisible_atoms ();
          break;
        case SHIFT_TWO_MOTION:
          break;
        case SHIFT_TWO_UP:
          hide_invisible_atoms ();
          break;
        case SHIFT_THREE_DOWN:
          select_atom_for_invis ();
          break;
        case SHIFT_THREE_MOTION:
          break;
        case SHIFT_THREE_UP:
          break;
        }
    }
}


void
store_mouse_coords (void)
{
  /* Store the mouse coordinates */

  mouse_coord[0] = bevent->x;
  mouse_coord[1] = bevent->y;
}


void
motion_start (void)
{
  store_mouse_coords ();

  mouse_motion = 1;
  redraw = 1;
  canvas_cb (canvas, NULL, NULL);
}


void
motion_stop (void)
{
  mouse_motion = 0;
  redraw = 1;
  canvas_cb (canvas, NULL, NULL);
}


void
xy_rotation (void)
{
  int i;
  double axis[3], mod, phi;

  /* Determine the axis to rotate about */

  axis[0] = (double) (mouse_coord[1] - bevent->y);
  axis[1] = (double) (mouse_coord[0] - bevent->x);
  axis[2] = 0;

  mod = mod_of_vec (axis);
  if (mod == 0) return;

  for (i = 0; i < 2; i++)       /* axis[2] = 0; ignore */
    {
      axis[i] /= mod;
    }

  /* Determine the amount of rotation; full canvas_width -> 180 degree
     rotation */

  phi = (double) (PI  * mod / canvas_width);

  atoms_sorted = 0;
  redraw = 1;

  rotate_atoms (axis, phi, 1, 1);

  store_mouse_coords ();
}


void
z_rotation (void)
{
  double axis[3], vector[3], mod, phi;

  /* Determine the axis to rotate about */

  vector[0] = (double) (mouse_coord[0] - bevent->x);
  vector[1] = (double) (mouse_coord[1] - bevent->y);
  vector[2] = 0;

  mod = mod_of_vec (vector);
  if (mod == 0) return;

  axis[0] = 0;
  axis[1] = 0;
  axis[2] = (vector[0] > 0) ? 1 : -1; /* Direction depends on X motion */

  /* Determine the amount of rotation; full canvas_width -> 180 degree
     rotation */

  phi = (double) (PI  * mod / canvas_width);

  atoms_sorted = 0;
  redraw = 1;

  rotate_atoms (axis, phi, 1, 1);

  store_mouse_coords ();
}


void
select_atom_for_measure (void)
{
  void update_selected(int);
  void clear_message_area (void);

  char message[64];
  int closest;

  if (no_atoms == 0) return;

  /* Here, 1 means ignore invisible atoms in the selection process */

  closest = select_atom_internal (1);
  update_selected (closest);

  if (atoms[closest].sel == 1)
    {
      sprintf (message, "Atom %d selected: x = %9.4f, y = %9.4f, z = %9.4f",
               closest + 1,
               atoms[closest].x,
               atoms[closest].y,
               atoms[closest].z);

      echo_to_message_area (message);
    }
  else
    {
      clear_message_area ();
    }
}


void xy_translation (void)
{

  int i;
  double vector[3];

  vector[0] = (double) (bevent->x - mouse_coord[0]);
  vector[1] = (double) -(bevent->y - mouse_coord[1]); /* Y is down (X11) */
  vector[2] = 0;

  for (i = 0; i < 2; i++)       /* vector[2] = 0; ignore */
    {
      /* sensitivity should be customizable */
      vector[i] /= XY_MOUSE_SENSITIVITY;
    }

  translate_atoms (vector);

  store_mouse_coords ();

}


void z_translation (void)
{

  double vector[3];

  vector[0] = 0;
  vector[1] = 0;
  vector[2] = (double) (bevent-> x - mouse_coord[0]);

  /* sensitivity should be customizable */
  vector[2] /= Z_MOUSE_SENSITIVITY;

  translate_atoms (vector);

  store_mouse_coords ();

}


void
select_atom_for_edit (void)
{
  int closest;

  if (no_atoms == 0) return;

  if (edit_posn_dialog == NULL)
    {
      echo_to_message_area
        ("Edit selections only allowed when Edit->Positions is open");
      return;
    }
  /* Here, 1 means do ignore invisible atoms in the selection
     process */

  closest = select_atom_internal (1);

  if (closest == -1) return;
  
  if (atoms[closest].edit == 1)
    {
      atoms[closest].edit = 0;
    }
  else
    {
      atoms[closest].edit = 1;
    }
  
  redraw = 1;
  canvas_cb(canvas, NULL, NULL);
      
}


void show_invisible_atoms (void)
{
  view_ghost_atoms = 1;
  redraw = 1;
  canvas_cb (canvas, NULL, NULL);
  view_ghost_atoms = 0;
}


void hide_invisible_atoms (void)
{
  view_ghost_atoms = 0;
  redraw = 1;
  canvas_cb (canvas, NULL, NULL);
}


void
select_atom_for_invis (void)
{
  
  int count_visible_atoms (void);
  void update_selected(int);
  struct frame * get_selected_frame (void);
  void update_bbox (void);

  int closest, no_visible_atoms;

  struct frame *this_frame;

  if (no_atoms == 0) return;

  /* Here, 0 means don't ignore invisible atoms in the selection
     process */

  closest = select_atom_internal (0);

  if (closest == -1) return;

  this_frame = get_selected_frame ();

  if (atoms[closest].visi == 1)
    {
      atoms[closest].visi = 0;
      this_frame->atom[closest].visi = 0;
    }
  else
    {
      atoms[closest].visi = 1;
      this_frame->atom[closest].visi = 1;
    }

  no_visible_atoms = count_visible_atoms ();
  update_bbox ();
  redraw = 1;
  canvas_cb(canvas, NULL, NULL);
      
}

/* Function to determine whether we are currently rotating or
   translating */

Boolean
mouse_motion_p (void)
{
  return (mouse_motion);
}


/* If ignore_invisible is true, we don't care if the atoms are invisible
 or not */

int
select_atom_internal (Boolean ignore_invisible)
{

  void convert_to_canvas_coords(double *, double *, Boolean);
  
  int i, closest, first_value_set = 0;

  double a[3], b[2], dist, dist_keep = 0;
  
  closest = -1;
  
  for(i = 0; i < no_atoms; i++)
    {
      if((atoms[i].visi == True) || (ignore_invisible == False))
        {
          double x, y;

          a[0] = atoms[i].x;
          a[1] = atoms[i].y;
          a[2] = atoms[i].z;

          convert_to_canvas_coords(a, b, 0);

          x = (double) (b[0] - bevent->x);
          y = (double) (b[1] - bevent->y);

          dist = (x * x) + (y * y);

          if(first_value_set == 1)
            {
              if(dist < dist_keep)
                {
                  dist_keep = dist;
                  closest = i;
                }
            }
          else
            {
              dist_keep = dist; /* Set for the first atom */
              closest = i;
              first_value_set = 1;
            }
          
        }
    }
  
  return (closest);
  
}


void
update_selected (int closest)
{

  void update_lengths_dialog(Boolean);

  static int i,j,no_selected;

  Boolean deselected;

  if(sel_init == 0){
    for(i=0;i<4;i++){
      selected[i]=-1; /* set initial values */
    }
    sel_init=1;
  }

  /* If selected atom is invisible, deselect */

  for(i = 0; i < 4; i++)
    {
      if(selected[i] != -1)
        {
          if(atoms[selected[i]].visi == 0)
            {
              selected[i]=-1;
            }
        }
    }
  

  /* find out how many atoms selected */

  no_selected=0;
  for(i=0;i<4;i++){
    if(selected[i]==-1){
      break;
    }
    no_selected++;
  }

  /* if closest is already selected, deselect and pack
     selected */

  deselected=0;

  for(i=0;i<4;i++){
    if(selected[i]==closest){
      /* deselect and pack */
      selected[i]=-1;
      for(j=i;j<3;j++){
        selected[j]=selected[j+1];
      }
      selected[3]=-1;
      deselected=1;
      break;
    }
  }
 
  /* if four atoms already selected, pop the first off the
     top of the stack (unless already deselected one of them) */

  if(!deselected){
    if(no_selected==4){
      for(i=0;i<3;i++){
        selected[i]=selected[i+1];
      }
      selected[3]=closest;
    }else{
      selected[no_selected]=closest;
    }
  }

  for(i=0;i<no_atoms;i++){
    atoms[i].sel=0;
  }
  
  for(i=0;i<4;i++){
    if(selected[i]==-1){
      break;
    }
    atoms[selected[i]].sel=1;
  }

  update_lengths_dialog(True);
  
  /* update visual */

  redraw=1;
  canvas_cb(canvas,NULL,NULL);

}

/* If called with sel_changes_flag == True, then the selections have
   changed and we need to calculate the lengths, angles and torsion
   angle again. Otherwise, we just update the atomic coordinates
*/

void
update_lengths_dialog(Boolean sel_changed_flag)
{

  double get_angle(int, int, int);
  double get_length(int, int);
  double get_torsion(int, int, int, int);

  int i,j,k,w,seli,selj,selk,sell;

  char string[100];
  char *labels = "ABCD";

  double distance,angle,torsion;

  XmString label;

  w=0;

  if(meas_dialog != NULL){
    for(i=0;i<4;i++){
      seli=selected[i];
      if(seli != -1){
        sprintf(string,"%2c  :%9.4f %9.4f %9.4f   %-2s    %d",
                labels[i],atoms[seli].opx,atoms[seli].opy,atoms[seli].opz,
                atoms[seli].label, seli + 1);
      }else{
        sprintf(string,"%2c  :                                       ",
                labels[i]);
      }
      
      label=XmStringCreateLocalized(string);
      XtVaSetValues(meas_label_w[w], XmNlabelString, label, NULL);
      w++;
    }

    if(sel_changed_flag){

      for(i=0;i<3;i++){
        seli=selected[i];
        for(j=i+1;j<4;j++){
          selj=selected[j];
          if((seli != -1) && (selj != -1)){
            distance=get_length(seli,selj);
            
            sprintf(string,"%c-%c  :  %10.4f",labels[i],labels[j],distance);
            label=XmStringCreateLocalized(string);
            
          }else{
            sprintf(string,"%c-%c  :            ",labels[i],labels[j]);
            label=XmStringCreateLocalized(string);
          }
          
          XtVaSetValues(meas_label_w[w], XmNlabelString, label, NULL);
          w++;        
        }
      }
      
      for(i=0;i<2;i++){
        seli=selected[i];
        for(j=i+1;j<3;j++){
          selj=selected[j];
          for(k=j+1;k<4;k++){
            selk=selected[k];
            if((seli != -1) && (selj != -1) && (selk != -1)){
              angle=get_angle(seli,selj,selk);
              
              sprintf(string,"  %c-%c-%c  :  %10.4f",
                      labels[i],labels[j],labels[k],angle);
            }else{
              sprintf(string,"  %c-%c-%c  :            ",
                      labels[i],labels[j],labels[k]);
            }
            
            label=XmStringCreateLocalized(string);
            XtVaSetValues(meas_label_w[w], XmNlabelString, label, NULL);
            w++;
          }   
        }
      }
      
      seli=selected[0];
      selj=selected[1];
      selk=selected[2];
      sell=selected[3];
      
      if((sell != -1)){ /* selected -> torsion available */
        torsion=get_torsion(seli,selj,selk,sell);
        sprintf(string, "A-B-C-D  :  %10.4f",torsion);
      }else{
        sprintf(string, "A-B-C-D  :            ");
      }
      
      label=XmStringCreateLocalized(string);
      XtVaSetValues(meas_label_w[w], XmNlabelString, label, NULL);
    
      
    }
    
    XmStringFree(label);
    
  }
}


void
reset_orientation(void)
{

  void change_frame(int, Boolean, Boolean);

  int i,j;

  for(i=0;i<3;i++){
    for(j=0;j<3;j++){
      if(i==j){
        global_matrix[i][j]=1;
      }else{
        global_matrix[i][j]=0;
      }
    }
  }
  
  /* Atoms will no longer be sorted */

  atoms_sorted=0;

  change_frame(frame_no,False,False);

}


void
reset_position(void)
{

  void change_frame(int, Boolean, Boolean);

  int i;

  for(i=0;i<3;i++){
    global_vector[i]=0;
  }

  change_frame(frame_no,False, False);

}


void
reflect_x (void)
{

  struct frame * get_first_frame ();

  int i;

  struct frame *this_frame;

  this_frame = get_first_frame ();

  while (this_frame != NULL)
    {
      for (i = 0; i < this_frame->no_atoms; i++)
        {
          this_frame->atom[i].x *= -1.0;
        }

      if (this_frame->bbox_available)
        {
          this_frame->bbox[0][0] *= -1.0;
          this_frame->bbox[0][1] *= -1.0;
        }

      this_frame = this_frame->next;
    }
}


void
reflect_y (void)
{

  struct frame * get_first_frame ();

  int i;

  struct frame *this_frame;

  this_frame = get_first_frame ();

  while (this_frame != NULL)
    {
      for (i = 0; i < this_frame->no_atoms; i++)
        {
          this_frame->atom[i].y *= -1.0;
        }

      if (this_frame->bbox_available)
        {
          this_frame->bbox[1][0] *= -1.0;
          this_frame->bbox[1][1] *= -1.0;
        }

      this_frame = this_frame->next;
    }

}


void
reflect_z (void)
{

  struct frame * get_first_frame ();

  int i;

  struct frame *this_frame;

  this_frame = get_first_frame ();

  while (this_frame != NULL)
    {
      for (i = 0; i < this_frame->no_atoms; i++)
        {
          this_frame->atom[i].z *= -1.0;
        }

      if (this_frame->bbox_available)
        {
          this_frame->bbox[2][0] *= -1.0;
          this_frame->bbox[2][1] *= -1.0;
        }

      this_frame = this_frame->next;
    }

}


void
invert_coords (void)
{

  struct frame * get_first_frame ();

  int i;

  struct frame *this_frame;

  this_frame = get_first_frame ();

  while (this_frame != NULL)
    {
      for (i = 0; i < this_frame->no_atoms; i++)
        {
          this_frame->atom[i].x *= -1.0;
          this_frame->atom[i].y *= -1.0;
          this_frame->atom[i].z *= -1.0;
        }

      if (this_frame->bbox_available)
        {
          this_frame->bbox[0][0] *= -1.0;
          this_frame->bbox[0][1] *= -1.0;
          this_frame->bbox[1][0] *= -1.0;
          this_frame->bbox[1][1] *= -1.0;
          this_frame->bbox[2][0] *= -1.0;
          this_frame->bbox[2][1] *= -1.0;
        }

      this_frame = this_frame->next;
    }

}
