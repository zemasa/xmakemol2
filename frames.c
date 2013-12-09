/*

Copyright (C) 1998, 1999, 2000, 2001, 2002, 2004, 2005, 2006 Matthew P. Hodges
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
/* #include <libgen.h> */
char * gnu_basename (char *);
#include <unistd.h>

#include <Xm/FileSB.h>
#include <Xm/Label.h>
#include <Xm/MessageB.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/Scale.h>
#include <Xm/ScrollBar.h>
#include <Xm/TextF.h>
#include <Xm/ToggleBG.h>

#ifdef XPM
#include <X11/xpm.h>
#endif

#include "draw.h"
#include "globals.h"

void echo_to_message_area(char *);
void clear_message_area (void);
void place_dialog_cb (Widget, XtPointer, XtPointer);

#ifdef HAVE_USLEEP
static int frame_speed = 1;
#endif

static int stop_anim;

#ifdef XPM
static Widget save_anim_dialog = NULL;
#endif

static Widget label_w,stop_b;

#ifdef HAVE_USLEEP
static Widget speed_scale_w, scale_scroll_bar;
#endif

void
make_frame_dlg(Widget parent)
{

  void bounce_cb(Widget, XtPointer, XtPointer);
  void centre_b_cb(Widget, XtPointer, XtPointer);
  void frame_dlg_cancel_cb(Widget, XtPointer, XtPointer);
  void frame_no_cb(Widget, XtPointer, XtPointer);
#ifdef HAVE_USLEEP
  void frame_speed_cb(Widget, XtPointer, XtPointer);
#endif
  void next_frame_cb(Widget, XtPointer, XtPointer);
  void prev_frame_cb(Widget, XtPointer, XtPointer);
  void rewind_cb(Widget, XtPointer, XtPointer);
  void start_anim_cb(Widget, XtPointer, XtPointer);
  void stop_anim_cb(Widget, XtPointer, XtPointer);
  void update_frame_label(void);
  
#ifdef XPM
  void make_anim_cb(Widget, XtPointer, XtPointer);
#endif

  int n=0;

#ifdef XPM
  Widget make_anim_b;
#endif

  Widget rc_ver,rc_hor,start_b,next_b,prev_b,bounce_b,rewind_b;
  Widget centre_b, child;

#ifdef HAVE_USLEEP
  int s, t, nkids;

  Arg tmpargs[2];

  Widget *kids;
#endif

  XmString label = NULL;

  if (! frames_dialog) {

    label = XmStringCreateLocalized ("Frames");   

    XtSetArg (args[n], XmNautoUnmanage, False); n++;
    XtSetArg (args[n], XmNdialogTitle, label); n++;

    frames_dialog = (Widget)
      XmCreateMessageDialog (parent, "frames", args, n);

    rc_ver = XtVaCreateManagedWidget
      ("rc_ver",xmRowColumnWidgetClass,frames_dialog,
       NULL);

    label_w=XtVaCreateManagedWidget
      ("Comment",
       xmLabelWidgetClass, rc_ver,
       XmNlabelString, label,
       XmNwidth, 400,
       NULL);

    update_frame_label();

    rc_hor = XtVaCreateManagedWidget
      ("rc_hor",xmRowColumnWidgetClass,rc_ver,
       XmNorientation, XmHORIZONTAL,
       NULL);
    
    start_b = XtVaCreateManagedWidget
      ("Start",xmPushButtonWidgetClass,rc_hor,
       NULL);
    
    XtAddCallback(start_b, XmNactivateCallback, start_anim_cb, NULL);

    stop_b = XtVaCreateManagedWidget
      ("Stop",xmPushButtonWidgetClass,rc_hor,
       NULL);
    
    XtAddCallback(stop_b, XmNactivateCallback, stop_anim_cb, NULL);
    
    next_b = XtVaCreateManagedWidget
      ("Next (>)",xmPushButtonWidgetClass,rc_hor,
       NULL);
    
    XtAddCallback(next_b, XmNactivateCallback, next_frame_cb, NULL);

    prev_b = XtVaCreateManagedWidget
      ("Previous (<)",xmPushButtonWidgetClass,rc_hor,
       NULL);
    
    XtAddCallback(prev_b, XmNactivateCallback, prev_frame_cb, NULL);

    rewind_b = XtVaCreateManagedWidget
      ("Rewind (^)",xmPushButtonWidgetClass,rc_hor,
       NULL);
    
    XtAddCallback(rewind_b, XmNactivateCallback, rewind_cb, NULL);

    bounce_b = XtVaCreateManagedWidget
      ("Bounce",xmPushButtonWidgetClass,rc_hor,
       NULL);

    XtAddCallback(bounce_b, XmNactivateCallback, bounce_cb, NULL);

#ifdef XPM
    make_anim_b = XtVaCreateManagedWidget
      ("Make anim",xmPushButtonWidgetClass,rc_hor,
       NULL);
    
    XtAddCallback(make_anim_b, XmNactivateCallback, make_anim_cb, NULL);
    
#endif

#ifdef HAVE_USLEEP
    label=XmStringCreateLocalized("Select speed");

    speed_scale_w = XtVaCreateManagedWidget
      ("Speed",xmScaleWidgetClass,
       rc_ver,
       XmNmaximum, 20,
       XmNminimum,  1,
       XmNvalue, (int) (20-frame_speed+1),
       XmNshowValue, True,
       XmNorientation, XmHORIZONTAL,
       XmNtitleString, label,
       NULL);

    /* need to access scale bar widget for callbacks */

    s=0;
    XtSetArg(tmpargs[s], XmNnumChildren, &nkids); s++;
    XtSetArg(tmpargs[s], XmNchildren, &kids); s++;

    XtGetValues(speed_scale_w, tmpargs, s);

    for(t=0;t<nkids;t++){
      if(XmIsScrollBar((Widget)kids[t])){
        scale_scroll_bar=(Widget)kids[t];
      }
    }

    XtAddCallback(speed_scale_w, XmNvalueChangedCallback,frame_speed_cb,NULL); 
    XtAddCallback(speed_scale_w, XmNdragCallback, frame_speed_cb, NULL);

#endif

    XtAddCallback(frames_dialog, XmNcancelCallback,
                  frame_dlg_cancel_cb, NULL);

    centre_b = XtVaCreateManagedWidget
      ("Centre each frame",xmToggleButtonGadgetClass,rc_ver,NULL);

    XtVaSetValues(centre_b,XmNset,False,NULL);
    centre_each_frame=0;
    
    XtAddCallback
      (centre_b,XmNvalueChangedCallback,centre_b_cb,NULL);
    
    XtVaCreateManagedWidget
      ("Select frame:", xmLabelWidgetClass, rc_ver,
         XmNalignment, XmALIGNMENT_BEGINNING,
         NULL);

    child = XtVaCreateManagedWidget
      ("select_frame", xmTextFieldWidgetClass, rc_ver, NULL);

    XtVaSetValues (child, XmNvalue, "1", NULL);

    XtAddCallback(frames_dialog, XmNokCallback, frame_no_cb, child);

    XtUnmanageChild (XmMessageBoxGetChild
                     (frames_dialog, XmDIALOG_HELP_BUTTON));
    
  }

  XtAddCallback (frames_dialog, XmNmapCallback, place_dialog_cb, NULL);

  XtManageChild(frames_dialog);

  XmStringFree (label);
}


void
frame_dlg_cancel_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{
  
  XtDestroyWidget(frames_dialog);
  frames_dialog=NULL;

}


void
frame_no_cb (Widget widget,
             XtPointer client_data,
             XtPointer call_data)
{
  void change_frame (int, Boolean, Boolean);

  char string[124], *temp_frame_string;
  long int temp_frame;

  XtVaGetValues (client_data, XmNvalue, &temp_frame_string, NULL);

  if ((strlen (temp_frame_string) == 0) ||
      (sscanf (temp_frame_string, "%ld", &temp_frame) <= 0))
    {
      echo_to_message_area ("Cannot parse string!");
      return;
    }
  
  temp_frame--; /* Frames from 0 to no_frames -1 */

  if((temp_frame < 0) || (temp_frame > (no_frames - 1)))
    {
    echo_to_message_area("Choice of frame out of bounds!");
    return;
    }

  /* if OK, change frame_no */

  frame_no = temp_frame;

  sprintf(string,"Frame %d selected",frame_no+1);
  echo_to_message_area(string);
    
  if(centre_each_frame){
    /* Change frame, reset title _and_ centre the frame */
    change_frame(frame_no, True, True);
  }else{
    /* Change frame, reset title, _don't_ centre the frame */
    change_frame(frame_no, True, False);
  }

}


void
start_anim_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{

  void change_frame(int, Boolean, Boolean);

  XEvent event;
  
  stop_anim=0;

  if(no_frames < 2){
    return;
  }

  while(1){ /* infinite loop, stop with button press */
    
    if(frame_no<no_frames-1){
      frame_no++;
    }else{
      frame_no=0;
    }

    if(centre_each_frame){
      /* Change frame, _don't_ reset title _and_ centre the frame */
      change_frame(frame_no, False, True);
    }else{
      /* Change frame, _don't_ reset title, _don't_ centre the frame */
      change_frame(frame_no, False, False);
    }
    
    XmUpdateDisplay(toplevel); /* handle all expose events during loop */

     /* Handle some events */
    
    while(XCheckMaskEvent
          (XtDisplay(toplevel),
           ButtonPressMask | ButtonReleaseMask | ButtonMotionMask,
           &event)){

      if(event.xany.window == XtWindow(stop_b)){
        XtDispatchEvent(&event);
      }
        
#ifdef HAVE_USLEEP
      if(event.xany.window == XtWindow(scale_scroll_bar)){
        XtDispatchEvent(&event);
      }
#endif

      if(event.xany.window == XtWindow(canvas)){
        XtDispatchEvent(&event);
        XSync(XtDisplay(toplevel),True); /* One event at a time */
      }

    }
      
    XFlush(XtDisplay(toplevel));

#ifdef HAVE_USLEEP
    /* Vary the speed */
    
    usleep(10000*(frame_speed - 1));
#endif

    if(stop_anim == 1){
      return;
    } /* stop button pressed; leave loop */
  }
}


void
stop_anim_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{

  void change_frame(int, Boolean, Boolean);

  stop_anim=1;

  if(centre_each_frame){
    /* Change frame, reset title _and_ centre the frame */
    change_frame(frame_no, True, True);
  }else{
    /* Change frame, reset title, _don't_ centre the frame */
    change_frame(frame_no, True, False);
  }

}


void
next_frame_cb(Widget w, XtPointer client_data, XtPointer call_data)
{

  void change_frame(int, Boolean, Boolean);

  if(frame_no==no_frames-1){
    frame_no=0;
  }else{
    frame_no++;
  }
  
  if(centre_each_frame){
    /* Change frame, reset title _and_ centre the frame */
    change_frame(frame_no, True, True);
  }else{
    /* Change frame, reset title, _don't_ centre the frame */
    change_frame(frame_no, True, False);
  }

}


void
prev_frame_cb(Widget w, XtPointer client_data, XtPointer call_data)
{

  void change_frame(int, Boolean, Boolean);
  
  if(frame_no==0){
    frame_no=no_frames-1;
  }else{
    frame_no--;
  }

  if(centre_each_frame){
    /* Change frame, reset title _and_ centre the frame */
    change_frame(frame_no, True, True);
  }else{
    /* Change frame, reset title, _don't_ centre the frame */
    change_frame(frame_no, True, False);
  }

}


void
rewind_cb(Widget w, XtPointer client_data, XtPointer call_data)
{
  
  void change_frame(int, Boolean, Boolean);

  frame_no=0;

  if(centre_each_frame){
    /* Change frame, reset title _and_ centre the frame */
    change_frame(frame_no, True, True);
  }else{
    /* Change frame, reset title, _don't_ centre the frame */
    change_frame(frame_no, True, False);
  }
  
}


void
bounce_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{

  void change_frame(int, Boolean, Boolean);

  static int frame_step=1;

  XEvent event;
  
  stop_anim=0;

  if(no_frames < 2){
    return;
  }

  while(1){ /* infinite loop, stop with button press */
    
    if(frame_no==no_frames-1){
      frame_step=-1;
#ifdef HAVE_USLEEP
      usleep(200000*(frame_speed));
#endif
    }else if(frame_no==0){
      frame_step=1;
#ifdef HAVE_USLEEP
      usleep(200000*(frame_speed));
#endif
    }

    frame_no+=frame_step;
    
    if(centre_each_frame){
      /* Change frame, _don't_ reset title _and_ centre the frame */
      change_frame(frame_no, False, True);
    }else{
      /* Change frame, _don't_ reset title, _don't_ centre the frame */
      change_frame(frame_no, False, False);
    }
    
    XmUpdateDisplay(toplevel); /* handle all expose events during loop */

     /* Handle some events */
    
    while(XCheckMaskEvent
          (XtDisplay(toplevel),
           ButtonPressMask | ButtonReleaseMask | ButtonMotionMask,
           &event)){

      if(event.xany.window == XtWindow(stop_b)){
        XtDispatchEvent(&event);
      }

#ifdef HAVE_USLEEP
      if(event.xany.window == XtWindow(scale_scroll_bar)){
        XtDispatchEvent(&event);
      }
#endif      

      if(event.xany.window == XtWindow(canvas)){
        XtDispatchEvent(&event);
        XSync(XtDisplay(toplevel),True); /* One event at a time */
      }

    }
      
    XFlush(XtDisplay(toplevel));

#ifdef HAVE_USLEEP
    /* Vary the speed */

    usleep(10000*(frame_speed-1));
#endif
    
    if(stop_anim == 1){
      return;
    } /* stop button pressed; leave loop */
  }
}


#ifdef XPM

void
make_anim_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{

  void save_anim_cb(Widget, XtPointer, XtPointer);
  XmString get_current_directory ();

  XmString title;

  if(!save_anim_dialog){
    save_anim_dialog=XmCreateFileSelectionDialog
      (toplevel,"anim_sel",NULL,0);

    title=XmStringCreateLocalized("Save Frames Animation");
    XtVaSetValues(save_anim_dialog, XmNdialogTitle, title, NULL);
    XmStringFree (title);

    XtAddCallback (save_anim_dialog, XmNokCallback, save_anim_cb, NULL);
    XtAddCallback (save_anim_dialog, XmNcancelCallback, 
                     (XtCallbackProc)XtUnmanageChild, NULL);

    /* No help available ... */
    
    XtUnmanageChild
        (XmFileSelectionBoxGetChild(save_anim_dialog,XmDIALOG_HELP_BUTTON));
  }

  /* Change to a directory that was just recently operated in any of the
     file selection dialogs and refresh the file list so that newly
     created files do show up */
  
  XtVaSetValues (save_anim_dialog, XmNdirectory, get_current_directory(),NULL);
  XmFileSelectionDoSearch (save_anim_dialog, NULL);

  XtManageChild(save_anim_dialog);
  XtPopup(XtParent(save_anim_dialog),XtGrabNone);
  
}


void
save_anim_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{
  void change_frame(int, Boolean, Boolean);
  void set_current_directory (XmString dir);
#ifdef GL
  void set_gl_copy_canvas (int);
#endif
  int write_xpm(char *, int);

  int i, success = 1;

  char buf[BUFSIZ], *anim_file, pixmap_file[1024];

  XmFileSelectionBoxCallbackStruct *cbs =
    (XmFileSelectionBoxCallbackStruct *) call_data;
   
  if (cbs) {
    if (!XmStringGetLtoR (cbs->value, XmFONTLIST_DEFAULT_TAG, &anim_file))
      return; /* internal error */

    /* Save directory shown in the dialog so that all the other file selection
       dialogs could be made to operate the very same directory */
    
    set_current_directory (cbs->dir);
  }

#ifdef GL
  set_gl_copy_canvas (1);
#endif

  for(i = 0; i < no_frames; i++)
    {
      frame_no = i;
    
      XmUpdateDisplay(toplevel); /* handle all expose events during loop */
    
      if(centre_each_frame)
        {
          /* Change frame, reset title _and_ centre the frame */
          change_frame(frame_no, True, True);
        }
      else
        {
          /* Change frame, reset title, _don't_ centre the frame */
          change_frame(frame_no, True, False);
        }
    
      sprintf(pixmap_file, "%s.%d.xpm", anim_file, (i + 1));

      /* Write the `canvas_pm' to the file. */
      if (write_xpm (pixmap_file, 1) == /*failure*/0)
        {
          /* Could not write the file.  Flag the error so that we know not to
             overwrite the error message currently in the message area with
             our status message. */
          success = /*failure*/ 0;
        }
    }

#ifdef GL
  set_gl_copy_canvas (0);
#endif

  if (success)
    {
      if(no_frames == 1)
        {
          sprintf(buf, "File %s.1.xpm written",
              gnu_basename(anim_file));
        }
      else
        {
          sprintf(buf, "Files %s.1..%d.xpm written",
              gnu_basename(anim_file), no_frames);
        }
      
      echo_to_message_area(buf);
    }

}
#endif


#ifdef HAVE_USLEEP
void
frame_speed_cb(Widget w, XtPointer client_data, XtPointer call_data)
{

  XmScaleCallbackStruct *cbs=
    (XmScaleCallbackStruct *)call_data;

  frame_speed = 20-cbs->value+1;

}
#endif


void
centre_b_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{
  
  if(centre_each_frame==1){
    centre_each_frame=0;
    echo_to_message_area("Not centering each frame");
  }else{
    centre_each_frame=1;
    echo_to_message_area("Centering each frame");
  }

}

/* update_label is redundant - remove at some point from calls to
   change_frame */

void
change_frame(int frame_no, Boolean update_label, Boolean centre_this_frame)
{

  double get_angle_axis(double *);
  void canvas_cb(Widget, XtPointer, XtPointer);
  void centre_atoms(void);
  void make_label_uppercase (char *);
  void rotate_atoms(double *,double,Boolean,Boolean);
  void update_bbox(void);
  void update_bond_matrix(Boolean);
  void update_frame_label(void);
  void frame_content_to_atoms (int);

  int i,j;

  double angle,axis[3];

  /* if no frames, return ... */

  if(no_frames == 0) return;

  changing_frame = True;

  frame_content_to_atoms (frame_no);

  /* Centre frame if required, by modifying global_vector[] */
  
  if(centre_this_frame){
    centre_atoms();
  }

  /* get angle-axis rotation from global_matrix positions... */

  for(i=0;i<3;i++){
    for(j=0;j<3;j++){
      angle_axis_matrix[i][j]=global_matrix[i][j];
    }
  }

  angle=get_angle_axis(axis);

  update_bbox();
  update_bond_matrix (True);

  redraw=0;
  rotate_atoms(axis,angle,0,False);

  redraw=1;
  canvas_cb(canvas,NULL,NULL);

  update_frame_label();

  changing_frame = False;
}


double
get_angle_axis(double *axis)
{

  int i,j;

  double angle,s,z,mod,r[3][3];

  for(i=0;i<3;i++){
    for(j=0;j<3;j++){
      r[i][j]=angle_axis_matrix[i][j];
    }
  }

  /* adapted from quater.f (ajs) */

  z=r[0][0]+r[1][1]+r[2][2]+1;

  if(z>0){
    angle=0.5*sqrt(z);
  }else{
    angle=0;
  }

  if(angle > 0.0001){
    axis[0]=0.25*(r[2][1]-r[1][2])/angle;
    axis[1]=0.25*(r[0][2]-r[2][0])/angle;
    axis[2]=0.25*(r[1][0]-r[0][1])/angle;
  }else{
    s=1-r[0][0]-r[1][1]-r[2][2];
    for(i=0;i<3;i++){
      z=2*r[i][i]+s;
      if(z > 0){
        axis[i]=0.5*sqrt(z);
      }else{
        axis[i]=0;
      }
    }

    if(angle > 0){
      if(r[1][0] < r[0][1]){axis[2]=-axis[2];}
      if(r[2][1] < r[1][2]){axis[0]=-axis[0];}
      if(r[0][2] < r[2][0]){axis[1]=-axis[1];}
    }else{
      if(axis[0] > 0.5){
        if(r[0][1]+r[1][0] < 0){axis[1]=-axis[1];}
        if(r[0][2]+r[2][0] < 0){axis[2]=-axis[2];}
      }else if(axis[1] > 0.5){
        if(r[0][1]+r[1][0] < 0){axis[0]=-axis[0];}
        if(r[1][2]+r[2][1] < 0){axis[2]=-axis[2];}
      }else if(axis[2] > 0.5){
        if(r[0][2]+r[2][0] < 0){axis[0]=-axis[0];}
        if(r[1][2]+r[2][1] < 0){axis[1]=-axis[1];}
      }
    }
  }
  
  angle=2*acos(angle);

  mod = (axis[0] * axis[0]) + (axis[1] * axis[1]) + (axis[2] * axis[2]);

  if(mod==0){
    axis[2]=1;
  }

  return(angle);

}


void
update_frame_label(void)
{

  struct frame * get_selected_frame (int);

  char string[1024];

  struct frame *this_frame;

  XmString xm_str;

  this_frame = get_selected_frame (frame_no);

  if(no_frames == 0)
    {
      sprintf(string, "No frames loaded");
    }
  else if(strlen(this_frame->comment) == 0)
    {
      sprintf(string, "Frame %d (%d atoms); comment \"(empty)\".",
              frame_no + 1,
              this_frame->no_atoms);
    }
  else
    {
      sprintf(string, "Frame %d (%d atoms); comment \"%s\".",
              frame_no + 1,
              this_frame->no_atoms,
              this_frame->comment);
    }

  if(frames_dialog != NULL)
    {

      /* Set label in frames_dialog ... */

      xm_str = XmStringCreateLocalized(string);
      XtVaSetValues (label_w, XmNlabelString, xm_str, NULL);

      XmStringFree(xm_str);
      clear_message_area (); 
    }
  else
    {
      echo_to_message_area (string);
    }
}


void
frame_key (Widget widget, XEvent *event, String *args, int *num_args)
{

  void change_frame(int, Boolean, Boolean);

  if (*num_args != 1)
    {
      XtError ("Wrong number of args!");
    }

  if (strcmp (args[0], "next") == 0)
    {
      if (frame_no == (no_frames - 1))
        {
          frame_no = 0;
        }
      else
        {
          frame_no++;
        }
    }
  else if (strcmp (args[0], "prev") == 0)
    {
      if (frame_no == 0)
        {
          frame_no = no_frames - 1;
        }
      else
        {
          frame_no--;
        }
    }
  else if (strcmp (args[0], "start") == 0)
    {
      frame_no = 0;
    }

  /* Select the frame */

  if (centre_each_frame)
    {
      change_frame(frame_no, True, True);
    }
  else
    {
      change_frame(frame_no, True, False);
    }
}
