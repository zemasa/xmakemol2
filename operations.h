#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <Xm/MessageB.h>
#include "globals.h"
#include <Xm/FileSB.h>
#include <Xm/Label.h>
#include <Xm/List.h>
#include <Xm/MessageB.h> /* aro */
#include <Xm/PushB.h>    /* aro */
#include <Xm/RowColumn.h>/* aro */
#include <Xm/TextF.h>    /* aro */
#include <Xm/ToggleB.h>  /* aro */
#include <Xm/Separator.h>
#include <Xm/ToggleBG.h>
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

void place_dialog_cb (Widget, XtPointer, XtPointer);
void echo_to_message_area(char *);

void write_file3(char *);
void write_file2(Widget, XtPointer, XtPointer);
void write_cb(Widget, XtPointer, XtPointer);
void operations_cb(Widget , XtPointer , XtPointer );
void make_reflection_dlg(Widget);
void normaliza(double *, double *);
double rotaz(double, double *);
void reflect_atoms(double *, double *, double *);
void ref_vec_button_cb(Widget , XtPointer , XtPointer );
void ref_pt_button_cb(Widget , XtPointer , XtPointer );
void undo_ref_button_cb(Widget , XtPointer , XtPointer );
void discard_ref_button_cb(Widget , XtPointer , XtPointer );
void cancel_ref_cb(Widget , XtPointer , XtPointer );
void ok_ref_cb(Widget , XtPointer , XtPointer );
void get_atoms_ready_cb(Widget , XtPointer , XtPointer );
void all_atoms_warning(Widget , XtPointer , XtPointer );
void get_selected_atoms(Widget , XtPointer , XtPointer );
void get_atoms_cb(Widget , XtPointer , XtPointer );
void make_inversion_dlg(Widget );
void invert_atoms(double , double , double );
void inv_button_cb(Widget , XtPointer , XtPointer );
void discard_inv_button_cb(Widget , XtPointer , XtPointer );
void undo_inv_button_cb(Widget , XtPointer , XtPointer );
void ok_inv_cb(Widget , XtPointer , XtPointer );
void cancel_inv_cb(Widget , XtPointer , XtPointer );
void toggled_axis_rot (Widget , XtPointer , XtPointer );
void toggled_angle_rot (Widget , XtPointer , XtPointer );
void untoggle_rotation_radio_box(Widget , XtPointer , XtPointer );
void make_rotation_dlg(Widget );
void create_vector_points(double , double , double , double , double , double );
//void multmatrix(int **, int *);
void rot_ate(double, int );
void rot_button_cb(Widget , XtPointer , XtPointer );
void discard_rot_button_cb(Widget , XtPointer , XtPointer );
void undo_rot_button_cb(Widget , XtPointer , XtPointer );
void cancel_rot_cb(Widget , XtPointer , XtPointer );
void ok_rot_cb(Widget , XtPointer , XtPointer );
void toggled_axis_trans (Widget , XtPointer , XtPointer );
void make_translation_dlg(Widget );
void trans_button_cb(Widget , XtPointer , XtPointer );
void discard_trans_button_cb(Widget , XtPointer , XtPointer );
void undo_trans_button_cb(Widget , XtPointer , XtPointer );
void cancel_trans_cb(Widget , XtPointer , XtPointer );
void ok_trans_cb(Widget , XtPointer , XtPointer );
void translate(double *);
void adjust_current_frame(void);
void update_bbox(void);
void update_bond_matrix(Boolean);
void make_add_atom_dlg(Widget parent);
void add_atom(double , double , double , char *);
void add_button_cb(Widget , XtPointer , XtPointer );
void ok_add_cb(Widget , XtPointer , XtPointer );
void discard_add_button_cb(Widget , XtPointer , XtPointer );
void undo_add_button_cb(Widget , XtPointer , XtPointer );
void cancel_add_cb(Widget , XtPointer , XtPointer );
void make_delete_dlg(Widget parent);
void delete_atom(double , double , double );
void delete_button_cb(Widget , XtPointer , XtPointer );
void ok_delete_cb(Widget , XtPointer , XtPointer );
void discard_delete_button_cb(Widget , XtPointer , XtPointer );
void undo_delete_button_cb(Widget , XtPointer , XtPointer );
void cancel_delete_cb(Widget , XtPointer , XtPointer );
