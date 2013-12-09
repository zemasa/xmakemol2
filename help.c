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

#include <Xm/MessageB.h>

#include "globals.h"

void place_dialog_cb (Widget, XtPointer, XtPointer);

void
help_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{
  
  void make_about_dlg(Widget);
  void make_doc_dlg(Widget);
  void make_mouse_dlg(Widget);
  void make_bugs_dlg(Widget);
  
  int item_no=(int)client_data;
  
  switch(item_no){
  case 0:
    make_about_dlg(main_w);
    break;
  case 1:
    make_doc_dlg(main_w);
    break;
  case 2:
    make_mouse_dlg(main_w);
    break;
  case 3:
    make_bugs_dlg(main_w);
    break;
  }
  
}


#define ABOUT_MSG "XMakemol version %s\nCopyright (C) 2007 Matthew P. Hodges\n\nXMakemol is free software; you can redistribute it and/or modify\nit under the terms of the GNU General Public License as published by\nthe Free Software Foundation; either version 2, or (at your option)\nany later version.\n\nXMakemol is distributed in the hope that it will be useful,\nbut WITHOUT ANY WARRANTY; without even the implied warranty of\nMERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\nGNU General Public License for more details.\n\nYou should have received a copy of the GNU General Public License\nalong with XMakemol; see the file COPYING.  If not, write to the Free Software\nFoundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA"


void
make_about_dlg(Widget parent)
{

  int n;
 
  char *about_string = NULL;

  static Widget about_dialog = NULL;

  XmString msg,title;

  if(about_dialog == NULL)
    {
      title = XmStringCreateLocalized("About XMakemol");
      
      n=0;
      XtSetArg(args[n], XmNdialogTitle, title); n++;

      if(about_string == NULL)
        {
          about_string = malloc (sizeof (ABOUT_MSG) + sizeof (VERSION));
        }
      
      sprintf(about_string, ABOUT_MSG, VERSION);
      msg = XmStringCreateLtoR(about_string, XmFONTLIST_DEFAULT_TAG);
      XtSetArg(args[n], XmNmessageString, msg); n++;
  
      about_dialog =
        XmCreateInformationDialog(main_w, "about" ,args, n);
  
      XmStringFree(title);

      XtUnmanageChild
        (XmMessageBoxGetChild(about_dialog, XmDIALOG_OK_BUTTON));
  
      XtUnmanageChild
        (XmMessageBoxGetChild(about_dialog, XmDIALOG_HELP_BUTTON));
      
    }
  
  XtManageChild(about_dialog);
  
}


#define DOC_MSG "For documentation, see:-\nhttp://www.nongnu.org/xmakemol."


void
make_doc_dlg(Widget parent)
{
  
  int n;
 
  char *doc_string = NULL;

  static Widget doc_dialog = NULL;

  XmString msg,title;

  if(doc_dialog == NULL)
    {
      
      title = XmStringCreateLocalized("Documentation");

      n=0;
      XtSetArg(args[n], XmNdialogTitle, title); n++;

      if(doc_string == NULL)
        {
          doc_string = malloc(sizeof(DOC_MSG));
        }
      
      sprintf(doc_string, DOC_MSG);
      msg = XmStringCreateLtoR(doc_string, XmFONTLIST_DEFAULT_TAG);
      XtSetArg(args[n], XmNmessageString, msg); n++;
  
      doc_dialog =
        XmCreateInformationDialog(main_w, "doc", args, n);
  
      XmStringFree(title);

      XtUnmanageChild
        (XmMessageBoxGetChild(doc_dialog, XmDIALOG_OK_BUTTON));
      
      XtUnmanageChild
        (XmMessageBoxGetChild(doc_dialog, XmDIALOG_HELP_BUTTON));
      
    }
  
  XtManageChild(doc_dialog);
  
}


#define MOUSE_MSG "These are the mouse translations:\n\nNo modifier:\n\nMouse-1         Rotate atoms about an axis in the XY plane\nMouse-2         Rotate atoms about the Z axis\nMouse-3         Select atom for the Measure dialog\n\nControl modifier:\n\nMouse-1         Translate atoms in the XY plane\nMouse-2         Translate atoms along the Z axis\nMouse-3         Select closest atom for the Edit->Positions dialog\n\nShift modifier:\n\nMouse-1         Create a rectangular region on the canvas\nMouse-2         Show invisible atoms\nMouse-3         Toggle closest atom's visibility"


void
make_mouse_dlg(Widget parent)
{
  
  int n;
 
  char *doc_string = NULL;

  static Widget mouse_dialog = NULL;

  XmString msg,title;

  if(mouse_dialog == NULL)
    {
      
      title = XmStringCreateLocalized("Mouse actions");

      n=0;
      XtSetArg(args[n], XmNdialogTitle, title); n++;

      if(doc_string == NULL)
        {
          doc_string = malloc(sizeof(MOUSE_MSG));
        }
      
      sprintf(doc_string, MOUSE_MSG);
      msg = XmStringCreateLtoR(doc_string, XmFONTLIST_DEFAULT_TAG);
      XtSetArg(args[n], XmNmessageString, msg); n++;
  
      mouse_dialog =
        XmCreateInformationDialog(main_w, "mouse", args, n);
  
      XmStringFree(title);

      XtUnmanageChild
        (XmMessageBoxGetChild(mouse_dialog, XmDIALOG_OK_BUTTON));
      
      XtUnmanageChild
        (XmMessageBoxGetChild(mouse_dialog, XmDIALOG_HELP_BUTTON));
      
    }
  
  XtAddCallback (mouse_dialog, XmNmapCallback, place_dialog_cb, NULL);

  XtManageChild(mouse_dialog);
  
}


#define BUGS_MSG "Send bug reports to:-\nxmakemol-bugs@nongnu.org\n\nAll bug reports should state which version of XMakemol is being used\nand the platform on which it is being run.\n\nYou should detail what you expected to happen and what actually did\nhappen."


void
make_bugs_dlg(Widget parent)
{
  
  
  int n;
 
  char *bugs_string = NULL;

  static Widget bugs_dialog = NULL;

  XmString msg,title;

  if(bugs_dialog == NULL)
    {
      
      title = XmStringCreateLocalized("Reporting bugs");

      n=0;
      XtSetArg(args[n], XmNdialogTitle, title); n++;
      
      if(bugs_string == NULL)
        {
          bugs_string = malloc(sizeof(BUGS_MSG));
        }
      
      sprintf(bugs_string, BUGS_MSG);
      msg = XmStringCreateLtoR(bugs_string, XmFONTLIST_DEFAULT_TAG);
      XtSetArg(args[n], XmNmessageString, msg); n++;
  
      bugs_dialog =
        XmCreateInformationDialog(main_w, "bugs", args, n);
  
      XtUnmanageChild
        (XmMessageBoxGetChild(bugs_dialog, XmDIALOG_OK_BUTTON));
  
      XtUnmanageChild
        (XmMessageBoxGetChild(bugs_dialog, XmDIALOG_HELP_BUTTON));
      
    }
  
  XtManageChild(bugs_dialog);

}
