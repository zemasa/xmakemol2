/*

Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006 Matthew P. Hodges
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

#define __FILE_C__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if HAVE_ALLOCA_H
#include <alloca.h>
#endif
#include <ctype.h> /* aro - needed for isalnum() */
#include <dirent.h>
#include <locale.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* #include <libgen.h> */
char * gnu_basename (char *);

#include <Xm/FileSB.h>
#include <Xm/Label.h>
#include <Xm/List.h>
#include <Xm/MessageB.h> /* aro */
#include <Xm/PushB.h>    /* aro */
#include <Xm/RowColumn.h>/* aro */
#include <Xm/TextF.h>    /* aro */
#include <Xm/ToggleB.h>  /* aro */

#include "globals.h"
#include "bonds.h"
#include "defs.h"
#include "draw.h"
#ifdef GL
#include "gl_funcs.h" /* aro - for visinfo */
#include "gl2ps.h"
#endif /* GL */
#include "view.h"
#include "bbox.h" /* wjq - for bbox_xyz */

#ifdef XPM
#include <X11/xpm.h>
#endif

void echo_to_message_area(char *);
struct frame * get_first_frame (void);
XmString get_current_directory (void);
void set_current_directory (XmString dir);
void place_dialog_cb (Widget, XtPointer, XtPointer);

static int no_elements;
static int bond_adjacency_lists_allocated_size;
//static int file_type = 0;
static int export_type = 0;

enum merge_types {MERGE_FRAME, MERGE_ALL};
static enum merge_types merge_type = MERGE_FRAME;

static Boolean aux_file_exists_p;
static Boolean aux_file_error_p;

Widget p_dialog = NULL;/* aro */

static struct frame *first_frame, *current_frame;

struct type
{
  int elem;                     /* corresponding element */
  char label[4];
  struct type *next;
};

static struct type *first_type, *current_type;

/* directory that was accessed in any of the file selection dialogs */
static XmString current_directory = NULL;


void
file_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{
  void open_file(char *, Boolean);

  /* aro: 
     Updates here: added print_file callback for selection box
     containing list of printers. 
     Also added:
     int i, j; 
     all char * and arrays; 
     FILE *printcap_file;
     Widgets p_dialog, print_printer, temp_widget; 
     XmStringTable printer_xmstrings. */
     
  void export_type_cb(Widget, XtPointer, XtPointer);
  void file_type_cb(Widget, XtPointer, XtPointer);
  void load_file_cb(Widget, XtPointer, XtPointer);
  void save_file_cb(Widget, XtPointer, XtPointer);
  void merge_file_cb(Widget, XtPointer, XtPointer);
  void export_file_cb(Widget, XtPointer, XtPointer);
  void print_file_cb(Widget, XtPointer, XtPointer);/* aro */
  void print_file_select_cb(Widget, XtPointer, XtPointer);/* aro */
  void cleanup_cb(Widget, XtPointer, XtPointer);/* aro */
  int item_no=(int) client_data, i, j;/* aro */
  char *printer_name, *line_ptr, line[1024], printer_strings[MAX_PRINTERS+1][80],
       print_command_string[1024];/* aro */
  FILE *printcap_file;/* aro */

  static Widget o_dialog,s_dialog,m_dialog,e_dialog;

  Widget rc[2], export_type_w, file_type_w, print_printer, print_command, temp_widget;/* aro */

  XmString title, bw_fig, col_fig, as_all, as_fra, con_fra, eps_bw, eps_co, aux;
  XmStringTable printer_xmstrings[MAX_PRINTERS];/* aro */

#ifdef XPM
  XmString xpm;
#endif

#ifdef GL
  XmString gl2ps_eps, gl2ps_pdf, gl2ps_svg;
#endif

  switch(item_no){
  case 0:
    if (!o_dialog) {
      o_dialog = XmCreateFileSelectionDialog
        (toplevel, "file_sel", NULL, 0);
      
      title=XmStringCreateLocalized ("Open File");    
      XtVaSetValues(o_dialog, XmNdialogTitle, title, NULL);

      title = XmStringCreateLocalized("*.xyz");
      XtVaSetValues(o_dialog, XmNpattern, title, NULL);

      XmStringFree (title);

      /* okCallback calls load_file_cb(), cancelCallback unmanages the
         FileSelectionDialog widget */

      XtAddCallback (o_dialog, XmNokCallback, load_file_cb, NULL);
      XtAddCallback (o_dialog, XmNcancelCallback, 
                     (XtCallbackProc)XtUnmanageChild, NULL);
      
      /* No help available ... */

      XtUnmanageChild
        (XmFileSelectionBoxGetChild(o_dialog,XmDIALOG_HELP_BUTTON));
      
    }
      
    /* Change to a directory that was just recently operated in any of the
       file selection dialogs and refresh the file list so that newly
       created files do show up */
    
    XtVaSetValues (o_dialog, XmNdirectory, get_current_directory (), NULL);
    XmFileSelectionDoSearch (o_dialog, NULL);

    XtAddCallback (o_dialog, XmNmapCallback, place_dialog_cb, NULL);

    XtManageChild (o_dialog);
    XtPopup (XtParent (o_dialog), XtGrabNone);
    break;
  case 1:
    open_file(current_file_name, TRUE);
    break;
  case 2:
    if (!s_dialog) {
      s_dialog = XmCreateFileSelectionDialog (toplevel, "file_sel", NULL, 0);
      
      title=XmStringCreateLocalized ("Save File");    
      XtVaSetValues(s_dialog, XmNdialogTitle, title, NULL);
      
      /* okCallback calls save_file(), cancelCallback unmanages the
         FileSelectionDialog widget */

      as_all  = XmStringCreateLocalized("XYZ (all)");
      as_fra  = XmStringCreateLocalized("XYZ (frame)");
      con_fra = XmStringCreateLocalized("XYZ + connectivities (frame)");
      aux     = XmStringCreateLocalized ("Auxiliary info");

      title=XmStringCreateLocalized ("File type");    
      
      file_type_w=XmVaCreateSimpleOptionMenu
        (s_dialog,"File type",title,'F', 0, file_type_cb,
         XmVaPUSHBUTTON, as_all,  'X', NULL, NULL,
         XmVaPUSHBUTTON, as_fra,  'Y', NULL, NULL,
         XmVaPUSHBUTTON, con_fra, 'Z', NULL, NULL,
         XmVaPUSHBUTTON, aux,     'A', NULL, NULL,
         NULL);
      
      XtManageChild(file_type_w);

      XmStringFree(as_all);
      XmStringFree(as_fra);
      XmStringFree(con_fra);
      XmStringFree (aux);

      XmStringFree(title);

      XtAddCallback(s_dialog, XmNokCallback, save_file_cb, NULL);
      XtAddCallback(s_dialog, XmNcancelCallback, 
                     (XtCallbackProc)XtUnmanageChild, NULL);

      /* No help available ... */

      XtUnmanageChild
        (XmFileSelectionBoxGetChild(s_dialog,XmDIALOG_HELP_BUTTON));

    }
      
    /* Change to a directory that was just recently operated in any of the
       file selection dialogs and refresh the file list so that newly
       created files do show up */
    
    XtVaSetValues (s_dialog, XmNdirectory, get_current_directory (), NULL);
    XmFileSelectionDoSearch (s_dialog, NULL);
      
    XtAddCallback (s_dialog, XmNmapCallback, place_dialog_cb, NULL);

    XtManageChild (s_dialog);
    XtPopup (XtParent (s_dialog), XtGrabNone);
    break;
  case 3:
    if (!m_dialog) {
      m_dialog = XmCreateFileSelectionDialog
        (toplevel, "file_sel", NULL, 0);
      
      title=XmStringCreateLocalized ("Merge With File");    
      XtVaSetValues (m_dialog, XmNdialogTitle, title, NULL);

      title = XmStringCreateLocalized ("*.xyz");
      XtVaSetValues (m_dialog, XmNpattern, title, NULL);

      {
        void merge_type_cb (Widget, XtPointer, XtPointer);

        Widget merge_type_w;
        XmString merge_frame, merge_all;

        merge_frame = XmStringCreateLocalized ("Use first frame");
        merge_all = XmStringCreateLocalized ("Use all frames");

        title = XmStringCreateLocalized ("Merge type");

        merge_type_w = XmVaCreateSimpleOptionMenu
          (m_dialog,
           "Merge type",
           title,
           'M', 0, merge_type_cb,
           XmVaPUSHBUTTON, merge_frame, 'f', NULL, NULL,
           XmVaPUSHBUTTON, merge_all,   'a', NULL, NULL,
           NULL);

        XtManageChild (merge_type_w);

        XmStringFree (merge_frame);
        XmStringFree (merge_all);
      }

      XmStringFree (title);

      XtAddCallback (m_dialog, XmNokCallback, merge_file_cb, NULL);
      XtAddCallback (m_dialog, XmNcancelCallback, 
                     (XtCallbackProc)XtUnmanageChild, NULL);
      
      /* No help available ... */

      XtUnmanageChild
        (XmFileSelectionBoxGetChild(m_dialog,XmDIALOG_HELP_BUTTON));
      
    }
      
    /* Change to a directory that was just recently operated in any of the
       file selection dialogs and refresh the file list so that newly
       created files do show up */
    
    XtVaSetValues (m_dialog, XmNdirectory, get_current_directory (), NULL);
    XmFileSelectionDoSearch (m_dialog, NULL);
    
    XtAddCallback (m_dialog, XmNmapCallback, place_dialog_cb, NULL);

    XtManageChild (m_dialog);
    XtPopup (XtParent (m_dialog), XtGrabNone);
    break;
    
  case 4:
    if (!e_dialog) {
      e_dialog = XmCreateFileSelectionDialog (toplevel, "file_sel", NULL, 0);
      
      title=XmStringCreateLocalized ("Export File");    
      XtVaSetValues(e_dialog, XmNdialogTitle, title, NULL);
      
      /* okCallback calls save_file(), cancelCallback unmanages the
         FileSelectionDialog widget */

      bw_fig  = XmStringCreateLocalized("Fig (b/w)");
      col_fig = XmStringCreateLocalized("Fig (colour)");
      eps_bw  = XmStringCreateLocalized("EPS (b/w)");
      eps_co  = XmStringCreateLocalized("EPS (colour)");

#ifdef XPM
      xpm   = XmStringCreateLocalized("XPM");
#endif

#ifdef GL
      gl2ps_eps = XmStringCreateLocalized("GL2PS (EPS)");
      gl2ps_pdf = XmStringCreateLocalized("GL2PS (PDF)");
      gl2ps_svg = XmStringCreateLocalized("GL2PS (SVG)");
#endif

      title=XmStringCreateLocalized ("File type");    
      
      export_type_w=XmVaCreateSimpleOptionMenu
        (e_dialog,"File type",title,'F', 0, export_type_cb,
         XmVaPUSHBUTTON, bw_fig,  'S', NULL, NULL,
         XmVaPUSHBUTTON, col_fig, 'T', NULL, NULL,
         XmVaPUSHBUTTON, eps_bw,  'E', NULL, NULL,
         XmVaPUSHBUTTON, eps_co,  'P', NULL, NULL,

#ifdef XPM
         XmVaPUSHBUTTON, xpm,    'X', NULL, NULL,
#endif

#ifdef GL
         XmVaPUSHBUTTON, gl2ps_eps,    'G', NULL, NULL,
         XmVaPUSHBUTTON, gl2ps_pdf,    'P', NULL, NULL,
         XmVaPUSHBUTTON, gl2ps_svg,    'V', NULL, NULL,
#endif

         NULL);
      
      XtManageChild(export_type_w);

      XmStringFree (bw_fig);
      XmStringFree (col_fig);
      XmStringFree(eps_bw);
      XmStringFree(eps_co);

#ifdef XPM
      XmStringFree(xpm);
#endif

#ifdef GL
      XmStringFree (gl2ps_eps);
      XmStringFree (gl2ps_pdf);
      XmStringFree (gl2ps_svg);
#endif
      
      XmStringFree(title);

      XtAddCallback(e_dialog, XmNokCallback, export_file_cb, (XtPointer) 3);
      XtAddCallback(e_dialog, XmNcancelCallback, 
                     (XtCallbackProc)XtUnmanageChild, NULL);

      /* No help available ... */

      XtUnmanageChild
        (XmFileSelectionBoxGetChild(e_dialog,XmDIALOG_HELP_BUTTON));

    }
      
    /* Change to a directory that was just recently operated in any of the
       file selection dialogs and refresh the file list so that newly
       created files do show up */
    
    XtVaSetValues (e_dialog, XmNdirectory, get_current_directory (), NULL);
    XmFileSelectionDoSearch (e_dialog, NULL);
      
    XtAddCallback (e_dialog, XmNmapCallback, place_dialog_cb, NULL);

    XtManageChild (e_dialog);
    XtPopup (XtParent (e_dialog), XtGrabNone);
    break;

  case 5:/* aro: Create print dialog */
    if (!p_dialog) {
      p_dialog = XmCreateMessageDialog(toplevel, "print dialog", NULL, 0);

      title=XmStringCreateLocalized ("Print File");
      XtVaSetValues(p_dialog, XmNdialogTitle, title, NULL);
      XmStringFree (title);

      /* Change OK button label to "Print" */
      temp_widget = XmMessageBoxGetChild(p_dialog, XmDIALOG_OK_BUTTON);
      title=XmStringCreateLocalized("Print");
      XtVaSetValues(temp_widget, XmNlabelString, title, NULL);
      XmStringFree (title);
      
      /* Main RowColumn */
      rc[0] = XtVaCreateManagedWidget("rc_main",
                                      xmRowColumnWidgetClass,
                                      p_dialog,
                                      XmNorientation, XmVERTICAL,
                                      NULL);
      /* First sub RowColumn */
      rc[1] = XtVaCreateManagedWidget("rc_sub_1",
                                      xmRowColumnWidgetClass,
                                      rc[0],
                                      XmNorientation, XmHORIZONTAL,
                                      NULL);

      XtVaCreateManagedWidget("File:",
                              xmLabelWidgetClass,
                              rc[1],
                              NULL);
      
      /* If current_file_name is an empty string, assume no file
         has been loaded */
      if(strcmp(current_file_name, "") == 0)
        title = XmStringCreateLocalized ("(no file loaded)");
      else
        title = XmStringCreateLocalized (current_file_name);

      XtVaCreateManagedWidget("file_name",
                              xmLabelWidgetClass,
                              rc[1],
                              XmNlabelString, title,
                              NULL);
      XmStringFree(title);

      /* Second sub RowColumn */
      rc[1] = XtVaCreateManagedWidget("rc_sub_2",
                                      xmRowColumnWidgetClass,
                                      rc[0],
                                      XmNorientation, XmHORIZONTAL,
                                      NULL);
      
      XtVaCreateManagedWidget("Printer:",
                              xmLabelWidgetClass,
                              rc[1],
                              NULL);

      /* Parse /etc/printcap to get list of available printers.
         User can still type in an arbitrary name */
      printcap_file = fopen("/etc/printcap", "r");

      i = 0, j = 0;
      if(printcap_file != NULL)
        {                        
          line_ptr = fgets(line, sizeof(line), printcap_file);
          while(line_ptr != NULL && j < MAX_PRINTERS)
            {
              /* If line[i] is an alphanumeric character, assume it is
                 part of printer name */
              if(isalnum((int)line[i]))
                {
                  printer_strings[j][i] = line[i];
                  i++;
                }
              /* If line[i] is '|' or ':' then we've reached end of
                 printer name, append '\0' to printer_strings
                 and get next line */
              else if(line[i] == '|' || line[i] == ':')
                {
                  printer_strings[j][i] = '\0';
                  line_ptr = fgets(line, sizeof(line), printcap_file);
                  i = 0;
                  j++;
                }
              /* If line[i] is not an alphanumeric character and 
                 it is not '|' or ':' then get next line of printcap_file */
              else
                {
                  line_ptr = fgets(line, sizeof(line), printcap_file);
                  i = 0;
                }
            }
          fclose(printcap_file);
        }

      /* Set first empty element of printer_strings to "ENDLIST" to denote end.
         If failure in opening printcap_file, then j == 0 */
      strcpy(printer_strings[j], "ENDLIST");

      /* Get default printer name if one of the environment 
         variables LPDEST or PRINTER is set */
      printer_name = getenv("LPDEST");
      if(printer_name == NULL)
        printer_name = getenv("PRINTER");
      if(printer_name == NULL)
	{
	  printer_name = alloca(32);
	  strcpy(printer_name, "(no default)");
	}
      
      /* Create printer name text field initialized with name of
         default printer */
      print_printer = XtVaCreateManagedWidget("p_name",
                                              xmTextFieldWidgetClass,
                                              rc[1],
                                              XmNcolumns, 20,
                                              XmNvalue, printer_name,
                                              NULL);
      /* Third sub RowColumn */
      rc[1] = XtVaCreateManagedWidget("rc_sub_3",
                                     xmRowColumnWidgetClass,
                                     rc[0],
                                     NULL);

      /* Convert printer strings to XmStrings */
      for(j = 0; strcmp(printer_strings[j], "ENDLIST"); j++)
        printer_xmstrings[j] = (XmString *)XmStringCreateLocalized(printer_strings[j]);


      /* Create scrolled list with names of first MAX_PRINTERS printers from
         print_cap file */
      i = 0;
      XtSetArg(args[i], XmNselectionPolicy, XmSINGLE_SELECT); i++;

      /* if j = 0, then no printers were found in /etc/printcap,
         so we don't want to set XmNitems to printer_xmstrings,
         or we'll get a warning about XmNitemCount not agreeing
         with XmNitems */
      if(j != 0) {XtSetArg(args[i], XmNitems, printer_xmstrings); i++;}
      XtSetArg(args[i], XmNitemCount, j); i++;
      XtSetArg(args[i], XmNvisibleItemCount, 5); i++;
      temp_widget = XmCreateScrolledList(rc[1], "Printers", args, i);
      
      XtManageChild(temp_widget);

      XtAddCallback(temp_widget, XmNsingleSelectionCallback, 
                    print_file_select_cb, print_printer);


      XtVaCreateManagedWidget("Copies:",
                              xmLabelWidgetClass,
                              rc[1],
                              NULL);

      /* Create text field to let user input number of copies to 
         be printed */
      XtVaCreateManagedWidget("num_copies",
                              xmTextFieldWidgetClass,
                              rc[1],
                              XmNvalue, "1",
                              XmNcolumns, 4,
                              NULL);
      
      /* Create radio box to let user select color or black & white
         output */
      rc[1] = XmCreateRadioBox(rc[0],"color/bw",NULL,0);
      XtVaSetValues(rc[1], XmNorientation, XmHORIZONTAL, NULL);

      XtVaCreateManagedWidget("color",
                              xmToggleButtonWidgetClass,
                              rc[1],
                              XmNset, True,
                              NULL);

      XtVaCreateManagedWidget("bw",
                              xmToggleButtonWidgetClass,
                              rc[1],
                              NULL);
      XtManageChild(rc[1]);

      /* Create Label and TextField to display current print command and format */
      XtVaCreateManagedWidget("Print command and format:",
                              xmLabelWidgetClass,
                              rc[0],
                              NULL);
      
      sprintf(print_command_string, "lpr -P%%printer%% -#%%num_copies%% %%filename%%");
      print_command = XtVaCreateManagedWidget("print_command_label",
                                              xmTextFieldWidgetClass,
                                              rc[0],
                                              XmNvalue, print_command_string,
                                              NULL);


      XtAddCallback(p_dialog, XmNokCallback, print_file_cb, print_command);

      /* No help available ... */
      XtUnmanageChild
        (XmMessageBoxGetChild(p_dialog,XmDIALOG_HELP_BUTTON));
    }else{
      /* Update file name */
      temp_widget = XtNameToWidget(p_dialog, "rc_main.rc_sub_1.file_name");

      /* If current_file_name is an empty string, assume no file
         has been loaded */
      if(strcmp(current_file_name, "") == 0)
        title = XmStringCreateLocalized ("(no file loaded)");
      else
        title = XmStringCreateLocalized (current_file_name);
      XtVaSetValues(temp_widget, XmNlabelString, title, NULL);
      XmStringFree(title);
    }

      XtAddCallback (p_dialog, XmNmapCallback, place_dialog_cb, NULL);

    XtManageChild (p_dialog);
    XtPopup (XtParent (p_dialog), XtGrabNone);
    break;

  case 6:
    cleanup_cb(toplevel, NULL, NULL); /* aro - cleanup before exit */
    exit (0);
    break;
  }

}


void
file_type_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{
  file_type = (int) client_data;
}

void
export_type_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{
  export_type = (int) client_data;
}


void
merge_type_cb (Widget widget,
               XtPointer client_data,
               XtPointer call_data)
{
  
  merge_type = (enum merge_types) client_data;

}

void
load_file_cb(Widget dialog, XtPointer client_data, XtPointer call_data)
{

  void open_file(char *, Boolean);

  /* aro - moved char file_name[1024] to char current_file_name[1024] 
     in globals.h */

  char *file = NULL;

  XmFileSelectionBoxCallbackStruct *cbs =
    (XmFileSelectionBoxCallbackStruct *) call_data;

  if (cbs) {
    if (!XmStringGetLtoR (cbs->value, XmFONTLIST_DEFAULT_TAG, &file))
      return; /* internal error */

    /* Save directory shown in the dialog so that all the other file selection
       dialogs could be made to operate the very same directory */

    set_current_directory (cbs->dir);
        
    /* save file name and open the file */
    
    (void) strcpy(current_file_name, file);
    open_file(current_file_name, FALSE);
    XtFree (file); /* free allocated data from XmStringGetLtoR() */
  }
  
  XtUnmanageChild(dialog); 
  
}

void
save_file_cb(Widget dialog, XtPointer client_data, XtPointer call_data)
{
  
  void write_file(char *);

  char *file_name, message_string[1024];

  XmFileSelectionBoxCallbackStruct *cbs =
    (XmFileSelectionBoxCallbackStruct *) call_data;
   
  if (cbs) {
    if (!XmStringGetLtoR (cbs->value, XmFONTLIST_DEFAULT_TAG, &file_name))
      return; /* internal error */

    /* Save directory shown in the dialog so that all the other file selection
       dialogs could be made to operate the very same directory */
    
    set_current_directory (cbs->dir);

    /* Check if we can open the file */

    if(fopen(file_name, "w") == NULL)
      {
        sprintf(message_string, "Cannot open %s for write", file_name);
        echo_to_message_area(message_string);
      }
    else
      {
        write_file(file_name);
      }

    XtFree (file_name); /* free allocated data from XmStringGetLtoR() */
  }
  
  XtUnmanageChild(dialog); 
  
}

void
merge_file_cb (Widget dialog,
               XtPointer client_data,
               XtPointer call_data)
{
  double get_angle_axis(double *);
  void open_file(char *, Boolean);
  void rotate_atoms(double *,double,Boolean,Boolean);
  void frame_content_to_atoms (int);
  struct frame * get_selected_frame (void);

  int i, j, merge_no_atoms;
  
  char *file = NULL, buf[BUFSIZ], eof_string[32] = "";
  char merge_file_name[1024], *temp_file_name;

  double angle, axis[3];
  
  Boolean eof_merge_file = False;

  FILE *merge_file, *temp_file;

  struct frame *this_frame;

  XmFileSelectionBoxCallbackStruct *cbs =
    (XmFileSelectionBoxCallbackStruct *) call_data;
  
  if (cbs) {
    if (!XmStringGetLtoR (cbs->value, XmFONTLIST_DEFAULT_TAG, &file))
      return; /* internal error */
    (void) strcpy (merge_file_name, file);
    XtFree (file); /* free allocated data from XmStringGetLtoR() */

    /* Save directory shown in the dialog so that all the other file
       selection dialogs could be made to operate the very same directory */
    
    set_current_directory (cbs->dir);
  }

  
  XtUnmanageChild(dialog); 

  merge_file = fopen (merge_file_name, "r");
  
  if(merge_file == NULL)
    {
      sprintf(buf,"No file %s",gnu_basename(merge_file_name));
      echo_to_message_area(buf);
      return;
    }
  
  /* Set up temporary file */

  temp_file_name =  alloca (32);
  strcpy (temp_file_name, "/tmp/xmakemol.XXXXXX");

  temp_file = fdopen (mkstemp (temp_file_name), "w");

  if(temp_file == NULL)
    {
      sprintf(buf,"Cannot open %s",temp_file_name);
      echo_to_message_area(buf);
      return;
    }
      
  /* Make sure none of this is drawn */

  inhibit_canvas_callback = 1;
  
  /* Reset the angle_axis_matrix */

  for(i = 0; i < 3; i++)
    {
      for(j = 0; j < 3; j++)
        {
          angle_axis_matrix[i][j] = global_matrix[i][j];
        }
    }

  /* Loop over frames */
  
  for(i = 0; i < no_frames; i++)
    {

      int frame_visible_atoms;
      int count_visible_atoms_in_frame (struct frame *);

      /* Rewind the merge file if appropriate */

      if (merge_type == MERGE_FRAME)
        {
          rewind (merge_file);
        }

      /* Check to see if we have reached the end of the merge file */

      if (eof_merge_file == False)
        {
          (void)fgets(buf, sizeof (buf), merge_file);

          if (feof (merge_file))
            {
              merge_no_atoms = 0;
              eof_merge_file = True;
              sprintf (eof_string, " - merge stopped after frame %d", i);
            }
          else
            {
              sscanf (buf, "%d", &merge_no_atoms);

              /* Gobble the comment line */
              (void)fgets (buf, sizeof (buf), merge_file);
            }
        }
          
      frame_content_to_atoms (i);
      this_frame = get_selected_frame();
      
      frame_visible_atoms =
        count_visible_atoms_in_frame (this_frame);

      fprintf(temp_file, "%d\n", frame_visible_atoms + merge_no_atoms);
      fprintf(temp_file, "%s\n", this_frame->comment);
      
      angle = get_angle_axis(axis);
      
      rotate_atoms(axis, angle, 0, False);
          
      for(j = 0; j < no_atoms; j++)
        {
          if(atoms[j].visi == 1)
            {

              /* We need to add the global_vector component as there
                 may have been translations */

              fprintf(temp_file,"%2s %12.6f %12.6f %12.6f\n",
                      atoms[j].label,
                      atoms[j].x + global_vector[0],
                      atoms[j].y + global_vector[1],
                      atoms[j].z + global_vector[2]);
            }
        }

      /* Now insert the merged frame if available */

      if (eof_merge_file == False)
        {
          for (j = 0; j < merge_no_atoms; j++)
            {
              (void)fgets (buf, sizeof (buf), merge_file);
              fputs (buf, temp_file);
            }
        }
    }

  fclose(temp_file);
  
  inhibit_canvas_callback = 0;

  open_file(temp_file_name, FALSE);
  
  sprintf(buf,"Files merged%s", eof_string);
  echo_to_message_area(buf);

  unlink (temp_file_name);

  /* If the Edit->Positions dialog is open, make the merged atoms the
     only ones that we can move */

  if (edit_posn_dialog != NULL)
    {
      for (i = 0; i < no_atoms; i++)
        {
          if (i < no_atoms - (merge_no_atoms))
            {
              atoms[i].edit = 0;
            }
        }
    }
}


void
export_file_cb(Widget dialog, XtPointer client_data, XtPointer call_data)
{
  
  void export_file(char *);

  char *file_name, message_string[1024];

  XmFileSelectionBoxCallbackStruct *cbs =
    (XmFileSelectionBoxCallbackStruct *) call_data;
   
  if (cbs) {
    if (!XmStringGetLtoR (cbs->value, XmFONTLIST_DEFAULT_TAG, &file_name))
      return; /* internal error */

    /* Save directory shown in the dialog so that all the other file selection
       dialogs could be made to operate the very same directory */
    
    set_current_directory (cbs->dir);

    /* Check if we can open the file */

    if(fopen(file_name, "w") == NULL)
      {
        sprintf(message_string, "Cannot open %s for write", file_name);
        echo_to_message_area(message_string);
      }
    else
      {
        export_file(file_name);
      }

    XtFree (file_name); /* free allocated data from XmStringGetLtoR() */
  }
  
  XtUnmanageChild(dialog); 
  
}


void
print_file_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{
  void write_eps(char *, FILE *, Boolean);
  void echo_to_message_area(char *);

  char *string, *temp_file_name, *p_name, *copies, p_command[1024];
  const char printer_str[] = "%printer%",
             numcopies_str[] = "%num_copies%",
             filename_str[] = "%filename%";
  char *printer_pos, *numcopies_pos, *filename_pos;
  int num_copies, i, j;
  Boolean print_color;
  Widget temp_w, print_command = (Widget) client_data;
  FILE * temp_f, *eps_file;
  XmString errorMsg;

  /* Get widget ID of color toggle */
  temp_w = XtNameToWidget(widget, "rc_main.color/bw.color");

  /* Get value of color toggle (true or false) */
  XtVaGetValues(temp_w, XmNset, &print_color, NULL);

  /* Create unique file name in /tmp */
  temp_file_name = alloca (32);
  strcpy (temp_file_name, "/tmp/xmakemol.XXXXXX");

  eps_file = fdopen (mkstemp (temp_file_name), "w");

  /* Write eps file to /tmp for printing */
  write_eps ("", eps_file, print_color);

  /* Get the name of the printer selected */
  temp_w = XtNameToWidget(widget, "rc_main.rc_sub_2.p_name");
  XtVaGetValues(temp_w, XmNvalue, &p_name, NULL);

  /* Get the number of copies to be printed */
  temp_w = XtNameToWidget(widget, "rc_main.rc_sub_3.num_copies");
  XtVaGetValues(temp_w, XmNvalue, &copies, NULL);
  num_copies = atoi(copies);

  /* Determine whether user is printing in color or black & white */
  temp_w = XtNameToWidget(widget, "rc_main.color/bw.color");
  XtVaGetValues(temp_w, XmNset, &print_color, NULL);

  if(strcmp(p_name, "(no default)") == 0 ||
     strcmp(p_name, "") == 0)
    {
      echo_to_message_area("Print Error - No Printer Selected");
    }
  else if(!(temp_f = fopen(current_file_name, "r")))
    {
      echo_to_message_area("Print Error - No File Loaded");
    }
  else
    {
      fclose(temp_f);

      string = alloca (32);
      XtVaGetValues(print_command, XmNvalue, &string, NULL);

      /* Create print command string */

      /* Check if user put all three variable fields in print command string */
      printer_pos = strstr(string, printer_str);
      numcopies_pos = strstr(string, numcopies_str);
      filename_pos = strstr(string, filename_str);

      if(printer_pos && numcopies_pos && filename_pos)
	{
	  for(i = 0, j = 0; string[i] != '\0'; i++)
	    {
	      if(string[i] == '%')
		{
		  switch(string[i+1])
		    {
		      case 'p':
			strcat(p_command, p_name);
			j += strlen(p_name);
			i += strlen(printer_str) - 1;
			break;

		      case 'n':
			strcat(p_command, copies);
			j += strlen(copies);
			i += strlen(numcopies_str) - 1;
			break;

		      case 'f':
			strcat(p_command, temp_file_name);
			j += strlen(temp_file_name);
			i += strlen(filename_str) - 1;
			break;

		      default:
			p_command[j] = string[i];
			p_command[j+1] = '\0';
			j++;
			break;
		    }
		}
	      else
		{
		  p_command[j] = string[i];
		  p_command[j+1] = '\0';
		  j++;
		}
	    }
	  (void)system(p_command);
	  echo_to_message_area("Print job sent");
	}
      else
        {
          errorMsg = XmStringCreateLocalized("Need to have %printer%, %num_copies%,\nand %filename% in print command string.");
          i = 0;
          XtSetArg(args[i], XmNmessageString, errorMsg); i++;
          temp_w = XmCreateErrorDialog(toplevel, "error", args, i);
          XtUnmanageChild(XmMessageBoxGetChild(temp_w,XmDIALOG_CANCEL_BUTTON));
          XtUnmanageChild(XmMessageBoxGetChild(temp_w,XmDIALOG_HELP_BUTTON));
          XtManageChild(temp_w);
        }
    }

  /* Remove eps file from /tmp */
  remove(temp_file_name);
}

void
print_file_select_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{
  XmListCallbackStruct *cbs = (XmListCallbackStruct *) call_data;
  Widget w = (Widget) client_data;
  char *p_select;

  /* Convert selected XmString to regular string */
  XmStringGetLtoR(cbs->item, XmFONTLIST_DEFAULT_TAG, &p_select);
  
  /* Update value of printer_name text field with 
     currently selected printer */
  XtVaSetValues(w, XmNvalue, p_select, NULL);
}

void
open_file(char *file_name, Boolean revert)
{
  void malloc_failed(char *);
  void pre_load_file_hooks (Boolean);
  void post_load_file_hooks (Boolean);
  void store_coords (void);
  void read_aux (char *);
  void set_selected_frame (struct frame *);

  int i, no_atoms_in_frame;
  int molecule;                 /* Molecule index */

  char buf[BUFSIZ], atom_label[4], mode[16];
  char *line_stat = NULL;

  FILE *in_file;
  Widget widget; /* this is only needed to show the bbox if it is
                    given in the .xyz-file */

#ifdef GL
  enum gl_render_types gl_current_render_type = DEFAULT;
#endif

  if (opendir (file_name))
    {
      sprintf (buf, "Cannot open directory %s", file_name);
      echo_to_message_area (buf);
      return;
    }
  else if (strcmp (file_name, "<STDIN>") == 0)
    {
      /* This means read from stdin */
      in_file = stdin;
      strcpy (file_name, "<STDIN>");
      strcpy (current_file_name, "<STDIN>");
    }
  else
    {
      /* Read from disk */
      in_file = fopen (file_name, "r");
    }

  if (in_file == NULL)
    {
      sprintf (buf, "xmakemol: cannot open %s for read", gnu_basename (file_name));
      echo_to_message_area (buf);
      return;
    }

  /* At this point, we're willing to read in a file... */

  pre_load_file_hooks (revert);

  /* read in frames */

  no_frames = 0;

  /* change to 0 if we find otherwise */

  while (1)
    {
      /* line declaring number of atoms */

      line_stat = fgets (buf, sizeof (buf), in_file);

      if (line_stat == NULL)
        {
          if (no_frames == 0)
            {
              fprintf (stderr, "xmakemol: read zero frames in file %s\n",
                       file_name);
              exit (1);
            }
          break;
        }

      sscanf (buf, "%d", &no_atoms_in_frame);
      
      no_frames++;

      /* Allocate the pointer to the frame */

      if (no_frames == 1)
        {
          first_frame = (struct frame *) malloc (sizeof (struct frame));
          current_frame = first_frame;
        }
      else
        {
          current_frame->next = (struct frame *) malloc (sizeof (struct frame));
          current_frame = current_frame->next;
        }

      if (current_frame == NULL)
        {
          malloc_failed ("current_frame");
        }

      /* Reset properties */

      current_frame->perspective_depth = 0;
      current_frame->perspective_scale = 0;

      current_frame->bbox_available = FALSE;

      molecule = 0;

      /* Allocate the contents of the frame */

      current_frame->no_atoms = no_atoms_in_frame;

      current_frame->atom =
        (struct atom *) malloc (no_atoms_in_frame * sizeof (struct atom));

      if (current_frame->atom == NULL)
        {
          malloc_failed ("current_frame->atom");
        }
    
      /* comment line -- NULL terminate */
      
      (void)fgets (buf, sizeof (buf), in_file);
      buf[strlen (buf) - 1] = '\0';
      strcpy (current_frame->comment, buf);
    
      for (i = 0; i < no_atoms_in_frame; i++)
        {

          void make_label_uppercase (char *);

          char *optional = NULL, prop[16], ucased_label[4];

          int type_found;

          (void)fgets(buf, sizeof (buf), in_file);

          /* read and store the atom coordinates */

          sscanf (buf, "%s %lf %lf %lf",
                  atom_label,
                  &current_frame->atom[i].x,
                  &current_frame->atom[i].y,
                  &current_frame->atom[i].z);



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	  current_frame->atom[i].opx = current_frame->atom[i].x;
          current_frame->atom[i].opy = current_frame->atom[i].y;
          current_frame->atom[i].opz = current_frame->atom[i].z;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    



      /* Look for optional properties */

          current_frame->atom[i].has_vector = 0;
          current_frame->atom[i].has_rgb = False;
          current_frame->atom[i].has_color_name = False;

          /* Read vectors (MAX_VECTORS_PER_ATOM allowed) */

          optional = buf;

          while (optional && strstr (optional, "atom_vector")) /* Vectors */
            {
              int nv;
              double x, y, z;

              optional = strstr (optional, "atom_vector");

              nv = current_frame->atom[i].has_vector;

              if (current_frame->atom[i].has_vector == MAX_VECTORS_PER_ATOM) /* One too many here */
                {
                  printf ("Too many Vectors (%d) read for atom %d\n", nv + 1, i + 1);
                  exit (2);
                }

              sscanf (optional, "%s %lf %lf %lf", prop, &x, &y, &z);

              current_frame->atom[i].vx[nv] = x;
              current_frame->atom[i].vy[nv] = y;
              current_frame->atom[i].vz[nv] = z;
             
              /* Move pointer past match */

              current_frame->atom[i].has_vector++;
              optional++;
            }

          /* Read ellipse data */

          optional = buf;

          if (optional && strstr (optional, "ellipse")) /* Ellipse properties */
            {
              double x, y, z, alpha, beta, gamma;

              optional = strstr (optional, "ellipse");

              sscanf (optional, "%s %lf %lf %lf %lf %lf %lf",
                      prop, &x, &y, &z, &alpha, &beta, &gamma);

              current_frame->atom[i].is_ellipse = 1;

              current_frame->atom[i].euler[0] = alpha;
              current_frame->atom[i].euler[1] = beta;
              current_frame->atom[i].euler[2] = gamma;

              current_frame->atom[i].shape[0] = x;
              current_frame->atom[i].shape[1] = y;
              current_frame->atom[i].shape[2] = z;

              optional++;
            }
          else
            {
              current_frame->atom[i].is_ellipse = 0;
            }

          /* Read crystal data */

          optional = buf;

          if (optional && strstr (optional, "crystal_origin"))
            {
              void set_crystal_origin (double *);

              double x[3];

              optional = strstr (optional, "crystal_origin");

              sscanf (optional, "%s %lf %lf %lf",
                      prop, &x[0], &x[1], &x[2]);

              set_crystal_origin (x);
              optional++;
            }

          optional = buf;

          if (optional && strstr (optional, "crystal_images"))
            {
              void set_crystal_images (int *);

              int p[3];

              optional = strstr (optional, "crystal_images");

              sscanf (optional, "%s %d %d %d",
                      prop, &p[0], &p[1], &p[2]);

              set_crystal_images (p);
              optional++;
            }

          optional = buf;

          while (optional && strstr (optional, "crystal_vector"))
            {
              void set_crystal_vector (int, double *);

              int i;
              double x[3];

              optional = strstr (optional, "crystal_vector");

              sscanf (optional, "%s %d %lf %lf %lf",
                      prop, &i, &x[0], &x[1], &x[2]);

              set_crystal_vector (i, x);
              optional++;
            }

          while (optional && strstr (optional, "atom_rgb"))
            {
              double red, green, blue;

              optional = strstr (optional, "atom_rgb");

              sscanf (optional, "%s %lf %lf %lf",
                      prop, &red, &green, &blue);

              current_frame->atom[i].red = red;
              current_frame->atom[i].green = green;
              current_frame->atom[i].blue = blue;

              current_frame->atom[i].has_rgb = True;
              optional++;
            }

          while (optional && strstr (optional, "atom_color"))
            {
              char color_name[32];

              optional = strstr (optional, "atom_color");

              sscanf (optional, "%s %s", prop, color_name);

              strcpy (current_frame->atom[i].color_name, color_name);
              current_frame->atom[i].has_color_name = True;
              optional++;
            }


          /* Read bounding box data */
	  
          optional = buf;

          if (optional && strstr (optional, "bbox_xyz"))
            {
              double xmin, xmax, ymin, ymax, zmin, zmax;
	      
              optional = strstr (optional, "bbox_xyz");
	      
              sscanf (optional, "%s %lf %lf %lf %lf %lf %lf",
                      prop, &xmin, &xmax, &ymin, &ymax, &zmin, &zmax);
      
              current_frame->bbox[0][0] = xmin;
              current_frame->bbox[0][1] = xmax;
	      
              current_frame->bbox[1][0] = ymin;
              current_frame->bbox[1][1] = ymax;

              current_frame->bbox[2][0] = zmin;
              current_frame->bbox[2][1] = zmax;

	      current_frame->bbox_available = TRUE;

	      /* If bbox_xyz is given in the input-file we show the bounding box.
		 and use the given bbox as default. */
	      bbox_type = FROM_FILE;
	      
	      bbox_flag = 1;
              if((widget = XtNameToWidget(nth_menu[4], "button_7")))
                XtVaSetValues(widget, XmNset, True, NULL);

              optional++;
            }


          while (optional && strstr (optional, "molecule"))
            {
              optional = strstr (optional, "molecule");

              /* Don't increment if this is the first atom of a frame;
                 that this is the start of a molecule is implied. */

              if (i > 0) molecule++;
              optional++;
            }

          current_frame->atom[i].molecule = molecule;

#ifdef GL
          while (optional && strstr (optional, "render_ball_and_stick"))
            {
              optional = strstr (optional, "render_ball_and_stick");

              gl_current_render_type = BALL_AND_STICK;
              optional++;
            }

          while (optional && strstr (optional, "render_tube"))
            {
              optional = strstr (optional, "render_tube");

              gl_current_render_type = TUBES;
              optional++;
            }

          current_frame->atom[i].gl_render_type = gl_current_render_type;
#endif /* GL */

          /* store the atom label */

          strcpy (current_frame->atom[i].label, atom_label);

          /* make an uppercased copy */

          strcpy (ucased_label, atom_label);
          make_label_uppercase (ucased_label);

          /* reset properties */

          current_frame->atom[i].visi = 1;

          /* See if this type of atom has already been read in -- if
             not, add it to the end of the type list */

          type_found = 0;

          current_type = first_type;

          while (1)
            {
              if (current_type == NULL) break;

              if (strcmp (ucased_label, current_type->label) == 0)
                {
                  type_found = 1;
                }

              if (current_type->next == NULL) break;

              current_type = current_type->next;
            }

          if (type_found == 0)
            {
              if (first_type == NULL)
                {
                  first_type =
                    (struct type *) malloc (sizeof (struct type));
                  current_type = first_type;
                }
              else
                {
                  current_type->next =
                    (struct type *) malloc (sizeof (struct type));

                  current_type = current_type->next;                
                }

              strcpy (current_type->label, ucased_label);
              current_type->next = NULL;
            }
        }
    }

  /* NULL terminate the list of frames */

  current_frame->next = NULL;
  
  /* Are any of the atoms hydrogen? */

  any_hydrogen = False;
  
  for (i = 0; i < 4; i++)
    {
      selected[i]=-1;
    }
  
  sel_init = 0;    /* remove any selections */
  atoms_sorted = 0;

  no_atoms = first_frame->no_atoms;

  /* Read auxiliary file if it exists */

  read_aux (file_name);

  post_load_file_hooks (revert);

  store_coords();

  if (revert)
    {
      strcpy (mode, "reverted");
    }
  else
    {
      strcpy (mode, "read");
    }

  if(no_frames == 1)
    {
      sprintf(buf, "File %s %s: 1 frame", gnu_basename (file_name), mode);
    }
  else
    {
      sprintf(buf, "File %s %s: %d frames", gnu_basename (file_name),
              mode, no_frames);
    }

  echo_to_message_area (buf);
}


void
pre_load_file_hooks (Boolean revert)
{
  void read_elements_file (void);
  void reset_crystal_status (void);
  void reset_frame_list (void);
  void reset_type_list (void);
  void save_perspective_data (void);

  /* List of things to do before reading in new data */

  if (revert) save_perspective_data ();

  read_elements_file ();
  reset_crystal_status ();
  reset_frame_list ();
  reset_type_list ();
}


void
reset_frame_list (void)
{

  struct frame *next_frame;

  current_frame = first_frame;

  while (current_frame != NULL)
    {
      next_frame = current_frame->next;
      free ((struct atom *) current_frame->atom);
      free ((struct frame *) current_frame);
      current_frame = next_frame;
    }

  first_frame = NULL;

}


void
reset_type_list (void)
{

  struct type *next_type;

  current_type = first_type;

  while (current_type != NULL)
    {
      next_type = current_type->next;
      free ((struct type *) current_type);
      current_type = current_type->next;
      current_type = next_type;
    }

  first_type = NULL;

}

void
calculate_frame_bboxes (void)
{
  void frame_content_to_atoms (int);

  struct frame *this_frame;
  int temp_frame = frame_no;

  int i;

  this_frame = first_frame;
  
  for(i = 0; i < no_frames; i++)
    {
      frame_content_to_atoms (i);

      /* If no bbox given for this frame, use the one from the first
         frame, if it exists */

      if (! this_frame->bbox_available)
        {
	  if (first_frame->bbox_available)
	    {
              this_frame->bbox[0][0] = first_frame->bbox[0][0];
              this_frame->bbox[0][1] = first_frame->bbox[0][1];

              this_frame->bbox[1][0] = first_frame->bbox[1][0];
              this_frame->bbox[1][1] = first_frame->bbox[1][1];

              this_frame->bbox[2][0] = first_frame->bbox[2][0];
              this_frame->bbox[2][1] = first_frame->bbox[2][1];

              this_frame->bbox_available = TRUE;
	    }
        }
      this_frame = this_frame->next;
    }
  frame_content_to_atoms (temp_frame);
}


void post_load_file_hooks (Boolean revert)
{

  void canvas_cb (Widget, XtPointer, XtPointer);
  void reallocate_atom_types (void);
  void reset_any_atoms (void);
  void reset_global_matrix (void);
  void set_default_depth (void);
  void update_bond_matrix (Boolean);
  void frame_content_to_atoms (int);
  void deactivate_region (void);
  void update_some_dialogs (void);
  void update_frame_label (void);
  void set_selected_frame (struct frame *);
  void check_frame_sizes (void);
  void calculate_frame_bboxes (void);
  void update_bbox (void);
  void load_perspective_data (void);

  /* List of things to do after reading in new data */

  check_frame_sizes ();
  set_selected_frame (NULL);
  reset_any_atoms ();           /* Must precede reallocate_atom_types */
  reallocate_atom_types ();

  if (revert)
    {
      double get_angle_axis (double *);
      void rotate_atoms (double *, double, Boolean, Boolean);

      int i, j;
      double angle, axis[3];

      frame_content_to_atoms (frame_no);

      /* get angle-axis rotation from global_matrix positions... */

      for (i = 0; i < 3; i++)
        {
          for (j = 0; j < 3; j++)
            {
              angle_axis_matrix[i][j] = global_matrix[i][j];
            }
        }

      angle = get_angle_axis (axis);
      rotate_atoms (axis, angle, 0, False);

      load_perspective_data ();
    }
  else
    {
      frame_no = 0;
      frame_content_to_atoms (frame_no);
      reset_global_matrix ();

      if (! aux_file_exists_p) set_default_depth ();
    }

  update_bond_matrix (True);
  deactivate_region ();
  update_frame_label ();
  update_some_dialogs ();
  calculate_frame_bboxes ();
  update_bbox ();

  redraw = 1;
  canvas_cb (canvas, NULL, NULL);

}

void
reset_any_atoms (void)
{

  int i, elem_found;

  for (i = 0; i < no_elements; i++)
    {
      element[i].any_atoms = False;
    }

  current_type = first_type;

  while (current_type != NULL)
    {

      elem_found = 0;

      for (i = 0; i < no_elements; i++)
        {

          if (strcmp (current_type->label, element[i].label) == 0)
            {
              element[i].any_atoms = True;
              current_type->elem = i;
              elem_found = 1;
            }
        }
     
      if (elem_found == 0)
        {
          fprintf (stderr, "xmakemol: element %s not found in elements file\n",
                   current_type->label);
          element[0].any_atoms = True;
          current_type->elem = 0; /* Dummy */
          strcpy (current_type->label, "DU"); /* Dummy */
        }

      current_type = current_type->next;
    }

  no_atom_types = 0;

  for (i = 0; i < no_elements; i++)
    {
      if (element[i].any_atoms == True)
        {
          no_atom_types++;
        }
    }

}


/* Expects label to be uppercase */

static Boolean dummy_message_not_shown = True;

int
get_element_index_for_type (char *label)
{

  int element_index = 0;

  current_type = first_type;

  while ((current_type != NULL) && (element_index == 0))
    {
      if (strcmp (current_type->label, label) == 0)
        {
          element_index = current_type->elem;
        }
      
      current_type = current_type->next;
    }

  if ((element_index == 0) || (dummy_message_not_shown))
    {
      /* FIXME: let's have a console for this, so that messages aren't
         swallowed up by subsequent ones */

      echo_to_message_area ("Dummy atoms detected");
      
      dummy_message_not_shown = False;
    }

  return (element_index);
}
         
      
void
reset_global_matrix (void)
{
  int i, j;

  /* To accommodate frames, and keep the relative orientations the
   * same we adopt this method:-
   *
   * We monitor a rotation matrix relative to the global axes. When
   * the frame is changed, the atomic coordinates are rotated into
   * this basis. */

  for (i = 0; i < 3; i++)
    {
      global_vector[i] = 0;

      for (j = 0; j < 3; j++)
        {
          if (i == j)
            {
              global_matrix[i][j] = 1;
            }
          else
            {
              global_matrix[i][j] = 0;
            }
        }
    }
}


void
reallocate_atom_types (void)
{
  int i, typei;

  if (atom_types != NULL)
    {
      free ((int *) atom_types);
    }

  atom_types = (int *) malloc (no_atom_types * sizeof (int));

  typei = 0;
  
  for (i = 0; i < no_elements; i++)
    {
      if(element[i].any_atoms)
        {
          atom_types[typei] = i;
          typei++;
        }
    }
}


void
assign_atom_types (void)
{
  int i, j;

  for (i = 0; i < no_atoms; i++)
    {
      for (j = 0; j < no_atom_types; j++)
        {
          if(strcmp (atoms[i].uppercase_label,
                     element[atom_types[j]].label) == 0)
            {
              atoms[i].type = j;
            }
        }
    }
}


void
set_default_depth (void)
{

  int xm_nint (double);
  void set_canvas_scale (double);
  void set_depth_all_frames (int);
  void set_scale_all_frames (int);
  void set_z_depth (double);

  int i;

  double x, y, z, r, r_max;

  r_max = 0;

  for (i = 0; i < no_atoms; i++)
    {
      x = atoms[i].x;
      y = atoms[i].y;
      z = atoms[i].z;

      r = (x * x) + (y * y) + (z * z);

      if (r == 0.0)
        {
          r = atoms[i].vdw_rad;
        }

      if (r > r_max)
        {
          r_max = r;
        }
    }

  r_max = sqrt (r_max);
  
  depth = xm_nint (DEPTH + r_max);          /* Maybe use z_max? */

  /* Pass the value of the depth to set_z_depth */

  set_depth_all_frames (depth);
  set_z_depth (depth);

  /* Pass the value of the original scale to set_canvas_scale - this
     could be set such that a point at r_max touches the side of the
     canvas, when in the correct orientation. We usually would like to room
     at the edge and an additional factor of 1.25 seems to work reasonably
     well */
  
  {
    int fudge = 2.0, v_scale_int;
    double scale;

    scale = canvas_width / (2.0 * fudge * r_max);
    set_canvas_scale (scale);

    v_scale_int = xm_nint (100.0 * log (scale + 1) / log(2.0));
    set_scale_all_frames (v_scale_int);
  }

  /* Update gl_eye so inital GL rendered image looks similar to initial
     X11 rendered image */
#ifdef GL
  {
    double get_gl_fov();
    double get_canvas_scale();
    double get_z_depth();
    void set_gl_eye(double);
    double get_gl_eye(void);

    double new_gl_eye;
    double gl_fov_rad;
    double scale_to_width;
    Widget eye_scale;
   
    gl_fov_rad = get_gl_fov() * (PI / 180);
    scale_to_width = (get_canvas_scale() * get_z_depth()) / (float)canvas_width;
    new_gl_eye = (r_max * 2 * scale_to_width) / tan(gl_fov_rad / 2);

    set_gl_eye(new_gl_eye);
    
    if(gl_render_dialog){
      eye_scale = XtNameToWidget(gl_render_dialog,
                                 "row_column.hor_row_column.eye_scale");
      XtVaSetValues(eye_scale, XmNvalue, (int) get_gl_eye(), NULL);
    }
    
  }
#endif /* GL */

}


void
write_file(char *file_name)
{
  void write_ascii_all(char *);
  void write_ascii_frame(char *, Boolean);
  void write_aux (char *);

  switch (file_type)
    {
    case 0:
      write_ascii_all(file_name);
      break;
    case 1:
      write_ascii_frame(file_name, False); /* No connectivities */      
      break;
    case 2:
      write_ascii_frame(file_name, True); /* With connectivities */
      break;
    case 3:                     /* Auxiliary information */
      write_aux (file_name);
      break;
     }
}

void
export_file(char *file_name)
{
  void write_eps(char *, FILE *, Boolean);
  void write_fig_file (char *, Boolean);
#ifdef XPM
  int write_xpm(char *, int);
#ifdef GL
  Boolean render_using_gl_p (void);
  void canvas_cb(Widget, XtPointer, XtPointer);
  void set_gl_copy_canvas (int);
#endif /* GL */
#endif /* XPM */

#ifdef GL
  void write_gl2ps (char *, GLint);
#endif

  switch (export_type)
    {
    case 0:
      write_fig_file (file_name, False);
      break;
    case 1:
      write_fig_file (file_name, True);
      break;
    case 2:
      write_eps(file_name, NULL, False); /* False = black and white */
      break;
    case 3:
      write_eps(file_name, NULL, True);  /* True  = colour */
      break;
#ifdef XPM
    case 4:
#ifdef GL
      /* Under these circumstances, we have to force a redraw to get
         the pixmap saved */
      if (render_using_gl_p ())
        {
          set_gl_copy_canvas (1);
          redraw = 1;
          canvas_cb (canvas, NULL, NULL);
          set_gl_copy_canvas (0);
        }
#endif /* GL */
      write_xpm(file_name, 0);
      break;
#endif

#ifdef GL
    case 5:
      write_gl2ps (file_name, GL2PS_EPS);
      break;
    case 6:
      write_gl2ps (file_name, GL2PS_PDF);
      break;
    case 7:
      write_gl2ps (file_name, GL2PS_SVG);
      break;
#endif
    }
}


void
write_ascii_frame(char *file_name, Boolean with_connectivities)
{

  int count_visible_atoms (void);
  struct frame * get_selected_frame (void);

  int i, no_visible_atoms;

  char buf[BUFSIZ];

  FILE *out_file;

  struct frame *this_frame;

  out_file=fopen(file_name,"w");

  this_frame = get_selected_frame ();
  
  no_visible_atoms = count_visible_atoms ();

  fprintf (out_file, "%d\n", no_visible_atoms);
  fprintf (out_file, "%s\n", this_frame->comment);
  
  for (i = 0; i < this_frame->no_atoms; i++)
    {
      if (atoms[i].visi)
        {
          fprintf (out_file, "%2s %12.6f %12.6f %12.6f", atoms[i].label,
                   atoms[i].x,atoms[i].y,atoms[i].z);

          if (with_connectivities == 1)
            {
              int ni;
              struct node *ptr = bond_adjacency_list[i];
          
              while(ptr != NULL)
                {
              
                  ni = (ptr->v);
              
                  fprintf(out_file, " %d", ni + 1);
              
                  ptr = ptr->next;

                }
            }
      
          fprintf(out_file, "\n");
      
        }
    }

  fclose(out_file);

  if(with_connectivities == 1)
    {
      sprintf (buf,
               "File %s written: %d atoms and 1 frame (XYZ + connectivities)",
               gnu_basename(file_name), no_visible_atoms);
    }
  else
    {
      sprintf (buf, "File %s written: %d atoms and 1 frame (XYZ)",
               gnu_basename(file_name), no_visible_atoms);
    }

  echo_to_message_area(buf);

}


void
write_ascii_all(char *file_name)
{

  void change_frame(int, Boolean, Boolean);
  double get_angle_axis(double *);
  void rotate_atoms(double *,double,Boolean,Boolean);
  void frame_content_to_atoms (int);
  struct frame * get_selected_frame (void);
  int count_visible_atoms_in_frame (struct frame *);

  int i, j, frame_visible_atoms;

  char buf[BUFSIZ];

  double angle, axis[3];
  
  FILE *out_file;

  struct frame *this_frame;

  out_file=fopen(file_name,"w");

  /* Make sure none of this is drawn */

  redraw = 0;
  
  /* Reset the angle_axis_matrix */

  for(i = 0; i < 3; i++)
    {
      for(j = 0; j < 3; j++)
        {
          angle_axis_matrix[i][j] = global_matrix[i][j];
        }
    }

  /* Loop over frames */

  for(i = 0; i < no_frames; i++)
    {

      frame_content_to_atoms (i);
      this_frame = get_selected_frame();

      frame_visible_atoms = count_visible_atoms_in_frame (this_frame);

      fprintf (out_file, "%d\n", frame_visible_atoms);
      fprintf (out_file, "%s\n", this_frame->comment);
      
      angle = get_angle_axis(axis);
      
      rotate_atoms(axis, angle, 0, False);
          
      for(j = 0; j < this_frame->no_atoms; j++)
        {
          if(atoms[j].visi == 1)
            {

              /* We need to add the global_vector component as there
                 may have been translations */

              fprintf(out_file,"%2s %12.6f %12.6f %12.6f\n",
                      atoms[j].label,
                      atoms[j].x + global_vector[0],
                      atoms[j].y + global_vector[1],
                      atoms[j].z + global_vector[2]);

            }
        }
    }
  
  fclose(out_file);

  if(no_frames == 1)
    {
      sprintf(buf, "File %s written: %d frame (XYZ)",
              gnu_basename(file_name), no_frames);
    }
  else
    {
      sprintf(buf, "File %s written: %d frames (XYZ)",
              gnu_basename(file_name), no_frames);
    }

  echo_to_message_area(buf);

  /* Now restore the coordinates */
  
  if(centre_each_frame == 1)
    {
      /* Change frame, don't reset title _and_ centre the frame */
      change_frame(frame_no, False, True);
    }
  else
    {
      /* Change frame, don't reset title, _don't_ centre the frame */
      change_frame(frame_no, False, False);
    }

}


void
write_eps(char *file_name, FILE *out_file, Boolean is_colour)
{

  void convert_to_canvas_coords(double *, double *, Boolean);
  double get_atom_radius(int);
  double get_hbond_scale(void);
  double get_h_bond_width(int, int);
  int get_no_dashes(void);
  double get_z_depth (void);
  void update_canvas_bond_points(int, int, Boolean, double);
  void update_canvas_hbond_points(int, int, Boolean);
  
  int i, si, sj, dash_length;
  int xmin, xmax, ymin, ymax;

  char string[1024];

  double radius, atom_coord[3], canvas_coord[2], hbond_length;
  
  struct node *ptr;
  
  /* We might have already opened the file, if we used mkstemp to
     generate the filename */

  if (out_file == NULL)
    {
      if (strlen (file_name) == 0)
        {
          echo_to_message_area ("xmakemol: no file name given in write_eps");
          return;
        }
      out_file = fopen (file_name, "w");
    }
  
  xmax = 0;
  ymax = 0;
  xmin = canvas_width;
  ymin = canvas_height;
  
  /* Get the bounding-box */
  
  for (i = 0; i < no_atoms; i++)
    {
      if (atoms[i].visi == 1)
	{
	  if (atom_flag == 1)
	    {
	      radius = get_atom_radius(i);
	      atom_coord[0] = atoms[i].x;
	      atom_coord[1] = atoms[i].y;
	      atom_coord[2] = atoms[i].z;
	      convert_to_canvas_coords(atom_coord, canvas_coord, 1);
	      if (canvas_coord[0] + radius > xmax) 
		{
		  xmax = (int)(canvas_coord[0] + radius);
		}
	      if (canvas_coord[0] - radius < xmin)
		{
		  xmin = (int)(canvas_coord[0] - radius);
		}
	      if (canvas_coord[1] + radius > ymax)
		{
		  ymax = (int)(canvas_coord[1] + radius);
		}
	      if (canvas_coord[1] - radius < ymin)
                {
                  ymin = (int)(canvas_coord[1] - radius);
                }
            }
	}
    }

  /* Don't worry too much about the axes, as they are in the upper right
   * corner anyway */

  if(axes_flag)
    {
      xmax = canvas_width;
      ymax = canvas_height;
    }
 
  fprintf(out_file,"%%!PS-Adobe-2.0 EPSF-2.0\n");
  fprintf(out_file,"%%%%Title:  XMakemol\n");
  fprintf(out_file,"%%%%BoundingBox: %d %d %d %d\n",xmin, ymin, xmax, ymax);
  fprintf(out_file,"%%%%EndComments\n");
  fprintf(out_file,"%%%%BeginProlog\n");
  fprintf(out_file,"/Cc {0 360 arc} def\n");
  fprintf(out_file,"/Cp {closepath} def\n");
  fprintf(out_file,"/Gs {gsave} def\n");
  fprintf(out_file,"/Gr {grestore} def\n");
  if(is_colour){
    fprintf(out_file,"/Sc {setrgbcolor} def\n");
  }else{
    fprintf(out_file,"/Wh {1.0 setgray} def\n");
  }
  fprintf(out_file,"/Bl {0.0 setgray} def\n");
  fprintf(out_file,"/St {stroke} def\n");
  fprintf(out_file,"/Fi {fill} def\n");
  fprintf(out_file,"/Mt {moveto} def\n");
  fprintf(out_file,"/Lt {lineto} def\n");
  fprintf(out_file,"/Sn {show newpath} def\n");
  if(is_colour){
    fprintf(out_file,"/Draw {Cp Gs Fi Gr Bl St} def\n");
  }else{
    fprintf(out_file,"/Draw {Cp Gs Wh Fi Gr Bl St} def\n");
  }
  fprintf(out_file,"%%%%EndProlog\n");
  fprintf(out_file,"%%%%BeginSetup\n");
  fprintf(out_file,"/Times-Roman findfont 10 scalefont setfont\n");
  fprintf(out_file,"%%%%EndSetup\n");
  fprintf(out_file, "1 setlinewidth\n");

  depth = get_z_depth ();

  for(i=0;i<no_atoms;i++){
    
    si=sorted_atoms[i];

    if(atom_flag){
      if((depth-atoms[si].z>Z_TOL) || (depth_is_on==0)){
        
        if(atoms[si].visi){

          if(is_colour)
            {
              
              fprintf(out_file,"%6.3f %6.3f %6.3f Sc\n",
                      (double)atoms[si].red/65536.0,
                      (double)atoms[si].green/65536.0,
                      (double)atoms[si].blue/65536.0);
            }

          radius = get_atom_radius(si);
          
          atom_coord[0] = atoms[si].x;
          atom_coord[1] = atoms[si].y;
          atom_coord[2] = atoms[si].z;
          
          convert_to_canvas_coords(atom_coord, canvas_coord, 1);
          
          fprintf(out_file,"%d %d %d Cc ",
                  (int) canvas_coord[0],
                  (int) canvas_coord[1],
                  (int) radius);
          
          fprintf(out_file,"Draw\n");
        }
      }
    }
    
    
    if(bond_flag == 1)
      {
        
        /* Draw the bonds */

        ptr = bond_adjacency_list[si];
        
        while(ptr != NULL)
          {
            
            sj = (ptr->v);
            
            if( (depth_is_on == 0) ||
                (((depth-atoms[si].z>Z_TOL) && (depth-atoms[sj].z>Z_TOL))))
              {

                if(atoms[si].visi && atoms[sj].visi)
                  {

                    if(atoms[si].z<=atoms[sj].z)
                      {

                        update_canvas_bond_points(si, sj, 1, 0.0);
                        
                        if(is_colour){
                          
                          fprintf(out_file,"%6.3f %6.3f %6.3f Sc\n",
                                  (double)atoms[si].red/65536.0,
                                  (double)atoms[si].green/65536.0,
                                  (double)atoms[si].blue/65536.0);
                          
                          /* Draw polygon 0-1-4-5 */

                          fprintf(out_file,"%d %d Mt ",
                                  (int) canvas_bond_points[0].x,
                                  (int) canvas_bond_points[0].y);
                          
                          fprintf(out_file,"%d %d Lt ",
                                  (int) canvas_bond_points[1].x,
                                  (int) canvas_bond_points[1].y);
                          
                          fprintf(out_file,"%d %d Lt ",
                                  (int) canvas_bond_points[4].x,
                                  (int) canvas_bond_points[4].y);
                          
                          fprintf(out_file,"%d %d Lt ",
                                  (int) canvas_bond_points[5].x,
                                  (int) canvas_bond_points[5].y);
                            
                          fprintf(out_file,"Draw\n");
                          
                          fprintf(out_file,"%6.3f %6.3f %6.3f Sc\n",
                                  (double)atoms[sj].red/65536.0,
                                  (double)atoms[sj].green/65536.0,
                                  (double)atoms[sj].blue/65536.0);
                
                          /* Draw polygon 2-1-4-3 */

                          fprintf(out_file,"%d %d Mt ",
                                  (int) canvas_bond_points[2].x,
                                  (int) canvas_bond_points[2].y);
                          
                          fprintf(out_file,"%d %d Lt ",
                                  (int) canvas_bond_points[1].x,
                                  (int) canvas_bond_points[1].y);
                          
                          fprintf(out_file,"%d %d Lt ",
                                  (int) canvas_bond_points[4].x,
                                  (int) canvas_bond_points[4].y);
                          
                          fprintf(out_file,"%d %d Lt ",
                                  (int) canvas_bond_points[3].x,
                                  (int) canvas_bond_points[3].y);
                            
                          fprintf(out_file,"Draw\n");
                            
                        }else{
                          
                          /* Draw polygon 0-2-3-5 */
                          
                          fprintf(out_file,"%d %d Mt ",
                                  (int) canvas_bond_points[0].x,
                                  (int) canvas_bond_points[0].y);
                          
                          fprintf(out_file,"%d %d Lt ",
                                  (int) canvas_bond_points[2].x,
                                  (int) canvas_bond_points[2].y);
                          
                          fprintf(out_file,"%d %d Lt ",
                                  (int) canvas_bond_points[3].x,
                                  (int) canvas_bond_points[3].y);
                          
                          fprintf(out_file,"%d %d Lt ",
                                  (int) canvas_bond_points[5].x,
                                  (int) canvas_bond_points[5].y);
                            
                          fprintf(out_file,"Draw\n");
                          
                        }
                      }
                  }
              }
            
            ptr = ptr->next;
            
          }
      }
    
    if((any_hydrogen == 1) && (hbond_flag == 1))
      {
        /* ... if there are any */

        ptr = hbond_adjacency_list[si];
        
        while(ptr != NULL)
          {
            sj = (ptr->v);
            
            if( (depth_is_on == 0) ||
                (((depth-atoms[si].z>Z_TOL)) && (depth-atoms[sj].z>Z_TOL)))
              {

                if(atoms[si].visi && atoms[sj].visi)
                  {

                    if(atoms[si].z<=atoms[sj].z)
                      {
                        double x, y;

                        update_canvas_hbond_points(si, sj, 1);
          
                        /* Don't print if dashlength is zero */

                        x = canvas_hbond_points[0].x - 
                          canvas_hbond_points[1].x;

                        y = canvas_hbond_points[0].y -
                          canvas_hbond_points[1].y;

                        hbond_length = sqrt ((x * x) + (y * y));

                        dash_length =
                          (int) (hbond_length / (2 * get_no_dashes() - 1));

                        if(dash_length > 0)
                          {
                            
                            /* Draw line 0-1 */

                            fprintf(out_file,"%d %d Mt ",
                                    (int) canvas_hbond_points[0].x,
                                    (int) canvas_hbond_points[0].y);

                            fprintf(out_file,"%d %d Lt ",
                                    (int) canvas_hbond_points[1].x,
                                    (int) canvas_hbond_points[1].y);
                        
                            fprintf(out_file, "%d setlinewidth\n",
                                    (int) get_h_bond_width(si, sj));

                            fprintf(out_file,"[%d %d] 0 setdash St [1 0] 0 setdash 1 setlinewidth\n",
                                    dash_length, dash_length);
                          }
                      }
                  }
              }
            
            ptr = ptr->next;
            
          }
      }
    
    if(at_nos_flag && atoms[si].visi)
      {
        fprintf(out_file, "%d %d Mt ",
                (int) canvas_coord[0],
                (int) canvas_coord[1]);

        fprintf(out_file, "(%d) Sn\n", si + 1);
      }
    
    if(at_sym_flag && atoms[si].visi)
      {
        fprintf(out_file, "%d %d Mt ",
                (int) canvas_coord[0],
                (int) canvas_coord[1]);
        
        fprintf(out_file, "(%s) Sn\n", atoms[si].label);
      }
    
  }

  if(axes_flag){
    fprintf(out_file,"%d %d Mt ",400,(canvas_width-50));
    fprintf(out_file,"%d %d Lt St\n",
            (int)(400+global_matrix[0][0]*30),
            (int)(canvas_width-(50-global_matrix[0][1]*30)));
    fprintf(out_file,"%d %d Mt (X) Sn\n",
            (int)(400+global_matrix[0][0]*40),
            (int)(canvas_width-(50-global_matrix[0][1]*40)));

    fprintf(out_file,"%d %d Mt ",400,(canvas_width-50));
    fprintf(out_file,"%d %d Lt St\n",
            (int)(400+global_matrix[1][0]*30),
            (int)(canvas_width-(50-global_matrix[1][1]*30)));
    fprintf(out_file,"%d %d Mt (Y) Sn\n",
            (int)(400+global_matrix[1][0]*40),
            (int)(canvas_width-(50-global_matrix[1][1]*40)));

    fprintf(out_file,"%d %d Mt ",400,(canvas_width-50));
    fprintf(out_file,"%d %d Lt St\n",
            (int)(400+global_matrix[2][0]*30),
            (int)(canvas_width-(50-global_matrix[2][1]*30)));
    fprintf(out_file,"%d %d Mt(Z) Sn\n",
            (int)(400+global_matrix[2][0]*40),
            (int)(canvas_width-(50-global_matrix[2][1]*40)));
  }

  fprintf(out_file,"showpage\n");
  fclose(out_file);
 

  if(is_colour == 1)
    {
      sprintf(string, "File %s written: EPS (colour)", gnu_basename(file_name));
    }
  else
    {
      sprintf(string, "File %s written: EPS (b/w)", gnu_basename(file_name));
    }

  echo_to_message_area(string);
  
}


void
write_aux (char *file_name)
{

  int frame_number = 1;

  struct frame *this_frame;

  FILE *aux_file;

  aux_file = fopen (file_name, "w");

  if (aux_file == NULL)
    {
      echo_to_message_area ("xmakemol: cannot write aux file");
      return;
    }

  this_frame = get_first_frame ();

  while (this_frame != NULL)
    {
      fprintf (aux_file, "# Frame %d\n", frame_number);

      fprintf (aux_file, "depth = %d\n", this_frame->perspective_depth);
      fprintf (aux_file, "scale = %d\n", this_frame->perspective_scale);

      this_frame = this_frame->next;
      frame_number++;
    }

  if (fclose (aux_file) != 0)
    {
      echo_to_message_area ("xmakemol: error closing aux file");
    }

}


void
read_aux (char *file_name)
{

  int frame_number, value;
  char *xyz_str = ".xyz", *sub_string, buf[BUFSIZ], prop[32], aux_file_name[1024];
  struct frame *this_frame = NULL;
  FILE *aux_file;

  aux_file_exists_p = True;
  aux_file_error_p = False;

  /* If the file name extension is .xyz, look for aux file with
     extension .aux; otherwise, just append .aux */

  strcpy (aux_file_name, file_name);
  sub_string = strstr (aux_file_name, xyz_str);

  if (sub_string != NULL)
    {
      strcpy (sub_string, ".aux");
    }
  else
    {
      strcat (aux_file_name, ".aux");
    }

  aux_file = fopen (aux_file_name, "r");

  if (aux_file == NULL)
    {
      aux_file_exists_p = False;
      return;
    }

  /* Read the information */

  while (1)
    {
      if (feof (aux_file)) break;
      (void)fgets (buf, sizeof (buf), aux_file);

      if (sscanf (buf, "# Frame %d", &frame_number))
        {
          if (this_frame == NULL)
            {
              this_frame = get_first_frame ();
            }
          else
            {
              this_frame = this_frame->next;
            }

          if (this_frame == NULL)
            {
              fprintf (stderr, "xmakemol: frame pointer is null in read_aux\n");
              exit (2);
            }
        }
      else if (sscanf (buf, "%s = %d", prop, &value))
        {
          if ((frame_number == 0) || (frame_number > no_frames))
            {
              aux_file_exists_p = False;
              fprintf (stderr, "xmakemol: error detected in aux file\n");
              aux_file_error_p = True;
              return;
            }

          if (strcmp (prop, "depth") == 0)
            {
              this_frame->perspective_depth = value;
            }
          else if (strcmp (prop, "scale") == 0)
            {
              this_frame->perspective_scale = value;
            }
          else
            {
              fprintf (stderr, "xmakemol: unrecognized property in aux file\n");
              aux_file_error_p = True;
            }
        }
    }
}



/* Save the pixmap `canvas_pm' to a named file.  The function returns false
   for failure and true for success. */
#ifdef XPM
int 
write_xpm(char *file_name, int quiet)
{
  int retcode, ok ;

  char string[1024];
  
  XpmAttributes xpm_attrib;

  Colormap cmap;

  /* Allocate colormap suitable for the canvas.  Note that the use of
     `DefaultColormap()' is known to succeed only because we have not been
     picky about the visual class being used.  This has to be fixed if we
     start using TrueColor visuals someday. */
#ifdef GL
  cmap = XCreateColormap(display, XtWindow(canvas), 
                         visinfo->visual, AllocNone);
#else  /* GL */
  cmap = DefaultColormap(display,screen_num);
#endif /* GL */

  /* Give visual, depth and colormap to the libXpm so that it does not
     ask them directly from the X11.  Write of an XPM file will fail at
     least on Octane2 if the colormap is not specified here.  (Octane2 is
     capable of having varying types of visual on a single screen so I guess
     the libXpm fails to query out the correct colors for the pixmap.) */
  xpm_attrib.valuemask = XpmDepth | XpmVisual | XpmColormap;
#ifdef GL
  xpm_attrib.depth = visinfo->depth;
  xpm_attrib.visual = visinfo->visual;
#else  /* X11 */
  xpm_attrib.depth = screen_depth;
  xpm_attrib.visual = DefaultVisual (display, screen_num); /* Correct? */
#endif /* X11 */
  xpm_attrib.colormap = cmap;

  /* Write file while dealing with possible errors. */
  retcode = XpmWriteFileFromPixmap (display,
                                    gnu_basename (file_name),
                                    canvas_pm,
                                    0,
                                    &xpm_attrib);

  if (retcode == XpmSuccess)
    {
      if (!quiet) {
        sprintf(string, "File %s written: XPM", gnu_basename(file_name));
        echo_to_message_area(string);
      }
      ok = /*success*/1;
    }
  else if (retcode == XpmOpenFailed)
    {
      sprintf (string, "Cannot open %s for write", gnu_basename (file_name));
      echo_to_message_area(string);
      ok = /*failure*/0;
    }
  else
    {
      sprintf(string, "Error %d: cannot write XPM file", retcode);
      echo_to_message_area(string);
      ok = /*failure*/0;
    }

#ifdef GL
  XFreeColormap(display, cmap);
#endif

  return ok;
}
#endif

#ifdef GL
void write_gl2ps (char *filename, GLint type)
{
  FILE *fp = fopen (filename, "w");
  void gl_render (void);
  GLint buffsize = 0, state = GL2PS_OVERFLOW;
  GLint viewport[4];

  glGetIntegerv(GL_VIEWPORT, viewport);

  while (state == GL2PS_OVERFLOW)
    {
      buffsize += 1024*1024;
      gl2psBeginPage ( "Title", "XMakemol", viewport,
                       type, GL2PS_BSP_SORT,
                       GL2PS_DRAW_BACKGROUND |
                       GL2PS_SIMPLE_LINE_OFFSET | GL2PS_SILENT |
                       GL2PS_OCCLUSION_CULL | GL2PS_BEST_ROOT,
                       GL_RGBA, 0, NULL, 0, 0, 0, buffsize,
                       fp, NULL );

      gl_render ();

      state = gl2psEndPage();
    }

  fclose (fp);
}
#endif

/* Minimum H-bond length allowed: should make this customizable */

#define MIN_HBOND_LENGTH2 (1.2 * 1.2)

void
update_bond_matrix(Boolean force_update)
{

  void free_bond_adjacency_list(void);
  void set_no_bonds (int);
  void set_no_hbonds (int);
  void update_lengths_dialog(Boolean);
    
  struct node *new_ptr;

  int i, j, no_bonds = 0, no_hbonds = 0;

  static int last_frame_no=0;
  
  double atom_sep2;

  static double last_bdfd_factor=0.0;
  static double last_hbdfd_factor=0.0;
  static double last_ibdfd_factor=0.0;
  static double last_ihbdfd_factor=0.0;

  /* We may not actually have to do the update. If the frame number
     has not changed and the bdfd_factor/hbdfd_factor are the same
     then we are wasting our time. These checks can be overridden if
     force_update is True */

    
  if((force_update == False) &&
     /* We might not need to update - check other things */
     (frame_no == last_frame_no) && 
     (last_bdfd_factor  == bdfd_factor)  &&
     (last_hbdfd_factor == hbdfd_factor) &&
     (last_ibdfd_factor  == ibdfd_factor) &&
     (last_ihbdfd_factor == ihbdfd_factor))
    {
      return;               /* We can return */
    }

  if(bond_adjacency_list)
    {
      free_bond_adjacency_list();
    }

  bond_adjacency_list = 
    (struct node **)malloc(no_atoms*sizeof(struct node *));
  
  hbond_adjacency_list = 
    (struct node **)malloc(no_atoms*sizeof(struct node *));

  /* Keep a track of this for freeing purposes */
  
  bond_adjacency_lists_allocated_size = no_atoms;
  
  for( i = 0; i < no_atoms; i++ )
    {
      bond_adjacency_list[i] = NULL;
      hbond_adjacency_list[i] = NULL;
    }
  
  /* Assign the bonds */

  for(i=0;i<no_atoms-1;i++)
    {
      for(j=i+1;j<no_atoms;j++)
        {
          double test;
          double x, y, z;

          x = atoms[i].x - atoms[j].x;
          y = atoms[i].y - atoms[j].y;
          z = atoms[i].z - atoms[j].z;

          atom_sep2 = (x * x) + (y * y) + (z * z);

          if (atoms[i].molecule == atoms[j].molecule)
            {
              /* Intramolecular tolerance */
              test = ibdfd_factor *
                BOND * 
                (atoms[i].cov_rad + atoms[j].cov_rad);
            }
          else
            {
              test = bdfd_factor *
                BOND * 
                (atoms[i].cov_rad + atoms[j].cov_rad);
            }

          test = test * test;

          if(atom_sep2 < test)
            {
              no_bonds++;
              /* So j is joined to i */

              new_ptr = (struct node *)malloc(sizeof(struct node));
              new_ptr->v=i;
              new_ptr->next = bond_adjacency_list[j];
              bond_adjacency_list[j]=new_ptr;            

              /* and i is joined to j */
                  
              new_ptr = (struct node *)malloc(sizeof(struct node));
              new_ptr->v=j;
              new_ptr->next = bond_adjacency_list[i];
              bond_adjacency_list[i]=new_ptr;
            }
        }
    }

  /* Assign the H-bonds */

/* There is no point doing this if there are no H atoms */

  if(any_hydrogen == 1)
    {
      
      for(i = 0; i < no_atoms - 1 ; i++)
        {
          for(j = i + 1; j < no_atoms; j++)
            {
              /* Test if _one_ of the atoms is hydrogen */
              
              if((( atoms[i].is_hydrogen) && (!atoms[j].is_hydrogen)) ||
                 ((!atoms[i].is_hydrogen) && ( atoms[j].is_hydrogen)))
                {
                  double bond_test, hbond_test;
                  double x, y, z;

                  x = atoms[i].x - atoms[j].x;
                  y = atoms[i].y - atoms[j].y;
                  z = atoms[i].z - atoms[j].z;

                  atom_sep2 = (x * x) + (y * y) + (z * z);

                  if (atoms[i].molecule == atoms[j].molecule)
                    {
                      /* Intramolecular tolerance */
                      bond_test = ibdfd_factor *
                        BOND * 
                        (atoms[i].cov_rad + atoms[j].cov_rad);
                      hbond_test = ihbdfd_factor * HBOND;
                    }
                  else
                    {
                      bond_test = bdfd_factor *
                        BOND * 
                        (atoms[i].cov_rad + atoms[j].cov_rad);
                      hbond_test = hbdfd_factor * HBOND;
                    }

                  hbond_test = hbond_test * hbond_test;
                  bond_test = bond_test * bond_test;
                  
                  if(
                     /* distance smaller than H-bond tolerance */
                     
                     (atom_sep2 < hbond_test) &&

                     /* distance greater than fixed minimum distance */

                     (atom_sep2 > MIN_HBOND_LENGTH2) &&
                     
                     /* distance larger than bond tolerance */
                     
                     (atom_sep2 > bond_test))
                    {
                      /* There is an H-bond */
                      no_hbonds++;
                      
                      /* So j is joined to i */
                      
                      new_ptr = (struct node *)malloc(sizeof(struct node));
                      new_ptr->v=i;
                      new_ptr->next = hbond_adjacency_list[j];
                      hbond_adjacency_list[j]=new_ptr;            
                      
                      /* and i is joined to j */
                      
                      new_ptr = (struct node *)malloc(sizeof(struct node));
                      new_ptr->v=j;
                      new_ptr->next = hbond_adjacency_list[i];
                      hbond_adjacency_list[i]=new_ptr;
                    }
                }
            }
        }
    }
  
  update_lengths_dialog(True);
  
  /* Update values so that we know what their values _were_ next time
     we call this function */

  last_frame_no = frame_no;
  last_bdfd_factor = bdfd_factor;
  last_hbdfd_factor = hbdfd_factor;
  last_ibdfd_factor = ibdfd_factor;
  last_ihbdfd_factor = ihbdfd_factor;
  
  set_no_bonds (no_bonds);
  set_no_hbonds (no_hbonds);
}


void
update_some_dialogs(void)
{

  void edit_posn_cancel_cb(Widget, XtPointer, XtPointer);
  void edit_visi_cancel_cb(Widget, XtPointer, XtPointer);
  void edit_elements_discard_cb(Widget, XtPointer, XtPointer);
  void make_edit_posn_dialog(Widget);
  void make_edit_visi_dialog(Widget);
  void make_edit_element_props_dialog(Widget);/* aro */
  void reset_pers_scales(void);

  /* Don't need to destroy this widget */

  if(pers_dialog){
    reset_pers_scales();
  }

  if(edit_visi_dialog){
    edit_visi_cancel_cb(edit_visi_dialog, NULL, NULL);
    make_edit_visi_dialog(main_w);
  }

  if(edit_posn_dialog){
    edit_posn_cancel_cb(edit_posn_dialog, NULL, NULL);
    make_edit_posn_dialog(main_w);
  }
  
  /* edit_element_props_dialog and p_dialog should also be 
     reset if file is loaded while it's open */
  if(edit_element_props_dialog){
    if(XtIsManaged(edit_element_props_dialog)){
      make_edit_element_props_dialog(main_w);
    }
  }

  if(p_dialog){
    if(XtIsManaged(p_dialog)){
      /* we want to go through the p_dialog creation routine
         to update p_dialog, so we pass 3 as client_data to file_cb */
      file_cb(NULL, (XtPointer) 3, NULL);
    }
  }

}


void
read_elements_file(void)
{
  void make_label_uppercase(char *);
  void set_smallest_cov_rad(double);

  int i;

  char line[1024],label[4],color[1024],*line_ptr;

  char *elem_file_name;
  
  double mass, cov_rad, vdw_rad;
  double smallest_cov_rad = -1;

  FILE *elem_file = NULL;

  if(!element){
    element=malloc(MAX_ELEMENTS*sizeof(struct elements));

    /* Check to see if XM_ELEMENTS is set */

    elem_file_name = getenv ("XM_ELEMENTS");

    if (elem_file_name)
      {
        /* If this is a directory, append `/elements' and try to open
           the resultant file */

        if (opendir (elem_file_name))
          {
            elem_file_name = strcat (elem_file_name, "/elements");
            elem_file = fopen (elem_file_name, "r");
          }
        else
          {
            elem_file = fopen (elem_file_name, "r");
          }
      }

    /* If still not found, try the default location */

    if (! elem_file)
      {
        elem_file = fopen (ELEMENTS, "r");
      }

    /* Last, try the current working directory */

    if (! elem_file)
      {
        elem_file = fopen ("elements", "r");
      }

    if (! elem_file)
      {
        fprintf (stderr, "xmakemol: cannot find elements file\n");
        exit (2);
      }
    
    largest_cov_rad=0.0;
    
    no_elements = 0;
    
  element:while(1)
      {
        line_ptr = fgets(line, sizeof(line), elem_file);

        if(line_ptr == NULL)
          {
            break;
          }

        if (line[0] == '!')
          {
            /* This is a comment -- ignore */
            goto element;
          }
        
        if(no_elements >= MAX_ELEMENTS)
          {
            no_elements--;      /* We can't allocate for this one */
            
            fprintf(stderr,
                    "xmakemol: number of elements in %s exceeds MAX_ELEMENTS (file.c)\n",
                    elem_file_name);
            break;
            
          }
        
        /* i is junk  - we don't trust the writer of elements ... */
        sscanf(line, "%d %s %lf %s %lf %lf",
               &i, label, &mass ,color, &cov_rad, &vdw_rad);
        
        i = no_elements;
        
        strcpy(element[i].label, label);
        make_label_uppercase(element[i].label);
        
        element[i].mass = mass;
        strcpy(element[i].color, color);

        element[i].cov_rad = cov_rad;
        element[i].vdw_rad = vdw_rad;

        element[i].color_allocated = 0;

        element[i].customized = 0; /* aro */

        if(cov_rad > largest_cov_rad)
          {
            largest_cov_rad=cov_rad;
          }

        if(smallest_cov_rad > 0.0)
          {
          
            if(cov_rad < smallest_cov_rad)
              {
                smallest_cov_rad = cov_rad;
              }
          }
        else
          {
            smallest_cov_rad = cov_rad;
          }
        
        no_elements++;
        
      }

    set_smallest_cov_rad(smallest_cov_rad);
    
  }
}

/* aro--> */
void
read_user_elements_file (void)
{
  /* Read in the file .xmakemol.elements from /home/<username>/ */

  double get_smallest_cov_rad();
  void make_label_uppercase(char *);
  void set_smallest_cov_rad(double);

  double smallest_cov_rad = get_smallest_cov_rad();
      
  int i;
    
  char line[1024],label[4],color[1024];
  char *user_elements_file_name, *line_ptr;
        
  double mass, cov_rad, vdw_rad;

  FILE *user_elements_file;

  /* check to see if .xmakemol.elements file exists in user's home directory */
  
  strcpy(line, getenv("HOME")); /* line used as temp variable here */
  user_elements_file_name = strcat(line, "/.xmakemol.elements");

  user_elements_file = fopen(user_elements_file_name, "r");

  if(user_elements_file != NULL)
    {                        
      line_ptr = fgets(line, sizeof(line), user_elements_file);
      while(line_ptr != NULL)
        {
          sscanf(line, "%s %lf %s %lf %lf",
                 label, &mass, color, &cov_rad, &vdw_rad);
          
          make_label_uppercase(label);
          for(i = 0; strcmp(element[i].label, label); i++)
            ;/* do nothing, for loop will set correct index */

          /* We don't copy label or mass information because those
             should be unchanged from default element properties */
          strcpy(element[i].color, color);
          
          element[i].cov_rad = cov_rad;

          element[i].vdw_rad = vdw_rad;
          
          element[i].color_allocated = 0;
          
          element[i].customized = 1;

          if(cov_rad > largest_cov_rad)
            {
              largest_cov_rad=cov_rad;
            }
          
          if(smallest_cov_rad > 0.0)
            {
              
              if(cov_rad < smallest_cov_rad)
                  smallest_cov_rad = cov_rad;
            }
          else
            {
              smallest_cov_rad = cov_rad;
            }

          line_ptr = fgets(line, sizeof(line), user_elements_file);
        }

      set_smallest_cov_rad(smallest_cov_rad);
      
      fclose(user_elements_file);
    }

}
/* <--aro */


void
malloc_failed(char *string)
{
  fprintf (stderr, "xmakemol: malloc failed in %s\n", string);
  exit (1);
}


void
make_label_uppercase(char *label)
{
  
  int i;

  /* Check if a lowercase letter */

  for (i = 0; i < strlen (label); i++)
    {
      if ((label[i] >= 'a') && (label[i] <= 'z'))
        {
          label[i] -= ('a' - 'A');
        }
    }
}


void
free_bond_adjacency_list(void)
{
  
  int i;
  
  if(bond_adjacency_list != NULL)
    {
      
      for ( i = 0; i < bond_adjacency_lists_allocated_size; i++ )
        {
          struct node *this_ptr, *next_ptr;

          this_ptr = bond_adjacency_list[i];

          while(this_ptr != NULL)
            {
              next_ptr = this_ptr->next;
              free((struct node *)this_ptr);
              this_ptr = next_ptr;
            }
        }

      free(bond_adjacency_list);
      bond_adjacency_list=NULL;
      
    }

  if(hbond_adjacency_list != NULL)
    {
      
      for ( i = 0; i < bond_adjacency_lists_allocated_size; i++ )
        {
          struct node *this_ptr, *next_ptr;

          this_ptr = hbond_adjacency_list[i];
          
          while(this_ptr != NULL)
            {
              next_ptr = this_ptr->next;
              free((struct node *)this_ptr);
              this_ptr = next_ptr;
            }
        }

      free(hbond_adjacency_list);
      hbond_adjacency_list=NULL;
      
    }
  
}


int
get_no_elements(void)
{

  return(no_elements);

}

/* The pointer to the selected frame, available through
   get_selected_frame () */

static struct frame *selected_frame, *previous_frame;

struct frame *
update_frame_pointer (int frame_index)
{

  int i;

  /* Store the previously selected frame */

  previous_frame = selected_frame;

/*    printf ("previous_frame = 0x%x\n", previous_frame); */

  /* Find the frame pointer for the frame with index frame_index */

  selected_frame = get_first_frame ();

  for (i = 0; i < frame_index; i++)
    {
      selected_frame = selected_frame->next;

      if (selected_frame == NULL)
        {
          fprintf (stderr,
                   "xmakemol: internal error (update_frame_pointer)\n");
/*            exit (1); */
        }
    }

  return (selected_frame);

}


struct frame *
get_selected_frame (void)
{

  return (selected_frame);
  
}


void
set_selected_frame (struct frame *new_frame)
{

  selected_frame = new_frame;

}


struct frame *
get_first_frame (void)
{

  return (first_frame);

}


struct frame *
get_previous_frame (void)
{

  return (previous_frame);

}


void
frame_content_to_atoms (int frame_no)
{
  void assign_atom_types (void);
  int crystal_p (void);
  void get_atom_data (struct frame *, Colormap, int, double, double, double);
  int * get_crystal_images (void);
  void set_canvas_scale (double);
  void set_z_depth (double);
  struct frame * update_frame_pointer (int);

  int i, selections_viable;
  int *images = NULL;

  static int last_no_atoms = 0;

  struct frame *this_frame;

  Colormap cmap;
 
  /* Get pointer to the current frame */  

  this_frame = update_frame_pointer (frame_no);

  if (crystal_p ())
    {
      images = get_crystal_images ();

      no_atoms = this_frame->no_atoms * images[0] * images[1] * images[2];
    }
  else
    {
      no_atoms = this_frame->no_atoms;
    }

  /* aro - use colormap specific to canvas and visual, default
     colormap may give inaccurate colors on some displays */

#ifdef GL
  cmap = XCreateColormap(display, XtWindow(canvas), 
                         visinfo->visual, AllocNone);
#else  /* GL */
  cmap = DefaultColormap(display,screen_num);
#endif /* GL */

  /* Allocate memory */

  if (atoms == NULL)
    {
      atoms = (struct atoms *) malloc (no_atoms * sizeof (struct atoms));
      sorted_atoms = (int *) malloc (no_atoms * sizeof (int));
    }
  else if (last_no_atoms != no_atoms)
    {
      free ((struct atoms *) atoms);
      atoms = (struct atoms *) malloc (no_atoms * sizeof (struct atoms));
      
      free ((int *) sorted_atoms);
      sorted_atoms = (int *) malloc (no_atoms * sizeof (int));
    }

  last_no_atoms = no_atoms;

  if (crystal_p ())
    {
      double * get_crystal_vector (void);

      int p, q, r;
      int offset = 0;
      
      double *cv;

      cv = get_crystal_vector ();

      for (p = 0; p < images[0]; p++)
        {
          for (q = 0; q < images[1]; q++)
            {
              for (r = 0; r < images[2]; r++)
                {
                  double x, y, z;

                  x = (p * cv[0]) + (q * cv[3]) + (r * cv[6]);
                  y = (p * cv[1]) + (q * cv[4]) + (r * cv[7]);
                  z = (p * cv[2]) + (q * cv[5]) + (r * cv[8]);

                  get_atom_data (this_frame, cmap, offset, x, y, z);
                  offset += this_frame->no_atoms;
                }
            }
        }
          
    }
  else
    {
      get_atom_data (this_frame, cmap, 0, 0.0, 0.0, 0.0);
    }

  assign_atom_types ();

  /* Bounding box */

  if (this_frame->bbox_available)
    {
      bbox_available = True;

      file_bbox.x_min = this_frame->bbox[0][0];
      file_bbox.x_max = this_frame->bbox[0][1];
      file_bbox.y_min = this_frame->bbox[1][0];
      file_bbox.y_max = this_frame->bbox[1][1];
      file_bbox.z_min = this_frame->bbox[2][0];
      file_bbox.z_max = this_frame->bbox[2][1];

      /* Setup bounding box vectors */

      file_bbox.v[0][0] = file_bbox.x_max; file_bbox.v[0][1] = file_bbox.y_min; file_bbox.v[0][2] = file_bbox.z_max;
      file_bbox.v[1][0] = file_bbox.x_max; file_bbox.v[1][1] = file_bbox.y_max; file_bbox.v[1][2] = file_bbox.z_max;
      file_bbox.v[2][0] = file_bbox.x_max; file_bbox.v[2][1] = file_bbox.y_min; file_bbox.v[2][2] = file_bbox.z_min;
      file_bbox.v[3][0] = file_bbox.x_max; file_bbox.v[3][1] = file_bbox.y_max; file_bbox.v[3][2] = file_bbox.z_min;
      file_bbox.v[4][0] = file_bbox.x_min; file_bbox.v[4][1] = file_bbox.y_min; file_bbox.v[4][2] = file_bbox.z_max;
      file_bbox.v[5][0] = file_bbox.x_min; file_bbox.v[5][1] = file_bbox.y_max; file_bbox.v[5][2] = file_bbox.z_max;
      file_bbox.v[6][0] = file_bbox.x_min; file_bbox.v[6][1] = file_bbox.y_min; file_bbox.v[6][2] = file_bbox.z_min;
      file_bbox.v[7][0] = file_bbox.x_min; file_bbox.v[7][1] = file_bbox.y_max; file_bbox.v[7][2] = file_bbox.z_min;

      for (i = 0; i < 8; i++)
        {
          file_bbox.v[i][0] += global_vector[0];
          file_bbox.v[i][1] += global_vector[1];
          file_bbox.v[i][2] += global_vector[2];
        }
    }
  else
    {
      bbox_available = False;
    }

  /* Check to see if selections are viable */

  for (i = 0; i < no_atoms; i++)
    {
      atoms[i].sel = 0;
    }

  selections_viable = 1;

  for (i = 0; i < 4; i++)
    {
      if (selected[i] >= no_atoms)
        {
          selections_viable = 0;
        }
    }

  if (selections_viable == 1)
    {
      for (i = 0; i < 4; i++)
        {
          if (selected[i] != -1)
            {
              atoms[selected[i]].sel = 1;
            }
        }
    }
  else
    {
      for (i = 0; i < 4; i++)
        {
          selected[i] = -1;
        }
    }
      
  /* Set up scale and depth */

  {
    void reset_pers_scales (void);

    int scale, depth;

    if ((this_frame->perspective_scale) && (this_frame->perspective_depth))
      {
        scale = this_frame->perspective_scale;
        scale = pow (2.0, scale / 100.0) - 1;
        set_canvas_scale (scale);

        depth = this_frame->perspective_depth;
        set_z_depth (depth);

        reset_pers_scales ();
      }
  }
  
#ifdef GL
  XFreeColormap(display, cmap);
#endif /* GL */

}

void
get_atom_data (struct frame *this_frame,
               Colormap cmap,
               int offset,
               double x, double y, double z)
{
  int i, j;
  int io, index;

  XColor xcolor;

  for (i = 0; i < this_frame->no_atoms; i++)
    {
      io = i + offset;

      sorted_atoms[io] = io;

      /* The atom coordinates */

      atoms[io].x = this_frame->atom[i].x + global_vector[0] + x;
      atoms[io].y = this_frame->atom[i].y + global_vector[1] + y;
      atoms[io].z = this_frame->atom[i].z + global_vector[2] + z;

      /* The atom vectors */

      atoms[io].has_vector = this_frame->atom[i].has_vector;

      for (j = 0; j < MAX_VECTORS_PER_ATOM; j++)
        {
          if (atoms[io].has_vector > j)
            {
              atoms[io].v[j][0] = this_frame->atom[i].vx[j];
              atoms[io].v[j][1] = this_frame->atom[i].vy[j];
              atoms[io].v[j][2] = this_frame->atom[i].vz[j];
            }
          else
            {
              atoms[io].v[j][0] = 0;
              atoms[io].v[j][1] = 0;
              atoms[io].v[j][2] = 0;
            }
        }

      /* Ellipse properties  */

      atoms[io].is_ellipse = this_frame->atom[i].is_ellipse;

      if (atoms[io].is_ellipse == 1)
        {
          double max_shape = 0.0;

          for (j = 0; j < 3; j++)
            {
              atoms[io].euler[j] = this_frame->atom[i].euler[j];
              atoms[io].shape[j] = this_frame->atom[i].shape[j];

              /* Keep track of max scaling for GL clipping purposes */

              if (atoms[io].shape[j] > max_shape)
                {
                  max_shape = atoms[io].shape[j];
                }
            }

          atoms[io].max_shape = max_shape;
        }
      else
        {
          atoms[io].max_shape = 1.0;
        }
      
      atoms[io].visi = this_frame->atom[i].visi;

      /* properties for which we need the index of the element */

      strcpy (atoms[io].label, this_frame->atom[i].label);
      strcpy (atoms[io].uppercase_label, this_frame->atom[i].label);
      make_label_uppercase (atoms[io].uppercase_label);

      index = get_element_index_for_type (atoms[io].uppercase_label);

      /* Set label for dummy atoms */

      if (index == 0)
        {
          strcpy (atoms[io].label, "DU");
          strcpy (atoms[io].uppercase_label, "DU");
        }

      atoms[io].cov_rad = element[index].cov_rad;

      if (element[index].vdw_rad != 0)
        {
          atoms[io].vdw_rad = element[index].vdw_rad;
        }
      else
        {
          atoms[io].vdw_rad = atoms[io].cov_rad;
        }

      atoms[io].mass = element[index].mass;

      if (this_frame->atom[i].has_rgb)
        {
          xcolor.red   = 65535 * this_frame->atom[i].red;
          xcolor.green = 65535 * this_frame->atom[i].green;
          xcolor.blue  = 65535 * this_frame->atom[i].blue;

          XAllocColor(display, cmap, &xcolor);
        }
      else if (this_frame->atom[i].has_color_name)
        {
          XParseColor(display, cmap, this_frame->atom[i].color_name, &xcolor);
            
          if (XAllocColor(display,cmap,&xcolor) == 0)
            {
              fprintf
                (stderr,
                 "xmakemol: cannot allocate colour %20s\n",
                 this_frame->atom[i].color_name);
                
              /* Allocate the atom as white instead */

              XParseColor(display, cmap, "white", &xcolor);
              XAllocColor(display, cmap, &xcolor);
            }
        }
      else if (element[index].color_allocated == 0)
        {
          XParseColor(display, cmap, element[index].color, &xcolor);
            
          if (XAllocColor(display,cmap,&xcolor) == 0)
            {
              fprintf
                (stderr,
                 "xmakemol: cannot allocate colour %20s\n",
                 element[index].color);
                
              /* Allocate the atom as white instead */

              XParseColor(display, cmap, "white", &xcolor);
              XAllocColor(display, cmap, &xcolor);
            }
            
          element[index].color_allocated = 1;
          element[index].pixel = xcolor.pixel;
          element[index].red   = xcolor.red;
          element[index].green = xcolor.green;
          element[index].blue  = xcolor.blue;
        }
      else
        {
          xcolor.pixel = element[index].pixel;
          xcolor.red = element[index].red;
          xcolor.green = element[index].green;
          xcolor.blue = element[index].blue;
        }

      atoms[io].pixel = xcolor.pixel;
      atoms[io].red   = xcolor.red;
      atoms[io].green = xcolor.green;
      atoms[io].blue  = xcolor.blue;

      /* Always reset edit status when changing atom contents */

      atoms[io].edit  = 1;

      if (strcmp (atoms[io].uppercase_label, "H") == 0)
        {
          atoms[io].is_hydrogen = 1;
          any_hydrogen = 1;
        }
      else
        {
          atoms[io].is_hydrogen = 0;
        }

      /* Render type */
      atoms[io].gl_render_type = this_frame->atom[i].gl_render_type;

      /* Molecule index */

      atoms[io].molecule = this_frame->atom[i].molecule;
    }
}


int
count_visible_atoms_in_frame (struct frame *this_frame)
{

  int i, count;

  count = 0;

  for (i = 0; i < this_frame->no_atoms; i++)
    {
      if (this_frame->atom[i].visi == 1)
        {
          count++;
        }
    }

  return (count);

}


void check_frame_sizes (void)
{

  void set_all_frames_same_size (int);

  struct frame* this_frame;

  int mismatch = 0, last_no_atoms;

  this_frame = get_first_frame ();

  last_no_atoms = this_frame->no_atoms;

  while (1)
    {
      this_frame = this_frame->next;
      if (this_frame == NULL) break;

      if (this_frame->no_atoms != last_no_atoms)
        {
          mismatch = 1;
          set_all_frames_same_size (0);
          break;
        }
    }

  if (mismatch == 0)
    {
      set_all_frames_same_size (1);
    }

}

/* Keep track of whether all frames have the same number of atoms */

static int all_frames_same_size = 0;

void set_all_frames_same_size (int arg)
{

  all_frames_same_size = arg;

}


int get_all_frames_same_size (void)
{

  return (all_frames_same_size);

}


void
set_current_directory (XmString dir)
{
 
  char *s;
  
  /* Free the old working directory */
  
  if (current_directory != NULL) {
    XmStringFree (current_directory);
  }
  
  /* Save the current working directory */
  
  current_directory = XmStringCopy (dir);
  
  /* Change to the current working directory so that files being created
     without explicit directory reference end up to the current directory */
  
  if (XmStringGetLtoR (dir, XmFONTLIST_DEFAULT_TAG, &s)) {
    chdir (s);
    XtFree (s);
  }
  
}

XmString
get_current_directory (void)
{

  if (current_directory == NULL) {
    /* Make current directory when no file selection dialog has been opened */
    current_directory = XmStringCreateLocalized (".");
  }
  
  return current_directory;
  
}

int *saved_depth, *saved_scale;

void save_perspective_data (void)
{
  int i;
  struct frame *this_frame;

  saved_depth = malloc (no_frames * sizeof (int));
  saved_scale = malloc (no_frames * sizeof (int));

  if ((saved_depth == NULL) || (saved_scale == NULL))
    {
      malloc_failed ("saved_depth or saved_scale");
    }

  this_frame = first_frame;

  i = 0;
  while (this_frame)
    {
      saved_depth[i] = this_frame->perspective_depth;
      saved_scale[i] = this_frame->perspective_scale;

      this_frame = this_frame->next;
      i++;
    }
}

void load_perspective_data (void)
{
  int i;
  struct frame *this_frame;

  this_frame = first_frame;

  i = 0;
  while (this_frame)
    {
      this_frame->perspective_depth = saved_depth[i];
      this_frame->perspective_scale = saved_scale[i];

      this_frame = this_frame->next;
      i++;
    }

  free (saved_depth);
  free (saved_scale);
}
