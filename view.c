/*

Copyright (C) 1998, 1999, 2000, 2001, 2004, 2005 Matthew P. Hodges
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

#define __VIEW_C__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <Xm/Xm.h>

#include "globals.h"
#include "view.h"

/* Function prototypes */

void echo_to_message_area(char *);

/* Private variables */

static int outline_mode = 0;

void
view_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{
  
  void canvas_cb(Widget, XtPointer, XtPointer);
  int get_no_bonds (void);
  int get_no_hbonds (void);
  void update_bbox(void);

  int item_no = (int) client_data;

  char message[128];

  switch(item_no){
  case 0:
    if(atom_flag==0){
      atom_flag=1;
      echo_to_message_area("Switching on atoms");
    }else{
      atom_flag=0;
      echo_to_message_area("Switching off atoms");
    }

    /* If atoms have been switched, the bounding box may need updating */

    if(bbox_flag == 1)
      {
        update_bbox();
      }

    break;
  case 1:
    if(bond_flag==0){
      bond_flag=1;
      sprintf (message, "Switching on bonds (%d)", get_no_bonds ());
      echo_to_message_area (message);
    }else{
      bond_flag=0;
      echo_to_message_area("Switching off bonds");
    }
    break;
  case 2:
    if(hbond_flag==0){
      hbond_flag=1;
      sprintf (message, "Switching on H-bonds (%d)", get_no_hbonds ());
      echo_to_message_area (message);
    }else{
      hbond_flag=0;
      echo_to_message_area("Switching off H-bonds");
    }
    break;
  case 3:
    if(vector_flag==0){
      vector_flag=1;
      echo_to_message_area("Switching on vectors");
    }else{
      vector_flag=0;
      echo_to_message_area("Switching off vectors");
    }
    break;
  case 4:
    if(at_nos_flag==0){
      at_nos_flag=1;
      echo_to_message_area("Switching on numbers");
    }else{
      at_nos_flag=0;
      echo_to_message_area("Switching off numbers");
    }
    break;
  case 5:
    if(at_sym_flag==0){
      at_sym_flag=1;
      echo_to_message_area("Switching on symbols");
    }else{
      at_sym_flag=0;
      echo_to_message_area("Switching off symbols");
    }
    break;
  case 6:
    if(axes_flag==0){
      axes_flag=1;
      echo_to_message_area("Switching on axes");
    }else{
      axes_flag=0;
      echo_to_message_area("Switching off axes");
    }
    break;
  case 7:
    if(bbox_flag==0){
      bbox_flag=1;
      echo_to_message_area("Switching on bounding box");
      update_bbox();
    }else{
      bbox_flag=0;
      echo_to_message_area("Switching off bounding box");
    }
    break;
  case 8:
    if(outline_mode==0){
      outline_mode=1;
      echo_to_message_area("Switching on outline");
    }else{
      outline_mode=0;
      echo_to_message_area("Switching off outline");
    }
    break;
  }
  
  redraw=1;
  canvas_cb(canvas,NULL,NULL);

}


void
centre_atoms(void)
{
  void canvas_cb(Widget, XtPointer, XtPointer);
  void update_lengths_dialog(Boolean);

  int i;

  double com[3],tot_mass;

  for(i=0;i<3;i++){
    com[i]=0;
  }
  
  tot_mass=0;
  
  for(i=0;i<no_atoms;i++){
    com[0]+=((atoms[i].x)*atoms[i].mass);
    com[1]+=((atoms[i].y)*atoms[i].mass);
    com[2]+=((atoms[i].z)*atoms[i].mass);
    tot_mass+=atoms[i].mass;
  }
  
  for(i=0;i<3;i++){
    com[i]/=tot_mass;
  }

  /* Change the atomic positions accordingly */

  for(i=0;i<no_atoms;i++){
    atoms[i].x-=com[0];
    atoms[i].y-=com[1];
    atoms[i].z-=com[2];
  }

  if (bbox_available)
    {
      for (i = 0; i < 8; i++)
        {
          file_bbox.v[i][0] -= com[0];
          file_bbox.v[i][1] -= com[1];
          file_bbox.v[i][2] -= com[2];
        }
    }

  /* Change the global_vector accordingly */

  for(i=0;i<3;i++){
    global_vector[i]-=com[i];
  }

}

static int no_bonds, no_hbonds;

void set_no_bonds (int value)
{
  no_bonds = value;
}

void set_no_hbonds (int value)
{
  no_hbonds = value;
}

int get_no_bonds (void)
{
  return (no_bonds);
}

int get_no_hbonds (void)
{
  return (no_hbonds);
}

Boolean outline_mode_p (void)
{
  return (outline_mode);
}
