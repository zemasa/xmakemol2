/*

Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2005, 2006, 2007 Matthew P. Hodges
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

#define __XMAKEMOL_C__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <locale.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <Xm/DrawingA.h>
#include <Xm/Label.h>
#include <Xm/MainW.h>
#include <Xm/Protocols.h> /* aro */

#include "defs.h"
#include "globals.h"
#include "view.h"

/* aro - might not be necessary if using mesa+linux */
#ifdef GL
#include <GL/GLwMDrawA.h>  /* OpenGL drawing area widget */
#endif /* GL */

#ifdef GL
#include "gl_funcs.h"
#endif /* GL */

void echo_to_message_area(char *);

int file_specified = 0;

char go_file[1024];
char xmakemol_version[5];

/* aro - made app global so we can have warning dialog
   in edit.c that does not allow user to continue
   until he/she responds to dialog */
XtAppContext app;


int
main(int argc, char **argv)
{
  void set_render_using_gl(Boolean); /* aro */
  void canvas_cb(Widget, XtPointer, XtPointer);
  void canvas_resize_cb(Widget, XtPointer, XtPointer);
  void canvas_expose_cb(Widget, XtPointer, XtPointer);
  void get_options(int, char **);
  void initialize_xmakemol(void);
  void make_menus(Widget, Widget);
  void open_file(char *, Boolean);
  void read_elements_file(void);
  void read_user_elements_file(void);/* aro */
  void reset_any_atoms(void);/* aro */
  void cleanup_cb(Widget, XtPointer, XtPointer); /* aro */
  char version_string[128];

  /* 
     Do modified translations have to come first?

     Each argument to track corresponds to an enum element
     (mouse_translations); see track.c
  */

  String translations =
    "Ctrl<Btn1Down>:    track(9)  \n\
     Ctrl<Btn1Motion>:  track(10) \n\
     Ctrl<Btn1Up>:      track(11) \n\
     Ctrl<Btn2Down>:    track(12) \n\
     Ctrl<Btn2Motion>:  track(13) \n\
     Ctrl<Btn2Up>:      track(14) \n\
     Ctrl<Btn3Down>:    track(15) \n\
     Ctrl<Btn3Motion>:  track(16) \n\
     Ctrl<Btn3Up>:      track(17) \n\
     Shift<Btn1Down>:   track(18) \n\
     Shift<Btn1Motion>: track(19) \n\
     Shift<Btn1Up>:     track(20) \n\
     Shift<Btn2Down>:   track(21) \n\
     Shift<Btn2Motion>: track(22) \n\
     Shift<Btn2Up>:     track(23) \n\
     Shift<Btn3Down>:   track(24) \n\
     Shift<Btn3Motion>: track(25) \n\
     Shift<Btn3Up>:     track(26) \n\
     <Btn1Down>:        track(0)  \n\
     <Btn1Motion>:      track(1)  \n\
     <Btn1Up>:          track(2)  \n\
     <Btn2Down>:        track(3)  \n\
     <Btn2Motion>:      track(4)  \n\
     <Btn2Up>:          track(5)  \n\
     <Btn3Down>:        track(6)  \n\
     <Btn3Motion>:      track(7)  \n\
     <Btn3Up>:          track(8)  \n\
     <Key><:            frame_key(prev)  \n\
     <Key>>:            frame_key(next)  \n\
     <Key>^:            frame_key(start)";

  String fallback_resources[] =
    {
      "xmakemol*fontList:	fixed",
      "xmakemol*background:	dark grey",
      NULL
    };

  Widget menubar=NULL;

  Atom wm_delete_window;
  
  XtActionProc track(Widget, XEvent *, String *, int *);
  XtActionProc frame_key(Widget, XEvent *, String *, int *);

  XtActionsRec actions;

 /* Set the version */

  strcpy (xmakemol_version, VERSION);

  XtSetLanguageProc (NULL, NULL, NULL);

  /* Initialize toolkit and parse command line options. */
  /* Creates the application context 'app' */

  toplevel = XtVaAppInitialize
    (&app, "XMakemol",
     NULL, 0, &argc, argv, fallback_resources, NULL);
  
  /* If user selects 'close' from window manager, do some cleanup
     before exiting */
  wm_delete_window = XmInternAtom(XtDisplay(toplevel),
                                  "WM_DELETE_WINDOW", False);
  XmAddWMProtocolCallback(toplevel, wm_delete_window,
                          cleanup_cb, NULL);
  
  /* Check the locale -- this must come after XtVaAppInitialize */

  if (setlocale (LC_NUMERIC, "C") == NULL)
    {
      printf ("xmakemol: could not set locale (LC_NUMERIC) to C\n");
      exit (1);
    }

  /* Add the "track" action/function used by the translation table */

  actions.string = "track";
  actions.proc = (XtActionProc) track;
  XtAppAddActions (app, &actions, 1);

  /* Add the "frame_key" action/function used by the translation table */

  actions.string = "frame_key";
  actions.proc = (XtActionProc) frame_key;
  XtAppAddActions (app, &actions, 1);

  /* main window contains a MenuBar and a Label displaying a pixmap */

  main_w = XtVaCreateManagedWidget
    ("main_window",
     xmMainWindowWidgetClass,   toplevel,
     NULL);

  /* generate menus */
  
  make_menus(main_w,menubar);

  get_options(argc,argv);
 
  /* set screen properties */

  /* aro - changed from using canvas to using toplevel because
     we need display before we create canvas, as the canvas
     widget is now glwMDrawingArea instead of xmDrawingArea and it takes
     visual info during the creation.  XtScreen changed similiarly, since it
     comes before canvas creation. */

  screen_ptr=XtScreen(toplevel);
  display=XtDisplay(toplevel);
  screen_num=DefaultScreen(display);
  screen_depth=DefaultDepth(display,screen_num);
  root_window=RootWindow(display,screen_num);

/* aro--> moved */

#ifdef GL
  glutInit (&argc, argv);

  visinfo = glXChooseVisual(display, screen_num, gl_attrib);

  if (!visinfo)
    {
      printf ("xmakemol: couldn't get an RGB, Double-buffered visual\n");
      exit (1);
    }

#endif /* GL */
  /* This returns the drawing area widget (canvas). We parse the
     translation table here for mouse action on the canvas */

  /* aro - if using mesa+linux, may be able to use regular 
     xmDrawingAreaWidgetClass with GL here */
  canvas = XtVaCreateManagedWidget
    ("canvas",
#ifdef GL
     xmDrawingAreaWidgetClass,
#else
     xmDrawingAreaWidgetClass,
#endif /* GL */
     main_w,
#ifdef GL
     GLwNvisualInfo, visinfo,
#endif /* GL */
     XmNtranslations, XtParseTranslationTable(translations),
     XmNwidth,  canvas_width,
     XmNheight, canvas_height,
     NULL);


/* <--aro moved */

  XtAddCallback(canvas,
#ifdef GL
                GLwNresizeCallback,
#else
                XmNresizeCallback, 
#endif /* GL */                
                canvas_resize_cb,NULL);
  

  XtAddCallback(canvas,
#ifdef GL                
                GLwNexposeCallback,
#else
                XmNexposeCallback, 
#endif /* GL */
                canvas_expose_cb,NULL);
  
  XtManageChild(canvas);
  
  sprintf (version_string,
           "XMakemol - version %s (see Help->About for non-warranty) ",
           xmakemol_version);

  message_area = XtVaCreateManagedWidget
    (version_string,
     xmLabelWidgetClass, main_w,
     XmNalignment,   XmALIGNMENT_BEGINNING,
     NULL);
  
  
  XmMainWindowSetAreas
    (main_w, menubar, NULL,
     message_area, NULL, canvas);

  /* Instantiate the toplevel widget and enter the main event loop.
     It's all callbacks from here ! */

  XtRealizeWidget(toplevel);

  /* Set up a graphical context (Xlib) */

  gcv.background=BlackPixelOfScreen(screen_ptr);

  gc=XCreateGC(display,XtWindow(canvas),GCBackground,&gcv);

  XtVaSetValues(canvas,XmNuserData,gc,NULL);

#ifdef GL

  /* For some X11 displays, we can't draw on the canvas_pm due to
  crashes. Draw on the canvas window instead and use a backing store
  if available. See also gl_funcs.c */

  {
    int backing_store;
    unsigned long valuemask;
    XSetWindowAttributes attributes;

    backing_store = DoesBackingStore (screen_ptr);

    if ((backing_store == WhenMapped) ||
        (backing_store == Always))
      {

        attributes.backing_store = Always;
        valuemask = CWBackingStore;

        XChangeWindowAttributes (display,
                                 XtWindow (canvas),
                                 valuemask,
                                 &attributes);
      }
  }

  glx_context = glXCreateContext (display, visinfo, NULL, GL_TRUE);
  
  canvas_pm = XCreatePixmap
      (display,
       XtWindow(canvas),
       canvas_width,
       canvas_height,
       visinfo->depth);

  glXMakeCurrent (display, XtWindow (canvas), glx_context);

#endif /* GL */

  initialize_xmakemol();

  read_elements_file();

  read_user_elements_file();/* aro: read in .xmakemol.elements from users home dir */

  reset_any_atoms();/* aro: initialize elements[i].any_atoms to False */
    /* On an sgi machine for some reason element[1].any_atoms == 239, 
       before any file was ever read */ 

  if (file_specified == 1)
    {
      /* aro: If file specified on command line, update
         current_file_name so proper name will appear
         in print dialog */
      strcpy (current_file_name, go_file);
      open_file (go_file, FALSE);
    }

  XtAppMainLoop(app);

  return(0);
}


void 
get_options(int argc, char **argv)
{
  char opt_list[16];

  int c, errflg = 0;

  extern char *optarg;

  Widget widget;

#ifdef GL
  strcpy (opt_list, "abhc:e:f:Guv"); /* extra -G option */
#else /* GL */
  strcpy (opt_list, "abhc:e:f:uv");
#endif

  while ((c = getopt (argc, argv, opt_list)) != EOF)
    {
      switch(c)
        {
        case 'a':
          atom_flag=0;
          if((widget = XtNameToWidget(nth_menu[4], "button_0")))
            XtVaSetValues(widget, XmNset, False, NULL);
          break;
        case 'b':
          bond_flag=0;
          if((widget = XtNameToWidget(nth_menu[4], "button_1")))
            XtVaSetValues(widget, XmNset, False, NULL);
          break;
        case 'h':
          hbond_flag=0;
          if((widget = XtNameToWidget(nth_menu[4], "button_2")))
            XtVaSetValues(widget, XmNset, False, NULL);
          break;
        case 'c':
          strcpy(bg_color,optarg);
          break;
        case 'e':
          strcpy(bb_color,optarg);
          break;
        case 'f':
          file_specified = 1;

          if (strcmp (optarg, "-") != 0)
            {
              strcpy (go_file, optarg);
            }
          else
            {
              /* Surely, noone will call a file by this name. */
              strcpy (go_file, "<STDIN>");
            }          
          break;
#ifdef GL
        case 'G':
          {
            void set_render_using_gl (Boolean);

            set_render_using_gl (False);
          }
          break;
#endif /* GL */
        case 'u':
          fprintf(stderr, "Usage: xmakemol [options]\n");
          fprintf(stderr, "       -a          Switch off atoms\n");
          fprintf(stderr, "       -b          Switch off bonds\n");
          fprintf(stderr, "       -h          Switch off hydrogen bonds\n");
          fprintf(stderr, "       -c <colour> Set the canvas colour\n");
          fprintf(stderr, "       -e <colour> Set the bounding box colour\n");
          fprintf(stderr, "       -f <file>   Read file on startup (use '-f -' for STDIN)\n");
#ifdef GL
          fprintf(stderr, "       -G          Switch off GL rendering\n");
#endif /* GL */
          fprintf(stderr, "       -u          Print usage information\n");
          fprintf(stderr, "       -v          Print version information\n");
          exit(2);
          break;
        case 'v':
          fprintf(stderr, "XMakemol version %s.\n", xmakemol_version);
          fprintf(stderr, "Copyright (C) 2007 Matthew P. Hodges.\n");
          fprintf(stderr, "XMakemol comes with ABSOLUTELY NO WARRANTY.\n");
          fprintf(stderr, "You may redistribute copies of XMakemol\n");
          fprintf(stderr, "under the terms of the GNU General Public License.\n");
          fprintf(stderr, "For more information about these matters, see the file named COPYING.\n");
          exit(2);
          break;
        case '?':
          errflg++;
        }
    }
  
  if(errflg != 0)
    {
      fprintf(stderr, "See xmakemol usage with \"xmakemol -u\"\n");
      exit(2);
    }
  
}


void
canvas_expose_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{
  void canvas_cb(Widget, XtPointer, XtPointer);

  XmDrawingAreaCallbackStruct *dacs =
    (XmDrawingAreaCallbackStruct *) call_data;
  XExposeEvent *event = (XExposeEvent *) dacs->event;

  if (! event->count)
    {
      redraw = 1;
      canvas_cb (canvas, NULL, NULL);
    }
}

void
canvas_resize_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{

  void canvas_cb(Widget, XtPointer, XtPointer);
#ifdef XPM
  void disable_show_logo(void);
#endif

#ifdef SGI_STEREO
  /* cut enum render_stereo_types get_gl_render_stereo(); */
#endif /* SGI_STEREO */
  char string[100];

  if(!XtIsRealized(canvas)){
    return;
  }

#ifdef XPM
  disable_show_logo(); /* Don't show after a resize event */
#endif
  XtVaGetValues(canvas,XmNwidth,&canvas_width,NULL);
  XtVaGetValues(canvas,XmNheight,&canvas_height,NULL);

#ifdef SGI_STEREO
  /*cut  if(get_gl_render_stereo()!=SGI_HARDWARE){ */
#endif /* SGI_STEREO */
    
    /* destroy old pixmap and make new */
    
    if(canvas_pm){
      XFreePixmap(display,canvas_pm);
    }
    
    canvas_pm=XCreatePixmap
      (display,
       XtWindow (canvas),
       canvas_width,
       canvas_height,
#ifdef GL
       visinfo->depth
#else /* GL */
       screen_depth
#endif /* GL */
       );
    
#ifdef GL
    glViewport (0, 0, canvas_width, canvas_height);
#endif /* GL */

  sprintf(string,"Resized to %d x %d",(int)canvas_width,(int)canvas_height);

#ifdef SGI_STEREO
  /*cut  }else{ */
    
    /* destroy old pixmap and make new */
  /* cut 
    if(canvas_pm){
      XFreePixmap(display,canvas_pm);
    }

    canvas_pm=XCreatePixmap
      (display,XtWindow(canvas),canvas_width,canvas_height,visinfo->depth);

    glViewport (0, 0, canvas_width, canvas_height);

    sprintf(string,"Resized to %d x %d (SGI Crystaleyes Mode)",
            (int)canvas_width,(int)canvas_height);

            } */
#endif /* SGI_STEREO */

    

  redraw=1;
  canvas_cb(canvas,NULL,NULL);

  echo_to_message_area(string);

}


void
initialize_xmakemol(void)
{

  /* Set variables so that if file not loaded, things behave
     correctly. */

  int i;

  for(i=0;i<4;i++){
    selected[i]=-1;
  }

  sel_init=0;    /* remove any selections */

}

void cleanup_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{
  /* do cleanup before exiting here */
#ifdef SGI_STEREO
  void set_gl_render_stereo(enum render_stereo_types);

  set_gl_render_stereo(NO_STEREO);
#endif /* SGI_STEREO */

  exit (0);
}


void place_dialog_cb (Widget widget, XtPointer client_data, XtPointer call_data)
{
  Dimension pw, w, wmdw, ws;
  Position px, x;

  XtVaGetValues (main_w,
                 XmNwidth, &pw,
                 XmNx, &px,
                 NULL);

  XtVaGetValues (widget,
                 XmNwidth, &w,
                 NULL);

  /* Window manager decoration widths */
  wmdw = 10;

  ws = WidthOfScreen (XtScreen (widget)); 
  
  if ((px + pw + w + (3 * wmdw)) < ws)
    {
      /* Place to right of main window */
      x = px + (pw + (2 * wmdw));
    }
  else if (px > (w + (3 * wmdw)))
    {
      /* Place to left of main window */
      x = px - (w + (2 * wmdw));
    }
  else
    {
      /* Place as far right as possible */
      x = ws - (w + (2 * wmdw));
    }

  /* NB: the WM can subvert this, rendering this useless */

  XtVaSetValues (widget,
                 XmNx, x,
                 NULL);
}
