/*

Copyright (C) 1998, 1999, 2000, 2001, 2002, 2005 Matthew P. Hodges
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
#include <Xm/TextF.h>
#include <Xm/ToggleBG.h>

#include "defs.h"
#include "globals.h"

void echo_to_message_area(char *);
void place_dialog_cb (Widget, XtPointer, XtPointer);

enum perspective_types {PERSPECTIVE_ALL,
                        PERSPECTIVE_CURRENT};

static enum perspective_types perspective_type = PERSPECTIVE_ALL;

static Widget d_scale, v_scale;

char string[BUFSIZ];

double deg_to_rad=(double)(180.0/PI);

void
cont_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{

  void animate(Widget);
  void make_frame_dlg(Widget);
  void make_pers_dlg(Widget);
  void measure(Widget);

  int item_no = (int) client_data;
  
  switch(item_no){
  case 0:
    make_frame_dlg(main_w);
    break;
  case 1:
    animate(main_w);
    break;
  case 2:
    measure(main_w);
    break;
  case 3:
    make_pers_dlg(main_w);
    break;
  }
  
}


void
measure(Widget parent)
{

  void meas_dlg_cancel_cb(Widget, XtPointer, XtPointer);
  void sel_reset_cb(Widget, XtPointer, XtPointer);
  void select_atom_cb(Widget, XtPointer, XtPointer);
  void update_lengths_dialog(Boolean);

  int i,j,k,w,n=0;

  static Widget meas_rc[6],meas_frame[4];
  Widget sep,select_atom_label_w,select_atom_w,sel_reset_w;

  XmString label=XmStringCreateLocalized("Measure");
  
  if(!meas_dialog){
    XtSetArg (args[n],XmNdialogTitle, label); n++;
    XtSetArg (args[n],XmNautoUnmanage, False); n++;
    XtSetArg (args[n],XmNdefaultPosition, False); n++;

    meas_dialog=(Widget)XmCreateMessageDialog(parent, "lengths", args, n);

    label=XmStringCreateLocalized(" ");

    meas_rc[0] = XtVaCreateManagedWidget
      ("lengths",
       xmRowColumnWidgetClass,meas_dialog,
       XmNorientation, XmVERTICAL,
       NULL);

    meas_frame[0] = XtVaCreateManagedWidget 
      ("frame1",
       xmFrameWidgetClass, meas_rc[0],
       XmNshadowType,      XmSHADOW_IN,
       NULL);

    meas_rc[1] = XtVaCreateManagedWidget
      ("lengths",
       xmRowColumnWidgetClass,meas_frame[0],
       XmNorientation, XmVERTICAL,
       NULL);


    XtVaCreateManagedWidget
      ("Label   X         Y         Z       Type  No.",
       xmLabelWidgetClass,meas_rc[1],
       XmNalignment, XmALIGNMENT_BEGINNING,
       NULL);

    if(!meas_label_w){
      meas_label_w=(Widget *)malloc(15*sizeof(Widget));
    }

    w=0;
    
    for(i=0;i<4;i++){
      
      meas_label_w[w] = XtVaCreateManagedWidget
        ("dim",
         xmLabelWidgetClass,meas_rc[1],
         XmNlabelString, label,
         NULL);
      w++;
    }

    
    meas_rc[2] = XtVaCreateManagedWidget
      ("lengths",
       xmRowColumnWidgetClass,meas_rc[0],
       XmNorientation, XmHORIZONTAL,
       NULL);

    meas_frame[1] = XtVaCreateManagedWidget 
      ("frame1",
       xmFrameWidgetClass, meas_rc[2],
       XmNshadowType,      XmSHADOW_IN,
       NULL);
    
    meas_rc[3] = XtVaCreateManagedWidget
      ("lengths",
       xmRowColumnWidgetClass,meas_frame[1],
       XmNorientation, XmVERTICAL,
       NULL);

    XtVaCreateManagedWidget
      ("         Lengths",
       xmLabelWidgetClass,meas_rc[3],
       XmNalignment, XmALIGNMENT_BEGINNING,
       NULL);

    for(i=0;i<3;i++){
      for(j=i+1;j<4;j++){

        meas_label_w[w] = XtVaCreateManagedWidget
          ("dim",
           xmLabelWidgetClass,meas_rc[3],
           XmNlabelString, label,
           NULL);
        w++;
      }
    }

    meas_rc[4] = XtVaCreateManagedWidget
      ("lengths",
       xmRowColumnWidgetClass,meas_rc[2] ,
       XmNorientation, XmVERTICAL,
       NULL);

    meas_frame[2] = XtVaCreateManagedWidget 
      ("frame1",
       xmFrameWidgetClass, meas_rc[4],
       XmNshadowType,      XmSHADOW_IN,
       NULL);
    
    meas_rc[5] = XtVaCreateManagedWidget
      ("lengths",
       xmRowColumnWidgetClass,meas_frame[2] ,
       XmNorientation, XmVERTICAL,
       NULL);

    XtVaCreateManagedWidget
      ("         Angles",
       xmLabelWidgetClass,meas_rc[5],
       XmNalignment, XmALIGNMENT_BEGINNING,
       NULL);
    
    for(i=0;i<2;i++){
      for(j=i+1;j<3;j++){
        for(k=j+1;k<4;k++){

          meas_label_w[w] = XtVaCreateManagedWidget
            ("dim",
             xmLabelWidgetClass,meas_rc[5],
             XmNlabelString, label,
             NULL);
          w++;
        }
      }
    }
    
    meas_frame[3] = XtVaCreateManagedWidget 
      ("frame1",
       xmFrameWidgetClass, meas_rc[4],
       XmNshadowType,      XmSHADOW_IN,
       NULL);

    meas_rc[5] = XtVaCreateManagedWidget
      ("lengths",
       xmRowColumnWidgetClass,meas_frame[3] ,
       XmNorientation, XmVERTICAL,
       NULL);

    XtVaCreateManagedWidget
      ("         Torsion",
       xmLabelWidgetClass,meas_rc[5],
       XmNalignment, XmALIGNMENT_BEGINNING,
       NULL);

    meas_label_w[w] = XtVaCreateManagedWidget
      ("dim",
       xmLabelWidgetClass,meas_rc[5],
       XmNlabelString, label,
       NULL);
    
    XmStringFree(label);
    
    sep=XmCreateSeparator(meas_rc[0],"sep",NULL,0);
    XtManageChild(sep);

    select_atom_label_w=XtVaCreateManagedWidget
      ("Select atom:",
       xmLabelWidgetClass,meas_rc[0],
       XmNalignment, XmALIGNMENT_BEGINNING,
       NULL);
    
    select_atom_w=XtVaCreateManagedWidget
      ("select",xmTextFieldWidgetClass,meas_rc[0], NULL);

    XtAddCallback(select_atom_w,XmNactivateCallback,select_atom_cb,NULL);

    sep=XmCreateSeparator(meas_rc[0],"sep",NULL,0);
    XtManageChild(sep);
    
    sel_reset_w=XtVaCreateManagedWidget
      ("Unselect all atoms",xmPushButtonWidgetClass,meas_rc[0],NULL);

    XtAddCallback(sel_reset_w,XmNactivateCallback,sel_reset_cb,NULL);

    /* If the user selects cancel, just destroy the meas_dialog */
    XtAddCallback((Widget)meas_dialog, XmNcancelCallback, 
                   meas_dlg_cancel_cb, NULL);

    /*    XtUnmanageChild
          (XmMessageBoxGetChild(meas_dialog,XmDIALOG_OK_BUTTON)); */

      /* No help available ... */

      XtUnmanageChild
        (XmMessageBoxGetChild(meas_dialog,XmDIALOG_HELP_BUTTON));    

      update_lengths_dialog(True);
  }

  XtAddCallback (meas_dialog, XmNmapCallback, place_dialog_cb, NULL);

  XtManageChild(meas_dialog);
  XtPopup(XtParent(meas_dialog), XtGrabNone);

}


void
meas_dlg_cancel_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{

  XtDestroyWidget(meas_dialog);
  meas_dialog=NULL;

}


double
get_length(int l, int m)
{

  double x, y, z, distance;

  x = atoms[l].opx - atoms[m].opx;
  y = atoms[l].opy - atoms[m].opy;
  z = atoms[l].opz - atoms[m].opz;

  distance = sqrt ((x * x) + (y * y) + (z * z));
  
  return (distance);

}


double
get_angle(int l, int m, int n)
{

  double dot_product(double *, double *);
  void normalize_vec(double *);

  double a[3],b[3],angle;

  /* work out angle */

  a[0]=atoms[l].opx-atoms[m].opx;
  a[1]=atoms[l].opy-atoms[m].opy;
  a[2]=atoms[l].opz-atoms[m].opz;

  normalize_vec(a);

  b[0]=atoms[n].opx-atoms[m].opx;
  b[1]=atoms[n].opy-atoms[m].opy;
  b[2]=atoms[n].opz-atoms[m].opz;

  normalize_vec(b);

  angle=dot_product(a,b);

  angle=acos(angle)*deg_to_rad;

  return(angle);
  
}


double
get_torsion(int l, int m, int n, int o)
{

  double dot_product(double *, double *);
  void normalize_vec(double *);
  void vector_product(double *, double *, double *);

  double lm[3], mn[3], no[3], p[3], q[3], temp[3], torsion;
    
  /* work out torsion angle:
     
     there are four atoms (l,m,n,o). Two planes are defined - the
     first contains atoms l, m and n and the second contains atoms m,
     n and o. The angle between these two planes (actually plane
     normals) is the torsion angle. Additionally, we need to assign a
     sign - the cross product of the two plane normals is either
     parallel or antiparallel to the vector between atoms m and n.
     Parallel -> +ve torsion angle and antiparallel -> -ve torsion
     angle. */

  /* The l->m vector */

  lm[0] = atoms[m].opx - atoms[l].opx;
  lm[1] = atoms[m].opy - atoms[l].opy;
  lm[2] = atoms[m].opz - atoms[l].opz;

  normalize_vec(lm);

  /* The m->n vector */

  mn[0] = atoms[n].opx - atoms[m].opx;
  mn[1] = atoms[n].opy - atoms[m].opy;
  mn[2] = atoms[n].opz - atoms[m].opz;

  normalize_vec(mn);

  /* The n->o vector */

  no[0] = atoms[o].opx - atoms[n].opx;
  no[1] = atoms[o].opy - atoms[n].opy;
  no[2] = atoms[o].opz - atoms[n].opz;

  normalize_vec(no);

  /* The plane normal of (l,m,n) */

  vector_product(p, lm, mn); /* p = lm x mn */
    
  normalize_vec(p);

  /* The plane normal of (m,n,o) */

  vector_product(q, mn, no); /* q =  mn x no */

  normalize_vec(q);

  /* The torsion angle is the angle between the two plane normals -
     the next step gives cos(torsion) */

  torsion = dot_product(p, q);

  /* some systems don't like acos(1.0) and return nan0x7fffffff */

  if(fabs((torsion - 1.0)) < 1e-10)
    {
      torsion = 0.0;
    }
  else
    {
      torsion = acos(torsion) * deg_to_rad;
    }
  
  /* Now determine the sign - determined from mn . (p x q) */

  vector_product(temp, p, q);   /* temp = p x q */

  if(dot_product(mn, temp) < 0.0)
    {
      torsion *= -1.0;          /* Negate */
    }
  
  return(torsion);
  
}


void
make_pers_dlg(Widget parent)
{
  void d_scale_cb(Widget, XtPointer, XtPointer);
  void depth_on_cb(Widget, XtPointer, XtPointer);
  void pers_dlg_cancel_cb(Widget, XtPointer, XtPointer);
  void perspective_type_cb (Widget, XtPointer, XtPointer);
  void reset_pers_scales(void);
  void v_scale_cb(Widget, XtPointer, XtPointer);
  
  int n=0;

  Widget depth_on, rc1, radio, button;

  XmString title;

  if(!pers_dialog){
    
    title=XmStringCreateLocalized("Perspective");
    n=0;
    XtSetArg (args[n],XmNdialogTitle,title); n++;
    XtSetArg (args[n],XmNwidth,300); n++;

    pers_dialog=XmCreateMessageDialog(parent,"v_scale",args,n);
    
    rc1=XtVaCreateManagedWidget
      ("rc1",xmRowColumnWidgetClass,pers_dialog,NULL);
    
    title=XmStringCreateLocalized("Alter scale");
    
    v_scale = XtVaCreateManagedWidget
      ("v_scale", xmScaleWidgetClass,
       rc1,
       XmNmaximum,  1000,
       XmNminimum,     0,
       XmNvalue,       0,
       XmNshowValue, True,
       XmNorientation, XmHORIZONTAL,
       XmNtitleString, title,
       NULL);
    
    XtAddCallback(v_scale, XmNdragCallback, v_scale_cb, NULL);
    XtAddCallback(v_scale, XmNvalueChangedCallback, v_scale_cb, NULL);
    
    title=XmStringCreateLocalized("Choose depth");
    
    d_scale = XtVaCreateManagedWidget
      ("d_scale", xmScaleWidgetClass,
       rc1,
       XmNmaximum,   100,
       XmNminimum,   0,
       XmNvalue,     1,
       XmNshowValue, True,
       XmNorientation, XmHORIZONTAL,
       XmNtitleString, title,
       NULL);

    /* This sets the value correctly from the current canvas_scale */

    reset_pers_scales();

    XmStringFree(title);

    XtAddCallback(d_scale, XmNdragCallback, d_scale_cb, NULL);
    XtAddCallback(d_scale, XmNvalueChangedCallback, d_scale_cb, NULL);

    depth_on=XtVaCreateManagedWidget
      ("Toggle depth",xmToggleButtonGadgetClass, rc1, NULL);

    if(depth_is_on){
      XtVaSetValues(depth_on,XmNset,True,NULL);
    }

    XtAddCallback(depth_on,XmNvalueChangedCallback, depth_on_cb,(XtPointer)1);

    XtManageChild (XmCreateSeparator (rc1, "sep", NULL, 0));

    radio = XmCreateRadioBox (rc1,
                              "pers_radio",
                              NULL,
                              0);

    button = XtVaCreateManagedWidget ("Act on all frames",
                                      xmToggleButtonGadgetClass,
                                      radio,
                                      NULL);

    if (perspective_type == PERSPECTIVE_ALL)
      {
        XtVaSetValues (button, XmNset, True, NULL);
      }

    XtAddCallback (button,
                   XmNvalueChangedCallback,
                   perspective_type_cb,
                   (XtPointer) PERSPECTIVE_ALL);

    button = XtVaCreateManagedWidget ("Act on current frame",
                                      xmToggleButtonGadgetClass,
                                      radio,
                                      NULL);

    if (perspective_type == PERSPECTIVE_CURRENT)
      {
        XtVaSetValues (button, XmNset, True, NULL);
      }

    XtAddCallback (button,
                   XmNvalueChangedCallback,
                   perspective_type_cb,
                   (XtPointer) PERSPECTIVE_CURRENT);

    XtManageChild (radio);

    XtAddCallback(pers_dialog, XmNcancelCallback,
                  pers_dlg_cancel_cb,NULL);

    XtUnmanageChild
      (XmMessageBoxGetChild(pers_dialog,XmDIALOG_OK_BUTTON));

    /* No help available ... */

      XtUnmanageChild
        (XmMessageBoxGetChild(pers_dialog,XmDIALOG_HELP_BUTTON));

  }
  
  XtAddCallback (pers_dialog, XmNmapCallback, place_dialog_cb, NULL);
  XtManageChild(pers_dialog);

}


void
pers_dlg_cancel_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{

  XtDestroyWidget(pers_dialog);
  pers_dialog=NULL;
  d_scale = NULL;
  v_scale = NULL;

}


void
v_scale_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{
  
  void canvas_cb(Widget, XtPointer, XtPointer);
  struct frame * get_selected_frame (void);
  void set_canvas_scale (double);
  void set_scale_all_frames (int);

  double scale;

  struct frame *this_frame;  

  XmScaleCallbackStruct *cbs=
    (XmScaleCallbackStruct *)call_data;

  int cb_value;

  cb_value = cbs->value;

  if (no_frames != 0)
    {
      if (perspective_type == PERSPECTIVE_CURRENT)
        {
          /* Update current frame */
          this_frame = get_selected_frame ();
          this_frame->perspective_scale = cb_value;
        }
      else
        {
          /* Update all frames */
          set_scale_all_frames (cb_value);
        }
    }

  /* cb_value can vary between 0 and 1000 so we can vary the scale
     between 0 and 1023 */

  scale = pow (2.0, cb_value / 100.0) - 1;
  
  set_canvas_scale (scale);
  
  redraw = 1;
  canvas_cb (canvas, NULL, NULL);
  
}


void
set_scale_all_frames (int value)
{

  struct frame * get_first_frame ();

  struct frame *this_frame;

  this_frame = get_first_frame ();

  while (this_frame != NULL)
    {
      this_frame->perspective_scale = value;
      this_frame = this_frame->next;
    }
}


void
d_scale_cb (Widget widget,
            XtPointer client_data,
            XtPointer call_data)
{

  void canvas_cb(Widget, XtPointer, XtPointer);
  struct frame * get_selected_frame (void);
  void set_depth_all_frames (int);
  void set_z_depth (double);
  
  struct frame *this_frame;  

  XmScaleCallbackStruct *cbs=
    (XmScaleCallbackStruct *)call_data;
  
  int cb_value;
  
  cb_value = cbs->value;
  depth = cb_value;

  if (no_frames != 0)
    {
      if (perspective_type == PERSPECTIVE_CURRENT)
        {
          /* Update current frame */
          this_frame = get_selected_frame ();
          this_frame->perspective_depth = cb_value;
        }
      else
        {
          /* Update all frames */
          set_depth_all_frames (cb_value);
        }
    }

  set_z_depth (depth);

  redraw = 1;
  canvas_cb (canvas, NULL, NULL);
  
}


void
set_depth_all_frames (int value)
{

  struct frame * get_first_frame ();

  struct frame *this_frame;

  this_frame = get_first_frame ();

  while (this_frame != NULL)
    {
      this_frame->perspective_depth = value;
      this_frame = this_frame->next;
    }
}


void
depth_on_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{

  void canvas_cb(Widget, XtPointer, XtPointer);

  if(depth_is_on==0){
    depth_is_on=1;
  }else{
    depth_is_on=0;
  }

  redraw=1;
  canvas_cb(canvas,NULL,NULL); /* update display */

}


void
perspective_type_cb (Widget widget,
                     XtPointer client_data,
                     XtPointer call_data)
{
  perspective_type = (enum perspective_types) client_data;
}


void
select_atom_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{

  void update_selected(int);

  int selected_atom;

  char* selected_atom_str,string[100];

  XtVaGetValues(widget,XmNvalue,&selected_atom_str,NULL);

  if ((strlen (selected_atom_str) == 0) ||
      (sscanf (selected_atom_str, "%d", &selected_atom) <= 0))
    {
      echo_to_message_area("Cannot parse string!");
      return;
    }

  selected_atom--;  /* 0..no_atoms-1 */

  if((selected_atom>no_atoms-1)||(selected_atom<0)){
    echo_to_message_area("Choice of atom out of bounds!");
    return;
  }else{
    update_selected(selected_atom);
    sprintf(string,"Atom %d selected",selected_atom+1);
    echo_to_message_area(string);
  }
     
}


void
sel_reset_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{

  void canvas_cb(Widget, XtPointer, XtPointer);
  void update_lengths_dialog(Boolean);

  int i;

  for(i = 0; i < 4; i++)
    {
      if(selected[i] != -1)
        {
          atoms[selected[i]].sel = 0;
          selected[i] = -1;
        }
    }

  redraw = 1;
  canvas_cb(canvas, NULL, NULL);

  update_lengths_dialog(True);
  
}


void
reset_pers_scales(void)
{
  
  double get_z_depth(void);
  double get_canvas_scale(void);
  int xm_nint (double);
  
  int v_scale_int, local_z_depth;
  
  v_scale_int = xm_nint (100.0 * log (get_canvas_scale() + 1) / log(2.0));
  
  if(d_scale != NULL)
    {

      local_z_depth = (int) get_z_depth();
      
      XtVaSetValues(d_scale,
                    XmNvalue, local_z_depth,
                    NULL);
    }
  

  if(v_scale != NULL)
    {
        XtVaSetValues(v_scale,
                    XmNvalue, v_scale_int,
                    NULL);
    }

}
