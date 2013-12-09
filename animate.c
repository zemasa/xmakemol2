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
#if HAVE_FTIME
#include <sys/timeb.h>
#else
#include <sys/time.h>
#include <unistd.h>
#endif
/* aro - use following headers if using timeval struct
   and gettimeofday function below (in case compiler can't
   find ftime() function).
#include <sys/time.h>
#include <unistd.h>
*/

/* #include <libgen.h> */
char * gnu_basename (char *);

#include <Xm/FileSB.h>
#include <Xm/Label.h>
#include <Xm/MessageB.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/Scale.h>
#include <Xm/Separator.h>
#include <Xm/TextF.h>

#ifdef XPM
#include <X11/xpm.h>
#endif

#include "globals.h"
#include "defs.h"
#include "bbox.h" /* wjq */

void echo_to_message_area(char *);
void place_dialog_cb (Widget, XtPointer, XtPointer);

static int anim_file_type;
static int stop_anim;

int rot_times;

double rot_angle,rot_axis[3];

static Widget stop_b;

void
animate(Widget parent)
{

  void anim_dlg_cancel_cb(Widget, XtPointer, XtPointer);
  void anim_save_anim_cb(Widget, XtPointer, XtPointer);
  void anim_start_anim_cb(Widget, XtPointer, XtPointer);
  void anim_stop_anim_cb(Widget, XtPointer, XtPointer);
  void axis_cb(Widget, XtPointer, XtPointer);
  void scale_cb(Widget, XtPointer, XtPointer);  
  void times_cb(Widget, XtPointer, XtPointer);

  int n;

  Widget axis,axis_label,rc,scale,sep,times,times_label;
  Widget rc_hor, start_b, save_b;
  

  XmString title;
  
  
  if(!anim_dialog){
    
    title=XmStringCreateLocalized("Animate selections");
    n=0;
    XtSetArg (args[n],XmNautoUnmanage, False); n++;
    XtSetArg (args[n],XmNdialogTitle,title); n++;
    XtSetArg (args[n],XmNwidth,300); n++;

    anim_dialog=XmCreateMessageDialog(parent,"animate",args,n);
    
    XmStringFree(title);

    rc=XtVaCreateManagedWidget
      ("rc",xmRowColumnWidgetClass,anim_dialog,
       NULL);
    
    title=XmStringCreateLocalized("Angle of rotation");
    
    scale = XtVaCreateManagedWidget
      ("angle", xmScaleWidgetClass,
       rc,
       XmNmaximum,   360,
       XmNminimum,   1,
       XmNvalue,     2,
       XmNshowValue, True,
       XmNorientation, XmHORIZONTAL,
       XmNtitleString, title,
       NULL);
 
    XtAddCallback(scale, XmNvalueChangedCallback, scale_cb, NULL);
    
    rot_angle=2;

    sep=XmCreateSeparator(rc,"sep",NULL,0);
    XtManageChild(sep);

    times_label= XtVaCreateManagedWidget
      ("Rotate how many times?",
       xmLabelWidgetClass,rc,
       XmNalignment, XmALIGNMENT_BEGINNING,
       NULL);

    times=XtVaCreateManagedWidget("times",xmTextFieldWidgetClass, rc, NULL);

    XtAddCallback(times,XmNlosingFocusCallback,times_cb,NULL);

    XtVaSetValues(times,
                  XmNvalue,"180",
                  NULL);

    rot_times=180;

    sep=XmCreateSeparator(rc,"sep",NULL,0);
    XtManageChild(sep);

    axis_label= XtVaCreateManagedWidget
      ("Rotate about which axis?",
       xmLabelWidgetClass,rc,
       XmNalignment, XmALIGNMENT_BEGINNING,
       NULL);

    axis=XtVaCreateManagedWidget("axis",xmTextFieldWidgetClass, rc, NULL);

    XtAddCallback(axis,XmNlosingFocusCallback,axis_cb,NULL);

    XtVaSetValues(axis,
                  XmNvalue,"1 1 1",
                  NULL);

    rot_axis[0]=1.0;rot_axis[1]=1.0;rot_axis[2]=1.0;

    sep=XmCreateSeparator(rc,"sep",NULL,0);
    XtManageChild(sep);

    /* Pixmap toggle button: set default to false */

    rc_hor = XtVaCreateManagedWidget
      ("rc_hor", xmRowColumnWidgetClass, rc,
       XmNorientation, XmHORIZONTAL,
       NULL);
    
    start_b = XtVaCreateManagedWidget
      ("Start", xmPushButtonWidgetClass, rc_hor, NULL);
    
    XtAddCallback(start_b, XmNactivateCallback, anim_start_anim_cb, NULL);
    
    stop_b = XtVaCreateManagedWidget
      ("Stop", xmPushButtonWidgetClass, rc_hor, NULL);
    
    XtAddCallback(stop_b, XmNactivateCallback, anim_stop_anim_cb, NULL);
    
    save_b = XtVaCreateManagedWidget
      ("Save", xmPushButtonWidgetClass, rc_hor, NULL);
    
    XtAddCallback(save_b, XmNactivateCallback,anim_save_anim_cb,NULL);

    
    XtAddCallback(anim_dialog, XmNcancelCallback,
                  anim_dlg_cancel_cb,NULL);

    /* Only cancel available ... */

    XtUnmanageChild
      (XmMessageBoxGetChild(anim_dialog, XmDIALOG_OK_BUTTON));
    
    XtUnmanageChild
      (XmMessageBoxGetChild(anim_dialog, XmDIALOG_HELP_BUTTON));

  }
  
  XtAddCallback (anim_dialog, XmNmapCallback, place_dialog_cb, NULL);
  XtManageChild(anim_dialog);

}


void
anim_dlg_cancel_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{

  XtDestroyWidget(anim_dialog);
  anim_dialog=NULL;

}


void
scale_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{
  
  XmScaleCallbackStruct *cbs=
    (XmScaleCallbackStruct *)call_data;

  rot_angle=cbs->value;
  
}

void
times_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{

  char* times_str;

  XtVaGetValues(widget,XmNvalue,&times_str,NULL);

  sscanf(times_str,"%d",&rot_times);

}

void
axis_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{
  
  char* axis_str;

  XtVaGetValues(widget,XmNvalue,&axis_str,NULL);

  sscanf(axis_str,"%lf %lf %lf",&rot_axis[0],&rot_axis[1],&rot_axis[2]);
  
}

void
anim_start_anim_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{

  void rotate_atoms(double *, double, Boolean, Boolean);

  int i;

  char string[1024];

  double phi = rot_angle*PI/180.0, time, last_time = 0.0;

  XEvent event;
  
#ifdef HAVE_FTIME
  static struct timeb tb;
#else
  struct timeval tv;
#endif
  /* aro - if xmakemol fails to compile due to use of ftime function,
     try uncommenting lines using struct timeval tv, and comment out lines
     using struct timeb tb.  Also need to uncomment appropiate #include lines
     above. */
  /* struct timeval tv; */

  stop_anim=0;
  
  if(rot_times == 0)
    {
      return;
    }
  
  /* Cycle */

  for(i=0;i<rot_times;i++){
    atoms_sorted=0; /* Depths _may_ have changed */
    redraw=1;
    rotate_atoms(rot_axis,phi,1,True);

    /* Get the time  */
#ifdef HAVE_FTIME
    ftime (&tb);

    time = tb.time + (tb.millitm / 1000.0);
#else
    gettimeofday(&tv,NULL);

    time= tv.tv_sec + (tv.tv_usec / 1000000.0);

#endif
    /* gettimeofday(&tv, NULL);
       time = tv.tv_sec + (tv.tv_usec / 1000000.0); */

    /* Only print to message area if more than 0.5 seconds has elasped
       since the last time */

    if (((time - last_time) > 0.5) || (i == (rot_times - 1)))
      {
        sprintf(string, "Animation: frame %d of %d", i+1, rot_times);
        echo_to_message_area(string);
        last_time = time;
      }
    
    XmUpdateDisplay(toplevel); /* handle all expose events during loop */

    /* Handle events:

       - pressing Animate button will stop animation */
      
    while(XCheckMaskEvent
          (XtDisplay(toplevel),
           ButtonPressMask | ButtonReleaseMask,
           &event))
      {
        if(event.xany.window == XtWindow(stop_b))
          {
            XtDispatchEvent(&event);
          }
      }
    
    XFlush(XtDisplay(toplevel));

    if((stop_anim == 1))
      {
        sprintf(string, "Animation: frame %d of %d", i+1, rot_times);
        echo_to_message_area(string);
        return;
      }
    
  }

}


void
anim_stop_anim_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{

  stop_anim = 1;
  
}


void
anim_save_anim_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{

  void anim_file_type_cb(Widget, XtPointer, XtPointer);
  void save_pix_anim_cb(Widget, XtPointer, XtPointer);
  XmString get_current_directory(void);

  static Widget pix_anim_dialog,anim_file_type_w;

  XmString title,save_asc;

#ifdef XPM
  XmString save_xpm;
#endif

  if(!pix_anim_dialog){
    pix_anim_dialog=XmCreateFileSelectionDialog
      (toplevel,"pix_anim_sel",NULL,0);

    title=XmStringCreateLocalized("Save Animation");
    XtVaSetValues(pix_anim_dialog, XmNdialogTitle, title, NULL);
    
    save_asc=XmStringCreateLocalized("XYZ");

#ifdef XPM
    save_xpm=XmStringCreateLocalized("XPM");
#endif
    
    title=XmStringCreateLocalized("File type");

    anim_file_type_w=XmVaCreateSimpleOptionMenu
        (pix_anim_dialog,"File type",title,'F', 0, anim_file_type_cb,
         XmVaPUSHBUTTON, save_asc, 'A', NULL, NULL,

#ifdef XPM
         XmVaPUSHBUTTON, save_xpm,    'X', NULL, NULL,
#endif

         NULL);
    
    anim_file_type=0;

    XtManageChild(anim_file_type_w);

    XmStringFree(save_asc);
    
#ifdef XPM
    XmStringFree(save_xpm);
#endif
    
    XmStringFree(title);

    XtAddCallback (pix_anim_dialog, XmNokCallback, save_pix_anim_cb, NULL);
    XtAddCallback (pix_anim_dialog, XmNcancelCallback, 
                   (XtCallbackProc)XtUnmanageChild, NULL);
    
    /* No help available ... */

      XtUnmanageChild
        (XmFileSelectionBoxGetChild(pix_anim_dialog,XmDIALOG_HELP_BUTTON));

  }

  /* Change to a directory that was just recently operated in any of the
     file selection dialogs and refresh the file list so that newly
     created files do show up */
  
  XtVaSetValues (pix_anim_dialog, XmNdirectory, get_current_directory(), NULL);
  XmFileSelectionDoSearch (pix_anim_dialog, NULL);
  
  XtManageChild(pix_anim_dialog);
  XtPopup(XtParent(pix_anim_dialog),XtGrabNone);
  
}


void
save_pix_anim_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{

  void set_current_directory (XmString dir);
  void write_anim_file(char *);

  char *file_name;

  XmFileSelectionBoxCallbackStruct *cbs =
    (XmFileSelectionBoxCallbackStruct *) call_data;
   
  if (cbs) {
    if (!XmStringGetLtoR (cbs->value, XmFONTLIST_DEFAULT_TAG, &file_name))
      return; /* internal error */

    /* Save directory shown in the dialog so that all the other file selection
       dialogs could be made to operate the very same directory */
    
    set_current_directory (cbs->dir);

    write_anim_file(file_name);

    XtFree(file_name); /* free allocated data from XmStringGetLtoR() */

  }

  XtUnmanageChild(widget);

}


void
write_anim_file(char *anim_file_name)
{

  void write_anim_asc_file(char *);

#ifdef XPM
  void write_anim_xpm_files(char *);
#endif

  if(anim_file_type == 0){
    write_anim_asc_file(anim_file_name);
#ifdef XPM
  }else if(anim_file_type == 1){
    write_anim_xpm_files(anim_file_name);
#endif
    
  }
  
}


void
write_anim_asc_file(char *anim_file_name)
{
  void change_frame (int, Boolean, Boolean);
  void rotate_atoms (double *, double, Boolean, Boolean);
  struct frame * get_selected_frame (void);
  int count_visible_atoms_in_frame (struct frame *);

  int i, j, frame_visible_atoms;

  char buf[BUFSIZ];

  double phi = rot_angle * PI / 180.0;

  FILE *out_file;

  struct frame *this_frame;

  if ((out_file = fopen (anim_file_name, "w")) == NULL)
    {
      sprintf (buf, "Cannot open %s for write", anim_file_name);
      echo_to_message_area (buf);
      return;
    }

  this_frame = get_selected_frame ();
  frame_visible_atoms = count_visible_atoms_in_frame (this_frame);

  for (i = 0; i < rot_times; i++)
    {
      atoms_sorted = 0; /* Depths _may_ have changed */
      rotate_atoms (rot_axis, phi, False, True);

      fprintf (out_file, "%d\n", frame_visible_atoms);
      fprintf (out_file, "Frame %d\n", i + 1);

      for (j = 0; j < no_atoms; j++)
        {
          if (atoms[j].visi == 1)
            {
              fprintf (out_file, "%2s %12.6f %12.6f %12.6f\n",
                       atoms[j].label, atoms[j].x,atoms[j].y, atoms[j].z); 
            }
        }
    
      XmUpdateDisplay (toplevel); /* handle all expose events during loop */

    }
  
  fclose (out_file);
  
  if (rot_times == 1)
    {
      sprintf (buf, "File %s written: %d frame (XYZ)",
              gnu_basename(anim_file_name), rot_times);
    }
  else
    {
      sprintf (buf, "File %s written: %d frames (XYZ)",
              gnu_basename(anim_file_name), rot_times);
    }

  echo_to_message_area (buf);
  
  /* Change back to where we were before */

  change_frame (frame_no, False, False);
  
}


#ifdef XPM
void
write_anim_xpm_files(char *anim_file_name)
{
#ifdef GL
  void set_gl_copy_canvas (int);
#endif
  int write_xpm(char *, int);

  void rotate_atoms(double *, double, Boolean, Boolean);

  int i, success = 1;

  char buf[BUFSIZ], pixmap_file[1024];

  double phi = rot_angle * PI / 180.0;

#ifdef GL
  set_gl_copy_canvas (1);
#endif

  for(i = 0; i < rot_times; i++)
    {
      redraw = 1;
      atoms_sorted = 0; /* Depths _may_ have changed */
      rotate_atoms(rot_axis, phi, 1, True);

      XmUpdateDisplay(toplevel); /* handle all expose events during loop */

      sprintf(pixmap_file, "%s.%d.xpm", anim_file_name, (i + 1));

      /* Write the `canvas_pm' to the file. */
      if (write_xpm(pixmap_file, 1) == /*failure*/0) {
        /* There was not enough memory for writing the file or the file did
           not open.  Luckily, the error message has been already printed to
           the message area so we do not need to known which was it.  We
           just flag the error so that the upcoming status message does not
           make the error message disappear from the message area.  */
        success = /*failure*/0;
      }
  }

#ifdef GL
  set_gl_copy_canvas (0);
#endif

  if (success) {
    if(rot_times == 1)
    {
      sprintf(buf, "File %s.1.xpm written",
          gnu_basename(anim_file_name));
    }
    else
    {
      sprintf(buf, "Files %s.1..%d.xpm written",
          gnu_basename(anim_file_name), rot_times);
    }
    
    echo_to_message_area(buf);
  }
  
}
#endif

void
anim_file_type_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{

  anim_file_type= (int) client_data;

}
