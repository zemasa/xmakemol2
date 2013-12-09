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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/MessageB.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/Scale.h>
#include <Xm/Separator.h>
#include <Xm/SeparatoG.h>
#include <Xm/ScrolledW.h>
#include <Xm/TextF.h>
#include <Xm/ToggleB.h> /* aro - added */
#include <Xm/ToggleBG.h>

#include "globals.h"
#include "bbox.h" /* included by wjq */
#include "defs.h"
#include "draw.h"
#include "vectors.h"
#include "view.h"

#define WHOLE_CANVAS   1
#define INSIDE_REGION  2
#define OUTSIDE_REGION 3

void echo_to_message_area(char *);
void place_dialog_cb (Widget, XtPointer, XtPointer);

/* These allow us to keep track of the edit_visi_dialog */

static int visi_act_on = 1;
static int posn_act_on = 1;

static Boolean edit_visi_dialog_pos_are_avail;
static Position edit_visi_dialog_xpos, edit_visi_dialog_ypos;

/* These allow us to keep track of the edit_posn_dialog */

static Boolean edit_posn_dialog_pos_are_avail;
static Position edit_posn_dialog_xpos, edit_posn_dialog_ypos;

static Widget bdfd_scale_w, hbdfd_scale_w, ibdfd_scale_w, ihbdfd_scale_w;
static Widget at_scale_w,bd_scale_w,hbd_scale_w,never_use_vdw_w;

static Widget vector_display_dialog = NULL;

static int last_value=0;
static double phi,axis[3];

struct atom_group {
  char label[4];
};

struct atom_group *atom_group;

static Widget x_rot,y_rot,z_rot;
Widget x_trans,y_trans,z_trans;

/* aro--> */

/* Widgets for use in edit_posn_dialog */
Widget rot_text_label, trans_text_label;
Widget x_rot_text,y_rot_text,z_rot_text, x_trans_text,y_trans_text,z_trans_text;
Widget x_rot_button,y_rot_button,z_rot_button,x_trans_button,y_trans_button,z_trans_button;

/* Structs used to create and keep track of linked list
   of widgets that have been changed in edit_element_props_dialog
   (i.e. text of a TextField changed by user) */
struct widgets
{
  Widget id;
  struct widgets *next;
}; 
struct widget_list
{
  struct widgets *curr;
  struct widgets *head;
} widgets_changed;

/* Struct for keeping track of the state of edit_element_props_dialog */
struct dialog_state
{
  Boolean show_all_elements;
  Boolean show_customized_elements;
  Boolean file_loaded;
} elem_props_dialog_state = {False, False, False};
/* <--aro */

Widget *edit_visi_type_btn=NULL;
Widget *edit_posn_type_btn=NULL;

void
edit_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{

  void make_edit_atbd_sizes_dialog(Widget);
  void make_edit_bond_fudges_dialog(Widget);
  void make_edit_bond_sizes_dialog(Widget);
  void make_edit_element_props_dialog(Widget);
  void make_edit_posn_dialog(Widget);
  void make_scale_coords_dialog (Widget);
  void make_edit_visi_dialog(Widget);
  void make_vector_display_dialog(Widget);
  void make_bbox_dialog(Widget);
#ifdef GL
  void make_gl_render_dialog (Widget);
#endif /* GL */

  int item_no = (int) client_data;

  switch(item_no){
  case 0:
    make_edit_visi_dialog(main_w);
    break;
  case 1:
    make_edit_posn_dialog(main_w);
    break;
  case 2:
    make_scale_coords_dialog (main_w);
    break;
  case 3:
    make_edit_atbd_sizes_dialog(main_w);
    break;
  case 4:
    make_edit_bond_fudges_dialog(main_w);
    break;
  case 5:
    make_vector_display_dialog(main_w);
    break;
  case 6:
    make_bbox_dialog(main_w);
    break;
  case 7:
    make_edit_element_props_dialog(main_w);
    break;
#ifdef GL
  case 8:
    make_gl_render_dialog (main_w);
#endif /* GL */
  }

}

void
make_edit_visi_dialog(Widget parent)
{

  void edit_visi_cancel_cb(Widget, XtPointer, XtPointer);
  void get_edit_visi_dialog_position(void);
  int get_no_elements (void);
  void visi_act_on_cb(Widget, XtPointer, XtPointer);
  void visi_atoms_type_cb(Widget, XtPointer, XtPointer);
  void visi_invert_cb(Widget, XtPointer, XtPointer);
  void visi_reset_cb(Widget, XtPointer, XtPointer);
  void visi_propagate_cb (Widget, XtPointer, XtPointer);

  int i,n,count;

  char istring[16];

  XmString title;

  Widget rc, region, button, invert, reset, propagate;
  
  if(!edit_visi_dialog){

    title = XmStringCreateLocalized("Edit visible atoms");
    
    n=0;
    XtSetArg (args[n],XmNautoUnmanage, False); n++;
    XtSetArg (args[n],XmNdialogTitle,title); n++;

    edit_visi_dialog=
      XmCreateMessageDialog(parent,"edit",args,n);

    rc = XtVaCreateManagedWidget
      ("rc",xmRowColumnWidgetClass,edit_visi_dialog,NULL);
    
    XtVaCreateManagedWidget
      ("Actions act on:",
       xmLabelWidgetClass,rc,
       XmNalignment, XmALIGNMENT_BEGINNING,
       NULL);

    region = XmCreateRadioBox(rc,
                              "toggle",
                              NULL,
                              0);

    button = XtVaCreateManagedWidget("all atoms",
                                     xmToggleButtonGadgetClass,
                                     region,
                                     NULL);

    if(visi_act_on == WHOLE_CANVAS)
      {
        XtVaSetValues(button, XmNset, True, NULL);
      }

    XtAddCallback(button,
                  XmNvalueChangedCallback,
                  visi_act_on_cb,
                  (XtPointer) WHOLE_CANVAS);

    button = XtVaCreateManagedWidget("atoms inside region",
                                     xmToggleButtonGadgetClass,
                                     region,
                                     NULL);

    if(visi_act_on == INSIDE_REGION)
      {
        XtVaSetValues(button, XmNset, True, NULL);
      }

    XtAddCallback(button,
                  XmNvalueChangedCallback,
                  visi_act_on_cb,
                  (XtPointer) INSIDE_REGION);

    button = XtVaCreateManagedWidget("atoms outside region",
                                     xmToggleButtonGadgetClass,
                                     region,
                                     NULL);

    if(visi_act_on == OUTSIDE_REGION)
      {
        XtVaSetValues(button, XmNset, True, NULL);
      }

    XtAddCallback(button,
                  XmNvalueChangedCallback,
                  visi_act_on_cb,
                  (XtPointer) OUTSIDE_REGION);

    XtManageChild(region);
    
    if(edit_visi_type_btn)
      {
        free(edit_visi_type_btn);
      }

    edit_visi_type_btn=malloc(no_atom_types*sizeof(Widget));

    if(atom_group)
      {
        free(atom_group);
      }
    
    atom_group=malloc(no_atom_types*sizeof(struct atom_group));
    
    count=0;

    for (i = 0; i < get_no_elements (); i++)
      {
        if (element[i].any_atoms)
          {
            sprintf(istring,"Toggle %s atoms",element[i].label);
            edit_visi_type_btn[count]=XtVaCreateManagedWidget
              (istring,xmPushButtonWidgetClass,rc,NULL);
            XtAddCallback(edit_visi_type_btn[count],XmNactivateCallback,
                          visi_atoms_type_cb,(XtPointer)count);

            strcpy(atom_group[count].label,element[i].label);

            count++;
          }
      }

    invert=XtVaCreateManagedWidget
      ("Invert selection",xmPushButtonWidgetClass,rc,NULL);

    XtAddCallback(invert,XmNactivateCallback,visi_invert_cb,NULL);

    reset=XtVaCreateManagedWidget
      ("Reselect all",xmPushButtonWidgetClass,rc,NULL);

    XtAddCallback(reset,XmNactivateCallback,visi_reset_cb,NULL);

    /* Determine whether visibility toggles act on a single frame or
       all frames */

    XtManageChild (XmCreateSeparator (rc, "sep", NULL, 0));

    propagate = XtVaCreateManagedWidget
      ("Propagate visibilities to all frames", xmPushButtonWidgetClass,
       rc, NULL);

    XtAddCallback (propagate, XmNactivateCallback, visi_propagate_cb, NULL);

    /* Deal with Cancel */

    XtAddCallback(edit_visi_dialog,XmNcancelCallback,edit_visi_cancel_cb,NULL);

    XtUnmanageChild
      (XmMessageBoxGetChild(edit_visi_dialog,XmDIALOG_OK_BUTTON));
    
    /* No help available ... */

    XtUnmanageChild
      (XmMessageBoxGetChild(edit_visi_dialog,XmDIALOG_HELP_BUTTON));
    
  }

  XtAddCallback (edit_visi_dialog, XmNmapCallback, place_dialog_cb, NULL);

  XtManageChild(edit_visi_dialog);

  /* set the position if it is available - else just get the position */

  if(edit_visi_dialog_pos_are_avail == 1)
    {
      XtVaSetValues(edit_visi_dialog,
                    XmNx, edit_visi_dialog_xpos,
                    XmNy, edit_visi_dialog_ypos,
                    NULL);
    }
  else
    {
      get_edit_visi_dialog_position();
    }

}


void
visi_atoms_type_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{
  
  void canvas_cb(Widget, XtPointer, XtPointer);
  int count_visible_atoms (void);
  int * get_atoms_in_region(void);
  Boolean region_active_p(void);
  void update_atoms_in_region(Boolean);
  void update_bbox (void);
  struct frame * get_selected_frame (void);
  
  int i, at_type, no_visible_atoms;
  int *atoms_in_region = NULL;

  struct frame *this_frame;

  at_type = (int) client_data;

  if((visi_act_on == INSIDE_REGION) ||
     (visi_act_on == OUTSIDE_REGION))
    {
      if(region_active_p() == 1)
        {
          update_atoms_in_region(1);
          atoms_in_region = get_atoms_in_region();
        }
      else
        {
          echo_to_message_area("Region is not active");
          return;
        }
    }
  
  this_frame = get_selected_frame ();

  for(i = 0; i < no_atoms; i++)
    {
      if(strcmp(atom_group[at_type].label, atoms[i].uppercase_label) == 0)
        {
          
          /* Act on an atom if:

             (1) The whole canvas is active
             (2) Inside the region is active and the atom is inside
             (3) Outside the region is active and the atom is outside */
          
          if((visi_act_on == WHOLE_CANVAS) ||
             ((visi_act_on == INSIDE_REGION)  && (atoms_in_region[i] == 1)) ||
             ((visi_act_on == OUTSIDE_REGION) && (atoms_in_region[i] == 0)))
            {
              if(atoms[i].visi == 0)
                {
                  atoms[i].visi = 1;

                  /* Could be a crystal */

                  if (i < this_frame->no_atoms)
                    {
                      this_frame->atom[i].visi = 1;
                    }
                }
              else
                {
                  atoms[i].visi = 0;

                  /* Could be a crystal */

                  if (i < this_frame->no_atoms)
                    {
                      this_frame->atom[i].visi = 0;
                    }
                }
            }
        }
    }

  update_bbox ();
  no_visible_atoms = count_visible_atoms ();
  redraw=1;
  canvas_cb(canvas,NULL,NULL);

}


void
visi_invert_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{
  
  void canvas_cb(Widget, XtPointer, XtPointer);
  int count_visible_atoms(void);
  int * get_atoms_in_region(void);
  Boolean region_active_p(void);
  void update_atoms_in_region(Boolean);
  void update_bbox (void);
  struct frame * get_selected_frame (void);

  int i, no_visible_atoms;
  int *atoms_in_region = NULL;

  struct frame *this_frame;


  if((visi_act_on == INSIDE_REGION) ||
     (visi_act_on == OUTSIDE_REGION))
    {
      if(region_active_p() == 1)
        {
          update_atoms_in_region(1);
          atoms_in_region = get_atoms_in_region();
        }
      else
        {
          echo_to_message_area("Region is not active");
          return;
        }
    }

  this_frame = get_selected_frame ();

  for(i = 0; i < no_atoms; i++)
    {
      if((visi_act_on == WHOLE_CANVAS) ||
         ((visi_act_on == INSIDE_REGION)  && (atoms_in_region[i] == 1)) ||
         ((visi_act_on == OUTSIDE_REGION) && (atoms_in_region[i] == 0)))
        {
          if(atoms[i].visi == 0)
            {
              atoms[i].visi = 1;
              /* Could be a crystal */
              if (i < this_frame->no_atoms)
                {
                  this_frame->atom[i].visi = 1;
                }
            }
          else
            {
              atoms[i].visi = 0;
              /* Could be a crystal */
              if (i < this_frame->no_atoms)
                {
                  this_frame->atom[i].visi = 0;
                }
            }
        }
    }

  no_visible_atoms = count_visible_atoms();
  update_bbox ();
  redraw=1;
  canvas_cb(canvas,NULL,NULL);

}


void
visi_reset_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{
  
  void canvas_cb(Widget, XtPointer, XtPointer);
  int count_visible_atoms (void);
  int * get_atoms_in_region(void);
  Boolean region_active_p(void);
  void update_atoms_in_region(Boolean);
  void update_bbox (void);
  struct frame * get_selected_frame (void);
  
  int i, no_visible_atoms;
  int *atoms_in_region = NULL;

  struct frame *this_frame;

  if((visi_act_on == INSIDE_REGION) ||
     (visi_act_on == OUTSIDE_REGION))
    {
      if(region_active_p() == 1)
        {
          update_atoms_in_region(1);
          atoms_in_region = get_atoms_in_region();
        }
      else
        {
          echo_to_message_area("Region is not active");
          return;
        }
    }

  this_frame = get_selected_frame ();

  for(i = 0; i < no_atoms; i++)
    {
      if((visi_act_on == WHOLE_CANVAS) ||
             ((visi_act_on == INSIDE_REGION)  && (atoms_in_region[i] == 1)) ||
             ((visi_act_on == OUTSIDE_REGION) && (atoms_in_region[i] == 0)))
            {
              atoms[i].visi = 1;
              /* Could be a crystal */
              if (i < this_frame->no_atoms)
                {
                  this_frame->atom[i].visi = 1;
                }
            }
    }

  no_visible_atoms = count_visible_atoms();
  update_bbox ();
  redraw=1;
  canvas_cb(canvas,NULL,NULL);

}


void
visi_propagate_cb (Widget widget,
                   XtPointer client_data,
                   XtPointer call_data)
{

  void echo_to_message_area (char *);
  int get_all_frames_same_size (void);
  struct frame * get_first_frame (void);

  int i;
  char string[128];

  struct frame *this_frame;

  if (no_frames == 1)
    {
      sprintf (string, "No other frames to propagate visibilities to");
    }
  else
    {
      if (get_all_frames_same_size () == 1)
        {
          this_frame = get_first_frame ();

          while (this_frame != NULL)
            {
              for (i = 0; i < this_frame->no_atoms; i++)
                {
                  this_frame->atom[i].visi = atoms[i].visi;
                }
              this_frame = this_frame->next;
            }
          sprintf (string, "Visibilites have been propagated to all frames");
        }
      else
        {
          sprintf (string,
                   "Action not permitted: variable number of atoms per frame");
        }
    }

  echo_to_message_area (string);

}


int
count_visible_atoms (void)
{

  void update_bbox (void);
  void update_selected (int);

  int i, no_visible_atoms;

  char string[128];
  
  no_visible_atoms = 0;

  for (i = 0; i < no_atoms; i++)
    {
      if (atoms[i].visi == 1)
        {
          no_visible_atoms++;
        }
    }

  sprintf (string, "There are %d visible atoms", no_visible_atoms);
  echo_to_message_area (string);

  return (no_visible_atoms);

}


void
edit_visi_cancel_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{

  void get_edit_visi_dialog_position(void);
  
  /* Get the position of the widget before killing it */

  get_edit_visi_dialog_position();
  
  XtDestroyWidget(edit_visi_dialog);
  edit_visi_dialog=NULL;

  free(edit_visi_type_btn);
  edit_visi_type_btn = NULL;
  
}


void
get_edit_visi_dialog_position(void)
{

  /* Get the position of the widget */
  
  XtVaGetValues(edit_visi_dialog,
                XmNx, &edit_visi_dialog_xpos,
                XmNy, &edit_visi_dialog_ypos,
                NULL);

  edit_visi_dialog_pos_are_avail = 1;
  
}


void
make_edit_posn_dialog(Widget parent)
{

  void edit_posn_discard_cb(Widget, XtPointer, XtPointer);
  void edit_posn_cancel_cb(Widget, XtPointer, XtPointer);
  void edit_posn_make_perm_cb(Widget, XtPointer, XtPointer);
  void edit_posn_invert_cb(Widget, XtPointer, XtPointer);
  void edit_posn_reset_cb(Widget, XtPointer, XtPointer);
  void get_edit_posn_dialog_position(void);
  int get_no_elements (void);
  void posn_act_on_cb(Widget, XtPointer, XtPointer);
  void posn_atoms_type_cb(Widget, XtPointer, XtPointer);
  void rot_store_geom(Widget, XtPointer, XtPointer);
  void toggle_global_cb(Widget, XtPointer, XtPointer);
  void trans_store_geom_cb(Widget, XtPointer, XtPointer);
  void x_rot_cb(Widget, XtPointer, XtPointer);
  void x_trans_cb(Widget, XtPointer, XtPointer);
   void y_rot_cb(Widget, XtPointer, XtPointer);
  void y_trans_cb(Widget, XtPointer, XtPointer);
  void z_rot_cb(Widget, XtPointer, XtPointer);
  void z_trans_cb(Widget, XtPointer, XtPointer);

/* aro--> */
  /* Callback functions for PushButtons and TextFields
     added for use in rotation and translation*/
  void x_rot_button_cb(Widget, XtPointer, XtPointer);
  void y_rot_button_cb(Widget, XtPointer, XtPointer);
  void z_rot_button_cb(Widget, XtPointer, XtPointer);
  void x_trans_button_cb(Widget, XtPointer, XtPointer);
  void y_trans_button_cb(Widget, XtPointer, XtPointer);
  void z_trans_button_cb(Widget, XtPointer, XtPointer);
  
  void x_rot_text_focus_cb(Widget, XtPointer, XtPointer);
  void y_rot_text_focus_cb(Widget, XtPointer, XtPointer);
  void z_rot_text_focus_cb(Widget, XtPointer, XtPointer);
  void x_trans_text_focus_cb(Widget, XtPointer, XtPointer);
  void y_trans_text_focus_cb(Widget, XtPointer, XtPointer);
  void z_trans_text_focus_cb(Widget, XtPointer, XtPointer);
/* <--aro */

  int i,n,count;

  char istring[16];

  Widget rc[2], frame, atoms_label, region, button, invert, reset;
  Widget discard,make_perm;

  XmString title;

  if(!edit_posn_dialog){

    title=XmStringCreateLocalized("Edit relative positions");

    n=0;
    XtSetArg (args[n],XmNautoUnmanage, False); n++;
    XtSetArg (args[n],XmNdialogTitle,title); n++;

    edit_posn_dialog=XmCreateMessageDialog(parent,"edit_posn",args,n);

    rc[0]=XtVaCreateManagedWidget
      ("rc",xmRowColumnWidgetClass,edit_posn_dialog,NULL);

/* aro--> */

    /* PushButton and TextField Widgets are created for use
       in rotation and translation, callbacks added to each */

    rc[1]=XtVaCreateManagedWidget
      ("epd_rc1",
       xmRowColumnWidgetClass,rc[0],
       XmNorientation, XmHORIZONTAL,
       NULL);
    
    rot_text_label = XtVaCreateManagedWidget
      ("Enter value for rotation about X, Y, or Z",
       xmLabelWidgetClass, rc[1],
       NULL);

    rc[1]=XtVaCreateManagedWidget
      ("epd_rc1",
       xmRowColumnWidgetClass,rc[0],
       XmNorientation, XmHORIZONTAL,
       NULL);
            
    x_rot_text = XtVaCreateManagedWidget
      ("x_rot_text", xmTextFieldWidgetClass,
       rc[1],
       XmNcolumns, 8,
       NULL);

    XtAddCallback(x_rot_text, XmNfocusCallback, x_rot_text_focus_cb, NULL);

    x_rot_button = XtVaCreateManagedWidget
      ("X", xmPushButtonWidgetClass,
       rc[1],
       NULL);

    XtAddCallback(x_rot_button, XmNactivateCallback, x_rot_button_cb, NULL);

    y_rot_text = XtVaCreateManagedWidget
      ("y_rot_text", xmTextFieldWidgetClass,
       rc[1],
       XmNcolumns, 8,
       NULL);

    XtAddCallback(y_rot_text, XmNfocusCallback, y_rot_text_focus_cb, NULL);

    y_rot_button = XtVaCreateManagedWidget
      ("Y", xmPushButtonWidgetClass,
       rc[1],
       NULL);

    XtAddCallback(y_rot_button, XmNactivateCallback, y_rot_button_cb, NULL);

    z_rot_text = XtVaCreateManagedWidget
      ("z_rot_text", xmTextFieldWidgetClass,
       rc[1],
       XmNcolumns, 8,
       NULL);

    XtAddCallback(z_rot_text, XmNfocusCallback, z_rot_text_focus_cb, NULL);

    z_rot_button = XtVaCreateManagedWidget
      ("Z", xmPushButtonWidgetClass,
       rc[1],
       NULL);

    XtAddCallback(z_rot_button, XmNactivateCallback, z_rot_button_cb, NULL);

    /* Create translation PushButtons and TextFields */

    rc[1]=XtVaCreateManagedWidget
      ("epd_rc1",
       xmRowColumnWidgetClass,rc[0],
       XmNorientation, XmHORIZONTAL,
       NULL);
    
    trans_text_label = XtVaCreateManagedWidget
      ("Enter value for translation along X, Y, or Z",
       xmLabelWidgetClass, rc[1],
       NULL);

    rc[1]=XtVaCreateManagedWidget
      ("epd_rc1",
       xmRowColumnWidgetClass,rc[0],
       XmNorientation, XmHORIZONTAL,
       NULL);
            
    x_trans_text = XtVaCreateManagedWidget
      ("x_trans_text", xmTextFieldWidgetClass,
       rc[1],
       XmNcolumns, 8,
       NULL);

    XtAddCallback(x_trans_text, XmNfocusCallback, x_trans_text_focus_cb, NULL);

    x_trans_button = XtVaCreateManagedWidget
      ("X", xmPushButtonWidgetClass,
       rc[1],
       NULL);

    XtAddCallback(x_trans_button, XmNactivateCallback, x_trans_button_cb, NULL);

    y_trans_text = XtVaCreateManagedWidget
      ("y_trans_text", xmTextFieldWidgetClass,
       rc[1],
       XmNcolumns, 8,
       NULL);

    XtAddCallback(y_trans_text, XmNfocusCallback, y_trans_text_focus_cb, NULL);

    y_trans_button = XtVaCreateManagedWidget
      ("Y", xmPushButtonWidgetClass,
       rc[1],
       NULL);

    XtAddCallback(y_trans_button, XmNactivateCallback, y_trans_button_cb, NULL);

    z_trans_text = XtVaCreateManagedWidget
      ("z_trans_text", xmTextFieldWidgetClass,
       rc[1],
       XmNcolumns, 8,
       NULL);

    XtAddCallback(z_trans_text, XmNfocusCallback, z_trans_text_focus_cb, NULL);

    z_trans_button = XtVaCreateManagedWidget
      ("Z", xmPushButtonWidgetClass,
       rc[1],
       NULL);

    XtAddCallback(z_trans_button, XmNactivateCallback, z_trans_button_cb, NULL);
/* <--aro */

    rc[1]=XtVaCreateManagedWidget
      ("epd_rc1",
       xmRowColumnWidgetClass,rc[0],
       XmNorientation, XmHORIZONTAL,
       NULL);

    title = XmStringCreateLocalized("Rotate atoms about X");
    
    x_rot = XtVaCreateManagedWidget
      ("x_rot",xmScaleWidgetClass,
       rc[1],
       XmNmaximum,  180,
       XmNminimum, -180,
       XmNvalue,      0,
       XmNshowValue, True,
       XmNorientation, XmHORIZONTAL,
       XmNtitleString, title,
       NULL);

    XtAddCallback(x_rot, XmNvalueChangedCallback, rot_store_geom, NULL);
    XtAddCallback(x_rot, XmNdragCallback, x_rot_cb, NULL);
    

    title = XmStringCreateLocalized("Translate atoms along X");

    x_trans = XtVaCreateManagedWidget
      ("x_trans",xmScaleWidgetClass,
       rc[1],
       XmNmaximum,  100,
       XmNminimum, -100,
       XmNvalue,      0,
       XmNshowValue, False,
       XmNorientation, XmHORIZONTAL,
       XmNtitleString, title,
       NULL);

    XtAddCallback(x_trans,XmNvalueChangedCallback,trans_store_geom_cb, NULL);
    XtAddCallback(x_trans, XmNdragCallback, x_trans_cb, NULL);


    rc[1]=XtVaCreateManagedWidget
      ("epd_rc1",
       xmRowColumnWidgetClass,rc[0],
       XmNorientation, XmHORIZONTAL,
       NULL);


    title = XmStringCreateLocalized("Rotate atoms about Y");

    y_rot = XtVaCreateManagedWidget
      ("y_rot",xmScaleWidgetClass,
       rc[1],
       XmNmaximum,  180,
       XmNminimum, -180,
       XmNvalue,      0,
       XmNshowValue, True,
       XmNorientation, XmHORIZONTAL,
       XmNtitleString, title,
       NULL);

    XtAddCallback(y_rot, XmNvalueChangedCallback, rot_store_geom, NULL);
    XtAddCallback(y_rot, XmNdragCallback, y_rot_cb, NULL);

    title = XmStringCreateLocalized("Translate atoms along Y");

    y_trans = XtVaCreateManagedWidget
      ("y_trans",xmScaleWidgetClass,
       rc[1],
       XmNmaximum,  100,
       XmNminimum, -100,
       XmNvalue,      0,
       XmNshowValue, False,
       XmNorientation, XmHORIZONTAL,
       XmNtitleString, title,
       NULL);

    XtAddCallback(y_trans,XmNvalueChangedCallback, trans_store_geom_cb, NULL);
    XtAddCallback(y_trans,XmNdragCallback, y_trans_cb, NULL);

    rc[1]=XtVaCreateManagedWidget
      ("epd_rc1",
       xmRowColumnWidgetClass,rc[0],
       XmNorientation, XmHORIZONTAL,
       NULL);

    title = XmStringCreateLocalized("Rotate atoms about Z");

    z_rot = XtVaCreateManagedWidget
      ("z_rot",xmScaleWidgetClass,
       rc[1],
       XmNmaximum,  180,
       XmNminimum, -180,
       XmNvalue,      0,
       XmNshowValue, True,
       XmNorientation, XmHORIZONTAL,
       XmNtitleString, title,
       NULL);

    XtAddCallback(z_rot, XmNvalueChangedCallback, rot_store_geom, NULL);
    XtAddCallback(z_rot, XmNdragCallback, z_rot_cb, NULL);

    title = XmStringCreateLocalized("Translate atoms along Z");

    z_trans = XtVaCreateManagedWidget
      ("z_trans",xmScaleWidgetClass,
       rc[1],
       XmNmaximum,  100,
       XmNminimum, -100,
       XmNvalue,      0,
       XmNshowValue, False,
       XmNorientation, XmHORIZONTAL,
       XmNtitleString, title,
       NULL);
    
    XtAddCallback(z_trans,XmNvalueChangedCallback,trans_store_geom_cb, NULL);
    XtAddCallback(z_trans,XmNdragCallback,z_trans_cb, NULL);    

    frame=XtVaCreateManagedWidget 
      ("frame1",
       xmFrameWidgetClass, rc[0],
       XmNshadowType,      XmSHADOW_IN,
       NULL);

    rc[1]=XtVaCreateManagedWidget
      ("types",
       xmRowColumnWidgetClass,frame,
       XmNorientation, XmVERTICAL,
       NULL);        

    atoms_label= XtVaCreateManagedWidget
      ("Manipulate which atoms?",
       xmLabelWidgetClass,rc[1],
       XmNalignment, XmALIGNMENT_BEGINNING,
       NULL);

    region = XmCreateRadioBox(rc[1],
                              "Act in:",
                              NULL,
                              0);

    button = XtVaCreateManagedWidget("Act on everything",
                                     xmToggleButtonGadgetClass,
                                     region,
                                     NULL);

    if(posn_act_on == WHOLE_CANVAS)
      {
        XtVaSetValues(button, XmNset, True, NULL);
      }

    XtAddCallback(button,
                  XmNvalueChangedCallback,
                  posn_act_on_cb,
                  (XtPointer) WHOLE_CANVAS);

    button = XtVaCreateManagedWidget("Act inside region",
                                     xmToggleButtonGadgetClass,
                                     region,
                                     NULL);

    if(posn_act_on == INSIDE_REGION)
      {
        XtVaSetValues(button, XmNset, True, NULL);
      }

    XtAddCallback(button,
                  XmNvalueChangedCallback,
                  posn_act_on_cb,
                  (XtPointer) INSIDE_REGION);

    button = XtVaCreateManagedWidget("Act outside region",
                                     xmToggleButtonGadgetClass,
                                     region,
                                     NULL);

    if(posn_act_on == OUTSIDE_REGION)
      {
        XtVaSetValues(button, XmNset, True, NULL);
      }

    XtAddCallback(button,
                  XmNvalueChangedCallback,
                  posn_act_on_cb,
                  (XtPointer) OUTSIDE_REGION);

    XtManageChild(region);


    if(edit_posn_type_btn){
      free(edit_posn_type_btn);
      free(atom_group);
    }

    edit_posn_type_btn=malloc(no_atom_types*sizeof(Widget));
    atom_group=malloc(no_atom_types*sizeof(struct atom_group));
    
    count=0;

    for (i = 0; i < get_no_elements(); i++)
      {
        if(element[i].any_atoms)
          {
            sprintf(istring,"Toggle %s atoms",element[i].label);
            edit_posn_type_btn[count]=XtVaCreateManagedWidget
              (istring,xmPushButtonWidgetClass,rc[1],NULL);
            XtAddCallback(edit_posn_type_btn[count],XmNactivateCallback,
                          posn_atoms_type_cb,(XtPointer)count);       
        
            strcpy(atom_group[count].label,element[i].label);

            count++;
          }
      }

    invert=XtVaCreateManagedWidget
      ("Invert selection",xmPushButtonWidgetClass,rc[1],NULL);

    XtAddCallback(invert,XmNactivateCallback,edit_posn_invert_cb,NULL);

    reset=XtVaCreateManagedWidget
      ("Reselect all",xmPushButtonWidgetClass,rc[1],NULL);

    XtAddCallback(reset,XmNactivateCallback,edit_posn_reset_cb,NULL);
    
    rc[1]=XtVaCreateManagedWidget
      ("types",
       xmRowColumnWidgetClass,rc[0],
       XmNorientation, XmHORIZONTAL,
       NULL);

    discard=XtVaCreateManagedWidget
      ("Discard changes",xmPushButtonWidgetClass,rc[1],NULL);

    XtAddCallback(discard,XmNactivateCallback,edit_posn_discard_cb, NULL);

    make_perm=XtVaCreateManagedWidget
      ("Make changes permanent",xmPushButtonWidgetClass,rc[1],NULL);

    XtAddCallback(make_perm,XmNactivateCallback,edit_posn_make_perm_cb,NULL);

    XtAddCallback(edit_posn_dialog,XmNcancelCallback,edit_posn_cancel_cb,NULL);
      
    XtUnmanageChild
      (XmMessageBoxGetChild(edit_posn_dialog,XmDIALOG_OK_BUTTON));

    /* No help available ... */

    XtUnmanageChild
      (XmMessageBoxGetChild(edit_posn_dialog,XmDIALOG_HELP_BUTTON));

  }

  XtAddCallback (edit_posn_dialog, XmNmapCallback, place_dialog_cb, NULL);

  XtManageChild(edit_posn_dialog);

  /* set the position if it is available - else just egtthe position */

  if(edit_posn_dialog_pos_are_avail == 1)
    {
      XtVaSetValues(edit_posn_dialog,
                    XmNx, edit_posn_dialog_xpos,
                    XmNy, edit_posn_dialog_ypos,
                    NULL);
    }
  else
    {
      get_edit_posn_dialog_position();
    }

}


void
edit_posn_cancel_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{

  void change_frame(int, Boolean, Boolean);
  void get_edit_posn_dialog_position(void);

  int i;

  /* Get the position of the widget before killing it */

  get_edit_posn_dialog_position();

  XtDestroyWidget(edit_posn_dialog);
  edit_posn_dialog=NULL;
  
  free(edit_posn_type_btn);
  edit_posn_type_btn = NULL;
  
   /* Reset the translation and rotation attributes of the atoms */

  for(i = 0; i < no_atoms; i++)
    {
      atoms[i].edit = 1;
    }

  change_frame(frame_no,False,False);

}


void
get_edit_posn_dialog_position(void)
{

  /* Get the position of the widget */
  
  XtVaGetValues(edit_posn_dialog,
                XmNx, &edit_posn_dialog_xpos,
                XmNy, &edit_posn_dialog_ypos,
                NULL);
  
  edit_posn_dialog_pos_are_avail = 1;
  
}


void
edit_posn_invert_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{

  void canvas_cb(Widget, XtPointer, XtPointer);
  int * get_atoms_in_region(void);
  Boolean region_active_p(void);
  void update_atoms_in_region(Boolean);

  int i;
  int *atoms_in_region = NULL;

  if((posn_act_on == INSIDE_REGION) ||
     (posn_act_on == OUTSIDE_REGION))
    {
      if(region_active_p() == 1)
        {
          update_atoms_in_region(1);
          atoms_in_region = get_atoms_in_region();
        }
      else
        {
          echo_to_message_area("Region is not active");
          return;
        }
    }

  for(i = 0; i < no_atoms; i++)
    {
      if((posn_act_on == WHOLE_CANVAS) ||
         ((posn_act_on == INSIDE_REGION)  && (atoms_in_region[i] == 1)) ||
         ((posn_act_on == OUTSIDE_REGION) && (atoms_in_region[i] == 0)))
        {
          if(atoms[i].edit == 0)
            {        
              atoms[i].edit = 1;
            }
          else
            {
              atoms[i].edit = 0;
            }
        }
    }

  redraw = 1;
  canvas_cb(canvas, NULL, NULL);

}


void
edit_posn_reset_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{
  
  void canvas_cb(Widget, XtPointer, XtPointer);
  int * get_atoms_in_region(void);
  Boolean region_active_p(void);
  void update_atoms_in_region(Boolean);

  int i;
  int *atoms_in_region = NULL;

  if((posn_act_on == INSIDE_REGION) ||
     (posn_act_on == OUTSIDE_REGION))
    {
      if(region_active_p() == 1)
        {
          update_atoms_in_region(1);
          atoms_in_region = get_atoms_in_region();
        }
      else
        {
          echo_to_message_area("Region is not active");
          return;
        }
    }

  for(i = 0; i < no_atoms; i++)
    {
      if((posn_act_on == WHOLE_CANVAS) ||
         ((posn_act_on == INSIDE_REGION)  && (atoms_in_region[i] == 1)) ||
         ((posn_act_on == OUTSIDE_REGION) && (atoms_in_region[i] == 0)))
        {
          atoms[i].edit = 1;
        }
    }

  redraw = 1;
  canvas_cb(canvas, NULL, NULL);

}


void
x_trans_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{

  void restore_geom(void);
  void translate_atoms(double *);

  double vec[3];

  XmScaleCallbackStruct *cbs=
    (XmScaleCallbackStruct *)call_data;

  vec[0] = (double) cbs->value/100;
  vec[1] = 0;
  vec[2] = 0;

  restore_geom();
  translate_atoms(vec);

}


void
y_trans_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{

  void restore_geom(void);
  void translate_atoms(double *);

  double vec[3];

  XmScaleCallbackStruct *cbs=
    (XmScaleCallbackStruct *)call_data;

  vec[0] = 0;
  vec[1] = (double) cbs->value/100;
  vec[2] = 0;

  restore_geom();
  translate_atoms(vec);

}


void
z_trans_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{

  void restore_geom(void);
  void translate_atoms(double *);

  double vec[3];

  XmScaleCallbackStruct *cbs=
    (XmScaleCallbackStruct *)call_data;

  vec[0] = 0;
  vec[1] = 0;
  vec[2] = (double) cbs->value/100;

  restore_geom();
  translate_atoms(vec);
  
}


void
trans_store_geom_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{

  void store_coords(void);

  store_coords();

  /* Reset scale bars after each translation */
  
  if(edit_posn_dialog){
    XtVaSetValues(x_trans,XmNvalue,0,NULL);
    XtVaSetValues(y_trans,XmNvalue,0,NULL);
    XtVaSetValues(z_trans,XmNvalue,0,NULL);
  }

}


void
x_rot_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{

  void rotate_atoms(double *,double,Boolean,Boolean);

  int x_rot;

  XmScaleCallbackStruct *cbs=
    (XmScaleCallbackStruct *)call_data;

  x_rot = cbs->value-last_value;
  last_value=cbs->value;

  phi=x_rot*PI/180.0;

  axis[0] = 1.0;
  axis[1] = 0.0;
  axis[2] = 0.0;

  atoms_sorted=0; /* Depths have changed */
  redraw=1;
  rotate_atoms(axis,phi,1,True);

}


void
y_rot_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{
  
  void rotate_atoms(double *,double,Boolean,Boolean);

  int y_rot;
  
  XmScaleCallbackStruct *cbs=
    (XmScaleCallbackStruct *)call_data;

  y_rot = cbs->value-last_value;
  last_value=cbs->value;

  phi=y_rot*PI/180.0;

  axis[0] = 0.0;
  axis[1] = 1.0;
  axis[2] = 0.0;

  atoms_sorted=0; /* Depths have changed */
  redraw=1;
  rotate_atoms(axis,phi,1,True);
  
}


void
z_rot_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{

  void rotate_atoms(double *,double,Boolean,Boolean);

  int z_rot;
  
  XmScaleCallbackStruct *cbs=
    (XmScaleCallbackStruct *)call_data;

  z_rot = cbs->value-last_value;
  last_value=cbs->value;

  phi=z_rot*PI/180.0;

  axis[0] = 0.0;
  axis[1] = 0.0;
  axis[2] = 1.0;

  redraw=1;
  rotate_atoms(axis,phi,1,True);

}


void
rot_store_geom(Widget widget, XtPointer client_data, XtPointer call_data)
{
  
  last_value=0;

  /* Reset scale bars after each rotation */
  
  if(edit_posn_dialog != NULL){
    XtVaSetValues(x_rot,XmNvalue,0,NULL);
    XtVaSetValues(y_rot,XmNvalue,0,NULL);
    XtVaSetValues(z_rot,XmNvalue,0,NULL);
  }

}

/* aro--> */

/* Callbacks for PushButtons and TextFields implemented. Most code to do 
   rotations copied from x_rot_cb, y_rot_cb, z_rot_cb, , and most code
   to do translations copied from x_trans_cb, y_trans_cb, z_trans_cb above */

void
x_rot_button_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{
  void rotate_atoms(double *,double,Boolean,Boolean);

  char *text;
  double rot_x;

  XtVaGetValues(x_rot_text, XmNvalue, &text, NULL);

  rot_x = atof(text);

  phi=rot_x*PI/180.0;

  axis[0] = 1.0;
  axis[1] = 0.0;
  axis[2] = 0.0; 

  atoms_sorted=0; /* Depths have changed */
  redraw=1;
  rotate_atoms(axis,phi,1,True);

}

void
y_rot_button_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{
  void rotate_atoms(double *,double,Boolean,Boolean);

  char *text;
  double rot_y;

  XtVaGetValues(y_rot_text, XmNvalue, &text, NULL);

  rot_y = atof(text);

  phi=rot_y*PI/180.0;

  axis[0] = 0.0;
  axis[1] = 1.0;
  axis[2] = 0.0; 

  atoms_sorted=0; /* Depths have changed */
  redraw=1;
  rotate_atoms(axis,phi,1,True);

}

void
z_rot_button_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{
  void rotate_atoms(double *,double,Boolean,Boolean);

  char *text;
  double rot_z;

  XtVaGetValues(z_rot_text, XmNvalue, &text, NULL);

  rot_z = atof(text);

  phi=rot_z*PI/180.0;

  axis[0] = 0.0;
  axis[1] = 0.0;
  axis[2] = 1.0; 

  atoms_sorted=0; /* Depths have changed */
  redraw=1;
  rotate_atoms(axis,phi,1,True);

}

void
x_trans_button_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{

  void restore_geom(void);
  void store_coords(void);
  void translate_atoms(double *);

  char *text;  
  double vec[3];

  XtVaGetValues(x_trans_text, XmNvalue, &text, NULL);

  vec[0] = atof(text);
  vec[1] = 0;
  vec[2] = 0;

  restore_geom();
  translate_atoms(vec);
  store_coords();
}

void
y_trans_button_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{

  void restore_geom(void);
  void store_coords(void);
  void translate_atoms(double *);

  char *text;  
  double vec[3];

  XtVaGetValues(y_trans_text, XmNvalue, &text, NULL);

  vec[0] = 0;
  vec[1] = atof(text);
  vec[2] = 0;

  restore_geom();
  translate_atoms(vec);
  store_coords();
}

void
z_trans_button_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{

  void restore_geom(void);
  void store_coords(void);
  void translate_atoms(double *);

  char *text;  
  double vec[3];

  XtVaGetValues(z_trans_text, XmNvalue, &text, NULL);

  vec[0] = 0;
  vec[1] = 0;
  vec[2] = atof(text);

  restore_geom();
  translate_atoms(vec);
  store_coords();
}

/* TextField callbacks simply set default button to be the corresponding button to
   the TextField (if TextField for x-axis gets focus, set default button to be
   the button that applies the change to the x-axis, etc...) */
void
x_rot_text_focus_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{
  XtVaSetValues(edit_posn_dialog, XmNdefaultButton, x_rot_button, NULL);
}

void
y_rot_text_focus_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{
  XtVaSetValues(edit_posn_dialog, XmNdefaultButton, y_rot_button, NULL);
}

void
z_rot_text_focus_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{
  XtVaSetValues(edit_posn_dialog, XmNdefaultButton, z_rot_button, NULL);
}

void
x_trans_text_focus_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{ 
  XtVaSetValues(edit_posn_dialog, XmNdefaultButton, x_trans_button, NULL);
}

void
y_trans_text_focus_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{
  XtVaSetValues(edit_posn_dialog, XmNdefaultButton, y_trans_button, NULL);
}

void
z_trans_text_focus_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{
  XtVaSetValues(edit_posn_dialog, XmNdefaultButton, z_trans_button, NULL);
}
/* <--aro */

void
posn_atoms_type_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{
  
  void canvas_cb(Widget, XtPointer, XtPointer);
  int * get_atoms_in_region(void);
  Boolean region_active_p(void);
  void update_atoms_in_region(Boolean);

  int i,at_type;
  int *atoms_in_region = NULL;

  at_type = (int) client_data;

  if((posn_act_on == INSIDE_REGION) ||
     (posn_act_on == OUTSIDE_REGION))
    {
      if(region_active_p() == 1)
        {
          update_atoms_in_region(1);
          atoms_in_region = get_atoms_in_region();
        }
      else
        {
          echo_to_message_area("Region is not active");
          return;
        }
    }

  for(i = 0; i < no_atoms; i++)
    {
      if(strcmp(atom_group[at_type].label, atoms[i].uppercase_label) == 0)
        {
          if((posn_act_on == WHOLE_CANVAS) ||
             ((posn_act_on == INSIDE_REGION)  && (atoms_in_region[i] == 1)) ||
             ((posn_act_on == OUTSIDE_REGION) && (atoms_in_region[i] == 0)))
            {
              if(atoms[i].edit == 0)
                {
                  atoms[i].edit = 1;
                }
              else
                {
                  atoms[i].edit = 0;
                }
            }
        }
    }
  
  redraw = 1;
  canvas_cb(canvas, NULL, NULL);

}


void
edit_posn_discard_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{

  void change_frame(int,Boolean,Boolean);

  change_frame(frame_no,True,False);

}


void
edit_posn_make_perm_cb(Widget widget,
                       XtPointer client_data,
                       XtPointer call_data)
{

  void get_gx_coords (void);
  struct frame * get_selected_frame ();

  int i;

  struct frame *this_frame;

  /* To make the changes permanent, get the gx coordinates (the coordintates
     in the global axis frame, and write them to frame[frame_no] */

  this_frame = get_selected_frame ();

  if (this_frame == NULL)
    {
      echo_to_message_area ("No data loaded");
      return;
    }

  get_gx_coords ();
 
  for (i = 0; i < this_frame->no_atoms; i++)
    {
      this_frame->atom[i].x = atoms[i].g[0];
      this_frame->atom[i].y = atoms[i].g[1];
      this_frame->atom[i].z = atoms[i].g[2];
    }
  
}


void
make_scale_coords_dialog (Widget parent)
{
  void scale_coords_atob_cb (Widget, XtPointer, XtPointer);
  void scale_coords_btoa_cb (Widget, XtPointer, XtPointer);
  void scale_coords_cancel_cb (Widget, XtPointer, XtPointer);
  void scale_coords_ok_cb (Widget, XtPointer, XtPointer);

  int n = 0;
  Widget atob, btoa, child, rc;
  XmString label = NULL;

  if (! scale_coords_dialog)
    {
      label = XmStringCreateLocalized ("Scale Coordinates");

      XtSetArg (args[n], XmNautoUnmanage, False); n++;
      XtSetArg (args[n], XmNdialogTitle, label); n++;

      scale_coords_dialog = (Widget)
        XmCreateMessageDialog (parent, "scale_coords", args, n);

      XtAddCallback (scale_coords_dialog, XmNcancelCallback,
                  scale_coords_cancel_cb, NULL);

      rc = XtVaCreateManagedWidget
        ("rc", xmRowColumnWidgetClass, scale_coords_dialog, NULL);

      btoa = XtVaCreateManagedWidget ("Bohr->Angstrom",
                                       xmPushButtonWidgetClass, rc, NULL);

      atob = XtVaCreateManagedWidget ("Angstrom->Bohr",
                                       xmPushButtonWidgetClass, rc, NULL);

      XtVaCreateManagedWidget
        ("Scale factor:", xmLabelWidgetClass, rc,
         XmNalignment, XmALIGNMENT_BEGINNING,
         NULL);

      child = XtVaCreateManagedWidget
      ("scale_factor", xmTextFieldWidgetClass, rc, NULL);

      XtVaSetValues (child, XmNvalue, "1.0", NULL);

      XtAddCallback (atob, XmNactivateCallback, scale_coords_atob_cb, child);
      XtAddCallback (btoa, XmNactivateCallback, scale_coords_btoa_cb, child);

      XtAddCallback (scale_coords_dialog, XmNokCallback,
                  scale_coords_ok_cb, child);

      XtUnmanageChild (XmMessageBoxGetChild
                       (scale_coords_dialog, XmDIALOG_HELP_BUTTON));

    }

  XtAddCallback (scale_coords_dialog, XmNmapCallback, place_dialog_cb, NULL);

  XtManageChild (scale_coords_dialog);

  XmStringFree (label);
}


void
scale_coords_atob_cb (Widget widget,
                      XtPointer client_data,
                      XtPointer call_data)
{
  XtVaSetValues (client_data, XmNvalue, "1.88972613392", NULL);
}


void
scale_coords_btoa_cb (Widget widget,
                      XtPointer client_data,
                      XtPointer call_data)
{
  XtVaSetValues (client_data, XmNvalue, "0.5291772083", NULL);
}


void
scale_coords_cancel_cb (Widget widget,
                        XtPointer client_data,
                        XtPointer call_data)
{
  XtUnmanageChild (scale_coords_dialog);
}


void
scale_coords_ok_cb (Widget widget,
                      XtPointer client_data,
                      XtPointer call_data)
{
  void scale_coords (char *);

  char *scale_factor_string;

  XtVaGetValues (client_data, XmNvalue, &scale_factor_string, NULL);

  scale_coords (scale_factor_string);
}


void scale_coords (char *scale_factor_string)
{
  void change_frame (int, Boolean, Boolean);
  struct frame * get_first_frame ();

  int i, j;
  char message[32];
  double scale_factor;

  struct frame *this_frame;

  if ((strlen (scale_factor_string) == 0) ||
      (sscanf (scale_factor_string, "%lf", &scale_factor) <= 0))
    {
      echo_to_message_area ("Cannot parse string!");
      return;
    }

  this_frame = get_first_frame ();

  while (this_frame != NULL)
    {
      for (i = 0; i < this_frame->no_atoms; i++)
        {
          this_frame->atom[i].x *= scale_factor;
          this_frame->atom[i].y *= scale_factor;
          this_frame->atom[i].z *= scale_factor;

          for (j = 0; j < MAX_VECTORS_PER_ATOM; j++)
            {
              if (this_frame->atom[i].has_vector > j)
                {
                  this_frame->atom[i].vx[j] *= scale_factor;
                  this_frame->atom[i].vy[j] *= scale_factor;
                  this_frame->atom[i].vz[j] *= scale_factor;
                }
            }
        }
      this_frame = this_frame->next;
    }

  change_frame(frame_no, False, False);
  sprintf (message, "Coordinates scaled by %lf", scale_factor);
  echo_to_message_area (message);
}

void
make_edit_atbd_sizes_dialog(Widget parent)
{

  void at_scale_cb(Widget, XtPointer, XtPointer);
  void bd_scale_cb(Widget, XtPointer, XtPointer);
  void edit_atbd_sizes_dlg_cancel_cb(Widget, XtPointer, XtPointer);
  double get_atom_scale(void);
  double get_bond_scale(void);
  double get_hbond_scale(void);  
  void hbd_scale_cb(Widget, XtPointer, XtPointer);
  void never_use_vdw_cb (Widget, XtPointer, XtPointer);

  int n=0;

  Widget rc;

  XmString title;

  if(!edit_atbd_sizes_dialog){

    title=XmStringCreateLocalized("Edit atom/bond sizes");

    n=0;
    XtSetArg(args[n],XmNdialogTitle,title); n++;

    edit_atbd_sizes_dialog=
      XmCreateMessageDialog(parent,"edit",args,n);

    rc=XtVaCreateManagedWidget
      ("rc",xmRowColumnWidgetClass,edit_atbd_sizes_dialog,NULL);

    title=XmStringCreateLocalized("Atoms: % covalent radius");

    at_scale_w=XtVaCreateManagedWidget
      ("at_scale_w", xmScaleWidgetClass,
       rc,
       XmNmaximum, 200,
       XmNminimum, 0,
       XmNvalue,   (int) (get_atom_scale() * 100),
       XmNshowValue, True,
       XmNorientation, XmHORIZONTAL,
       XmNtitleString, title,
       NULL);

    XtAddCallback(at_scale_w,XmNdragCallback, at_scale_cb,NULL);
    XtAddCallback(at_scale_w,XmNvalueChangedCallback, at_scale_cb,NULL);
    
    title=XmStringCreateLocalized("Bonds: % smallest covalent radius");

    bd_scale_w=XtVaCreateManagedWidget
      ("bd_scale_w", xmScaleWidgetClass,
       rc,
       XmNmaximum, 100,
       XmNminimum, 0,
       XmNvalue,   (int) (get_bond_scale() * 100),
       XmNshowValue, True,
       XmNorientation, XmHORIZONTAL,
       XmNtitleString, title,
       NULL);

    XtAddCallback(bd_scale_w,XmNdragCallback, bd_scale_cb,NULL);
    XtAddCallback(bd_scale_w,XmNvalueChangedCallback, bd_scale_cb,NULL);
    
    title=XmStringCreateLocalized("H-bonds: % H covalent radius");

    hbd_scale_w=XtVaCreateManagedWidget
      ("hbd_scale_w", xmScaleWidgetClass,
       rc,
       XmNmaximum, 100,
       XmNminimum, 0,
       XmNvalue,   (int) (get_hbond_scale() * 100),
       XmNshowValue, True,
       XmNorientation, XmHORIZONTAL,
       XmNtitleString, title,
       NULL);

    XtAddCallback(hbd_scale_w,XmNdragCallback, hbd_scale_cb,NULL);
    XtAddCallback(hbd_scale_w,XmNvalueChangedCallback, hbd_scale_cb,NULL);

    XtUnmanageChild
      (XmMessageBoxGetChild(edit_atbd_sizes_dialog,XmDIALOG_OK_BUTTON));
    
    XtAddCallback(edit_atbd_sizes_dialog, XmNcancelCallback,
                  edit_atbd_sizes_dlg_cancel_cb,NULL);

    /* No help available ... */

    XtUnmanageChild
      (XmMessageBoxGetChild(edit_atbd_sizes_dialog,XmDIALOG_HELP_BUTTON));

    XtManageChild (XmCreateSeparator (rc, "sep", NULL, 0));

    never_use_vdw_w = XtVaCreateManagedWidget
      ("Never use van der Waals radii",
       xmToggleButtonGadgetClass,
       rc,
       NULL);

    if (never_use_vdw)
      {
        XtVaSetValues (never_use_vdw_w, XmNset, True, NULL);
      }

    XtAddCallback (never_use_vdw_w,
                   XmNvalueChangedCallback,
                   never_use_vdw_cb,
                   (XtPointer) 1);
  }

  XtAddCallback (edit_atbd_sizes_dialog, XmNmapCallback, place_dialog_cb, NULL);

  XtManageChild(edit_atbd_sizes_dialog);

}


void
edit_atbd_sizes_dlg_cancel_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{

  XtDestroyWidget(edit_atbd_sizes_dialog);
  edit_atbd_sizes_dialog=NULL;

}


void
at_scale_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{

  void canvas_cb(Widget, XtPointer, XtPointer);
  void set_atom_scale(double);
  void update_bbox(void);

  int scale;
  
  XmScaleCallbackStruct *cbs=
    (XmScaleCallbackStruct *)call_data;
  
  scale = cbs->value;

  set_atom_scale((double) (scale/100.0));
  
  /* Because the bounding box incorporates the atoms, it may need
     updating */

  if((atom_flag == 1) && (bbox_flag == 1))
    {
      update_bbox();
    }

  redraw=1;
  canvas_cb(canvas,NULL,NULL);

}


void
bd_scale_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{

  void canvas_cb(Widget, XtPointer, XtPointer);
  void set_bond_scale(double);
  
  int scale;
  
  XmScaleCallbackStruct *cbs=
    (XmScaleCallbackStruct *)call_data;

  scale = cbs->value;

  set_bond_scale((double) (scale / 100.0));

  redraw=1;
  canvas_cb(canvas,NULL,NULL);

}


void
hbd_scale_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{

  void canvas_cb(Widget, XtPointer, XtPointer);
  void set_hbond_scale(double);
  
  int scale;
  
  XmScaleCallbackStruct *cbs=
    (XmScaleCallbackStruct *)call_data;

  scale = cbs->value;

  set_hbond_scale((double) (scale / 100.0));
  
  redraw=1;
  canvas_cb(canvas,NULL,NULL);

}

void
never_use_vdw_cb (Widget widget, XtPointer client_data, XtPointer call_data)
{

  void canvas_cb (Widget, XtPointer, XtPointer);

  if (never_use_vdw == 0)
    {
      never_use_vdw = 1;
    }
  else
    {
      never_use_vdw = 0;
    }

  redraw = 1;
  canvas_cb (canvas, NULL, NULL); /* update display */
}

void
make_edit_bond_fudges_dialog(Widget parent)
{

  void bdfd_scale_cb(Widget,XtPointer,XtPointer);
  void edit_bond_fudges_dlg_cancel_cb(Widget,XtPointer,XtPointer);
  void hbdfd_scale_cb(Widget,XtPointer,XtPointer);
  /* Intramolecular */
  void ibdfd_scale_cb (Widget, XtPointer, XtPointer);
  void ihbdfd_scale_cb (Widget, XtPointer, XtPointer);
  
  int n=0;

  Widget rc;

  XmString title;

  if(!edit_bond_fudges_dialog){

    title=XmStringCreateLocalized("Bond factors");

    n=0;
    XtSetArg(args[n],XmNdialogTitle,title); n++;
    
    edit_bond_fudges_dialog=
      XmCreateMessageDialog(parent,"edit",args,n);

    rc=XtVaCreateManagedWidget
      ("rc",xmRowColumnWidgetClass,edit_bond_fudges_dialog,NULL);

    title=XmStringCreateLocalized("Intermolecular bond factor");

    bdfd_scale_w=XtVaCreateManagedWidget
      ("bdfd_scale_w",xmScaleWidgetClass,
       rc,
       XmNmaximum, 200,
       XmNminimum, 0,
       XmNvalue,   (int)(bdfd_factor * 50),
       XmNshowValue, True,
       XmNorientation, XmHORIZONTAL,
       XmNtitleString, title,
       NULL);

    XtAddCallback(bdfd_scale_w,XmNdragCallback,bdfd_scale_cb,NULL);
    XtAddCallback(bdfd_scale_w,XmNvalueChangedCallback,bdfd_scale_cb,NULL);

    title = XmStringCreateLocalized ("Intramolecular bond factor");

    ibdfd_scale_w = XtVaCreateManagedWidget
      ("ibdfd_scale_w", xmScaleWidgetClass,
       rc,
       XmNmaximum, 200,
       XmNminimum, 0,
       XmNvalue, (int)(ibdfd_factor * 50),
       XmNshowValue, True,
       XmNorientation, XmHORIZONTAL,
       XmNtitleString, title,
       NULL);

    XtAddCallback (ibdfd_scale_w, XmNdragCallback, ibdfd_scale_cb, NULL);
    XtAddCallback (ibdfd_scale_w, XmNvalueChangedCallback, ibdfd_scale_cb, NULL);

    XtManageChild (XmCreateSeparator (rc, "sep", NULL, 0));

    title=XmStringCreateLocalized("Intermolecular H-bond factor");

    hbdfd_scale_w=XtVaCreateManagedWidget
      ("hbdfd_scale_w",xmScaleWidgetClass,
       rc,
       XmNmaximum, 100,
       XmNminimum, 0,
       XmNvalue,   (int)(hbdfd_factor * 50),
       XmNshowValue, True,
       XmNorientation, XmHORIZONTAL,
       XmNtitleString, title,
       NULL);

    XtAddCallback(hbdfd_scale_w,XmNdragCallback,hbdfd_scale_cb,NULL);
    XtAddCallback(hbdfd_scale_w,XmNvalueChangedCallback,hbdfd_scale_cb,NULL);

    title = XmStringCreateLocalized ("Intramolecular H-bond factor");

    ihbdfd_scale_w = XtVaCreateManagedWidget
      ("ihbdfd_scale_w", xmScaleWidgetClass,
       rc,
       XmNmaximum, 100,
       XmNminimum, 0,
       XmNvalue, (int)(ihbdfd_factor * 50),
       XmNshowValue, True,
       XmNorientation, XmHORIZONTAL,
       XmNtitleString, title,
       NULL);

    XtAddCallback (ihbdfd_scale_w, XmNdragCallback, ihbdfd_scale_cb, NULL);
    XtAddCallback (ihbdfd_scale_w, XmNvalueChangedCallback, ihbdfd_scale_cb, NULL);

    XtAddCallback(edit_bond_fudges_dialog, XmNcancelCallback,
                  edit_bond_fudges_dlg_cancel_cb,NULL);

    XtUnmanageChild
      (XmMessageBoxGetChild(edit_bond_fudges_dialog,XmDIALOG_OK_BUTTON));
    
    /* No help available ... */

    XtUnmanageChild
      (XmMessageBoxGetChild(edit_bond_fudges_dialog,XmDIALOG_HELP_BUTTON));

  }

  XtAddCallback (edit_bond_fudges_dialog, XmNmapCallback, place_dialog_cb, NULL);

  XtManageChild(edit_bond_fudges_dialog);

}


void
edit_bond_fudges_dlg_cancel_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{

  XtDestroyWidget(edit_bond_fudges_dialog);
  edit_bond_fudges_dialog=NULL;

}


void
bdfd_scale_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{

  void canvas_cb(Widget, XtPointer, XtPointer);
  void update_bond_matrix(Boolean);
  
  int scale;
  
  XmScaleCallbackStruct *cbs=
    (XmScaleCallbackStruct *)call_data;

  scale = cbs->value;
  bdfd_factor = (double) (scale/50.0);

  update_bond_matrix(False);

  redraw=1;
  canvas_cb(canvas,NULL,NULL);

}


void
hbdfd_scale_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{

  void canvas_cb(Widget, XtPointer, XtPointer);
  void update_bond_matrix(Boolean);

  int scale;
  
  XmScaleCallbackStruct *cbs=
    (XmScaleCallbackStruct *)call_data;

  scale = cbs->value;
  hbdfd_factor = (double) (scale/50.0);

  update_bond_matrix(False);

  redraw=1;
  canvas_cb(canvas,NULL,NULL);

}


void
ibdfd_scale_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{

  void canvas_cb(Widget, XtPointer, XtPointer);
  void update_bond_matrix(Boolean);

  int scale;
  
  XmScaleCallbackStruct *cbs=
    (XmScaleCallbackStruct *)call_data;

  scale = cbs->value;
  ibdfd_factor = (double) (scale/50.0);

  update_bond_matrix(False);

  redraw=1;
  canvas_cb(canvas,NULL,NULL);

}


void
ihbdfd_scale_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{

  void canvas_cb(Widget, XtPointer, XtPointer);
  void update_bond_matrix(Boolean);

  int scale;
  
  XmScaleCallbackStruct *cbs=
    (XmScaleCallbackStruct *)call_data;

  scale = cbs->value;
  ihbdfd_factor = (double) (scale/50.0);

  update_bond_matrix(False);

  redraw=1;
  canvas_cb(canvas,NULL,NULL);

}


void
make_vector_display_dialog(Widget main_window)
{
  
  void arrow_angle_scale_cb (Widget, XtPointer, XtPointer);
  void arrow_scale_scale_cb (Widget, XtPointer, XtPointer);
  void arrow_type_cb (Widget, XtPointer, XtPointer);
  double get_vector_arrow_angle (void);
  double get_vector_arrow_scale (void);
  double get_vector_scale (void);
  void vector_scale_scale_cb (Widget, XtPointer, XtPointer);
  void vector_display_dialog_cancel_cb (Widget, XtPointer, XtPointer);

  int n;
  
  Widget row_column, arrow_angle_scale, arrow_scale_scale, vector_scale_scale;
  Widget arrow_type_RadioBox, radio_button;

  XmString title;
  
  if(vector_display_dialog == NULL)
    {
      title = XmStringCreateLocalized("Edit vector display");
      
      n = 0;
      XtSetArg(args[n], XmNautoUnmanage, False); n++;
      XtSetArg(args[n], XmNdialogTitle, title); n++;
      
      vector_display_dialog =
        XmCreateMessageDialog(main_window, "vector_display", args, n);

      row_column = 
        XtVaCreateManagedWidget("rc",
                                xmRowColumnWidgetClass,
                                vector_display_dialog,
                                NULL);

      title = XmStringCreateLocalized("Vector scale (* 10)");
      
      vector_scale_scale =
        XtVaCreateManagedWidget("vector_scale_scale",
                                xmScaleWidgetClass,
                                row_column,
                                XmNminimum, -100,
                                XmNmaximum,  100,
                                XmNvalue,    (int) (10 * get_vector_scale ()),
                                XmNshowValue, True,
                                XmNorientation, XmHORIZONTAL,
                                XmNtitleString, title,
                                NULL);
      
      /* Add callbacks */

      XtAddCallback(vector_scale_scale,
                    XmNvalueChangedCallback,
                    vector_scale_scale_cb,
                    NULL);

      XtAddCallback(vector_scale_scale,
                    XmNdragCallback,
                    vector_scale_scale_cb,
                    NULL);


      title = XmStringCreateLocalized("Vector arrow angle");

      arrow_angle_scale =
        XtVaCreateManagedWidget("arrow_angle_scale",
                                xmScaleWidgetClass,
                                row_column,
                                XmNminimum,   0,
                                XmNmaximum,  80,
                                XmNvalue,    (int) get_vector_arrow_angle (),
                                XmNshowValue, True,
                                XmNorientation, XmHORIZONTAL,
                                XmNtitleString, title,
                                NULL);
      
      /* Add callbacks */

      XtAddCallback(arrow_angle_scale,
                    XmNvalueChangedCallback,
                    arrow_angle_scale_cb,
                    NULL);

      XtAddCallback(arrow_angle_scale,
                    XmNdragCallback,
                    arrow_angle_scale_cb,
                    NULL);


      title = XmStringCreateLocalized
        ("Vector arrow scale (% of total vector length)");

      arrow_scale_scale =
        XtVaCreateManagedWidget("arrow_scale_scale",
                                xmScaleWidgetClass,
                                row_column,
                                XmNminimum,   0,
                                XmNmaximum,  50,
                                XmNvalue, 
                                 (int) (100 * get_vector_arrow_scale ()),
                                XmNshowValue, True,
                                XmNorientation, XmHORIZONTAL,
                                XmNtitleString, title,
                                NULL);
      
      /* Add callbacks */

      XtAddCallback(arrow_scale_scale,
                    XmNvalueChangedCallback,
                    arrow_scale_scale_cb,
                    NULL);

      XtAddCallback(arrow_scale_scale,
                    XmNdragCallback,
                    arrow_scale_scale_cb,
                    NULL);

      /* Type of arrow rendering */

      XtManageChild (XmCreateSeparator (row_column, "sep", NULL, 0));

      XtVaCreateManagedWidget
        ("Type of arrow:",
         xmLabelWidgetClass, row_column, 
         XmNalignment, XmALIGNMENT_BEGINNING,
         NULL);

      arrow_type_RadioBox = XmCreateRadioBox (row_column,
                                              "arrow_type",
                                              NULL,
                                              0);

      radio_button = XtVaCreateManagedWidget ("Open",
                                              xmToggleButtonGadgetClass,
                                              arrow_type_RadioBox,
                                              NULL);
      
      if (arrow_type == ARROW_OPEN)
        {
          XtVaSetValues (radio_button, XmNset, True, NULL);
        }

      XtAddCallback(radio_button,
                    XmNvalueChangedCallback,
                    arrow_type_cb,
                    (XtPointer) ARROW_OPEN);

      radio_button = XtVaCreateManagedWidget ("Closed",
                                              xmToggleButtonGadgetClass,
                                              arrow_type_RadioBox,
                                              NULL);

      if (arrow_type == ARROW_CLOSED)
        {
          XtVaSetValues (radio_button, XmNset, True, NULL);
        }

      XtAddCallback(radio_button,
                    XmNvalueChangedCallback,
                    arrow_type_cb,
                    (XtPointer) ARROW_CLOSED);

      radio_button = XtVaCreateManagedWidget ("Filled",
                                              xmToggleButtonGadgetClass,
                                              arrow_type_RadioBox,
                                              NULL);

      if (arrow_type == ARROW_FILLED)
        {
          XtVaSetValues (radio_button, XmNset, True, NULL);
        }

      XtAddCallback(radio_button,
                    XmNvalueChangedCallback,
                    arrow_type_cb,
                    (XtPointer) ARROW_FILLED);

      XtManageChild (arrow_type_RadioBox);

      /* Cancel callback */

      XtAddCallback(vector_display_dialog,
                    XmNcancelCallback,
                    vector_display_dialog_cancel_cb,
                    NULL);

      /* Unmanage the OK button */

      XtUnmanageChild(XmMessageBoxGetChild(vector_display_dialog,
                                           XmDIALOG_OK_BUTTON));
    
      /* Unmanage the HELP button */

      XtUnmanageChild(XmMessageBoxGetChild(vector_display_dialog,
                                           XmDIALOG_HELP_BUTTON));

      /* Free title */

      XmStringFree(title);

    }
  
  XtAddCallback (vector_display_dialog, XmNmapCallback, place_dialog_cb, NULL);

  XtManageChild(vector_display_dialog);

}


void
arrow_type_cb (Widget widget,
               XtPointer client_data,
               XtPointer call_data)
{

  void canvas_cb (Widget, XtPointer, XtPointer);

  arrow_type = (enum arrow_types) client_data;

  redraw = 1;
  canvas_cb (canvas, NULL, NULL);

}


void
vector_display_dialog_cancel_cb(Widget widget,
                                XtPointer client_data,
                                XtPointer call_data)
{

  XtDestroyWidget(vector_display_dialog);
  vector_display_dialog = NULL;
}


void vector_scale_scale_cb(Widget vector_scale_scale,
                           XtPointer client_data,
                           XtPointer call_data)
{

  void canvas_cb(Widget, XtPointer, XtPointer);
  void set_vector_scale(double);
  
  int scale;
  static int last_scale;
  
  XmScaleCallbackStruct *cbs =
    (XmScaleCallbackStruct *) call_data;

  scale = cbs->value;
  
  if(scale == last_scale)
    {
      return;
    }
  
  last_scale = scale;

  set_vector_scale(scale/10.0);

  if (vector_flag == 1)
    {
      redraw = 1;
      canvas_cb (main_w, NULL, NULL);
    }
  
}


void arrow_angle_scale_cb(Widget arrow_angle_scale,
                          XtPointer client_data,
                          XtPointer call_data)
{

  void canvas_cb (Widget, XtPointer, XtPointer);
  void set_vector_arrow_angle (double);
  
  int scale;
  static int last_scale;
  
  XmScaleCallbackStruct *cbs =
    (XmScaleCallbackStruct *) call_data;

  scale = cbs->value;
  
  if(scale == last_scale)
    {
      return;
    }
  
  last_scale = scale;

  set_vector_arrow_angle ((double) scale);

  if (vector_flag == 1)
    {
      redraw = 1;
      canvas_cb (main_w, NULL, NULL);
    }
  
}


void arrow_scale_scale_cb(Widget arrow_scale_scale,
                          XtPointer client_data,
                          XtPointer call_data)
{

  void canvas_cb (Widget, XtPointer, XtPointer);
  void set_vector_arrow_scale (double);
  
  int scale;
  static int last_scale;
  
  XmScaleCallbackStruct *cbs =
    (XmScaleCallbackStruct *) call_data;

  scale = cbs->value;
  
  if(scale == last_scale)
    {
      return;
    }
  
  last_scale = scale;

  set_vector_arrow_scale (scale / 100.0);

  if (vector_flag == 1)
    {
      redraw = 1;
      canvas_cb (main_w, NULL, NULL);
    }
  
}


void visi_act_on_cb(Widget widget,
                    XtPointer client_data,
                    XtPointer call_data)
{

  int which = (int) client_data;

  visi_act_on = which;

}


void posn_act_on_cb(Widget widget,
                    XtPointer client_data,
                    XtPointer call_data)
{

  int which = (int) client_data;

  posn_act_on = which;

}


/* wjq--> */
void
make_bbox_dialog(Widget parent)
{
	
	void bbox_dlg_cancel_cb(Widget, XtPointer, XtPointer);
	void bbox_ok_cb(Widget, XtPointer, XtPointer);
	void bbox_reset_cb(Widget, XtPointer, XtPointer);
	void echo_to_message_area (char *);
	/* It's rubish to do it the following way, but due to my inexperience it is the easiest.
	 * At least it works now. I hope to correct it during the next weeks				*/
	void Xmin_cb(Widget, XtPointer, XtPointer);
	void Ymin_cb(Widget, XtPointer, XtPointer);
	void Zmin_cb(Widget, XtPointer, XtPointer);
	void Xmax_cb(Widget, XtPointer, XtPointer);
	void Ymax_cb(Widget, XtPointer, XtPointer);
	void Zmax_cb(Widget, XtPointer, XtPointer);
	void bbox_determine_cb (Widget, XtPointer, XtPointer);
	void update_bbox (void);
	int n;
	char Value_str[10];

	Widget widget,rc,bbLabel;
	Widget bbox_RadioBox, radio_button;
	Widget bbox_reset_button;

	/* If no file is loaded we give a message and exit. */
	
        if (no_frames == 0)
	  {
	    echo_to_message_area("It is only possible to edit the bounding box if a file is loaded!");
	    return;
	  }
	
	/* If this window is opened, we show the bounding box.
	   Who wants to edit something he can't see. If it is not needed,
	   the user can switch off the bbox via the menu.    */
	bbox_flag = 1;
        if((widget = XtNameToWidget(nth_menu[4], "button_7")))
          XtVaSetValues(widget, XmNset, True, NULL);
    
	update_bbox();
	if(!bbox_dialog){

		XmString title;
    		XmString cancel_str;

		title=XmStringCreateLocalized("Edit Bounding Box");
    		cancel_str=XmStringCreateLocalized("Close");
		n=0;
		XtSetArg (args[n],XmNautoUnmanage, False); n++;
		XtSetArg (args[n],XmNdialogTitle,title); n++;
		XtSetArg (args[n],XmNwidth,320); n++;
		XtSetArg (args[n],XmNcancelLabelString, cancel_str); n++;
		XtSetArg (args[n],XmNentryAlignment, XmALIGNMENT_CENTER); n++;

		bbox_dialog=XmCreateMessageDialog(parent,"Bounding Box",args,n);
    
		XmStringFree(cancel_str);
		XmStringFree(title);
		
		rc=XtVaCreateManagedWidget
		  ("rc",xmRowColumnWidgetClass,bbox_dialog,
		NULL);
		
	/* Determine borders of bounding box automatically or by hand? */

		XtVaCreateManagedWidget
		  ("Determine size of bounding box...",
		   xmLabelWidgetClass, rc, 
		   XmNalignment, XmALIGNMENT_CENTER,
		   NULL);

		bbox_RadioBox = XmCreateRadioBox (rc,
		                                  "bbox_type",
		                                  NULL,
		                                  0);
		XtVaSetValues (bbox_RadioBox,
			       XmNorientation,	XmHORIZONTAL,
			       XmNalignment,	XmALIGNMENT_CENTER,
			       NULL);

		radio_button = XtVaCreateManagedWidget ("automatically  ",
		                                        xmToggleButtonGadgetClass,
		                                        bbox_RadioBox,
		                                        NULL);
		
		if (bbox_type == AUTOMATIC)
		  {
		    XtVaSetValues (radio_button, XmNset, True, NULL);
		  }

		XtAddCallback(radio_button,
		              XmNvalueChangedCallback,
		              bbox_determine_cb,
		              (XtPointer) AUTOMATIC);

		radio_button = XtVaCreateManagedWidget ("from file",
		                                        xmToggleButtonGadgetClass,
		                                        bbox_RadioBox,
		                                        NULL);

		if (bbox_type == FROM_FILE)
		  {
		    XtVaSetValues (radio_button, XmNset, True, NULL);
		  }

		XtAddCallback(radio_button,
		              XmNvalueChangedCallback,
		              bbox_determine_cb,
		              (XtPointer) FROM_FILE);

		XtManageChild (bbox_RadioBox); 
		
		XtManageChild (XmCreateSeparator (rc, "sep", NULL, 0));

	/* Here we enter the borders of the bounding box */

		bbMin[0] = XmCreateRowColumn (rc, "bbMin",  NULL, 0);

		XtVaSetValues (bbMin[0], /*XmNpacking, XmPACK_COLUMN,  */
			XmNnumColumns,		6,
			XmNorientation,		XmHORIZONTAL,
			XmNisAligned,		True,
			XmNentryAlignment, 	XmALIGNMENT_END,
		NULL);
		
	/* Xmin starts here */	
		
		bbLabel = XtVaCreateManagedWidget
		      ("Xmin:",
		       xmLabelWidgetClass,bbMin[0],
		NULL);
		XtManageChild(bbLabel);

		bbMin[1]=XtVaCreateManagedWidget("Xmin",xmTextFieldWidgetClass, bbMin[0], NULL);
		sprintf(Value_str,"%.2f",bbox_by_hand.x_min);
		XtVaSetValues(bbMin[1],
			XmNvalue, Value_str,
			XmNcolumns, 5,
		NULL);
		XtManageChild(bbMin[1]);
		
		XtAddCallback(bbMin[1],XmNvalueChangedCallback,Xmin_cb,NULL);
		
	/* Ymin starts here */	
		
		bbLabel = XtVaCreateManagedWidget
		      (" Ymin:",
		       xmLabelWidgetClass,bbMin[0],
		NULL);
		XtManageChild(bbLabel);

		bbMin[2]=XtVaCreateManagedWidget("Ymin",xmTextFieldWidgetClass, bbMin[0], NULL);
		sprintf(Value_str,"%.2f",bbox_by_hand.y_min);
		XtVaSetValues(bbMin[2],
			XmNvalue, Value_str,
			XmNcolumns, 5,
		NULL);
		XtManageChild(bbMin[2]);
		
		XtAddCallback(bbMin[2],XmNvalueChangedCallback,Ymin_cb,NULL);


	/* Zmin starts here */	
		
		bbLabel = XtVaCreateManagedWidget
		      (" Zmin:",
		       xmLabelWidgetClass,bbMin[0],
		NULL);
		XtManageChild(bbLabel);

		bbMin[3]=XtVaCreateManagedWidget("Zmin",xmTextFieldWidgetClass, bbMin[0], NULL);
		sprintf(Value_str,"%.2f",bbox_by_hand.z_min);
		XtVaSetValues(bbMin[3],
			XmNvalue, Value_str,
			XmNcolumns, 5,
		NULL);
		XtManageChild(bbMin[3]);
		
		XtAddCallback(bbMin[3],XmNvalueChangedCallback,Zmin_cb,NULL);

		XtManageChild(bbMin[0]);

	/* Here the entry of max values starts */
		
		bbMax[0] = XmCreateRowColumn (rc, "bbMax",  NULL, 0);

		XtVaSetValues (bbMax[0], /*XmNpacking, XmPACK_COLUMN,  */
			XmNnumColumns,		6,
			XmNorientation,		XmHORIZONTAL,
			XmNisAligned,		True,
			XmNentryAlignment, 	XmALIGNMENT_END,
		NULL);
		
	/* Xmax starts here */	
		
		bbLabel = XtVaCreateManagedWidget
		      ("Xmax:",
		       xmLabelWidgetClass,bbMax[0],
		NULL);
		XtManageChild(bbLabel);

		bbMax[1]=XtVaCreateManagedWidget("Xmax",xmTextFieldWidgetClass, bbMax[0], NULL);
		sprintf(Value_str,"%.2f",bbox_by_hand.x_max);
		XtVaSetValues(bbMax[1],
			XmNvalue, Value_str,
			XmNcolumns, 5,
		NULL);
		XtManageChild(bbMax[1]);
		
		XtAddCallback(bbMax[1],XmNvalueChangedCallback,Xmax_cb,NULL);
		
	/* Ymax starts here */	
		
		bbLabel = XtVaCreateManagedWidget
		      (" Ymax:",
		       xmLabelWidgetClass,bbMax[0],
		NULL);
		XtManageChild(bbLabel);

		bbMax[2]=XtVaCreateManagedWidget("Ymax",xmTextFieldWidgetClass, bbMax[0], NULL);
		sprintf(Value_str,"%.2f",bbox_by_hand.y_max);
		XtVaSetValues(bbMax[2],
			XmNvalue, Value_str,
			XmNcolumns, 5,
		NULL);
		XtManageChild(bbMax[2]);
		
		XtAddCallback(bbMax[2],XmNvalueChangedCallback,Ymax_cb,NULL);


	/* Zmax starts here */	
		
		bbLabel = XtVaCreateManagedWidget
		      (" Zmax:",
		       xmLabelWidgetClass,bbMax[0],
		NULL);
		XtManageChild(bbLabel);

		bbMax[3]=XtVaCreateManagedWidget("Zmax",xmTextFieldWidgetClass, bbMax[0], NULL);
		sprintf(Value_str,"%.2f",bbox_by_hand.z_max);
		XtVaSetValues(bbMax[3],
			XmNvalue, Value_str,
			XmNcolumns, 5,
		NULL);
		XtManageChild(bbMax[3]);
		
		XtAddCallback(bbMax[3],XmNvalueChangedCallback,Zmax_cb,NULL);

		XtManageChild(bbMax[0]);

		/* Add Button to reset the bounding box to the automatically
		 * calculated values. */

		bbox_reset_button = XtVaCreateManagedWidget ("Reset",
				           xmPushButtonWidgetClass, bbox_dialog, NULL);
    		XtAddCallback(bbox_reset_button, XmNactivateCallback,
			      bbox_reset_cb, NULL);
    
		/* We use the Cancel-Button to close the dialog... */

		XtAddCallback(bbox_dialog, XmNcancelCallback,
			      bbox_dlg_cancel_cb,NULL);

		/* The OK-Button isn't used in this dialog and help is not yet available */
    
		XtUnmanageChild
		      (XmMessageBoxGetChild(bbox_dialog, XmDIALOG_OK_BUTTON));

		XtUnmanageChild
		      (XmMessageBoxGetChild(bbox_dialog, XmDIALOG_HELP_BUTTON));

  	}
  
        XtAddCallback (bbox_dialog, XmNmapCallback, place_dialog_cb, NULL);

	XtManageChild(bbox_dialog);
}

void
bbox_dlg_cancel_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{

  XtDestroyWidget(bbox_dialog);
  bbox_dialog=NULL;

}

void
bbox_ok_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{

  void update_bbox_dialog(void);
  char *line=NULL;

  XmSelectionBoxCallbackStruct *cbs =
    (XmSelectionBoxCallbackStruct *) call_data;

  update_bbox_dialog();
  XmStringGetLtoR (cbs->value, XmFONTLIST_DEFAULT_TAG, &line);

}

void
bbox_reset_cb (Widget widget, XtPointer client_data, XtPointer call_data)
{
  void update_bbox (void);
  void update_bbox_dialog(void);
  
  bbox_by_hand.x_max = 0.0;
  bbox_by_hand.y_max = 0.0;
  bbox_by_hand.z_max = 0.0;
                           
  bbox_by_hand.x_min = 0.0;
  bbox_by_hand.y_min = 0.0;
  bbox_by_hand.z_min = 0.0;

  update_bbox_dialog ();
}


void
Xmin_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{
  void canvas_cb(Widget, XtPointer, XtPointer);
  void update_bbox (void);
  char* Value_str;

  XtVaGetValues(widget,XmNvalue,&Value_str,NULL);
  bbox_by_hand.x_min = strtod(Value_str,NULL);
  update_bbox();
  redraw = 1;
  canvas_cb(canvas,NULL,NULL);
}

void
Ymin_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{
  void canvas_cb(Widget, XtPointer, XtPointer);
  void update_bbox (void);
  char* Value_str;

  XtVaGetValues(widget,XmNvalue,&Value_str,NULL);
  bbox_by_hand.y_min = strtod(Value_str,NULL);
  update_bbox();
  redraw = 1;
  canvas_cb(canvas,NULL,NULL);
}

void
Zmin_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{
  void canvas_cb(Widget, XtPointer, XtPointer);
  void update_bbox (void);
  char* Value_str;

  XtVaGetValues(widget,XmNvalue,&Value_str,NULL);
  bbox_by_hand.z_min = strtod(Value_str,NULL);
  update_bbox();
  redraw = 1;
  canvas_cb(canvas,NULL,NULL);
}

void
Xmax_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{
  void canvas_cb(Widget, XtPointer, XtPointer);
  void update_bbox (void);
  char* Value_str;

  XtVaGetValues(widget,XmNvalue,&Value_str,NULL);
  bbox_by_hand.x_max = strtod(Value_str,NULL);
  update_bbox();
  redraw = 1;
  canvas_cb(canvas,NULL,NULL);
}
void
Ymax_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{
  void canvas_cb(Widget, XtPointer, XtPointer);
  void update_bbox (void);
  char* Value_str;

  XtVaGetValues(widget,XmNvalue,&Value_str,NULL);
  bbox_by_hand.y_max = strtod(Value_str,NULL);
  update_bbox();
  redraw = 1;
  canvas_cb(canvas,NULL,NULL);
}

void
Zmax_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{
  void canvas_cb(Widget, XtPointer, XtPointer);
  void update_bbox (void);
  char* Value_str;

  XtVaGetValues(widget,XmNvalue,&Value_str,NULL);
  bbox_by_hand.z_max = strtod(Value_str,NULL);
  update_bbox();
  redraw = 1;
  canvas_cb(canvas,NULL,NULL);
}

void
bbox_determine_cb (Widget widget,
               XtPointer client_data,
               XtPointer call_data)
{

  void update_bbox (void);
  void canvas_cb (Widget, XtPointer, XtPointer);

  bbox_type = (enum bbox_type) client_data;

  update_bbox();
  redraw = 1;
  canvas_cb (canvas, NULL, NULL);

}

void update_bbox_dialog(void){
	
	char Value_str[10];

	if (bbox_dialog != NULL){
		sprintf(Value_str,"%.2f",bbox_by_hand.x_min);
		XtVaSetValues(bbMin[1],
			XmNvalue, Value_str,
			XmNcolumns, 5,
		NULL);
		sprintf(Value_str,"%.2f",bbox_by_hand.y_min);
		XtVaSetValues(bbMin[2],
			XmNvalue, Value_str,
			XmNcolumns, 5,
		NULL);
		sprintf(Value_str,"%.2f",bbox_by_hand.z_min);
		XtVaSetValues(bbMin[3],
			XmNvalue, Value_str,
			XmNcolumns, 5,
		NULL);
		sprintf(Value_str,"%.2f",bbox_by_hand.x_max);
		XtVaSetValues(bbMax[1],
			XmNvalue, Value_str,
			XmNcolumns, 5,
		NULL);
		sprintf(Value_str,"%.2f",bbox_by_hand.y_max);
		XtVaSetValues(bbMax[2],
			XmNvalue, Value_str,
			XmNcolumns, 5,
		NULL);
		sprintf(Value_str,"%.2f",bbox_by_hand.z_max);
		XtVaSetValues(bbMax[3],
			XmNvalue, Value_str,
			XmNcolumns, 5,
		NULL);
	}
}
/* <--wjq */

void
make_edit_element_props_dialog(Widget parent)
{
  int get_no_elements(void);
/* aro--> */
  void edit_elements_apply_cb(Widget, XtPointer, XtPointer);
  void edit_elements_cancel_cb(Widget, XtPointer, XtPointer);
  void edit_elements_discard_cb(Widget, XtPointer, XtPointer);
  void edit_elements_record_id_cb(Widget, XtPointer, XtPointer);
  void edit_elements_save_cb(Widget, XtPointer, XtPointer);
  void free_widget_list_memory_cb(Widget, XtPointer, XtPointer);
  void revert_to_defaults_cb(Widget, XtPointer, XtPointer);
  void show_elements_cb(Widget, XtPointer, XtPointer);
  void update_edit_element_props_dialog();

  Dimension dimension_width, dimension_height;
  int width;
  Widget v_scroll, temp_widget, radio_box;
  Widget header_label[6];
  WidgetList rc_wlist;
  int i, n;

  /* aro - Moved edit_element_props_dialog to globabls.h so it can be used in
     update_some_dialogs() */
  Widget scrolled_w, row_column[2];
  XmString title, current, all, custom;

  if (edit_element_props_dialog == NULL)
    {
      /* aro - make sure head pointer is NULL at beginning */
      widgets_changed.head = NULL;

      /* aro - changed from "View edit element properties" */
      title = XmStringCreateLocalized("Edit element properties");
      
      n = 0;
      XtSetArg (args[n], XmNautoUnmanage, False); n++;
      XtSetArg (args[n], XmNdialogTitle, title);  n++;

/* aro--> */
      /* Set default button to none, so user can apply changes 
         simply by pressing <Enter> */
      XtSetArg (args[n], XmNdefaultButtonType, XmDIALOG_NONE);  n++;
/* <--aro */

      edit_element_props_dialog =
        XmCreateMessageDialog (parent, "edit_elements", args, n);

      scrolled_w = XtVaCreateManagedWidget ("frame",
                                            xmScrolledWindowWidgetClass,
                                            edit_element_props_dialog,
                                            XmNscrollingPolicy, XmAUTOMATIC,
                                            NULL);

      row_column[0] = XtVaCreateManagedWidget ("rc_element_props",
                                               xmRowColumnWidgetClass,
                                               scrolled_w,
                                               NULL);

/* aro--> */
      /* Add headings for element property fields;
         Need to create temporary TextField widget first so we can get 
         appropiate XmNwidth for each field as it will appear onscreen, 
         and use these values to set the XmNwidth of the widgets for 
         the Labels used as headers, then destroy temp widget. Also 
         create another label to display when no file is loaded and no 
         elements are being displayed. */
       
      row_column[1] = XtVaCreateManagedWidget ("rc",
                                               xmRowColumnWidgetClass,
                                               row_column[0],
                                               XmNorientation, XmHORIZONTAL,
                                               NULL);

      temp_widget = XtVaCreateWidget("temp",
                                     xmTextFieldWidgetClass,
                                     edit_element_props_dialog,
                                     XmNcolumns, 2,
                                     NULL);
      XtVaGetValues(temp_widget, XmNwidth, &dimension_width, NULL);
      XtVaGetValues(temp_widget, XmNheight, &dimension_height, NULL);

      /* We want to be able to see approximately 10 rows in scrolled_w,
         so taking into account the spacing between rows, we multiply 
         dimension_height by 15 */
      XtVaSetValues(scrolled_w, XmNheight, dimension_height * 15, NULL);
      header_label[0] = XtVaCreateWidget ("elem", 
                                          xmLabelWidgetClass,
                                          row_column[1],
                                          XmNrecomputeSize, False,
                                          XmNwidth, dimension_width,
                                          NULL);
      
      XtVaSetValues(temp_widget, XmNcolumns, 7, NULL);
      XtVaGetValues(temp_widget, XmNwidth, &dimension_width, NULL);
      header_label[1] = XtVaCreateWidget ("atom mass",
                                          xmLabelWidgetClass,
                                          row_column[1],
                                          XmNrecomputeSize, False,
                                          XmNwidth, dimension_width,
                                          NULL);

      XtVaSetValues(temp_widget, XmNcolumns, 20, NULL);
      XtVaGetValues(temp_widget, XmNwidth, &dimension_width, NULL);
      header_label[2] = XtVaCreateWidget ("xmakemol color",
                                          xmLabelWidgetClass,
                                          row_column[1],
                                          XmNrecomputeSize, False,
                                          XmNwidth, dimension_width,
                                          NULL);
      
      XtVaSetValues(temp_widget, XmNcolumns, 5, NULL);
      XtVaGetValues(temp_widget, XmNwidth, &dimension_width, NULL);
      header_label[3] = XtVaCreateWidget ("cov rad",
                                          xmLabelWidgetClass,
                                          row_column[1],
                                          XmNrecomputeSize, False,
                                          XmNwidth, dimension_width,
                                          NULL);
      
      header_label[4] = XtVaCreateWidget ("vdw rad",
                               xmLabelWidgetClass,
                               row_column[1],
                               XmNrecomputeSize, False,
                               XmNwidth, dimension_width,
                               NULL);

      header_label[5] = XtVaCreateWidget("No file loaded.  To see all "
                                         "elements, click \"Show All\"",
                                         xmLabelWidgetClass,
                                         row_column[1],
                                         NULL);
      XtDestroyWidget(temp_widget);
/* <--aro */

      for(i = 0; i < get_no_elements(); i++)
        {
          
          char color[20], mass[8], cov_rad[6], vdw_rad[6];

          if (strlen (element[i].label) > 0)
            {
              /* aro - each RowColumn containing property TextFields is not 
                 managed right away because first we check if 
                 element[i].any_atoms or 
                 elem_props_dialog_state.show_all_elements is true.  If 
                 either is true, then we manage row_column later in 
                 update_edit_element_props_dialog */
        
              row_column[1] = XtVaCreateWidget ("rc",
                                                xmRowColumnWidgetClass,
                                                row_column[0],
                                                XmNorientation, XmHORIZONTAL,
                                                NULL);

              XtVaCreateManagedWidget ("label",
                                       xmTextFieldWidgetClass,
                                       row_column[1],
                                       XmNvalue, element[i].label,
                                       XmNeditable, False,
                                       XmNcolumns, 2,
                                       NULL);

              sprintf(mass, "%7.3f", element[i].mass);

              XtVaCreateManagedWidget ("label",
                                       xmTextFieldWidgetClass,
                                       row_column[1],
                                       XmNvalue, mass,
                                       XmNeditable, False,
                                       XmNcolumns, strlen(mass),
                                       NULL);

              sprintf(color, "%s", element[i].color);

              /* aro - We assign temp_widget the id of the TextField widgets 
                 so we can add callbacks, and we change XmNeditable to True 
                 so the fields are editable. We also give widgets unique names 
                 that are descriptive of the element property each displays
                 so we will be able to access them using XtNameToWidget in
                 other functions */

              temp_widget = XtVaCreateManagedWidget ("color", 
                                       xmTextFieldWidgetClass,
                                       row_column[1],
                                       XmNvalue, color,
                                       XmNeditable, True,
                                       XmNcolumns, 20,
                                       XmNuserData, i,
                                       NULL);
/* aro--> */
              XtAddCallback(temp_widget, XmNvalueChangedCallback,
                              edit_elements_record_id_cb, NULL);
              XtAddCallback(temp_widget, XmNactivateCallback,
                            edit_elements_apply_cb, NULL);
/* <--aro */

              sprintf(cov_rad, "%5.3f", element[i].cov_rad);

              temp_widget = XtVaCreateManagedWidget ("cov_rad",
                                       xmTextFieldWidgetClass,
                                       row_column[1],
                                       XmNvalue, cov_rad,
                                       XmNeditable, True,
                                       XmNcolumns, strlen(cov_rad),
                                       XmNuserData, i,  
                                       NULL);
/* aro--> */          
              XtAddCallback(temp_widget, XmNvalueChangedCallback,
                            edit_elements_record_id_cb, NULL);
              XtAddCallback(temp_widget, XmNactivateCallback,
                            edit_elements_apply_cb, NULL);
/* <--aro */
              sprintf(vdw_rad, "%5.3f", element[i].vdw_rad);

              temp_widget = XtVaCreateManagedWidget ("vdw_rad",
                                       xmTextFieldWidgetClass,
                                       row_column[1],
                                       XmNvalue, vdw_rad,
                                       XmNeditable, True,
                                       XmNcolumns, strlen(vdw_rad),
                                       XmNuserData, i,
                                       NULL);
/* aro--> */
              XtAddCallback(temp_widget, XmNvalueChangedCallback, 
                            edit_elements_record_id_cb, NULL);
              XtAddCallback(temp_widget, XmNactivateCallback,
                            edit_elements_apply_cb, NULL);
/* <--aro */         
            }
        }

/* aro--> */
      /* I could not figure out how, if it is possible, to have a scrolled 
         window set its own width dimension such that no horizontal scroll 
         bar would be required to see the entire width of the scrolled 
         window's contents. So below we roughly approximate the width 
         required by adding up the widths of each TextField, each 
         TextField's margin, the spacing in the RowColumn between each 
         TextField, etc, plus a small amount more for padding, and then we 
         set the XmNwidth of the scrolled window to this value. */

      XtVaGetValues(row_column[1], XmNchildren, &rc_wlist, NULL);
      if(rc_wlist)
        {
          /* rc_wlist corresponds to header labels:
             rc_wlist[0] = element abbreviation label
             rc_wlist[1] = element mass label
             rc_wlist[2] = element color in XMakemol label
             rc_wlist[3] = element covalent radius label
             rc_wlist[4] = element van der Waals radius label */

          XtVaGetValues(rc_wlist[0], XmNwidth, &dimension_width, NULL);
          width = (int)dimension_width;
          
          XtVaGetValues(rc_wlist[1], XmNwidth, &dimension_width, NULL);
          width += (int)dimension_width;
          
          XtVaGetValues(rc_wlist[2], XmNwidth, &dimension_width, NULL);
          width += (int)dimension_width;

          XtVaGetValues(rc_wlist[3], XmNwidth, &dimension_width, NULL);
          width += (int)dimension_width;

          XtVaGetValues(rc_wlist[4], XmNwidth, &dimension_width, NULL);
          width += (int)dimension_width;

          XtVaGetValues(row_column[1], XmNmarginWidth, &dimension_width, NULL);
          width += 2 * (int)dimension_width;/* take into account two margins */

          XtVaGetValues(row_column[1], XmNspacing, &dimension_width, NULL);
          width += 6 * (int)dimension_width;/* six spaces in a row */
          
          XtVaGetValues(scrolled_w, XmNshadowThickness, 
                        &dimension_width, NULL);
          width += 2 * (int)dimension_width;/* take into account two shadows */
          
          XtVaGetValues(scrolled_w, XmNmarginWidth, &dimension_width, NULL);
          width += (int)dimension_width;

          XtVaGetValues(scrolled_w, XmNwidth, &dimension_width, NULL);

          XtVaGetValues(scrolled_w, XmNspacing, &dimension_width, NULL);
          width += (int)dimension_width;

          XtVaGetValues(scrolled_w, XmNverticalScrollBar, &v_scroll, NULL);
          XtVaGetValues(v_scroll, XmNwidth, &dimension_width, NULL);
          width += (int)dimension_width;
         
          /* extra padding to ensure no horizontal scroll bar needed in 
             scrolled window */
          width += 50; 

          XtVaSetValues(edit_element_props_dialog, XmNwidth, width, NULL);
        }

/* <--aro */

/* aro--> */
      /* Here we create a radio box at the top of the dialog
         to select which elements are shown, and 5 buttons at the
         bottom of dialog:
           Save Customizations, 
           Apply Customizations, 
           Delete All Customizations, 
           Revert to Saved Customizations,
           Cancel (we don't use the default cancel button
                   because it appears in different dimensions) */
      
      current = XmStringCreateLocalized("Current Elements");
      all     = XmStringCreateLocalized("All Elements");
      custom  = XmStringCreateLocalized("Customized Elements");
      
      radio_box = XmCreateRadioBox(edit_element_props_dialog,"show elements",
                                   args,0);
      XtVaSetValues(radio_box,
                    XmNorientation, XmHORIZONTAL, 
                    XmNradioAlwaysOne, True,
                    NULL);
      
      temp_widget = XtVaCreateManagedWidget("button1",
                                            xmToggleButtonGadgetClass,
                                            radio_box,
                                            XmNlabelString, current,
                                            XmNset, True,
                                            NULL);
      
      XtAddCallback(temp_widget, XmNvalueChangedCallback, 
                    show_elements_cb, row_column[0]);

      temp_widget = XtVaCreateManagedWidget("button2",
                                            xmToggleButtonGadgetClass,
                                            radio_box,
                                            XmNlabelString, all,
                                            NULL);
      
      XtAddCallback(temp_widget, XmNvalueChangedCallback, 
                    show_elements_cb, row_column[0]);
      
      temp_widget = XtVaCreateManagedWidget("button3",
                                            xmToggleButtonGadgetClass,
                                            radio_box,
                                            XmNlabelString, custom,
                                            NULL);
      
      XtAddCallback(temp_widget, XmNvalueChangedCallback, 
                    show_elements_cb, row_column[0]);

      XtManageChild(radio_box);

      XmStringFree(current);
      XmStringFree(all);
      XmStringFree(custom);

      title = XmStringCreateLocalized("Save Customizations");
      temp_widget = XtVaCreateManagedWidget("button",
                                            xmPushButtonWidgetClass,
                                            edit_element_props_dialog,      
                                            XmNlabelString, title,
                                            NULL);
      XtAddCallback (temp_widget, XmNactivateCallback,
      edit_elements_save_cb, NULL);

      title = XmStringCreateLocalized("Apply Customizations");
      temp_widget = XtVaCreateManagedWidget("button",
                                            xmPushButtonWidgetClass,
                                            edit_element_props_dialog,
                                            XmNlabelString, title,
                                            NULL);
      XtAddCallback (temp_widget, XmNactivateCallback,
                     edit_elements_apply_cb, NULL);

      title = XmStringCreateLocalized("Delete All Customizations");
      temp_widget = XtVaCreateManagedWidget("button",
                                            xmPushButtonWidgetClass,
                                            edit_element_props_dialog,
                                            XmNlabelString, title,
                                            NULL);
      XtAddCallback (temp_widget, XmNactivateCallback,
                     (XtCallbackProc) revert_to_defaults_cb, 
                      row_column[0]);

      title = XmStringCreateLocalized("Revert to Saved Customizations");
      temp_widget = XtVaCreateManagedWidget("button",
                                            xmPushButtonWidgetClass,
                                            edit_element_props_dialog,
                                            XmNlabelString, title,
                                            NULL);
 
      XtAddCallback(temp_widget, XmNactivateCallback,
                    edit_elements_discard_cb, NULL);
      
      title = XmStringCreateLocalized("Cancel");
      temp_widget = XtVaCreateManagedWidget("button",
                                            xmPushButtonWidgetClass,
                                            edit_element_props_dialog,
                                            XmNlabelString, title,
                                            NULL);
      
      /* Unmanage dialog when user clicks "Cancel" */
      XtAddCallback(temp_widget, XmNactivateCallback,
                    edit_elements_cancel_cb, NULL);
      
      /* When edit_element_props_dialog is unmapped (which should happen 
         when it is unmanaged), call edit_elements_discard_cb to discard 
         changes and free memory used by widgets_changed list */
      XtAddCallback(edit_element_props_dialog, XmNunmapCallback,
                    edit_elements_discard_cb, NULL);

/* <--aro */

      /* Don't provide any help */
      
      XtUnmanageChild
        (XmMessageBoxGetChild(edit_element_props_dialog,
        XmDIALOG_HELP_BUTTON));

      /* Don't need OK button */
      XtUnmanageChild
        (XmMessageBoxGetChild(edit_element_props_dialog,
        XmDIALOG_OK_BUTTON));

      /* aro - Don't need Cancel button */
      XtUnmanageChild
        (XmMessageBoxGetChild(edit_element_props_dialog,
        XmDIALOG_CANCEL_BUTTON));

    }
  
  /* aro - We need to check which elements are currently loaded and 
     manage and unmanage the rows in the elements dialog accordingly */
  update_edit_element_props_dialog();
  
  XtAddCallback (edit_element_props_dialog, XmNmapCallback, place_dialog_cb, NULL);

  XtManageChild(edit_element_props_dialog);

}

/* aro--> */
void
update_edit_element_props_dialog()
{
  /* Since edit_element_props_dialog is only managed and unmanaged,
     we need to update the elements and the header label displayed
     each time it is open. First we check if a file has been
     loaded by checking whether element[i].any_atoms is true for
     all elements. If no file loaded, and user is not showing all
     elements, then a label indicating that no file is loaded will
     appear.  Otherwise the header containing labels for each property
     field appears.  Next we check whether a row containing an element's
     properties should be visible based on whether the user is showing all
     customized, or current elements, and if current elements then whether 
     an element has any atoms loaded */

  int get_no_elements();

  int i, numChildren;
  Widget temp_w, rc;
  WidgetList w_list, sub_w_list;

  temp_w = XtNameToWidget(edit_element_props_dialog, "frame");
  XtVaGetValues(temp_w, XmNworkWindow, &rc, NULL);

  /* Temporarily hide row_column[0] while we make changes */
  XtUnmanageChild(rc);

  /* Get children and number of children of row_column[0] */
  XtVaGetValues(rc, XmNchildren, &w_list, NULL);
  XtVaGetValues(rc, XmNnumChildren, &numChildren, NULL);

  /* Get children of first row child of row_column[0] (header labels) */
  XtVaGetValues(w_list[0], XmNchildren, &sub_w_list, NULL);

  for(i = 0; elem_props_dialog_state.file_loaded == False && 
        i < get_no_elements(); i++)
    {
        if(element[i].any_atoms == True)
        elem_props_dialog_state.file_loaded = True;
    }
  
  /* Doesn't matter if we try to manage a label already managed, 
     XtManageChild will just ignore it */

  /* If file is not loaded and we're not showing all or customized 
     elements, display no-file-loaded label. */
  if(elem_props_dialog_state.file_loaded == False && 
     elem_props_dialog_state.show_all_elements == False &&
     elem_props_dialog_state.show_customized_elements == False)
    {
      XtUnmanageChildren(sub_w_list, 5);/* element properties headers */
      XtManageChild(sub_w_list[5]);/* no-file-loaded label */
    }
  else
    {
      XtUnmanageChild(sub_w_list[5]);/* no-file-loaded label */
      XtManageChildren(sub_w_list, 5);/* element properties headers */
    }
  
  /* Start with i = 1 becuase w_list[0] is header label. */
  for(i = 1; i < numChildren; i++)
    {
      /* Display appropiate elements */
      if(elem_props_dialog_state.show_all_elements 
         ||
         (!elem_props_dialog_state.show_customized_elements && 
          element[i-1].any_atoms) 
         ||
         (elem_props_dialog_state.show_customized_elements && 
          element[i-1].customized))
        XtManageChild(w_list[i]);
      else
        XtUnmanageChild(w_list[i]);
    }

  /* Make row_column[0] visible again */
  XtManageChild(rc);
}


void
edit_elements_record_id_cb(Widget widget, XtPointer client_data, 
                           XtPointer call_data)
{
  /* We record the id of the widget that called this callback in 
     a linked list */

  if(widgets_changed.head == NULL)/* if NULL then it's first call 
                                     to edit_elements_record_cb */
    { 
      widgets_changed.head = malloc(sizeof(struct widgets));
      widgets_changed.curr = widgets_changed.head;
      widgets_changed.curr->next = NULL;
    }
  else/* if not NULL then edit_elements_record_cb has already been called */
    { 
      widgets_changed.curr->next = malloc(sizeof(struct widgets)); 
      widgets_changed.curr = widgets_changed.curr->next;
      widgets_changed.curr->next = NULL;
    }
  widgets_changed.curr->id = widget;/* add this widget's id to changed 
                                       widgets list */

  /* Once we've recorded a widget's id, we don't need to get it again */
  XtRemoveCallback(widget, XmNvalueChangedCallback, 
                   (XtCallbackProc) edit_elements_record_id_cb, NULL);

}

void
edit_elements_apply_cb(Widget widget, XtPointer client_data, 
                       XtPointer call_data)
{
  /* To apply changes made in edit_element_props_dialog, we traverse
     the widgets_changed linked list, and for each node we check whether
     it is the widget id of a "color", "cov_rad", or "vdw_rad" property
     TextField.  We then change the property of the corresponding
     element and make the effects take place immediately by calling
     change_frame.  Lastly we free the memory used by widgets_changed list */

  void canvas_cb(Widget, XtPointer, XtPointer);
  void change_frame(int, Boolean, Boolean);
  void free_widget_list_memory_cb(Widget, XtPointer, XtPointer);
  void update_bond_matrix(Boolean);

  int index, i, j;
  double cov_rad, vdw_rad;
  char *color, *num_string, *widget_name;

  if(widgets_changed.head != NULL)/* if head is not NULL then some 
                                     value(s) changed */
    {
      for(widgets_changed.curr = widgets_changed.head; 
          widgets_changed.curr != NULL; 
          widgets_changed.curr = widgets_changed.curr->next)
        {
          widget_name = XtName(widgets_changed.curr->id);

          /* get index of element to update */
          XtVaGetValues(widgets_changed.curr->id, XmNuserData, &index, NULL);

          /* if field edited was color field, do this */
          if(strcmp(widget_name, "color") == 0)
            {
              XtVaGetValues(widgets_changed.curr->id, XmNvalue, &color, NULL);

              /* We want to save colors without whitespace (e.g. "DarkGreen" 
                 instead of "Dark Green") because we read them in using 
                 whitespace delimited fields, so we go through character 
                 array and remove whitespace */
              for(i = 0; color[i] != '\0'; i++)
                {
                  if(color[i] == ' ')
                    for(j = i; color[j] != '\0'; j++)
                      color[j] = color[j+1];
                }

              /* Update text field with new string */
              XtVaSetValues(widgets_changed.curr->id, XmNvalue, color, NULL);

              strcpy(element[index].color, color); 
              element[index].color_allocated = 0;
              element[index].customized = 1;
            }

          /* if field edited was covalent radius field, do this */
          else if(strcmp(widget_name, "cov_rad") == 0)
            {
              XtVaGetValues(widgets_changed.curr->id, XmNvalue, 
                            &num_string, NULL);

              cov_rad = atof(num_string);/* convert string to double */
              
              element[index].cov_rad = cov_rad;
              element[index].customized = 1;

              update_bond_matrix(False);
            }

          /* if field edited was van der Waals radius field, do this */
          else if(strcmp(widget_name, "vdw_rad") == 0)
            {
              XtVaGetValues(widgets_changed.curr->id, XmNvalue, 
                            &num_string, NULL);

              vdw_rad = atof(num_string);/* convert string to double */

              element[index].vdw_rad = vdw_rad;
              element[index].customized = 1;

              update_bond_matrix(False);
            }

          /* Once we've applied changes corresponding to widget,
             we need to add valueChangedCallback back so if
             user changes value again the widget's id will
             be recorded */

          XtAddCallback(widgets_changed.curr->id, XmNvalueChangedCallback,
                        (XtCallbackProc) edit_elements_record_id_cb, NULL);
        }
      /* update display */
      change_frame(frame_no, False, False);

      /* free widget_list memory */
      free_widget_list_memory_cb(widget, NULL, NULL);
    }
  update_edit_element_props_dialog();
}

void
edit_elements_cancel_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{
  XtUnmanageChild(edit_element_props_dialog);
}

void
edit_elements_save_cb(Widget widget, XtPointer client_data, 
                      XtPointer call_data)
{
  /* To save all changes user has made in edit_element_props_dialog,
     we first call edit_elements_apply_cb so the changes will take effect
     in main window and element array immediately, then open 
     .xmakemol.elements in user's home dir.  For each element that has 
     been customized, we record all of its properties in .xmakemol.elements */
  
  int get_no_elements();

  int i;
  char *users_home_dir_path, *user_elements_file_name, temp[80];
  FILE * out_file;
  
  edit_elements_apply_cb(widget, client_data, call_data);

  users_home_dir_path = getenv("HOME");
  
  /* We're not supposed to modify char * pointer returned by getenv,
     so we copy it to a character array */
  strcpy(temp, users_home_dir_path);

  user_elements_file_name = strcat(temp, "/.xmakemol.elements");
  out_file = fopen(user_elements_file_name, "w");

  /* If element is customized, save properties in .xmakemol.elements */
  for(i = 0; i < get_no_elements(); i++)
    {
      if(element[i].customized == 1)
        { 
          fprintf(out_file, "%2s%9.3f   %-12s %5.3f   %5.3f\n",
                  element[i].label,
                  element[i].mass,
                  element[i].color,
                  element[i].cov_rad,
                  element[i].vdw_rad);
        }
    }
  fclose(out_file);

  update_edit_element_props_dialog();
}

void
edit_elements_discard_cb(Widget widget, XtPointer client_data, 
                         XtPointer call_data)
{
  /* To discard any changes that were made in edit_element_props_dialog before
     they have been saved, free memory allocated for element 
     array, read in elements file, then read in .xmakemol.elements file, and 
     call change_frame to make element colors appear as elements array now 
     indicates.  We also free memory used by widgets_changed list.

     *Note* this only discards changes made after the last time the user  
     selected "Save Customizations" or "Delete All Customizations" in
     edit_element_props_dialog */
  
  void change_frame(int, Boolean, Boolean);
  void free_widget_list_memory_cb(Widget, XtPointer, XtPointer);
  void read_elements_file();
  void read_user_elements_file();
  void update_bond_matrix(Boolean);

  Boolean wasManaged = False;
  char cov_rad[6], vdw_rad[6];
  int i, numChildren;
  Widget rc, scrolled_window;
  WidgetList sub_w_list, w_list;

  free(element);
  element = NULL;
  read_elements_file();/* refresh element array with original elements file */
  read_user_elements_file();/* update element array with entries in 
                               .xmakemol.elements */
  update_bond_matrix(False);
  change_frame(frame_no, False, False);

  /* Work down through widget hierarchy in edit_element_props_dialog to get
     children and number of children of row_column[0] */
  scrolled_window = XtNameToWidget(edit_element_props_dialog, "frame");
  XtVaGetValues(scrolled_window, XmNworkWindow, &rc, NULL);
  XtVaGetValues(rc, XmNchildren, &w_list, NULL);
  XtVaGetValues(rc, XmNnumChildren, &numChildren, NULL);
  
  /* If dialog is managed, unmanage scrolled_window's workWindow while 
     making changes */
  if(XtIsManaged(edit_element_props_dialog))
    {
      wasManaged = True;
      XtUnmanageChild(rc);
    }
  
  /* w_list[0] = header labels, so we start at w_list[1] */
  for(i = 1; i < numChildren; i++)
    {
      XtVaGetValues(w_list[i], XmNchildren, &sub_w_list, NULL);

      /* Remove callbacks */
      XtRemoveCallback(sub_w_list[2], XmNvalueChangedCallback, 
                       edit_elements_record_id_cb, NULL);
      XtRemoveCallback(sub_w_list[3], XmNvalueChangedCallback, 
                       edit_elements_record_id_cb, NULL);
      XtRemoveCallback(sub_w_list[4], XmNvalueChangedCallback, 
                       edit_elements_record_id_cb, NULL);

      /* Update values */
      XtVaSetValues(sub_w_list[2], XmNvalue, element[i-1].color, NULL);
      sprintf(cov_rad, "%5.3f", element[i-1].cov_rad);
      XtVaSetValues(sub_w_list[3], XmNvalue, cov_rad, NULL);
      sprintf(vdw_rad, "%5.3f", element[i-1].vdw_rad);
      XtVaSetValues(sub_w_list[4], XmNvalue, vdw_rad, NULL);

      /* Put callbacks back in place */
      XtAddCallback(sub_w_list[2], XmNvalueChangedCallback,
                    edit_elements_record_id_cb, NULL);
      XtAddCallback(sub_w_list[3], XmNvalueChangedCallback,
                    edit_elements_record_id_cb, NULL);
      XtAddCallback(sub_w_list[4], XmNvalueChangedCallback,
                    edit_elements_record_id_cb, NULL);
    }
  /* Free memory used by widgets_changed list */
  free_widget_list_memory_cb(widget, NULL, NULL);

  /* Manage scrolled_window's workWindow if dialog was managed upon entry 
     of this function */
  if(wasManaged)
    XtManageChild(rc);

  update_edit_element_props_dialog();
}

/* The functions ask_user and response are for use in the defaults function
   to ask the user if they really want to continue, not allowing them to do 
   anything else in the application until they respond.  This code imitates 
   that in a response by Dan Heller to subject 214 in the Motif FAQ at 
   http://www.rahul.net/kenton/mfaq.html */
int
ask_user(Widget parent, char *question)
{
  void response(Widget, XtPointer, XtPointer);

  XmString message, yes, no;
  Widget msgBox;
  int answer = 2;
  extern XtAppContext app;
  
  message = XmStringCreateLocalized(question);
  msgBox = XmCreateQuestionDialog(edit_element_props_dialog, "dialog", 
                                  NULL, 0);
  yes = XmStringCreateLocalized("Yes");
  no = XmStringCreateLocalized("No");
  XtVaSetValues(msgBox,
                XmNdialogStyle,        XmDIALOG_APPLICATION_MODAL,
                XmNokLabelString,      yes,
                XmNcancelLabelString,  no,
                XmNmessageString,      message,
                NULL);
  XmStringFree(message);
  
  XtAddCallback(msgBox, XmNokCallback, response, &answer);
  XtAddCallback(msgBox, XmNcancelCallback, response, &answer);

  XtUnmanageChild(XmMessageBoxGetChild(msgBox, XmDIALOG_HELP_BUTTON));
  XtManageChild(msgBox);

  while (answer == 2 || XtAppPending(app))
    XtAppProcessEvent(app, XtIMAll);

  return answer;
}

void
response(Widget widget, XtPointer client_data, XtPointer call_data)
{
  int *answer = (int *)client_data;
  XmAnyCallbackStruct *reason = (XmAnyCallbackStruct *)call_data;

  switch (reason->reason)
    {
    case XmCR_OK:
      *answer = 1;
      break;
    case XmCR_CANCEL:
      *answer = 0;
      break;
    default:
      *answer = 0;
    return;
    }
}

void
revert_to_defaults_cb(Widget widget, XtPointer client_data, 
                      XtPointer call_data)
{
  /* To revert to default element properties, we remove .xmakemol.elements from
     user's home dir, read in elements file, update bond information,
     call change_frame to make changes take effect immediately in main window, 
     and update the properties in edit_element_props_dialog. */
  
  void change_frame(int, Boolean, Boolean);
  void read_elements_file();
  void update_bond_matrix(Boolean);

  Widget w = (Widget) client_data;
  WidgetList w_list, sub_w_list;
  char color[20], mass[8], cov_rad[6], vdw_rad[6];
  char *user_elements_file_name, temp[80];
  int i, numChildren;

  /* Make sure the user really wants to continue */
  if(ask_user(edit_element_props_dialog, 
              "Are you sure? This will delete all customizations!") == 1)
    {
      strcpy(temp, getenv("HOME"));
      user_elements_file_name = strcat(temp, "/.xmakemol.elements");
      remove(user_elements_file_name);
      
      /* Free element and set it to NULL so call to read_elements_file() will
         read in file */
      free(element);
      element = NULL;
      read_elements_file();
      
      /* Update the bond information as this may have changed from what 
         the user had defined in .xmakemol.elements */
      update_bond_matrix(False);
      
      /* Make changes appear onscreen */
      change_frame(frame_no, False, False);
      
      /* Hide row_column[0] while we make changes to 
         edit_element_props_dialog */
      XtUnmanageChild(w);
      
      /* Get children of row_column[0] and number of children (these 
         will be the rows of properties) */
      XtVaGetValues(w, XmNchildren, &w_list, NULL);
      XtVaGetValues(w, XmNnumChildren, &numChildren, NULL);
      
      /* Start loop with i = 1 because 0th child of row_column[0] is
         row_column widget containing header labels */
      for(i = 1; i < numChildren; i++)
        {             
          XtVaGetValues(w_list[i], XmNchildren, &sub_w_list, NULL);
          
          /* When updating the properties we remove the XmNvalueChangedCallback
             for those widgets that have callbacks registered, because
             we have no reason to record these widget id's in the 
             widgets_changed list (these changes don't need to be written to 
             .xmakemol.elements) */
          XtRemoveCallback(sub_w_list[2], XmNvalueChangedCallback, 
                           edit_elements_record_id_cb, NULL);
          XtRemoveCallback(sub_w_list[3], XmNvalueChangedCallback, 
                           edit_elements_record_id_cb, NULL);
          XtRemoveCallback(sub_w_list[4], XmNvalueChangedCallback, 
                           edit_elements_record_id_cb, NULL);
          
          XtVaSetValues(sub_w_list[0], XmNvalue, element[i-1].label, NULL);
          sprintf(mass, "%7.3f", element[i-1].mass);
          XtVaSetValues(sub_w_list[1], XmNvalue, mass, NULL);
          sprintf(color, "%s", element[i-1].color);
          XtVaSetValues(sub_w_list[2], XmNvalue, color, NULL);
          sprintf(cov_rad, "%5.3f", element[i-1].cov_rad);
          XtVaSetValues(sub_w_list[3], XmNvalue, cov_rad, NULL);
          sprintf(vdw_rad, "%5.3f", element[i-1].vdw_rad);
          XtVaSetValues(sub_w_list[4], XmNvalue, vdw_rad, NULL);
          
          /* Put callbacks back in place */
          XtAddCallback(sub_w_list[2], XmNvalueChangedCallback,
                        edit_elements_record_id_cb, NULL);
          XtAddCallback(sub_w_list[3], XmNvalueChangedCallback,
                        edit_elements_record_id_cb, NULL);
          XtAddCallback(sub_w_list[4], XmNvalueChangedCallback,
                        edit_elements_record_id_cb, NULL);
        }
  
      /* Unhide row_column[0] */
      XtManageChild(w);
    }
  update_edit_element_props_dialog();

}

void
show_elements_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{
  /* This function toggles whether the user sees the properties of all 
     elements, current elements, or customized elements in 
     edit_element_props_dialog */

  int i, numChildren;
  Widget w = (Widget) client_data;
  XmToggleButtonCallbackStruct *cbs = 
    (XmToggleButtonCallbackStruct *) call_data;
  WidgetList w_list, sub_w_list;
  char *button_name = XtName(widget);
  
  if(cbs->set == True)
    {
      /* Get children of row_column[0] and number of children */
      XtVaGetValues(w, XmNchildren, &w_list, NULL);
      XtVaGetValues(w, XmNnumChildren, &numChildren, NULL);
      
      /* Get children of the first row child of row_column[0] (these will be 
         header labels) */
      XtVaGetValues(w_list[0], XmNchildren, &sub_w_list, NULL);
      
      XtUnmanageChild(w);/* temporarily hide row_column[0] while we make 
                            changes */
      
      /* Change state of edit_element_props_dialog */
      if(strcmp(button_name, "button3") == 0)/* Customized Elements */
        {
          elem_props_dialog_state.show_all_elements = False;
          elem_props_dialog_state.show_customized_elements = True;

          /* sub_w_list[5] == file-not-loaded label */
          XtUnmanageChild(sub_w_list[5]);
          /* sub_w_list[0..4] == header labels */
          XtManageChildren(sub_w_list, 5);
        }
      else if(strcmp(button_name, "button1") == 0)/* Current Elements */
        {
          elem_props_dialog_state.show_all_elements = False;
          elem_props_dialog_state.show_customized_elements = False;
          
          if(elem_props_dialog_state.file_loaded == False)
            {
              /* sub_w_list[0..4] == header labels */
              XtUnmanageChildren(sub_w_list, 5);
              /* sub_w_list[5] == file-not-loaded label */
              XtManageChild(sub_w_list[5]);
            }
          else
            {
              /* sub_w_list[5] == file-not-loaded label */
              XtUnmanageChild(sub_w_list[5]);
              /* sub_w_list[0..4] == header labels */
              XtManageChildren(sub_w_list, 5);
            }
        }
      else if(strcmp(button_name, "button2") == 0)/* All Elements */
        {
          elem_props_dialog_state.show_all_elements = True;
          elem_props_dialog_state.show_customized_elements = False;
          
          /* sub_w_list[5] == file-not-loaded label */
          XtUnmanageChild(sub_w_list[5]);
          /* sub_w_list[0..4] == header labels */
          XtManageChildren(sub_w_list, 5);
        }
      else
        fprintf(stderr, "Error - edit_element_properties_dialog radio box\n");
      
      
      for(i = 1; i < numChildren; i++)
        {
          /* If (w_list[i] isn't managed, and *show_all_elements is True) or
             element[i-1] has atoms currently displayed on canvas, then manage 
             w_list[i] to make element[i-1]'s properties  visible */
          
          if(elem_props_dialog_state.show_all_elements 
             ||
             (elem_props_dialog_state.show_all_elements == False && 
              elem_props_dialog_state.show_customized_elements == False && 
              element[i-1].any_atoms) 
             ||
             (elem_props_dialog_state.show_customized_elements && 
              element[i-1].customized))
            XtManageChild(w_list[i]);
          else
            XtUnmanageChild(w_list[i]);
        }
      
      XtManageChild(w);/* changes finished, unhide row_column[0] */
    }
}

void
free_widget_list_memory_cb(Widget widget, XtPointer client_data, 
                           XtPointer call_data)
{
  /* Free the memory used by the widgets_changed linked list */

  struct widget_list temp;

  if(widgets_changed.head != NULL)/* if memory has been allocated, free it */
    {
      widgets_changed.curr = widgets_changed.head;
      while(widgets_changed.curr != NULL)
        {
          temp.curr = widgets_changed.curr->next;
          free(widgets_changed.curr);
          widgets_changed.curr = temp.curr;
        }
      widgets_changed.head = NULL;
    }

}
/* <--aro */

#ifdef GL
void
make_gl_render_dialog (Widget parent)
{
  Boolean render_using_gl_p (void);
  int get_gl_render_type (void);
  void gl_eye_cb (Widget, XtPointer, XtPointer);
  void gl_fov_cb (Widget, XtPointer, XtPointer);
  int gl_get_lighting (void);
  int gl_get_shininess (void);
  int gl_get_specular_lighting (void);
  void gl_lighting_cb (Widget, XtPointer, XtPointer);
  void gl_specular_lighting_cb (Widget, XtPointer, XtPointer);
  void gl_no_atom_segments_cb (Widget, XtPointer, XtPointer);
  void gl_no_bond_segments_cb (Widget, XtPointer, XtPointer);
  void gl_render_cb (Widget, XtPointer, XtPointer);
  void gl_render_type_button_cb (Widget, XtPointer, XtPointer);
  void gl_shininess_scale_cb (Widget, XtPointer, XtPointer);
  double get_gl_eye (void);
  double get_gl_fov (void);
  int get_gl_no_atom_segments (void);
  int get_gl_no_bond_segments (void);
  void set_gl_render_stereo(enum render_stereo_types); /* aro */
  enum render_stereo_types get_gl_render_stereo(void); /* aro */
  void render_stereo_cb(Widget, XtPointer, XtPointer); /* aro */
  void gl_sep_cb (Widget, XtPointer, XtPointer);       /* aro */
  int get_gl_sep (void);                               /* aro */
  int n;

  Widget gl_render, gl_lighting, row_column, radio_box, radio_button, 
    gl_stereo, hor_row_column;

  Widget no_atom_segments_scale, no_bond_segments_scale, shininess_scale,
    fov_scale, eye_scale;

  Widget seperation_scale; /* aro */
  /* aro - moved Widget gl_render_dialog to globals.h so we can access it
     in gl_funcs.c, specifically to change value of eye_scale slider */

  XmString xmstring;

  if (gl_render_dialog == NULL)
    {
      xmstring = XmStringCreateLocalized ("Edit GL rendering...");

      n = 0;
      XtSetArg (args[n], XmNautoUnmanage, False); n++;
      XtSetArg (args[n], XmNdialogTitle, xmstring);  n++;

      gl_render_dialog =
        XmCreateMessageDialog (parent, "gl_render", args, n);

      row_column = XtVaCreateManagedWidget
        ("row_column",
         xmRowColumnWidgetClass,
         gl_render_dialog,
         NULL);

      gl_render = XtVaCreateManagedWidget
        ("Render using GL",
         xmToggleButtonGadgetClass,
         row_column,
         NULL);

      XtVaSetValues (gl_render, XmNset, render_using_gl_p (), NULL);
      XtAddCallback (gl_render, XmNvalueChangedCallback, gl_render_cb, NULL);
/* aro--> */
      XtVaCreateManagedWidget
        ("sep",
         xmSeparatorGadgetClass,
         row_column,
         NULL);

      radio_box = XmCreateRadioBox
        (row_column,
         "radio",
         NULL,
         0);

      XtVaSetValues(radio_box, XmNorientation, XmHORIZONTAL, NULL);

      gl_stereo = XtVaCreateManagedWidget
        ("No Stereo",
         xmToggleButtonGadgetClass,
         radio_box,
         XmNset, True,
         NULL);
      
      XtAddCallback (gl_stereo, XmNvalueChangedCallback, 
                     render_stereo_cb, (XtPointer) 0);/* 0 = NO_STEREO */

      gl_stereo = XtVaCreateManagedWidget
        ("Stereo Pair",
         xmToggleButtonGadgetClass,
         radio_box,
         XmNset, False,
         NULL);
      
      XtAddCallback (gl_stereo, XmNvalueChangedCallback, 
                     render_stereo_cb, (XtPointer) 1);/* 1 = SIDE_BY_SIDE */

      gl_stereo = XtVaCreateManagedWidget
        ("Red/Blue Stereo",
         xmToggleButtonGadgetClass,
         radio_box,
         XmNset, False,
         NULL);
      
      XtAddCallback (gl_stereo, XmNvalueChangedCallback, 
                     render_stereo_cb, (XtPointer) 2);/* 2 = RED/BLUE */

#ifdef SGI_STEREO      
      gl_stereo = XtVaCreateManagedWidget
        ("SGI Stereo (top)",
         xmToggleButtonGadgetClass,
         radio_box,
         XmNset, False,
         NULL);
      
      XtAddCallback (gl_stereo, XmNvalueChangedCallback, 
                     render_stereo_cb, (XtPointer) 3);/* 3 = SGI_HARDWARE */
#endif /* SGI_STEREO */

      XtManageChild(radio_box);

      xmstring = XmStringCreateLocalized ("Stereo seperation");
     
      seperation_scale = XtVaCreateManagedWidget
        ("sep_scale",
         xmScaleWidgetClass,
         row_column,
         XmNmaximum,  50,
         XmNminimum,  10,
         XmNvalue, get_gl_sep (),
         XmNshowValue, False,
         XmNprocessingDirection, XmMAX_ON_LEFT,
         XmNorientation, XmHORIZONTAL,
         XmNtitleString, xmstring,
         NULL);

      XtAddCallback (seperation_scale, XmNdragCallback, gl_sep_cb, NULL);
      XtAddCallback (seperation_scale, XmNvalueChangedCallback, gl_sep_cb, NULL);

      /* Not using XtSetSensitive code for now, causes problems
         with HP/UX
      if(get_gl_render_stereo() && render_using_gl_p())
        XtSetSensitive (seperation_scale, True);
      else
        XtSetSensitive (seperation_scale, False);
      */

/* <--aro */
      XtVaCreateManagedWidget
        ("sep",
         xmSeparatorGadgetClass,
         row_column,
         NULL);

      hor_row_column = XtVaCreateManagedWidget
        ("hor_row_column",
         xmRowColumnWidgetClass,
         row_column,
         XmNorientation, XmHORIZONTAL,
         NULL);

      gl_lighting = XtVaCreateManagedWidget
        ("Lighting",
         xmToggleButtonGadgetClass,
         hor_row_column,
         NULL);

      XtVaSetValues (gl_lighting, XmNset, gl_get_lighting (), NULL);
      XtAddCallback (gl_lighting, XmNvalueChangedCallback, gl_lighting_cb, NULL);


      gl_lighting = XtVaCreateManagedWidget
        ("Spotlight",
         xmToggleButtonGadgetClass,
         hor_row_column,
         NULL);

      XtVaSetValues (gl_lighting, XmNset, gl_get_specular_lighting (), NULL);
      XtAddCallback (gl_lighting, XmNvalueChangedCallback, gl_specular_lighting_cb, NULL);

      xmstring = XmStringCreateLocalized ("Diffuseness of spotlight");

      shininess_scale = XtVaCreateManagedWidget
        ("shininess_scale",
         xmScaleWidgetClass,
         row_column,
         XmNmaximum, 128,
         XmNminimum, 0,
         XmNvalue, gl_get_shininess (),
         XmNshowValue, True,
         XmNorientation, XmHORIZONTAL,
         XmNtitleString, xmstring,
         NULL);

      XtAddCallback (shininess_scale, XmNdragCallback, gl_shininess_scale_cb, NULL);
      XtAddCallback (shininess_scale, XmNvalueChangedCallback, gl_shininess_scale_cb, NULL);

      XtVaCreateManagedWidget
        ("sep",
         xmSeparatorGadgetClass,
         row_column,
         NULL);

      radio_box = XmCreateRadioBox
        (row_column,
         "radio",
         NULL,
         0);

      XtVaSetValues(radio_box, XmNorientation, XmHORIZONTAL, NULL);

      radio_button = XtVaCreateManagedWidget
        ("Ball and Stick",
         xmToggleButtonGadgetClass,
         radio_box,
         NULL);

      if (get_gl_render_type () == BALL_AND_STICK)
        {
          XtVaSetValues (radio_button, XmNset, True, NULL);
        }

      XtAddCallback
        (radio_button,
         XmNvalueChangedCallback,
         gl_render_type_button_cb,
         (XtPointer) BALL_AND_STICK);

      radio_button = XtVaCreateManagedWidget
        ("Tubes",
         xmToggleButtonGadgetClass,
         radio_box,
         NULL);

      if (get_gl_render_type () == TUBES)
        {
          XtVaSetValues (radio_button, XmNset, True, NULL);
        }

      XtAddCallback
        (radio_button,
         XmNvalueChangedCallback,
         gl_render_type_button_cb,
         (XtPointer) TUBES);

      XtManageChild (radio_box);

      XtVaCreateManagedWidget
        ("sep",
         xmSeparatorGadgetClass,
         row_column,
         NULL);

      hor_row_column = XtVaCreateManagedWidget
        ("hor_row_column",
         xmRowColumnWidgetClass,
         row_column,
         XmNorientation, XmHORIZONTAL,
         NULL);

      xmstring = XmStringCreateLocalized ("Number of atom segments");

      no_atom_segments_scale = XtVaCreateManagedWidget
        ("no_atom_segments_scale",
         xmScaleWidgetClass,
         hor_row_column,
         XmNmaximum, 36,
         XmNminimum, 6,
         XmNvalue, get_gl_no_atom_segments (),
         XmNshowValue, True,
         XmNorientation, XmHORIZONTAL,
         XmNtitleString, xmstring,
         NULL);

      XtAddCallback (no_atom_segments_scale, XmNdragCallback, gl_no_atom_segments_cb, NULL);
      XtAddCallback (no_atom_segments_scale, XmNvalueChangedCallback, gl_no_atom_segments_cb, NULL);

      xmstring = XmStringCreateLocalized ("Number of bond segments");

      no_bond_segments_scale = XtVaCreateManagedWidget
        ("no_bond_segments_scale",
         xmScaleWidgetClass,
         hor_row_column,
         XmNmaximum, 36,
         XmNminimum, 6,
         XmNvalue, get_gl_no_bond_segments (),
         XmNshowValue, True,
         XmNorientation, XmHORIZONTAL,
         XmNtitleString, xmstring,
         NULL);

      XtAddCallback (no_bond_segments_scale, XmNdragCallback, gl_no_bond_segments_cb, NULL);
      XtAddCallback (no_bond_segments_scale, XmNvalueChangedCallback, gl_no_bond_segments_cb, NULL);

      XtVaCreateManagedWidget
        ("sep",
         xmSeparatorGadgetClass,
         row_column,
         NULL);

      hor_row_column = XtVaCreateManagedWidget
        ("hor_row_column",
         xmRowColumnWidgetClass,
         row_column,
         XmNorientation, XmHORIZONTAL,
         NULL);
      

      xmstring = XmStringCreateLocalized ("Field of view (degrees)");

      fov_scale = XtVaCreateManagedWidget
        ("fov_scale",
         xmScaleWidgetClass,
         hor_row_column,
         XmNmaximum, 180,
         XmNminimum,   0,
         XmNvalue, (int) get_gl_fov (),
         XmNshowValue, True,
         XmNorientation, XmHORIZONTAL,
         XmNtitleString, xmstring,
         NULL);

      XtAddCallback (fov_scale, XmNdragCallback, gl_fov_cb, NULL);
      XtAddCallback (fov_scale, XmNvalueChangedCallback, gl_fov_cb, NULL);

      xmstring = XmStringCreateLocalized ("Position of eye along z");

      eye_scale = XtVaCreateManagedWidget
        ("eye_scale",
         xmScaleWidgetClass,
         hor_row_column,
         XmNmaximum, 1000,
         XmNminimum,    0,
         XmNvalue, (int) get_gl_eye (),
         XmNshowValue, True,
         XmNorientation, XmHORIZONTAL,
         XmNtitleString, xmstring,
         NULL);

      XtAddCallback (eye_scale, XmNdragCallback, gl_eye_cb, NULL);
      XtAddCallback (eye_scale, XmNvalueChangedCallback, gl_eye_cb, NULL);
      
      /* Remove OK/Help buttons */

      XtUnmanageChild
        (XmMessageBoxGetChild (gl_render_dialog,
                               XmDIALOG_OK_BUTTON));

      XtUnmanageChild
        (XmMessageBoxGetChild (gl_render_dialog,
                               XmDIALOG_HELP_BUTTON));

      /* Manage Cancel button */

      XtAddCallback (gl_render_dialog,
                     XmNcancelCallback,
                     (XtCallbackProc) XtUnmanageChild,
                     NULL);

      XmStringFree (xmstring);
    }

  XtAddCallback (gl_render_dialog, XmNmapCallback, place_dialog_cb, NULL);

  XtManageChild (gl_render_dialog);

}


void
gl_render_cb (Widget widget,
              XtPointer client_data,
              XtPointer call_data)
{
  enum render_stereo_types get_gl_render_stereo (void); /* aro */
  Boolean render_using_gl_p(); /* aro */
  void canvas_cb (Widget, XtPointer, XtPointer);
  int crystal_p (void);
  void set_render_using_gl (Boolean);
  void set_gl_render_stereo (enum render_stereo_types);
  void update_bbox (void);

  XmScaleCallbackStruct *cbs = (XmScaleCallbackStruct *) call_data;

  set_render_using_gl (cbs->value);

  /* The SGI Stereo button may have been selected before GL rendering 
     was turned on.  If that's the case, call set_render_stereo
     to change the monitor to stereo mode */
  if(get_gl_render_stereo() == SGI_HARDWARE) {
    if(cbs->value) {
      set_gl_render_stereo(SGI_HARDWARE);
    }
    else{
      set_gl_render_stereo(NO_STEREO);
    }
  }
  /* Not using XtSetSensitive code for now, causes problems with
     HP/UX
  if(get_gl_render_stereo() && render_using_gl_p())
    XtSetSensitive(XtNameToWidget(XtParent(widget), "sep_scale"), True);
  else
    XtSetSensitive(XtNameToWidget(XtParent(widget), "sep_scale"), False);
  */

  update_bbox ();

  redraw = 1;
  canvas_cb (canvas, NULL, NULL);

}


void
gl_lighting_cb (Widget widget,
                XtPointer client_data,
                XtPointer call_data)
{

  void canvas_cb (Widget, XtPointer, XtPointer);
  void gl_set_lighting (int);

  XmScaleCallbackStruct *cbs = (XmScaleCallbackStruct *) call_data;

  gl_set_lighting (cbs->value);

  redraw = 1;
  canvas_cb (canvas, NULL, NULL);

}


void
gl_specular_lighting_cb (Widget widget,
                XtPointer client_data,
                XtPointer call_data)
{

  void canvas_cb (Widget, XtPointer, XtPointer);
  void gl_set_specular_lighting (int);

  XmScaleCallbackStruct *cbs = (XmScaleCallbackStruct *) call_data;

  gl_set_specular_lighting (cbs->value);

  redraw = 1;
  canvas_cb (canvas, NULL, NULL);

}


void gl_shininess_scale_cb (Widget widget,
                            XtPointer client_data,
                            XtPointer call_data)
{
  void canvas_cb (Widget, XtPointer, XtPointer);
  void gl_set_shininess (int);

  XmScaleCallbackStruct *cbs = (XmScaleCallbackStruct *) call_data;

  gl_set_shininess (cbs->value);

  redraw = 1;
  canvas_cb (canvas, NULL, NULL);
}

void
gl_no_atom_segments_cb (Widget widget,
                        XtPointer client_data,
                        XtPointer call_data)
{

  void canvas_cb (Widget, XtPointer, XtPointer);
  void set_gl_no_atom_segments (int);

  XmScaleCallbackStruct *cbs = (XmScaleCallbackStruct *) call_data;

  set_gl_no_atom_segments ((int) cbs->value);

  redraw = 1;
  canvas_cb (canvas, NULL, NULL);
}


void
gl_no_bond_segments_cb (Widget widget,
                        XtPointer client_data,
                        XtPointer call_data)
{

  void canvas_cb (Widget, XtPointer, XtPointer);
  void set_gl_no_bond_segments (int);

  XmScaleCallbackStruct *cbs = (XmScaleCallbackStruct *) call_data;

  set_gl_no_bond_segments ((int) cbs->value);

  redraw = 1;
  canvas_cb (canvas, NULL, NULL);
}


void
gl_fov_cb (Widget widget,
           XtPointer client_data,
           XtPointer call_data)
{

  void canvas_cb (Widget, XtPointer, XtPointer);
  void set_gl_fov (double);

  XmScaleCallbackStruct *cbs = (XmScaleCallbackStruct *) call_data;

  set_gl_fov ((double) cbs->value);

  redraw = 1;
  canvas_cb (canvas, NULL, NULL);
}

void
gl_eye_cb (Widget widget,
           XtPointer client_data,
           XtPointer call_data)
{

  void canvas_cb(Widget, XtPointer, XtPointer);
  void set_gl_eye (double);

  XmScaleCallbackStruct *cbs = (XmScaleCallbackStruct *) call_data;

  set_gl_eye ((double) cbs->value);

  redraw = 1;
  canvas_cb (canvas, NULL, NULL);
}

/* aro--> */
void
gl_sep_cb (Widget widget,
           XtPointer client_data,
           XtPointer call_data)
{
  void canvas_cb(Widget, XtPointer, XtPointer);
  void set_gl_sep (int);

  XmScaleCallbackStruct *cbs = (XmScaleCallbackStruct *) call_data;
  
  set_gl_sep ((int) cbs->value);

  redraw = 1;
  canvas_cb(canvas, NULL, NULL);
}
/* <--aro */

void
gl_render_type_button_cb (Widget widget,
                          XtPointer client_data,
                          XtPointer call_data)
{

  void canvas_cb(Widget, XtPointer, XtPointer);
  void set_gl_render_type (int);
  void update_bbox (void);

  int type = (int) client_data;

  static int last_type = -1;

  if (type == last_type)
    {
      return;
    }
  else
    {
      last_type = type;
    }

  set_gl_render_type (type);

  update_bbox ();

  redraw = 1;
  canvas_cb (canvas, NULL, NULL);
}

/* aro--> */
void render_stereo_cb(Widget widget, 
                      XtPointer client_data, 
                      XtPointer call_data)
{
  enum render_stereo_types get_gl_render_stereo();
  void set_gl_render_stereo(enum render_stereo_types);
  void canvas_cb(Widget, XtPointer, XtPointer);
  Boolean render_using_gl_p();

  XmScaleCallbackStruct *cbs = (XmScaleCallbackStruct *) call_data;
  int stereo_type = (enum render_stereo_types) client_data;

  /* Don't need variable while not using XtSetSensitive code
  Widget sep_widget = XtNameToWidget(XtParent(XtParent(widget)), 
                                     "sep_scale");
  */

  if(cbs->value == True)
    {
      if(stereo_type == NO_STEREO)
        {
          set_gl_render_stereo(NO_STEREO);
        }
      else if(stereo_type == SIDE_BY_SIDE)
        {
          set_gl_render_stereo (SIDE_BY_SIDE);
        }
      else if(stereo_type == RED_BLUE)
        {
          set_gl_render_stereo (RED_BLUE);
        }
      else if(stereo_type == SGI_HARDWARE)
        {
          set_gl_render_stereo (SGI_HARDWARE);
        }
      else
        {
          fprintf(stderr, "Error - invalid stereo type in render stereo \n");
        }
    }

  /* Not using XtSetSensitive code for now, causes problems with
     HP/UX
  if(get_gl_render_stereo() && render_using_gl_p())
    XtSetSensitive(sep_widget, True);
  else
    XtSetSensitive(sep_widget, False);      
  */

  redraw = 1;
  canvas_cb(canvas, NULL, NULL);
}
/* <--aro */

#endif /* GL */

/* aro--> */

/* aro - Wait until we have facility to save preferences to implement 
   changing of background color...

void make_set_bg_color_dialog(Widget parent)
{
  void canvas_cb(Widget, XtPointer, XtPointer);
  void set_bg_color_cb(Widget, XtPointer, XtPointer);

  XmString xmstring;
  Widget rc, label, textField;
  int n;

  if(set_bg_color_dialog == NULL)
    {
      xmstring = XmStringCreateLocalized ("Set Background Color");

      n = 0;
      XtSetArg (args[n], XmNdefaultButtonType, XmDIALOG_NONE); n++;
      XtSetArg (args[n], XmNdialogTitle, xmstring);  n++;

      set_bg_color_dialog = XmCreateMessageDialog (parent, "set_bg_color", 
                                                   args, n);

      rc = XtVaCreateManagedWidget("rc",
                                   xmRowColumnWidgetClass,
                                   set_bg_color_dialog,
                                   XmNorientation, XmVERTICAL,
                                   NULL);
                                   
      label = XtVaCreateManagedWidget("Enter new background color",
                                      xmLabelWidgetClass,
                                      rc,
                                      NULL);

      textField = XtVaCreateManagedWidget("new_bg_color",
                                          xmTextFieldWidgetClass,
                                          rc,
                                          XmNeditable, True,
                                          XmNvalue, bg_color,
                                          NULL);
      
      XtAddCallback(textField, XmNactivateCallback, set_bg_color_cb, NULL);

      XtUnmanageChild
        (XmMessageBoxGetChild (set_bg_color_dialog,
                               XmDIALOG_CANCEL_BUTTON));
      
      XtUnmanageChild
        (XmMessageBoxGetChild (set_bg_color_dialog,
                               XmDIALOG_HELP_BUTTON));
    }
  XtManageChild(set_bg_color_dialog);

}

void
set_bg_color_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{
  void canvas_cb(Widget, XtPointer, XtPointer);
  
  char *new_bg_color;
  extern int gl_initialized;

  XtVaGetValues(widget, XmNvalue, &new_bg_color, NULL);

  strcpy(bg_color, new_bg_color);
  bg_color_parsed = 0;
  gl_initialized = 0;

  redraw = 1;
  canvas_cb (canvas, NULL, NULL);
}
*/
/* <--aro */
