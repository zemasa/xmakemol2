/*

Copyright (C) 1998, 1999, 2000, 2001, 2005 Matthew P. Hodges
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
#include <unistd.h> /* aro */

#include <Xm/MessageB.h>

#include "defs.h"
#include "globals.h"

double mod_of_vec (double *vec)
{
  double mod;

  mod = sqrt ((vec[0] * vec[0]) +
              (vec[1] * vec[1]) +
              (vec[2] * vec[2]));
  
  return (mod);
}


void normalize_vec (double *vec)
{
  double mod;

  mod = ((vec[0] * vec[0]) +
         (vec[1] * vec[1]) +
         (vec[2] * vec[2]));

  if (mod == 0) return;

  mod = sqrt (mod);

  vec[0] /= mod;
  vec[1] /= mod;
  vec[2] /= mod;
}


double dot_product (double *vec_1,
                    double *vec_2)
{
  double product;

  product = ((vec_1[0] * vec_2[0]) +
             (vec_1[1] * vec_2[1]) +
             (vec_1[2] * vec_2[2]));

  return (product);
}
  

void vector_product (double *vec_1,
                     double *vec_2,
                     double *vec_3)
{
  vec_1[0] = (vec_2[1] * vec_3[2]) - (vec_2[2] * vec_3[1]);
  vec_1[1] = (vec_2[2] * vec_3[0]) - (vec_2[0] * vec_3[2]);
  vec_1[2] = (vec_2[0] * vec_3[1]) - (vec_2[1] * vec_3[0]);
}

/* Return the nearest integer --- named to avoid clashes with the
   sometimes present function, nint */

int xm_nint (double value)
{
  
  return ((int) (value + 0.5));
    
}

void
echo_to_message_area (char *message)
{

  XmString xm_message;

  xm_message = XmStringCreateLocalized (message);

  XtVaSetValues (message_area,
                 XmNlabelString,
                 xm_message,
                 NULL);
}

void
clear_message_area (void)
{
  echo_to_message_area (" ");   /* Gross */
}

void
euler_to_matrix (double *angle, double *matrix)
{

  double alpha, beta, gamma;
  double ca, cb, cg, sa, sb, sg;

  alpha = angle[0] * DEG2RAD;
  beta  = angle[1] * DEG2RAD;
  gamma = angle[2] * DEG2RAD;

  ca = cos (alpha); cb = cos (beta); cg = cos (gamma);
  sa = sin (alpha); sb = sin (beta); sg = sin (gamma);

  /* Construct the matrix consisting of the composite rotation of:

  gamma about Z
  beta  about Y
  alpha about Z
  
  */

  matrix[0] = cg*(cb*ca) - sg*sa;
  matrix[1] = -(sg*(cb*ca)) - cg*sa;
  matrix[2] = sb*ca;

  matrix[3] = cg*(cb*sa) + sg*ca;
  matrix[4] = cg*ca - sg*(cb*sa);
  matrix[5] = sb*sa;

  matrix[6] = -(cg*sb);
  matrix[7] = sg*sb;
  matrix[8] = cb;

}


#ifdef SGI_STEREO

/* aro - returns current refresh frequency 
         in Hz for SGI machines */
int get_monitor_frequency()
{
  char buf[1024] = "";
  char space_tab[] = " \t";
  char refresh_rate[10];
  char *p;
  FILE *output;
  int index;
  int rounded_refresh;

  /* redirect stderr so error messages won't go to terminal 
     (the "2>" notation does not work with csh/tcsh, but popen
     uses '/bin/sh -c' to execute the command, and sh uses this
     notation */
  output = popen("/usr/gfx/gfxinfo 2> /dev/null | grep Hz", "r");

  /* Error check */
  if(output == NULL) {
    fprintf(stderr, "Error: popen() failed in get_monitor_frequency()\n");
    exit(1);
  }
    
  /* Display line containing display info */
  while (fgets(buf, 1024, output) != NULL)
    {
      index = strspn(buf, space_tab);
      strcpy(buf, &buf[index]);
      /* printf("Line containing display info:\n>> %s", buf); */
    }

  if(strlen(buf) != 0)
    {
      p = (char*) strstr(buf, "Hz");
      for(index = 0; p[index] != ' '; index--)
        {/*NOOP*/}

      if(abs(index) > 10)
        {
          fprintf(stderr, "The current refresh rate seems to be abnormally long.\n");
          exit(1);
        }
      
      index++;
      strncpy(refresh_rate, &p[index], abs(index));
      refresh_rate[index] = '\0';
    }
  else
    {
      fprintf(stderr, "Error: Could not retrieve display refresh rate\n");
      exit(1);
    }
  
  p = (char *) strstr(refresh_rate, ".");
  if(p != NULL)
    {
      /* printf("Refresh rate contains decimal, rounding off\n"); */
      if(p[1] >= '5')
        rounded_refresh = atoi(refresh_rate) + 1;
      else
        rounded_refresh = atoi(refresh_rate);
    }
  else
    rounded_refresh = atoi(refresh_rate);
  
  /* printf("Refresh rate: %dHz\n", rounded_refresh); */
  
  pclose(output);

  return rounded_refresh;
}

/* aro - set monitor refresh frequency to freq */
void set_monitor_frequency(int freq)
{
  char string[40];

  sprintf(string, "/usr/gfx/setmon -n %d", freq);

  system(string);

}
#endif /* SGI_STEREO */
