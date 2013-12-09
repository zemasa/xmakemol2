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

#include <Xm/Xm.h>

#include "globals.h"
#include "view.h"
#include "operations.h"

void
make_menus(Widget main_w, Widget menubar)
{

  void cont_cb(Widget, XtPointer, XtPointer);
  void edit_cb(Widget, XtPointer, XtPointer);
  void file_cb(Widget, XtPointer, XtPointer);
  void help_cb(Widget, XtPointer, XtPointer);
  void track_cb(Widget, XtPointer, XtPointer);
  void view_cb(Widget, XtPointer, XtPointer);
  void operations_cb(Widget, XtPointer, XtPointer);

  Widget widget;

  XmString file,open,revert,save,merge,export,print,quit;/* aro */       /* 0th menu */
  XmString cont,fram,anim,meas,pers;                   /* 1st */
  XmString edit,visi,posn,sc_co,ab_sz,bd_fd,vdisp,ed_bbox,elem;   /* 2nd */
  XmString track,tr_rotl,tr_roto,cent,tr_reor,tr_repo,
    reflect_x,reflect_y,reflect_z,invert;              /* 3rd */
  XmString view,atoms,bonds,hbonds,vects,at_nos;     /* 4th */
  XmString operations, /*Reflection,*/ Inversion, Rotation, Translation, Write, Add, Delete;
  XmString at_sym,axes,bbox,outl;
  XmString help,about,doc,mouse,bugs;                        /* 5th */

#ifdef GL
  XmString gl_ren;
#endif /* GL */

  /* XmStrings for accelerators */

  XmString accel1, accel2, accel3, accel4, accel5, accel6;

   /* Create a simple MenuBar that contains the five menus */
    
  file  = XmStringCreateLocalized("File");
  cont  = XmStringCreateLocalized("Control");
  edit  = XmStringCreateLocalized("Edit");
  track = XmStringCreateLocalized("Track");
  view  = XmStringCreateLocalized("View");
  operations = XmStringCreateLocalized("Operations");
  help  = XmStringCreateLocalized("Help");
    
  menubar = XmVaCreateSimpleMenuBar
    (main_w, "menubar",
     XmVaCASCADEBUTTON, file,  'F',
     XmVaCASCADEBUTTON, cont,  'C',
     XmVaCASCADEBUTTON, edit,  'E',
     XmVaCASCADEBUTTON, track, 'T',
     XmVaCASCADEBUTTON, view,  'V',
     XmVaCASCADEBUTTON, operations, 'O',
     XmVaCASCADEBUTTON, help,  'H',
     NULL);
    
  XmStringFree(file);
  XmStringFree(cont);
  XmStringFree(edit);
  XmStringFree(track);
  XmStringFree(view);
  XmStringFree(operations);
  XmStringFree(help);

  /* Tell the menubar which button is the help menu  */
    
  if ((widget = XtNameToWidget (menubar, "button_6")))
    XtVaSetValues (menubar, XmNmenuHelpWidget, widget, NULL);
    
  /* Zeroth menu is the File menu -- callback is file_cb() */
    
  open   = XmStringCreateLocalized("Open...");
  revert = XmStringCreateLocalized("Revert");
  save   = XmStringCreateLocalized("Save...");
  merge  = XmStringCreateLocalized("Merge...");
  export = XmStringCreateLocalized("Export...");
  print  = XmStringCreateLocalized("Print..."); /* aro */
  quit   = XmStringCreateLocalized("Quit");

  accel1 = XmStringCreateLocalized("Ctrl+O");
  accel2 = XmStringCreateLocalized("Ctrl+S");
  accel3 = XmStringCreateLocalized("Ctrl+Q");

  nth_menu[0] = XmVaCreateSimplePulldownMenu
    (menubar, "file_menu", 0, file_cb,
     XmVaPUSHBUTTON, open,  'O', "Ctrl<Key>O", accel1,
     XmVaPUSHBUTTON, revert,'R', NULL, NULL,
     XmVaPUSHBUTTON, save,  'S', "Ctrl<Key>S", accel2,
     XmVaPUSHBUTTON, merge, 'M', NULL, NULL,
     XmVaSEPARATOR,
     XmVaPUSHBUTTON, export, 'E', NULL, NULL,
     XmVaSEPARATOR,
     XmVaPUSHBUTTON, print, 'P', NULL, NULL,/* aro */
     XmVaSEPARATOR,/* aro */
     XmVaPUSHBUTTON, quit,  'Q', "Ctrl<Key>Q", accel3,
     NULL);

  XmStringFree(open);
  XmStringFree(revert);
  XmStringFree(save);
  XmStringFree(merge);
  XmStringFree(export);
  XmStringFree(print);/* aro */
  XmStringFree(quit);

  XmStringFree(accel1);
  XmStringFree(accel2);
  XmStringFree(accel3);

  /* First menu is the control menu -- callback is cont_cb() */

  fram = XmStringCreateLocalized("Frames...");
  anim = XmStringCreateLocalized("Animate...");
  meas = XmStringCreateLocalized("Measure...");
  pers = XmStringCreateLocalized("Perspective...");

  accel1 = XmStringCreateLocalized("Ctrl+F");
  accel2 = XmStringCreateLocalized("Ctrl+M");
  accel3 = XmStringCreateLocalized("Ctrl+P");

  nth_menu[1] = XmVaCreateSimplePulldownMenu
    (menubar, "cont_menu", 1, cont_cb,
     XmVaPUSHBUTTON, fram, 'F', "Ctrl<Key>F", accel1,
     XmVaPUSHBUTTON, anim, 'A', NULL, NULL,
     XmVaPUSHBUTTON, meas, 'M', "Ctrl<Key>M", accel2,
     XmVaPUSHBUTTON, pers, 'P', "Ctrl<Key>P", accel3,
     NULL);

  XmStringFree(fram);
  XmStringFree(anim);
  XmStringFree(meas);
  XmStringFree(pers);
  
  XmStringFree(accel1);
  XmStringFree(accel2);
  XmStringFree(accel3);

  /* Second menu is the edit menu -- callback is edit_cb() */

  visi=XmStringCreateLocalized("Visible...");
  posn=XmStringCreateLocalized("Positions...");
  sc_co=XmStringCreateLocalized("Scale coordinates...");
  ab_sz=XmStringCreateLocalized("Atom/bond sizes...");
  bd_fd=XmStringCreateLocalized("Bond factors...");
  vdisp = XmStringCreateLocalized("Vector display...");
  ed_bbox=XmStringCreateLocalized("Bounding Box..."); 
  elem = XmStringCreateLocalized ("Element properties...");
#ifdef GL
  gl_ren = XmStringCreateLocalized ("GL rendering...");
#endif /* GL */

  accel1 = XmStringCreateLocalized("Ctrl+V");
  accel2 = XmStringCreateLocalized("Ctrl+A");
  accel3 = XmStringCreateLocalized("Ctrl+B");

  nth_menu[2] = XmVaCreateSimplePulldownMenu
    (menubar, "edit_menu", 2, edit_cb,
     XmVaPUSHBUTTON, visi,  'V', "Ctrl<Key>V", accel1,
     XmVaPUSHBUTTON, posn,  'P', NULL, NULL,
     XmVaPUSHBUTTON, sc_co, 'S', NULL, NULL,
     XmVaSEPARATOR,
     XmVaPUSHBUTTON, ab_sz, 'A', "Ctrl<Key>A", accel2,
     XmVaPUSHBUTTON, bd_fd, 'B', "Ctrl<Key>B", accel3,
     XmVaSEPARATOR,
     XmVaPUSHBUTTON, vdisp, 'D', NULL, NULL,
     XmVaPUSHBUTTON, ed_bbox, 'O', NULL, NULL,
     XmVaPUSHBUTTON, elem,  'E', NULL, NULL,
#ifdef GL
     XmVaPUSHBUTTON, gl_ren, 'G', NULL, NULL,
#endif /* GL */
     NULL);

  XmStringFree(visi);
  XmStringFree(posn);
  XmStringFree(sc_co);
  XmStringFree(ab_sz);
  XmStringFree(bd_fd);
  XmStringFree(vdisp);
  XmStringFree(ed_bbox);
  XmStringFree (elem);
#ifdef GL
  XmStringFree (gl_ren);
#endif /* GL */
  
  XmStringFree(accel1);
  XmStringFree(accel2);
  XmStringFree(accel3);

  /* Third menu is the track menu -- callback is track_cb() */

  tr_rotl = XmStringCreateLocalized ("Rotate about local COM");
  tr_roto = XmStringCreateLocalized ("Rotate about origin");
  cent = XmStringCreateLocalized ("Centre");
  tr_reor = XmStringCreateLocalized("Original orientation");
  tr_repo = XmStringCreateLocalized("Original position");
  reflect_x = XmStringCreateLocalized("Reflect x coords");
  reflect_y = XmStringCreateLocalized("Reflect y coords");
  reflect_z = XmStringCreateLocalized("Reflect z coords");
  invert = XmStringCreateLocalized("Invert through centre");

  nth_menu[3] = XmVaCreateSimplePulldownMenu
    (menubar, "track_menu", 3, track_cb,
     XmVaRADIOBUTTON, tr_rotl, 'l', NULL, NULL,
     XmVaRADIOBUTTON, tr_roto, 'o', NULL, NULL,
     XmVaSEPARATOR,
     XmVaPUSHBUTTON, cent, 'C', NULL, NULL,
     XmVaPUSHBUTTON, tr_reor, 'o', NULL, NULL,
     XmVaPUSHBUTTON, tr_repo, 'p',NULL,NULL,
     XmVaSEPARATOR,
     XmVaPUSHBUTTON, reflect_x,'x',NULL,NULL,
     XmVaPUSHBUTTON, reflect_y,'y',NULL,NULL,
     XmVaPUSHBUTTON, reflect_z,'z',NULL,NULL,
     XmVaSEPARATOR,
     XmVaPUSHBUTTON, invert,'I',NULL,NULL,
     XmNradioBehavior, True,     /* RowColumn resources to enforce */
     NULL);

  XmStringFree(tr_rotl);
  XmStringFree(tr_roto);
  XmStringFree(cent);
  XmStringFree(tr_reor);
  XmStringFree(tr_repo);
  XmStringFree(reflect_x);
  XmStringFree(reflect_y);
  XmStringFree(reflect_z);
  XmStringFree(invert);

  /* Initialize menu so that "Rotations" is selected. */
  
  if((widget = XtNameToWidget (nth_menu[3], "button_0")))
    XtVaSetValues (widget, XmNset, True, NULL);

  /* Fourth menu is the view menu -- callback is view_cb() */

  atoms   = XmStringCreateLocalized("Atoms");
  bonds   = XmStringCreateLocalized("Bonds");
  hbonds  = XmStringCreateLocalized("H-Bonds");
  vects   = XmStringCreateLocalized("Vectors");
  at_nos  = XmStringCreateLocalized("Numbers");
  at_sym  = XmStringCreateLocalized("Symbols");
  axes    = XmStringCreateLocalized("Axes");
  bbox    = XmStringCreateLocalized("Bounding Box");
  outl    = XmStringCreateLocalized("Outline");
  
  accel1 = XmStringCreateLocalized("A");
  accel2 = XmStringCreateLocalized("B");
  accel3 = XmStringCreateLocalized("H");
  accel4 = XmStringCreateLocalized("N");
  accel5 = XmStringCreateLocalized("S");

  nth_menu[4] = XmVaCreateSimplePulldownMenu
    (menubar, "view_menu", 4, view_cb,
     XmVaTOGGLEBUTTON, atoms,   'A', "<Key>A", accel1,
     XmVaTOGGLEBUTTON, bonds,   'B', "<Key>B", accel2,
     XmVaTOGGLEBUTTON, hbonds,  'H', "<Key>H", accel3,
     XmVaTOGGLEBUTTON, vects,   'V', NULL,NULL,
     XmVaSEPARATOR,
     XmVaTOGGLEBUTTON, at_nos,  'N', "<Key>N", accel4,
     XmVaTOGGLEBUTTON, at_sym,  'S', "<Key>S", accel5,
     XmVaSEPARATOR,
     XmVaTOGGLEBUTTON, axes,    'x', NULL,NULL,
     XmVaTOGGLEBUTTON, bbox,    'o', NULL,NULL,
     XmVaSEPARATOR,
     XmVaTOGGLEBUTTON, outl,    'u', NULL, NULL,
     NULL);

  if(atom_flag==1){
    if((widget = XtNameToWidget(nth_menu[4], "button_0")))
      XtVaSetValues(widget, XmNset, True, NULL);
  }
  
  if(bond_flag==1){
    if((widget = XtNameToWidget(nth_menu[4], "button_1")))
      XtVaSetValues(widget, XmNset, True, NULL);
  }

  if(hbond_flag==1){
    if((widget = XtNameToWidget(nth_menu[4], "button_2")))
      XtVaSetValues(widget, XmNset, True, NULL);
  }

  if(vector_flag==1){
    if((widget = XtNameToWidget(nth_menu[4], "button_3")))
      XtVaSetValues(widget, XmNset, True, NULL);
  }

  XmStringFree(atoms);
  XmStringFree(bonds);
  XmStringFree(hbonds);
  XmStringFree(vects);
  XmStringFree(at_nos);
  XmStringFree(at_sym);
  XmStringFree(axes);
  XmStringFree(bbox);
  XmStringFree(outl);
  
  XmStringFree(accel1);
  XmStringFree(accel2);
  XmStringFree(accel3);
  XmStringFree(accel4);
  XmStringFree(accel5);

  /* Operations menu*/

  //Reflection = XmStringCreateLocalized("Reflection");
  Inversion = XmStringCreateLocalized("Inversion");
  Rotation = XmStringCreateLocalized("Rotation");
  Translation = XmStringCreateLocalized("Translation");
  Write = XmStringCreateLocalized("Save only transformations");
  Add = XmStringCreateLocalized("Add a new atom");
  Delete = XmStringCreateLocalized("Delete selected atom(s)");
  

  nth_menu[5] = XmVaCreateSimplePulldownMenu
	(menubar, "operations_menu", 5, operations_cb,
	//XmVaPUSHBUTTON, Reflection, 'r', NULL, NULL,
	XmVaPUSHBUTTON, Inversion, 'I', NULL, NULL,
	XmVaPUSHBUTTON, Rotation, 'R', NULL, NULL,
	XmVaPUSHBUTTON, Translation, 'T', NULL, NULL,
	XmVaPUSHBUTTON, Write, 'W', NULL, NULL,
	XmVaPUSHBUTTON, Add, 'A', NULL, NULL,
	XmVaPUSHBUTTON, Delete, 'D', NULL, NULL,
	NULL);

  //XmStringFree(Reflection);
  XmStringFree(Inversion);
  XmStringFree(Rotation);
  XmStringFree(Translation);
  XmStringFree(Write);
  XmStringFree(Add);
  XmStringFree(Delete);

  /* Fifth is the help menu -- callback is help_cb() */

  about = XmStringCreateLocalized("About...");
  doc   = XmStringCreateLocalized("Doc...");
  mouse = XmStringCreateLocalized("Mouse...");
  bugs  = XmStringCreateLocalized("Bugs...");
  
  nth_menu[6] = XmVaCreateSimplePulldownMenu
    (menubar, "help_menu", 6, help_cb,
     XmVaPUSHBUTTON, about, 'A', NULL, NULL,
     XmVaPUSHBUTTON, doc,   'D', NULL, NULL,
     XmVaPUSHBUTTON, mouse,   'M', NULL, NULL,
     XmVaPUSHBUTTON, bugs,  'B', NULL, NULL,
     NULL);

  XmStringFree(about);
  XmStringFree(doc);
  XmStringFree(mouse);
  XmStringFree(bugs);

  XtManageChild (menubar);

}
