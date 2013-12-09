#include "operations.h"

#define DEBUG 1

typedef struct undo{
	double x;
	double y;
	double z;
	double alpha;
	char   label[4];
	struct undo *previous;
}UNDO; 

typedef struct undoreflect{
	double vec1[3];
	double vec2[3];
	double ponto[3];
	struct undoreflect *previous;
}UNDOREF;

  void file_type_cb(Widget, XtPointer, XtPointer);

  Widget ic[2], label, ux_text, uy_text, uz_text, vx_text, vy_text, vz_text, ref_vec_button, ax_text, ay_text, az_text, bx_text, by_text, bz_text, cx_text, cy_text, cz_text, ref_pt_button, trans_button, undo_trans_button, discard_trans_button, tx_text, ty_text, tz_text, rot_button, rot_button_tog, undo_rot_button, discard_rot_button, rax_text, ray_text, raz_text, rbx_text, rby_text, rbz_text, ralpha_text, file_type_w, zalpha, add_button, undo_add_button, discard_add_button, delete_button, undo_delete_button, discard_delete_button, inv_button, undo_inv_button, discard_inv_button, discard_ref_button, undo_ref_button, get_atoms_ref, get_atoms_ref_apply;

  XmString title, as_all, as_fra, con_fra, aux;

  int inv_counter, ref_counter;

  int toggle_axis_rot_set=0;
  int toggle_angle_rot_set=0;

  static Widget reflection_dialog = NULL;
  static Widget inversion_dialog = NULL;
  static Widget rotation_dialog = NULL;
  static Widget translation_dialog = NULL;
  static Widget write_dialog = NULL;
  static Widget add_dialog = NULL;
  static Widget delete_dialog = NULL;
  static Widget s_dialog;

  double trans_global_vector[3], op_vector[3];

  UNDO *undo_stack=NULL, *undor_stack=NULL, *undoa_stack=NULL, *undod_stack=NULL, *undoi_stack=NULL;
  UNDOREF *undoref=NULL;

  struct atoms sel_atoms[3];
  double sel_vec1[3], sel_vec2[3];

/* Writes atoms to file, saving only the operation coordinates */
void write_file3(char *file_name){

  struct frame * get_selected_frame (void);

  struct frame *this_frame;

  int i, no_visible_atoms;

  FILE *f;

  f = fopen(file_name, "w");

  this_frame = get_selected_frame ();
  
  no_visible_atoms = count_visible_atoms ();

  fprintf (f, "%d\n", no_visible_atoms);
  fprintf (f, "%s\n", this_frame->comment);

  for(i=0;i<no_atoms;i++) if (atoms[i].visi) fprintf (f, "%2s %12.6f %12.6f %12.6f \n", atoms[i].label,atoms[i].opx,atoms[i].opy,atoms[i].opz);

  fclose(f);
}


void write_file2(Widget dialog, XtPointer client_data, XtPointer call_data){
  
  char *file_name,message_string[1024];

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
        write_file3(file_name);
      }

    XtFree (file_name); /* free allocated data from XmStringGetLtoR() */
  }
  
  XtUnmanageChild(dialog); 

}


void write_cb(Widget dialog, XtPointer client_data, XtPointer call_data){

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
	write_file2(dialog, client_data, call_data);
      }

    XtFree (file_name); /* free allocated data from XmStringGetLtoR() */
  }
  
  XtUnmanageChild(dialog); 
  
}

void deselect(){
	int i;
    for(i=0;i<no_atoms;i++) atoms[i].sel=False;
}

/* Creates operations menu */
void operations_cb(Widget widget, XtPointer client_data, XtPointer call_data){
	int item_no=(int)client_data;

  switch(item_no){
  /*case 0:
    make_reflection_dlg(main_w);
    break;
    */
  case 0:
    make_inversion_dlg(main_w);
    break;
  case 1:
    make_rotation_dlg(main_w);
    break;
  case 2:
    make_translation_dlg(main_w);
    break;
  case 3:
    if (!s_dialog) {
      s_dialog = XmCreateFileSelectionDialog (toplevel, "file_sel", NULL, 0);
      
      title=XmStringCreateLocalized ("Save File");    
      XtVaSetValues(s_dialog, XmNdialogTitle, title, NULL);
      
     
      XtAddCallback(s_dialog, XmNokCallback, write_cb, NULL);
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
    break;
  case 4:
    make_add_atom_dlg(main_w);
  case 5:
    make_delete_dlg(main_w);
    break;
  default:
  	break;
  }
}

/* Creates reflection window */
void make_reflection_dlg(Widget parent){

  int n;
 
  char *reflection_string = NULL;

  XmString title;

	ref_counter = 0;
	undoref = NULL;

 if(reflection_dialog == NULL)
    {
      title = XmStringCreateLocalized("Reflection");
      
      n=0;
      XtSetArg(args[n], XmNdialogTitle, title); n++;
 
      reflection_dialog =
        XmCreateMessageDialog(main_w, "reflection" ,args, n);
 
      ic[0]=XtVaCreateManagedWidget
      ("ic",xmRowColumnWidgetClass,reflection_dialog,NULL);

      ic[1]=XtVaCreateManagedWidget
      ("epd_ic1",
       xmRowColumnWidgetClass,ic[0],
       XmNorientation, XmHORIZONTAL,
       NULL);

      label = XtVaCreateManagedWidget
      ("Vectors + Point:",
       xmLabelWidgetClass, ic[1],
       NULL);

      ic[1]=XtVaCreateManagedWidget
      ("epd_ic1",
       xmRowColumnWidgetClass,ic[0],
       XmNorientation, XmHORIZONTAL,
       NULL);

      label = XtVaCreateManagedWidget
      ("u",
       xmLabelWidgetClass, ic[1],
       NULL);

      ux_text = XtVaCreateManagedWidget
      ("ux_text", xmTextFieldWidgetClass,
       ic[1],
       XmNcolumns, 8,
       NULL);

      label = XtVaCreateManagedWidget
      ("X",
       xmLabelWidgetClass, ic[1],
       NULL);

      uy_text = XtVaCreateManagedWidget
      ("uy_text", xmTextFieldWidgetClass,
       ic[1],
       XmNcolumns, 8,
       NULL);

      label = XtVaCreateManagedWidget
      ("Y",
       xmLabelWidgetClass, ic[1],
       NULL);

      uz_text = XtVaCreateManagedWidget
      ("uz_text", xmTextFieldWidgetClass,
       ic[1],
       XmNcolumns, 8,
       NULL);

      label = XtVaCreateManagedWidget
      ("Z",
       xmLabelWidgetClass, ic[1],
       NULL);

      ic[1]=XtVaCreateManagedWidget
      ("epd_ic1",
       xmRowColumnWidgetClass,ic[0],
       XmNorientation, XmHORIZONTAL,
       NULL);

      label = XtVaCreateManagedWidget
      ("v",
       xmLabelWidgetClass, ic[1],
       NULL);

      vx_text = XtVaCreateManagedWidget
      ("vx_text", xmTextFieldWidgetClass,
       ic[1],
       XmNcolumns, 8,
       NULL);

      label = XtVaCreateManagedWidget
      ("X",
       xmLabelWidgetClass, ic[1],
       NULL);

      vy_text = XtVaCreateManagedWidget
      ("vy_text", xmTextFieldWidgetClass,
       ic[1],
       XmNcolumns, 8,
       NULL);

      label = XtVaCreateManagedWidget
      ("Y",
       xmLabelWidgetClass, ic[1],
       NULL);

      vz_text = XtVaCreateManagedWidget
      ("vz_text", xmTextFieldWidgetClass,
       ic[1],
       XmNcolumns, 8,
       NULL);

      label = XtVaCreateManagedWidget
      ("Z",
       xmLabelWidgetClass, ic[1],
       NULL);

      ic[1]=XtVaCreateManagedWidget
      ("epd_ic1",
       xmRowColumnWidgetClass,ic[0],
       XmNorientation, XmHORIZONTAL,
       NULL);

      label = XtVaCreateManagedWidget
      ("P",
       xmLabelWidgetClass, ic[1],
       NULL);

      cx_text = XtVaCreateManagedWidget
      ("cx_text", xmTextFieldWidgetClass,
       ic[1],
       XmNcolumns, 8,
       NULL);

      label = XtVaCreateManagedWidget
      ("X",
       xmLabelWidgetClass, ic[1],
       NULL);

      cy_text = XtVaCreateManagedWidget
      ("cy_text", xmTextFieldWidgetClass,
       ic[1],
       XmNcolumns, 8,
       NULL);

      label = XtVaCreateManagedWidget
      ("Y",
       xmLabelWidgetClass, ic[1],
       NULL);

      cz_text = XtVaCreateManagedWidget
      ("cz_text", xmTextFieldWidgetClass,
       ic[1],
       XmNcolumns, 8,
       NULL);

      label = XtVaCreateManagedWidget
      ("Z",
       xmLabelWidgetClass, ic[1],
       NULL);

      ref_vec_button = XtVaCreateManagedWidget
      ("Apply", xmPushButtonWidgetClass,
       ic[1],
       NULL);

      XtAddCallback(ref_vec_button, XmNactivateCallback, ref_vec_button_cb, NULL);

      ic[1]=XtVaCreateManagedWidget
      ("epd_ic1",
       xmRowColumnWidgetClass,ic[0],
       XmNorientation, XmHORIZONTAL,
       NULL);

      label = XtVaCreateManagedWidget
      ("Points:",
       xmLabelWidgetClass, ic[1],
       NULL);

      ic[1]=XtVaCreateManagedWidget
      ("epd_ic1",
       xmRowColumnWidgetClass,ic[0],
       XmNorientation, XmHORIZONTAL,
       NULL);

      label = XtVaCreateManagedWidget
      ("A",
       xmLabelWidgetClass, ic[1],
       NULL);

      ax_text = XtVaCreateManagedWidget
      ("ax_text", xmTextFieldWidgetClass,
       ic[1],
       XmNcolumns, 8,
       NULL);

      label = XtVaCreateManagedWidget
      ("X",
       xmLabelWidgetClass, ic[1],
       NULL);

      ay_text = XtVaCreateManagedWidget
      ("ay_text", xmTextFieldWidgetClass,
       ic[1],
       XmNcolumns, 8,
       NULL);

      label = XtVaCreateManagedWidget
      ("Y",
       xmLabelWidgetClass, ic[1],
       NULL);

      az_text = XtVaCreateManagedWidget
      ("az_text", xmTextFieldWidgetClass,
       ic[1],
       XmNcolumns, 8,
       NULL);

      label = XtVaCreateManagedWidget
      ("Z",
       xmLabelWidgetClass, ic[1],
       NULL);

      ic[1]=XtVaCreateManagedWidget
      ("epd_ic1",
       xmRowColumnWidgetClass,ic[0],
       XmNorientation, XmHORIZONTAL,
       NULL);

      label = XtVaCreateManagedWidget
      ("B",
       xmLabelWidgetClass, ic[1],
       NULL);

      bx_text = XtVaCreateManagedWidget
      ("bx_text", xmTextFieldWidgetClass,
       ic[1],
       XmNcolumns, 8,
       NULL);

      label = XtVaCreateManagedWidget
      ("X",
       xmLabelWidgetClass, ic[1],
       NULL);

      by_text = XtVaCreateManagedWidget
      ("by_text", xmTextFieldWidgetClass,
       ic[1],
       XmNcolumns, 8,
       NULL);

      label = XtVaCreateManagedWidget
      ("Y",
       xmLabelWidgetClass, ic[1],
       NULL);

      bz_text = XtVaCreateManagedWidget
      ("bz_text", xmTextFieldWidgetClass,
       ic[1],
       XmNcolumns, 8,
       NULL);

      label = XtVaCreateManagedWidget
      ("Z",
       xmLabelWidgetClass, ic[1],
       NULL);

      ic[1]=XtVaCreateManagedWidget
      ("epd_ic1",
       xmRowColumnWidgetClass,ic[0],
       XmNorientation, XmHORIZONTAL,
       NULL);

      label = XtVaCreateManagedWidget
      ("C",
       xmLabelWidgetClass, ic[1],
       NULL);

      cx_text = XtVaCreateManagedWidget
      ("cx_text", xmTextFieldWidgetClass,
       ic[1],
       XmNcolumns, 8,
       NULL);

      label = XtVaCreateManagedWidget
      ("X",
       xmLabelWidgetClass, ic[1],
       NULL);

      cy_text = XtVaCreateManagedWidget
      ("cy_text", xmTextFieldWidgetClass,
       ic[1],
       XmNcolumns, 8,
       NULL);

      label = XtVaCreateManagedWidget
      ("Y",
       xmLabelWidgetClass, ic[1],
       NULL);

      cz_text = XtVaCreateManagedWidget
      ("cz_text", xmTextFieldWidgetClass,
       ic[1],
       XmNcolumns, 8,
       NULL);

      label = XtVaCreateManagedWidget
      ("Z",
       xmLabelWidgetClass, ic[1],
       NULL);

      ref_vec_button = XtVaCreateManagedWidget
      ("Apply", xmPushButtonWidgetClass,
       ic[1],
       NULL);

      ic[1]=XtVaCreateManagedWidget
      ("epd_ic1",
       xmRowColumnWidgetClass,ic[0],
       XmNorientation, XmHORIZONTAL,
       NULL);

      get_atoms_ref = XtVaCreateManagedWidget
      ("Get atoms", xmPushButtonWidgetClass,
       ic[1],
       NULL);

      ic[1]=XtVaCreateManagedWidget
      ("epd_ic1",
       xmRowColumnWidgetClass,ic[0],
       XmNorientation, XmHORIZONTAL,
       NULL);

     discard_ref_button = XtVaCreateManagedWidget
      ("Discard all changes", xmPushButtonWidgetClass,
       ic[1],
       NULL);

     undo_ref_button = XtVaCreateManagedWidget
      ("Undo last change", xmPushButtonWidgetClass,
       ic[1],
       NULL);

      XtAddCallback(get_atoms_ref, XmNactivateCallback, get_atoms_cb, NULL);
      XtAddCallback(discard_ref_button, XmNactivateCallback, discard_ref_button_cb, NULL);
      XtAddCallback(undo_ref_button, XmNactivateCallback, undo_ref_button_cb, NULL);
      XtAddCallback(reflection_dialog, XmNcancelCallback, cancel_ref_cb, NULL);
      XtAddCallback(reflection_dialog, XmNokCallback, ok_ref_cb, NULL);

      XtAddCallback(ref_vec_button, XmNactivateCallback, ref_pt_button_cb, NULL);

      XmStringFree(title);

      XtUnmanageChild
        (XmMessageBoxGetChild(reflection_dialog, XmDIALOG_HELP_BUTTON));
      
    }
  
  XtManageChild(reflection_dialog);
  
}

/* Normalizes a vector */
void normaliza(double *vec, double *norm){
	int modulo;
	modulo = sqrt((pow(vec[0],2)) + (pow(vec[1],2)) + (pow(vec[2],2)));
	norm[0] = vec[0]/modulo;
	norm[1] = vec[1]/modulo;
	norm[2] = vec[2]/modulo;
}

double rotaz(double alpha, double ponto[3]){
	double module,vunit[3],z;
	int i=0,counter=0;
	long double pi = 3.1415926535897932384626433832795028841971693993751058209749;
	
	puts("------------------ROTAZ-------------------\n");
	if(DEBUG) printf("rotazArgumentos: alpha=%f, ponto(%f,%f,%f)\n",alpha,ponto[0],ponto[1], ponto[2]);
	
	module = sqrt((pow(op_vector[0],2)) + (pow(op_vector[1],2)) + (pow(op_vector[2],2)));
	if(DEBUG) printf("module=%f\n",module);

	vunit[0] = op_vector[0];
	vunit[1] = op_vector[1];
	vunit[2] = op_vector[2];
	if(DEBUG) printf("vunit(%f,%f,%f)\n",vunit[0],vunit[1],vunit[2]);

	z = ponto[0]*(((vunit[0])*(vunit[2])*(1-(cos(alpha))))-((vunit[1])*(sin(alpha)))) + ponto[1]*(((vunit[1])*(vunit[2])*(1-(cos(alpha))))+((vunit[0])*(sin(alpha)))) + ponto[2]*((pow(vunit[2],2))+((1-(pow(vunit[2],2)))*cos(alpha)));
	if(DEBUG) printf("z=%f\n",z);

	return(z);
}

/* Reflects atoms */
void reflect_atoms(double *vec1, double *vec2, double ponto[3]){
	double cross_product[3], coseno, alpha, cross_mod, k[3], interno, cross_product2[3];
	double vec[3],newz;
	int i;
	UNDOREF *aux=NULL;

	ref_counter++;
	if(!undoref){
		undoref = (UNDOREF *) malloc(sizeof(UNDOREF));
		undoref->previous = NULL;
		undoref->vec1[0] = vec1[0];
		undoref->vec1[1] = vec1[1];
		undoref->vec1[2] = vec1[2];
		undoref->vec2[0] = vec2[0];
		undoref->vec2[1] = vec2[1];
		undoref->vec2[2] = vec2[2];
		undoref->ponto[0] = ponto[0];
		undoref->ponto[1] = ponto[1];
		undoref->ponto[2] = ponto[2];
	}
	else{
		aux = (UNDOREF *) malloc(sizeof(UNDOREF));
		aux->vec1[0] = vec1[0];
		aux->vec1[1] = vec1[1];
		aux->vec1[2] = vec1[2];
		aux->vec2[0] = vec2[0];
		aux->vec2[1] = vec2[1];
		aux->vec2[2] = vec2[2];
		undoref->ponto[0] = ponto[0];
		undoref->ponto[1] = ponto[1];
		undoref->ponto[2] = ponto[2];
		aux->previous = undoref;
		undoref = aux;
	}
	
	if(DEBUG) puts("------------------REFLECTION-------------------\n");
	if(DEBUG) printf("reflectArgs: vec1(%f,%f,%f) vec2(%f,%f,%f) ponto(%f,%f,%f)\n",vec1[0],vec1[1],vec1[2],vec2[0],vec2[1],vec2[2],ponto[0],ponto[1],ponto[2]);

	k[0]=0.0;
	k[1]=0.0;
	k[2]=1.0;
   
	cross_product[0] = (vec1[1]*vec2[2]) - (vec1[2]*vec2[1]);
	cross_product[1] = (vec1[2]*vec2[0]) - (vec1[0]*vec2[2]);
	cross_product[2] = (vec1[0]*vec2[1]) - (vec1[1]*vec2[0]);
	if(DEBUG) printf("cross_product(%f,%f,%f)\n",cross_product[0],cross_product[1],cross_product[2]);

	cross_mod = sqrt((pow(cross_product[0],2)) + (pow(cross_product[1],2)) + (pow(cross_product[2],2)));
	if(DEBUG) printf("cross_mod=%f\n",cross_mod);
	
	if(cross_mod==0) return;
	
	// normalizar ???
	cross_product[0] = cross_product[0]/cross_mod;
	cross_product[1] = cross_product[1]/cross_mod;
	cross_product[2] = cross_product[2]/cross_mod;
	
	interno = cross_product[2];
	if(DEBUG) printf("interno=%f\n",interno);
	coseno = interno/cross_mod;
	if(DEBUG) printf("coseno=%f\n",coseno);
	alpha = acos(coseno);
	if(DEBUG) printf("alpha=%f\n",alpha);

	cross_product2[0] = cross_product[1];
	cross_product2[1] = cross_product[0]*(-1);
	cross_product2[2] = 0;
	if(DEBUG) printf("cross_product2(%f,%f,%f)\n",cross_product2[0],cross_product2[1],cross_product2[2]);

	op_vector[0] = cross_product2[0];
	op_vector[1] = cross_product2[1];
	op_vector[2] = cross_product2[2];

	if(cross_product2[0]==0 && cross_product2[1]==0){
		op_vector[2]=1;
		alpha=0;
	}
	
	newz=rotaz(alpha,ponto);
	if(DEBUG) printf("newz=%f\n",newz);
	
	vec[0]=0.0;
	vec[1]=0.0;
	vec[2]=-newz;

/* A reflection is achieved with a rotation followed by a translation */
	rot_ate(alpha,0);
	translate(vec);

// falta fazer apenas para a seleccao
	for(i=0;i<no_atoms;i++){
		atoms[i].z*=(-1);
		atoms[i].opz*=(-1);
	}

	vec[2]=newz;
	translate(vec);
	rot_ate(-alpha,0);

  atoms_sorted=0; /* Depths have changed */
  redraw=1;
  deactivate_region();
  clear_message_area();
  canvas_cb(canvas,NULL,NULL);
  update_lengths_dialog(False);
}

/* Parses reflection data and reflects (vector)*/
void ref_vec_button_cb(Widget widget, XtPointer client_data, XtPointer call_data){
	char *ux, *uy, *uz, *vx, *vy, *vz, *cx, *cy, *cz;
	double ax2, ay2, az2, bx2, by2, bz2, cx2, cy2, cz2;
	double vec1[3], vec2[3], ponto[3];

	XtVaGetValues(ux_text, XmNvalue, &ux, NULL);
	ax2 = (ux)?atof(ux):0;
	XtVaGetValues(uy_text, XmNvalue, &uy, NULL);
	ay2 = (uy)?atof(uy):0;
	XtVaGetValues(uz_text, XmNvalue, &uz, NULL);
	az2 = (uz)?atof(uz):0;
	XtVaGetValues(vx_text, XmNvalue, &vx, NULL);
	bx2 = (vx)?atof(vx):0;
	XtVaGetValues(vy_text, XmNvalue, &vy, NULL);
	by2 = (vy)?atof(vy):0;
	XtVaGetValues(vz_text, XmNvalue, &vz, NULL);
	bz2 = (vz)?atof(vz):0;
	XtVaGetValues(cx_text, XmNvalue, &cx, NULL);
	cx2 = (cx)?atof(cx):0;
	XtVaGetValues(cy_text, XmNvalue, &cy, NULL);
	cy2 = (cy)?atof(cy):0;
	XtVaGetValues(cz_text, XmNvalue, &cz, NULL);
	cz2 = (cz)?atof(cz):0;

	vec1[0] = ax2;
	vec1[1] = ay2;
	vec1[2] = az2;
	vec2[0] = bx2;
	vec2[1] = by2;
	vec2[2] = bz2;
	ponto[0] = cx2;
	ponto[1] = cy2;
	ponto[2] = cz2;
	
	reflect_atoms(vec1,vec2, ponto);

	free(ux);
	free(uy);
	free(uz);
	free(vx);
	free(vy);
	free(vz);
	free(cx);
	free(cy);
	free(cz);
}

/* Parses reflection data and reflects (points)*/
void ref_pt_button_cb(Widget widget, XtPointer client_data, XtPointer call_data){
	char *ax, *ay, *az, *bx, *by, *bz, *cx, *cy, *cz;
	double ax2, ay2, az2, bx2, by2, bz2, cx2, cy2, cz2;
	double vec1[3], vec2[3], ponto[3];
	int i;

	XtVaGetValues(ax_text, XmNvalue, &ax, NULL);
	ax2 = (ax)?atof(ax):0;
	XtVaGetValues(ay_text, XmNvalue, &ay, NULL);
	ay2 = (ay)?atof(ay):0;
	XtVaGetValues(az_text, XmNvalue, &az, NULL);
	az2 = (az)?atof(az):0;
	XtVaGetValues(bx_text, XmNvalue, &bx, NULL);
	bx2 = (bx)?atof(bx):0;
	XtVaGetValues(by_text, XmNvalue, &by, NULL);
	by2 = (by)?atof(by):0;
	XtVaGetValues(bz_text, XmNvalue, &bz, NULL);
	bz2 = (bz)?atof(bz):0;
	XtVaGetValues(cx_text, XmNvalue, &cx, NULL);
	cx2 = (cx)?atof(cx):0;
	XtVaGetValues(cy_text, XmNvalue, &cy, NULL);
	cy2 = (cy)?atof(cy):0;
	XtVaGetValues(cz_text, XmNvalue, &cz, NULL);
	cz2 = (cz)?atof(cz):0;

	create_vector_points(bx2, by2, bz2, ax2, ay2, az2);
	for(i=0;i<3;i++) vec1[i]=op_vector[i];

	create_vector_points(bx2, by2, bz2, cx2, cy2, cz2);
	for(i=0;i<3;i++) vec2[i]=op_vector[i];

	ponto[0] = bx2;
	ponto[1] = by2;
	ponto[2] = bz2;

	reflect_atoms(vec1, vec2, ponto);

	free(ax);
	free(ay);
	free(az);
	free(bx);
	free(by);
	free(bz);
	free(cx);
	free(cy);
	free(cz);

}

/* Discards last reflection */
void undo_ref_button_cb(Widget widget, XtPointer client_data, XtPointer call_data){
	UNDOREF *undo_aux=NULL;
	
	if(ref_counter<=0) return;

	ref_counter--;
	reflect_atoms(undoref->vec1, undoref->vec2, undoref->ponto);
	ref_counter--;	

	undo_aux = undoref;
	undoref = undoref->previous;
	free(undo_aux);

  atoms_sorted=0; /* Depths have changed */
  redraw=1;
  deactivate_region();
  clear_message_area();
  canvas_cb(canvas,NULL,NULL);
  update_lengths_dialog(False);
}

/* Discards all reflections */
void discard_ref_button_cb(Widget widget, XtPointer client_data, XtPointer call_data){

	if(ref_counter==0) return;

	while(ref_counter) {
		ref_counter++;
		undo_ref_button_cb(widget, client_data, call_data);
		ref_counter--;
	}
  	ref_counter=0;

  atoms_sorted=0; /* Depths have changed */
  redraw=1;
  deactivate_region();
  clear_message_area();
  canvas_cb(canvas,NULL,NULL);
  update_lengths_dialog(False);

}

/* Exits reflection window and discards all reflections */
void cancel_ref_cb(Widget widget, XtPointer client_data, XtPointer call_data){
	discard_ref_button_cb(widget, client_data, call_data);
}

/* Exits reflection window */
void ok_ref_cb(Widget widget, XtPointer client_data, XtPointer call_data){
	UNDOREF *undo_aux=NULL;

	while(undoref){
		undo_aux = undoref;
		undoref = undoref->previous;
		free(undo_aux);
	}
}

/* Reflects and resets global variables */
void get_atoms_ready_cb(Widget widget, XtPointer client_data, XtPointer call_data){
	int i=0;	
	double point[3];
	
	point[0]=sel_atoms[0].x;
	point[1]=sel_atoms[0].y;
	point[2]=sel_atoms[0].z;
	reflect_atoms(sel_vec1, sel_vec2, point);
	for(;i<3;i++){
		sel_vec1[i]=0;
		sel_vec2[i]=0;
	}
}

/* Parses input data and warns if all atoms will be afected */
void all_atoms_warning(Widget widget, XtPointer client_data, XtPointer call_data){

  char *text2 = "All atoms will be reflected!" ;
	int n,i,counter;	
 static Widget select_dialog = NULL;
 	XmString msg,title;
 
	for(i=0,counter=0;i<no_atoms && !counter;i++){
		if(atoms[i].sel==True){
				sel_atoms[0]=atoms[i];
				counter++;
			}
		}
	
	if(!counter){	

      title = XmStringCreateLocalized("Atoms selection");
      
      n=0;
      XtSetArg(args[n], XmNdialogTitle, title); n++;
      
      msg = XmStringCreateLtoR(text2, XmFONTLIST_DEFAULT_TAG);
      XtSetArg(args[n], XmNmessageString, msg); n++;
  
      select_dialog =
        XmCreateInformationDialog(widget, "select" ,args, n);

      XmStringFree(title);

    XtAddCallback (select_dialog, XmNokCallback, get_atoms_ready_cb, NULL);
    XtAddCallback (select_dialog, XmNcancelCallback, get_atoms_cb, NULL);
    
      XtUnmanageChild
        (XmMessageBoxGetChild(select_dialog, XmDIALOG_HELP_BUTTON));
      
  }else get_atoms_ready_cb(widget,client_data,call_data);
  
  XtManageChild(select_dialog);	 	
}

/* Searches for selected atoms */
void get_selected_atoms(Widget widget, XtPointer client_data, XtPointer call_data){

  char *text2 = "Select the atoms that will suffer the reflection (for all the atoms, select none) and click 'OK'" ;
 	int i=0,counter=0;

 static Widget select_dialog = NULL;

  int n;	

 XmString msg,title;

/* Check if 3 and only 3 atoms were checked */
	for(i=0,counter=0;i<no_atoms;i++){
		if(atoms[i].sel==True){
			if(counter<3){
				sel_atoms[counter]=atoms[i];
				counter++;
			}
			else counter++;
		}
	}

	if(counter!=3){

      title = XmStringCreateLocalized("Atoms selection");
      
      n=0;
      XtSetArg(args[n], XmNdialogTitle, title); n++;
      
      msg = XmStringCreateLtoR("You need to select 3 (and only 3) atoms!", XmFONTLIST_DEFAULT_TAG);
      XtSetArg(args[n], XmNmessageString, msg); n++;
  
      select_dialog =
        XmCreateErrorDialog(widget, "select" ,args, n);

      XmStringFree(title);

    XtAddCallback (select_dialog, XmNokCallback,get_atoms_cb, NULL);

      XtUnmanageChild
        (XmMessageBoxGetChild(select_dialog, XmDIALOG_HELP_BUTTON));
	
	}else{

	create_vector_points(sel_atoms[1].x, sel_atoms[1].y, sel_atoms[1].z, sel_atoms[0].x, sel_atoms[0].y, sel_atoms[0].z);
	for(i=0;i<3;i++) sel_vec1[i]=op_vector[i];

	create_vector_points(sel_atoms[1].x, sel_atoms[1].y, sel_atoms[1].z, sel_atoms[2].x, sel_atoms[2].y, sel_atoms[2].z);
	for(i=0;i<3;i++) sel_vec2[i]=op_vector[i];

  if(select_dialog == NULL)
    {
      title = XmStringCreateLocalized("Atoms selection");
      
      n=0;
      XtSetArg(args[n], XmNdialogTitle, title); n++;
      
      msg = XmStringCreateLtoR(text2, XmFONTLIST_DEFAULT_TAG);
      XtSetArg(args[n], XmNmessageString, msg); n++;
  
      select_dialog =
        XmCreateInformationDialog(widget, "select" ,args, n);

      XmStringFree(title);

    XtAddCallback (select_dialog, XmNokCallback, all_atoms_warning, NULL);

      XtUnmanageChild
        (XmMessageBoxGetChild(select_dialog, XmDIALOG_HELP_BUTTON));
      
    }
      
    }
  
  XtManageChild(select_dialog);
	
}

/* Burocracies */
void get_atoms_cb(Widget widget, XtPointer client_data, XtPointer call_data){
 
  char *text1 = "Select 3 atoms that define the plane and click 'OK'" ;

  static Widget select_dialog = NULL;

  int n;

  XmString msg,title;

  if(select_dialog == NULL)
    {
      title = XmStringCreateLocalized("Atoms selection");
      
      n=0;
      XtSetArg(args[n], XmNdialogTitle, title); n++;
      
      msg = XmStringCreateLtoR(text1, XmFONTLIST_DEFAULT_TAG);
      XtSetArg(args[n], XmNmessageString, msg); n++;
  
      select_dialog =
        XmCreateInformationDialog(widget, "select" ,args, n);

      XmStringFree(title);

    XtAddCallback (select_dialog, XmNokCallback, get_selected_atoms, NULL);

      XtUnmanageChild
        (XmMessageBoxGetChild(select_dialog, XmDIALOG_HELP_BUTTON));
      
    }
  
  XtManageChild(select_dialog);
}

/* Creates inversion window */
void make_inversion_dlg(Widget parent){
  int n;

  XmString title;

  undoi_stack = (UNDO *) malloc(sizeof(UNDO));

  inv_counter=0;

  undoi_stack->previous = NULL;
  undoi_stack->x=0;
  undoi_stack->y=0;
  undoi_stack->z=0;

 if(inversion_dialog == NULL)
    {
      title = XmStringCreateLocalized("Inversion");
      
      n=0;
      XtSetArg(args[n], XmNdialogTitle, title); n++;
  
      inversion_dialog =
        XmCreateMessageDialog(main_w, "Inversion" ,args, n);
  
       ic[0]=XtVaCreateManagedWidget
      ("ic",xmRowColumnWidgetClass,inversion_dialog,NULL);

      ic[1]=XtVaCreateManagedWidget
      ("epd_ic1",
       xmRowColumnWidgetClass,ic[0],
       XmNorientation, XmHORIZONTAL,
       NULL);

      label = XtVaCreateManagedWidget
      ("Point",
       xmLabelWidgetClass, ic[1],
       NULL);

      rax_text = XtVaCreateManagedWidget
      ("rax_text", xmTextFieldWidgetClass,
       ic[1],
       XmNcolumns, 8,
       NULL);

      label = XtVaCreateManagedWidget
      ("X",
       xmLabelWidgetClass, ic[1],
       NULL);

      ray_text = XtVaCreateManagedWidget
      ("ray_text", xmTextFieldWidgetClass,
       ic[1],
       XmNcolumns, 8,
       NULL);

      label = XtVaCreateManagedWidget
      ("Y",
       xmLabelWidgetClass, ic[1],
       NULL);

      raz_text = XtVaCreateManagedWidget
      ("raz_text", xmTextFieldWidgetClass,
       ic[1],
       XmNcolumns, 8,
       NULL);

      label = XtVaCreateManagedWidget
      ("Z",
       xmLabelWidgetClass, ic[1],
       NULL);

      inv_button = XtVaCreateManagedWidget
      ("Apply", xmPushButtonWidgetClass,
       ic[1],
       NULL);

     ic[1]=XtVaCreateManagedWidget
      ("epd_ic1",
       xmRowColumnWidgetClass,ic[0],
       XmNorientation, XmHORIZONTAL,
       NULL);

     discard_inv_button = XtVaCreateManagedWidget
      ("Discard all changes", xmPushButtonWidgetClass,
       ic[1],
       NULL);

     undo_inv_button = XtVaCreateManagedWidget
      ("Undo last change", xmPushButtonWidgetClass,
       ic[1],
       NULL);


      XtAddCallback(inv_button, XmNactivateCallback, inv_button_cb, NULL);
      XtAddCallback(discard_inv_button, XmNactivateCallback, discard_inv_button_cb, NULL);
      XtAddCallback(undo_inv_button, XmNactivateCallback, undo_inv_button_cb, NULL);
      XtAddCallback(inversion_dialog,XmNcancelCallback, cancel_inv_cb, NULL);
      XtAddCallback(inversion_dialog,XmNokCallback, ok_inv_cb, NULL);

      XmStringFree(title);

      XtUnmanageChild
        (XmMessageBoxGetChild(inversion_dialog, XmDIALOG_HELP_BUTTON));

    }
  
  XtManageChild(inversion_dialog);
  
}

/* Inverts atoms */
void invert_atoms(double x, double y, double z){
	int i=0, counter=0;
	
	for(i=0;i<no_atoms;i++){
		if(atoms[i].sel==True){
			counter++;
			atoms[i].x-=x;
			atoms[i].y-=y;
			atoms[i].z-=z;

			atoms[i].opx-=x;
			atoms[i].opy-=y;
			atoms[i].opz-=z;

			atoms[i].x*=(-1);
			atoms[i].y*=(-1);
			atoms[i].z*=(-1);

			atoms[i].opx*=(-1);
			atoms[i].opy*=(-1);
			atoms[i].opz*=(-1);

			atoms[i].x+=x;
			atoms[i].y+=y;
			atoms[i].z+=z;

			atoms[i].opx+=x;
			atoms[i].opy+=y;
			atoms[i].opz+=z;
		}
	}

	if(counter==0){
		for(i=0;i<no_atoms;i++){
			atoms[i].x-=x;
			atoms[i].y-=y;
			atoms[i].z-=z;

			atoms[i].opx-=x;
			atoms[i].opy-=y;
			atoms[i].opz-=z;

			atoms[i].x*=(-1);
			atoms[i].y*=(-1);
			atoms[i].z*=(-1);

			atoms[i].opx*=(-1);
			atoms[i].opy*=(-1);
			atoms[i].opz*=(-1);

			atoms[i].x+=x;
			atoms[i].y+=y;
			atoms[i].z+=z;

			atoms[i].opx+=x;
			atoms[i].opy+=y;
			atoms[i].opz+=z;
		}
	}
}

/* Parses and inverts */
void inv_button_cb(Widget widget, XtPointer client_data, XtPointer call_data){

	char *ax=NULL,*ay=NULL,*az=NULL;
	double x,y,z;
	UNDO *aux=NULL;

	inv_counter++;

	XtVaGetValues(rax_text,XmNvalue,&ax,NULL);
	x = (ax)?atof(ax):0;
	XtVaGetValues(ray_text,XmNvalue,&ay,NULL);
	y = (ay)?atof(ay):0;
	XtVaGetValues(raz_text,XmNvalue,&az,NULL);
	z = (az)?atof(az):0;

	invert_atoms(x,y,z);

	aux = (UNDO *) malloc(sizeof(UNDO));	
	aux->x = undoi_stack->x;
	aux->y = undoi_stack->y;
	aux->z = undoi_stack->z;

	aux->previous = undoi_stack->previous;
	undoi_stack->x = x;
	undoi_stack->y = y;
	undoi_stack->z = z;
	undoi_stack->previous = aux;

  atoms_sorted=0; /* Depths have changed */
  redraw=1;
  deactivate_region();
  clear_message_area();
  canvas_cb(canvas,NULL,NULL);
  update_lengths_dialog(False);
}

/* Discards all inversions made */
void discard_inv_button_cb(Widget widget, XtPointer client_data, XtPointer call_data){
	void undo_inv_button_cb(Widget, XtPointer, XtPointer);
	UNDO *undo_aux=NULL;

	if(inv_counter==0) return;


	while(undoi_stack->previous) undo_inv_button_cb(widget, client_data, call_data);

  inv_counter=0;

  atoms_sorted=0; /* Depths have changed */
  redraw=1;
  deactivate_region();
  clear_message_area();
  canvas_cb(canvas,NULL,NULL);
  update_lengths_dialog(False);
}

/* Discards last inversion made */
void undo_inv_button_cb(Widget widget, XtPointer client_data, XtPointer call_data){

	UNDO *undo_aux=NULL;

	if(inv_counter==0) return;

	if(!undoi_stack->previous){
		invert_atoms(undoi_stack->x, undoi_stack->y, undoi_stack->z);
		undoi_stack->x=0;
		undoi_stack->y=0;
		undoi_stack->z=0;
	}else{
		invert_atoms(undoi_stack->x, undoi_stack->y, undoi_stack->z);
		undo_aux = (UNDO *) malloc(sizeof(UNDO));
		undo_aux = undoi_stack;
		undoi_stack=undoi_stack->previous;
		free(aux);
	}

	inv_counter--;

  atoms_sorted=0; /* Depths have changed */
  redraw=1;
  deactivate_region();
  clear_message_area();
  canvas_cb(canvas,NULL,NULL);
  update_lengths_dialog(False);
}

/* Exits inversion window */
void ok_inv_cb(Widget widget, XtPointer client_data, XtPointer call_data){

	UNDO *undo_aux=NULL;

	if(!undoi_stack->previous){
		undoi_stack->x=0;
		undoi_stack->y=0;
		undoi_stack->z=0;
	}else{
		undo_aux = (UNDO *) malloc(sizeof(UNDO));
		undo_aux = undoi_stack;
		undoi_stack=undoi_stack->previous;
		free(aux);
	}
}

/* Exits and discards allinversions made */
void cancel_inv_cb(Widget widget, XtPointer client_data, XtPointer call_data){
	discard_inv_button_cb(widget, client_data, call_data);
}

/* Treats toggled axis */
void toggled_axis_rot (Widget widget, XtPointer client_data, XtPointer call_data)
{
    int which = (int) client_data;
    XmToggleButtonCallbackStruct *state =
                        (XmToggleButtonCallbackStruct *) call_data;
    if (state->set == XmSET)
        toggle_axis_rot_set = which;
    else
        toggle_axis_rot_set = 0;
}

/* Treats toggled angles */
void toggled_angle_rot(Widget widget, XtPointer client_data, XtPointer call_data)
{
    int which = (int) client_data;
    XmToggleButtonCallbackStruct *state =
                        (XmToggleButtonCallbackStruct *) call_data;
    if (state->set == XmSET)
        toggle_angle_rot_set = which;
    else
        toggle_angle_rot_set = 0;
}

/* Creates rotation window */
void make_rotation_dlg(Widget parent){

  int n;

  XmString title;

  Widget x_axis, y_axis, z_axis, radio_box, radio_box_angle, trinta, quarenta5, sessenta, noventa, centoe80;

  void rot_button_cb(Widget, XtPointer, XtPointer);
  void discard_rot_button_cb(Widget, XtPointer, XtPointer);
  void undo_rot_button_cb(Widget, XtPointer, XtPointer);
  void cancel_rot_cb(Widget, XtPointer, XtPointer);
  void ok_rot_cb(Widget, XtPointer, XtPointer);

  undor_stack = NULL;

 if(rotation_dialog == NULL)
    {
      title = XmStringCreateLocalized("Rotation");
      
      n=0;
      XtSetArg(args[n], XmNdialogTitle, title); n++;
  
      rotation_dialog =
        XmCreateMessageDialog(main_w, "Rotation" ,args, n);

      ic[0]=XtVaCreateManagedWidget
      ("ic",xmRowColumnWidgetClass,rotation_dialog,NULL);

      ic[1]=XtVaCreateManagedWidget
      ("epd_ic1",
       xmRowColumnWidgetClass,ic[0],
       XmNorientation, XmHORIZONTAL,
       NULL);

      label = XtVaCreateManagedWidget
      ("Points",
       xmLabelWidgetClass, ic[1],
       NULL);

      ic[1]=XtVaCreateManagedWidget
      ("epd_ic1",
       xmRowColumnWidgetClass,ic[0],
       XmNorientation, XmHORIZONTAL,
       NULL);

      label = XtVaCreateManagedWidget
      ("A",
       xmLabelWidgetClass, ic[1],
       NULL);

      rax_text = XtVaCreateManagedWidget
      ("rax_text", xmTextFieldWidgetClass,
       ic[1],
       XmNcolumns, 8,
       NULL);

      label = XtVaCreateManagedWidget
      ("X",
       xmLabelWidgetClass, ic[1],
       NULL);

      ray_text = XtVaCreateManagedWidget
      ("ray_text", xmTextFieldWidgetClass,
       ic[1],
       XmNcolumns, 8,
       NULL);

      label = XtVaCreateManagedWidget
      ("Y",
       xmLabelWidgetClass, ic[1],
       NULL);

      raz_text = XtVaCreateManagedWidget
      ("raz_text", xmTextFieldWidgetClass,
       ic[1],
       XmNcolumns, 8,
       NULL);

      label = XtVaCreateManagedWidget
      ("Z",
       xmLabelWidgetClass, ic[1],
       NULL);

      ic[1]=XtVaCreateManagedWidget
      ("epd_ic1",
       xmRowColumnWidgetClass,ic[0],
       XmNorientation, XmHORIZONTAL,
       NULL);

      label = XtVaCreateManagedWidget
      ("B",
       xmLabelWidgetClass, ic[1],
       NULL);

      rbx_text = XtVaCreateManagedWidget
      ("rbx_text", xmTextFieldWidgetClass,
       ic[1],
       XmNcolumns, 8,
       NULL);

      label = XtVaCreateManagedWidget
      ("X",
       xmLabelWidgetClass, ic[1],
       NULL);

      rby_text = XtVaCreateManagedWidget
      ("rby_text", xmTextFieldWidgetClass,
       ic[1],
       XmNcolumns, 8,
       NULL);

      label = XtVaCreateManagedWidget
      ("Y",
       xmLabelWidgetClass, ic[1],
       NULL);

      rbz_text = XtVaCreateManagedWidget
      ("rbz_text", xmTextFieldWidgetClass,
       ic[1],
       XmNcolumns, 8,
       NULL);

      label = XtVaCreateManagedWidget
      ("Z",
       xmLabelWidgetClass, ic[1],
       NULL);

      ic[1]=XtVaCreateManagedWidget
      ("epd_ic1",
       xmRowColumnWidgetClass,ic[0],
       XmNorientation, XmHORIZONTAL,
       NULL);

      label = XtVaCreateManagedWidget
      ("Angle",
       xmLabelWidgetClass, ic[1],
       NULL);

      ralpha_text = XtVaCreateManagedWidget
      ("ralpha_text", xmTextFieldWidgetClass,
       ic[1],
       XmNcolumns, 8,
       NULL);
       
     ic[1]=XtVaCreateManagedWidget
      ("epd_ic1",
       xmRowColumnWidgetClass,ic[0],
       XmNorientation, XmHORIZONTAL,
       NULL);

    XtManageChild (XmCreateSeparator (ic[1], "sep", NULL, 0));

      label = XtVaCreateManagedWidget
      ("Common rotations:",
       xmLabelWidgetClass, ic[1],
       NULL);

      ic[1]=XtVaCreateManagedWidget
      ("epd_ic1",
       xmRowColumnWidgetClass,ic[0],
       XmNorientation, XmHORIZONTAL,
       NULL);

    radio_box = XmCreateRadioBox (ic[1], "radio_box", NULL, 0);

    x_axis = XmCreateToggleButtonGadget (radio_box, "X Axis", NULL, 0);
    XtAddCallback (x_axis, XmNvalueChangedCallback, toggled_axis_rot, (XtPointer) 1);
    XtManageChild (x_axis);

    y_axis = XmCreateToggleButtonGadget (radio_box, "Y Axis", NULL, 0);
    XtAddCallback (y_axis, XmNvalueChangedCallback, toggled_axis_rot, (XtPointer) 2);
    XtManageChild (y_axis);

    z_axis = XmCreateToggleButtonGadget (radio_box, "Z Axis", NULL, 0);
    XtAddCallback (z_axis, XmNvalueChangedCallback, toggled_axis_rot, (XtPointer) 3);
    XtManageChild (z_axis);

    XtManageChild (radio_box);

    radio_box_angle = XmCreateRadioBox (ic[1], "radio_box_angle", NULL, 0);

    trinta = XmCreateToggleButtonGadget (radio_box_angle, "30 Degrees", NULL, 0);
    XtAddCallback (trinta, XmNvalueChangedCallback, toggled_angle_rot, (XtPointer) 30);
    XtManageChild (trinta);

   quarenta5 = XmCreateToggleButtonGadget (radio_box_angle, "45 Degrees", NULL, 0);
    XtAddCallback (quarenta5, XmNvalueChangedCallback, toggled_angle_rot, (XtPointer) 45);
    XtManageChild (quarenta5);
 
  sessenta = XmCreateToggleButtonGadget (radio_box_angle, "60 Degrees", NULL, 0);
    XtAddCallback (sessenta, XmNvalueChangedCallback, toggled_angle_rot, (XtPointer) 60);
    XtManageChild (sessenta);

   noventa = XmCreateToggleButtonGadget (radio_box_angle, "90 Degrees", NULL, 0);
    XtAddCallback (noventa, XmNvalueChangedCallback, toggled_angle_rot, (XtPointer) 90);
    XtManageChild (noventa);

   centoe80 = XmCreateToggleButtonGadget (radio_box_angle, "180 Degrees", NULL, 0);
    XtAddCallback (centoe80, XmNvalueChangedCallback, toggled_angle_rot, (XtPointer) 180);
    XtManageChild (centoe80);

    XtManageChild (radio_box_angle);

     ic[1]=XtVaCreateManagedWidget
      ("epd_ic1",
       xmRowColumnWidgetClass,ic[0],
       XmNorientation, XmHORIZONTAL,
       NULL);

     rot_button = XtVaCreateManagedWidget
      ("Apply", xmPushButtonWidgetClass,
       ic[1],
       NULL);

     ic[1]=XtVaCreateManagedWidget
      ("epd_ic1",
       xmRowColumnWidgetClass,ic[0],
       XmNorientation, XmHORIZONTAL,
       NULL);
     discard_rot_button = XtVaCreateManagedWidget
      ("Discard all changes", xmPushButtonWidgetClass,
       ic[1],
       NULL);

     undo_rot_button = XtVaCreateManagedWidget
      ("Undo last change", xmPushButtonWidgetClass,
       ic[1],
       NULL);

      XtAddCallback(rot_button, XmNactivateCallback, rot_button_cb, NULL);
      XtAddCallback(discard_rot_button, XmNactivateCallback, discard_rot_button_cb, NULL);
      XtAddCallback(undo_rot_button, XmNactivateCallback, undo_rot_button_cb, NULL);
      XtAddCallback(rotation_dialog,XmNcancelCallback, cancel_rot_cb, NULL);
      XtAddCallback(rotation_dialog,XmNokCallback, ok_rot_cb, NULL);
      
      XmStringFree(title);

      XtUnmanageChild
        (XmMessageBoxGetChild(rotation_dialog, XmDIALOG_HELP_BUTTON));

    }
  
  XtManageChild(rotation_dialog);
  
}

/* Creates a vector from two given points */
void create_vector_points(double ax, double ay, double az, double bx, double by, double bz){
	op_vector[0] = bx-ax;
	op_vector[1] = by-ay;
	op_vector[2] = bz-az;
}

/* Rotates atoms */
void rot_ate(double alpha, int degrees){
	double module,vunit[3],x,y,z;
	int i=0,counter=0;
	long double pi = 3.1415926535897932384626433832795028841971693993751058209749;
	
	if(DEBUG)puts("------------------ROTACAO-------------------\n");
	if(DEBUG)printf("Parametros: %f %d\n",alpha,degrees);
	if(DEBUG)printf("op_vector: %f %f %f\n",op_vector[0],op_vector[1],op_vector[2]);
	
	module = sqrt((pow(op_vector[0],2)) + (pow(op_vector[1],2)) + (pow(op_vector[2],2)));
	if(DEBUG)printf("module: %f\n",module);
	
	vunit[0] = op_vector[0]/module;
	vunit[1] = op_vector[1]/module;
	vunit[2] = op_vector[2]/module;
	if(DEBUG)printf("vunit: %f %f %f\n",vunit[0],vunit[1],vunit[2]);

	if(degrees) alpha = alpha*(pi/180);
	if(DEBUG)printf("degrees: %d\n",degrees);
		
	for(i=0;i<no_atoms;i++){
	    if(atoms[i].sel==True){
		x = atoms[i].x*((pow(vunit[0],2))+((1-(pow(vunit[0],2)))*cos(alpha))) + atoms[i].y*(((vunit[0])*(vunit[1])*(1-(cos(alpha))))-((vunit[2])*(sin(alpha)))) + atoms[i].z*(((vunit[0])*(vunit[2])*(1-(cos(alpha))))+((vunit[1])*(sin(alpha))));
		y = atoms[i].x*(((vunit[0])*(vunit[1])*(1-(cos(alpha))))+((vunit[2])*(sin(alpha)))) + atoms[i].y*((pow(vunit[1],2))+((1-(pow(vunit[1],2)))*cos(alpha))) + atoms[i].z*(((vunit[1])*(vunit[2])*(1-(cos(alpha))))-((vunit[0])*(sin(alpha))));
		z = atoms[i].x*(((vunit[0])*(vunit[2])*(1-(cos(alpha))))-((vunit[1])*(sin(alpha)))) + atoms[i].y*(((vunit[1])*(vunit[2])*(1-(cos(alpha))))+((vunit[0])*(sin(alpha)))) + atoms[i].z*((pow(vunit[2],2))+((1-(pow(vunit[2],2)))*cos(alpha)));
		atoms[i].x = x;
		atoms[i].y = y;
		atoms[i].z = z;
		counter++;
	    }
	}

	for(i=0;i<no_atoms;i++){
	    if(atoms[i].sel==True){
		x = atoms[i].opx*((pow(vunit[0],2))+((1-(pow(vunit[0],2)))*cos(alpha))) + atoms[i].opy*(((vunit[0])*(vunit[1])*(1-(cos(alpha))))-((vunit[2])*(sin(alpha)))) + atoms[i].opz*(((vunit[0])*(vunit[2])*(1-(cos(alpha))))+((vunit[1])*(sin(alpha))));
		y = atoms[i].opx*(((vunit[0])*(vunit[1])*(1-(cos(alpha))))+((vunit[2])*(sin(alpha)))) + atoms[i].opy*((pow(vunit[1],2))+((1-(pow(vunit[1],2)))*cos(alpha))) + atoms[i].opz*(((vunit[1])*(vunit[2])*(1-(cos(alpha))))-((vunit[0])*(sin(alpha))));
		z = atoms[i].opx*(((vunit[0])*(vunit[2])*(1-(cos(alpha))))-((vunit[1])*(sin(alpha)))) + atoms[i].opy*(((vunit[1])*(vunit[2])*(1-(cos(alpha))))+((vunit[0])*(sin(alpha)))) + atoms[i].opz*((pow(vunit[2],2))+((1-(pow(vunit[2],2)))*cos(alpha)));
		atoms[i].opx = x;
		atoms[i].opy = y;
		atoms[i].opz = z;
	    }
	}

	if(counter==0){
		for(i=0;i<no_atoms;i++){
			x = atoms[i].x*((pow(vunit[0],2))+((1-(pow(vunit[0],2)))*cos(alpha))) + atoms[i].y*(((vunit[0])*(vunit[1])*(1-(cos(alpha))))-((vunit[2])*(sin(alpha)))) + atoms[i].z*(((vunit[0])*(vunit[2])*(1-(cos(alpha))))+((vunit[1])*(sin(alpha))));
			y = atoms[i].x*(((vunit[0])*(vunit[1])*(1-(cos(alpha))))+((vunit[2])*(sin(alpha)))) + atoms[i].y*((pow(vunit[1],2))+((1-(pow(vunit[1],2)))*cos(alpha))) + atoms[i].z*(((vunit[1])*(vunit[2])*(1-(cos(alpha))))-((vunit[0])*(sin(alpha))));
			z = atoms[i].x*(((vunit[0])*(vunit[2])*(1-(cos(alpha))))-((vunit[1])*(sin(alpha)))) + atoms[i].y*(((vunit[1])*(vunit[2])*(1-(cos(alpha))))+((vunit[0])*(sin(alpha)))) + atoms[i].z*((pow(vunit[2],2))+((1-(pow(vunit[2],2)))*cos(alpha)));
			atoms[i].x = x;
			atoms[i].y = y;
			atoms[i].z = z;
		}

		for(i=0;i<no_atoms;i++){
			x = atoms[i].opx*((pow(vunit[0],2))+((1-(pow(vunit[0],2)))*cos(alpha))) + atoms[i].opy*(((vunit[0])*(vunit[1])*(1-(cos(alpha))))-((vunit[2])*(sin(alpha)))) + atoms[i].opz*(((vunit[0])*(vunit[2])*(1-(cos(alpha))))+((vunit[1])*(sin(alpha))));
			y = atoms[i].opx*(((vunit[0])*(vunit[1])*(1-(cos(alpha))))+((vunit[2])*(sin(alpha)))) + atoms[i].opy*((pow(vunit[1],2))+((1-(pow(vunit[1],2)))*cos(alpha))) + atoms[i].opz*(((vunit[1])*(vunit[2])*(1-(cos(alpha))))-((vunit[0])*(sin(alpha))));
			z = atoms[i].opx*(((vunit[0])*(vunit[2])*(1-(cos(alpha))))-((vunit[1])*(sin(alpha)))) + atoms[i].opy*(((vunit[1])*(vunit[2])*(1-(cos(alpha))))+((vunit[0])*(sin(alpha)))) + atoms[i].opz*((pow(vunit[2],2))+((1-(pow(vunit[2],2)))*cos(alpha)));
			atoms[i].opx = x;
			atoms[i].opy = y;
			atoms[i].opz = z;
		}
	}

  atoms_sorted=0; /* Depths have changed */
  redraw=1;
  deactivate_region();
  clear_message_area();
  canvas_cb(canvas,NULL,NULL);
  update_lengths_dialog(False);
}

/* Parses rotation data and calls rot_ate */
void rot_button_cb(Widget widget, XtPointer client_data, XtPointer call_data){
	
	char *ax=NULL,*ay=NULL,*az=NULL,*bx=NULL,*by=NULL,*bz=NULL,*alpha=NULL;
	double ax2, ay2, az2, bx2, by2, bz2, cx2, cy2, cz2;	
	UNDO *aux;

	XtVaGetValues(rax_text,XmNvalue,&ax,NULL);
	XtVaGetValues(ray_text,XmNvalue,&ay,NULL);
	XtVaGetValues(raz_text,XmNvalue,&az,NULL);
	XtVaGetValues(rbx_text,XmNvalue,&bx,NULL);
	XtVaGetValues(rby_text,XmNvalue,&by,NULL);
	XtVaGetValues(rbz_text,XmNvalue,&bz,NULL);
	XtVaGetValues(ralpha_text,XmNvalue,&alpha,NULL);
	ax2 = (ax)?atof(ax):0;
	ay2 = (ay)?atof(ay):0;
	az2 = (az)?atof(az):0;
	bx2 = (bx)?atof(bx):0;
	by2 = (by)?atof(by):0;
	bz2 = (bz)?atof(bz):0;

	create_vector_points(ax2,ay2,az2,bx2,by2,bz2);

	if(undor_stack){
		aux = (UNDO *) malloc(sizeof(UNDO));	
		aux->x = undor_stack->x;
		aux->y = undor_stack->y;
		aux->z = undor_stack->z;
		aux->alpha = undor_stack->alpha;
		aux->previous = undor_stack->previous;
		undor_stack->x = op_vector[0];
		undor_stack->y = op_vector[1];
		undor_stack->z = op_vector[2];
		undor_stack->alpha = -atof(alpha);
		undor_stack->previous = aux;
	}else{
		undor_stack = (UNDO *) malloc(sizeof(UNDO));
		undor_stack->x = op_vector[0];
		undor_stack->y = op_vector[1];
		undor_stack->z = op_vector[2];
		undor_stack->alpha = -atof(alpha);
		undor_stack->previous = NULL;
	}

	if(!toggle_angle_rot_set && !toggle_axis_rot_set) rot_ate(atof(alpha),1);
	if(!toggle_angle_rot_set && toggle_axis_rot_set) {
		switch(toggle_axis_rot_set){
			case(1): op_vector[0] = 1;
				 op_vector[1] = 0;
				 op_vector[2] = 0;
				undor_stack->x = op_vector[0];
				undor_stack->y = op_vector[1];
				undor_stack->z = op_vector[2];
				 rot_ate(atof(alpha),1);
				 break;
			case(2): op_vector[0] = 0;
				 op_vector[1] = 1;
				 op_vector[2] = 0;
				undor_stack->x = op_vector[0];
				undor_stack->y = op_vector[1];
				undor_stack->z = op_vector[2];
				 rot_ate(atof(alpha),1);
				 break;
			case(3): op_vector[0] = 0;
				 op_vector[1] = 0;
				 op_vector[2] = 1;
				undor_stack->x = op_vector[0];
				undor_stack->y = op_vector[1];
				undor_stack->z = op_vector[2];
				 rot_ate(atof(alpha),1);
				 break;
			default: puts("A very strange error occured");
				 exit(-1);
		}
	}
	if(toggle_angle_rot_set && !toggle_axis_rot_set){
		switch(toggle_angle_rot_set){
			case(30): rot_ate(30,1);
		   		  undor_stack->alpha = -30;
				  break;
			case(45): rot_ate(45,1);
				  undor_stack->alpha = -45;
				  break;
			case(60): rot_ate(60,1);
				  undor_stack->alpha = -60;
				  break;
			case(90): rot_ate(90,1);
				  undor_stack->alpha = -90;
				  break;
			case(180): rot_ate(180,1);
				  undor_stack->alpha = -180;
				   break;
			default: puts("Strange error occured");
				 exit(-1);
		}
	}
	if(toggle_angle_rot_set && toggle_axis_rot_set){
		switch(toggle_axis_rot_set){
			case(1): op_vector[0] = 1;
				 op_vector[1] = 0;
				 op_vector[2] = 0;
				undor_stack->x = op_vector[0];
				undor_stack->y = op_vector[1];
				undor_stack->z = op_vector[2];
				switch(toggle_angle_rot_set){
					case(30): rot_ate(30,1);
				   		  undor_stack->alpha = -30;
						  break;
					case(45): rot_ate(45,1);
						  undor_stack->alpha = -45;
						  break;
					case(60): rot_ate(60,1);
						  undor_stack->alpha = -60;
						  break;
					case(90): rot_ate(90,1);
						  undor_stack->alpha = -90;
						  break;
					case(180): rot_ate(180,1);
						  undor_stack->alpha = -180;
						   break;
					default: puts("Strange error occured");
						 exit(-1);
				}
				break;
			case(2): op_vector[0] = 0;
				 op_vector[1] = 1;
				 op_vector[2] = 0;
				undor_stack->x = op_vector[0];
				undor_stack->y = op_vector[1];
				undor_stack->z = op_vector[2];			
				switch(toggle_angle_rot_set){
					case(30): rot_ate(30,1);
				   		  undor_stack->alpha = -30;
						  break;
					case(45): rot_ate(45,1);
						  undor_stack->alpha = -45;
						  break;
					case(60): rot_ate(60,1);
						  undor_stack->alpha = -60;
						  break;
					case(90): rot_ate(90,1);
						  undor_stack->alpha = -90;
						  break;
					case(180): rot_ate(180,1);
						  undor_stack->alpha = -180;
						   break;
					default: puts("Strange error occured");
						 exit(-1);
				}
						 break;
			case(3): op_vector[0] = 0;
				 op_vector[1] = 0;
				 op_vector[2] = 1;
				undor_stack->x = op_vector[0];
				undor_stack->y = op_vector[1];
				undor_stack->z = op_vector[2];
				switch(toggle_angle_rot_set){
					case(30): rot_ate(30,1);
				   		  undor_stack->alpha = -30;
						  break;
					case(45): rot_ate(45,1);
						  undor_stack->alpha = -45;
						  break;
					case(60): rot_ate(60,1);
						  undor_stack->alpha = -60;
						  break;		
					case(90): rot_ate(90,1);
						  undor_stack->alpha = -90;
						  break;
					case(180): rot_ate(180,1);
						  undor_stack->alpha = -180;
						   break;
					default: puts("Strange error occured");
						 exit(-1);
				}
						break;
			default: puts("Strange error occured");
				 exit(-1);
		}
	}


	free(ax);
	free(ay);
	free(az);
	free(bx);
	free(by);
	free(bz);
	free(alpha);

}
  
/* Discards all changes made since the rotation window was opened */
void discard_rot_button_cb(Widget widget, XtPointer client_data, XtPointer call_data){
	int i=0;
	UNDO *aux=NULL;

	if(!undor_stack) return;
	while(undor_stack){
		op_vector[0]=undor_stack->x;
		op_vector[1]=undor_stack->y;
		op_vector[2]=undor_stack->z;
		rot_ate(undor_stack->alpha,1);

		aux = undor_stack;
		undor_stack=undor_stack->previous;
		free(aux);
	}
	for(i=0;i<3;i++) global_vector[i]=trans_global_vector[i];
  
  redraw=1;
  deactivate_region();
  clear_message_area();
  canvas_cb(canvas,NULL,NULL);
  update_lengths_dialog(False);
}

/* Discards the last rotation made */
void undo_rot_button_cb(Widget widget, XtPointer client_data, XtPointer call_data){
	UNDO *aux=NULL;
	
	if(undor_stack){
		op_vector[0]=undor_stack->x;
		op_vector[1]=undor_stack->y;
		op_vector[2]=undor_stack->z;
		rot_ate(undor_stack->alpha,1);

		aux = undor_stack;
		undor_stack=undor_stack->previous;
		free(aux);
	}
}
void cancel_rot_cb(Widget widget, XtPointer client_data, XtPointer call_data){
	discard_rot_button_cb(widget, client_data, call_data);
}

void ok_rot_cb(Widget widget, XtPointer client_data, XtPointer call_data){
	UNDO *aux=NULL;

	while(undor_stack){
		aux = (UNDO *) malloc(sizeof(UNDO));
		aux = undor_stack;
		undor_stack=undor_stack->previous;
		free(aux);
	}
}

/* Creates the translation window */
void make_translation_dlg(Widget parent){
  int n;

  XmString title;

  Widget x_axis, y_axis, z_axis, radio_box;

  void trans_button_cb(Widget, XtPointer, XtPointer);
  void discard_trans_button_cb(Widget, XtPointer, XtPointer);
  void undo_trans_button_cb(Widget, XtPointer, XtPointer);
  void cancel_trans_cb(Widget, XtPointer, XtPointer);
  void ok_trans_cb(Widget, XtPointer, XtPointer);

  for(n=0;n<3;n++) trans_global_vector[n]=global_vector[n];

  undo_stack = (UNDO *) malloc(sizeof(UNDO));

  undo_stack->previous = NULL;
  undo_stack->x=0;
  undo_stack->y=0;
  undo_stack->z=0;

  if(translation_dialog == NULL)
    {
      title = XmStringCreateLocalized("Translation");
      
      n=0;
      XtSetArg(args[n], XmNdialogTitle, title); n++;

      translation_dialog =
        XmCreateMessageDialog(main_w, "Translation" ,args, n);

      ic[0]=XtVaCreateManagedWidget
      ("ic",xmRowColumnWidgetClass,translation_dialog,NULL);

      ic[1]=XtVaCreateManagedWidget
      ("epd_ic1",
       xmRowColumnWidgetClass,ic[0],
       XmNorientation, XmHORIZONTAL,
       NULL);

      label = XtVaCreateManagedWidget
      ("Coordinates:",
       xmLabelWidgetClass, ic[1],
       NULL);

      ic[1]=XtVaCreateManagedWidget
      ("epd_ic1",
       xmRowColumnWidgetClass,ic[0],
       XmNorientation, XmHORIZONTAL,
       NULL);

      ux_text = XtVaCreateManagedWidget
      ("ux_text", xmTextFieldWidgetClass,
       ic[1],
       XmNcolumns, 8,
       NULL);

      label = XtVaCreateManagedWidget
      ("X",
       xmLabelWidgetClass, ic[1],
       NULL);

      uy_text = XtVaCreateManagedWidget
      ("uy_text", xmTextFieldWidgetClass,
       ic[1],
       XmNcolumns, 8,
       NULL);

      label = XtVaCreateManagedWidget
      ("Y",
       xmLabelWidgetClass, ic[1],
       NULL);

      uz_text = XtVaCreateManagedWidget
      ("uz_text", xmTextFieldWidgetClass,
       ic[1],
       XmNcolumns, 8,
       NULL);

      label = XtVaCreateManagedWidget
      ("Z",
       xmLabelWidgetClass, ic[1],
       NULL);  
	       
	   ic[1]=XtVaCreateManagedWidget
	      ("epd_ic1",
	       xmRowColumnWidgetClass,ic[0],
	       XmNorientation, XmHORIZONTAL,
	       NULL);
	       

     trans_button = XtVaCreateManagedWidget
      ("Apply", xmPushButtonWidgetClass,
       ic[1],
       NULL);

     ic[1]=XtVaCreateManagedWidget
      ("epd_ic1",
       xmRowColumnWidgetClass,ic[0],
       XmNorientation, XmHORIZONTAL,
       NULL);

     discard_trans_button = XtVaCreateManagedWidget
      ("Discard all changes", xmPushButtonWidgetClass,
       ic[1],
       NULL);

     undo_trans_button = XtVaCreateManagedWidget
      ("Undo last change", xmPushButtonWidgetClass,
       ic[1],
       NULL);

      XtAddCallback(trans_button, XmNactivateCallback, trans_button_cb, NULL);
      XtAddCallback(discard_trans_button, XmNactivateCallback, discard_trans_button_cb, NULL);
      XtAddCallback(undo_trans_button, XmNactivateCallback, undo_trans_button_cb, NULL);
      XtAddCallback(translation_dialog,XmNcancelCallback, cancel_trans_cb, NULL);
      XtAddCallback(translation_dialog,XmNokCallback, ok_trans_cb, NULL);

      XmStringFree(title);

      XtUnmanageChild
        (XmMessageBoxGetChild(translation_dialog, XmDIALOG_HELP_BUTTON));
  }

  XtManageChild(translation_dialog);
  
}

/* Parses the translation data and calls for translate */
void trans_button_cb(Widget widget, XtPointer client_data, XtPointer call_data){

	char *ux=NULL, *uy=NULL, *uz=NULL;
	double vec[3];
	void translate(double *);
	UNDO *undo_aux=NULL;

	XtVaGetValues(ux_text, XmNvalue, &ux, NULL);
	vec[0] = (ux)?atof(ux):0;
	XtVaGetValues(uy_text, XmNvalue, &uy, NULL);
	vec[1] = (ux)?atof(uy):0;
	XtVaGetValues(uz_text, XmNvalue, &uz, NULL);
	vec[2] = (ux)?atof(uz):0;
	
	undo_aux = (UNDO *) malloc(sizeof(UNDO));
	undo_aux->previous = undo_stack->previous;
	undo_aux->x = undo_stack->x;
	undo_aux->y = undo_stack->y;
	undo_aux->z = undo_stack->z;

	undo_stack->previous = undo_aux;
	undo_stack->x = 0-vec[0];
	undo_stack->y = 0-vec[1];
	undo_stack->z = 0-vec[2];

	translate(vec);

	free(ux);
	free(uy);
	free(uz);

}

/* Discards all translations made since the translation window was opened */
void discard_trans_button_cb(Widget widget, XtPointer client_data, XtPointer call_data){
	int i=0;
	UNDO *aux=NULL;
	double vec[3];

	if(undo_stack->previous==NULL){
		vec[0]=undo_stack->x;
		vec[1]=undo_stack->y;
		vec[2]=undo_stack->z;
		translate(vec);
	}
	else{
	while(undo_stack->previous){
		vec[0]=undo_stack->x;
		vec[1]=undo_stack->y;
		vec[2]=undo_stack->z;
		translate(vec);

		aux = (UNDO *) malloc(sizeof(UNDO));
		aux = undo_stack;
		undo_stack=undo_stack->previous;
		free(aux);
	}
	vec[0]=undo_stack->x;
	vec[1]=undo_stack->y;
	vec[2]=undo_stack->z;
	translate(vec);
	}

	for(i=0;i<3;i++) global_vector[i]=trans_global_vector[i];

	undo_stack->x=0;
	undo_stack->y=0;
	undo_stack->z=0;
  
  redraw=1;
  deactivate_region();
  clear_message_area();
  canvas_cb(canvas,NULL,NULL);
  update_lengths_dialog(False);
}

/* Discards the last translation made */
void undo_trans_button_cb(Widget widget, XtPointer client_data, XtPointer call_data){
	double vec[3];
	UNDO *aux=NULL;

	if(undo_stack->previous==NULL){
		vec[0]=undo_stack->x;
		vec[1]=undo_stack->y;
		vec[2]=undo_stack->z;
		translate(vec);
		undo_stack->x = 0;
		undo_stack->y = 0;
		undo_stack->z = 0;
	}
	else{
		vec[0]=undo_stack->x;
		vec[1]=undo_stack->y;
		vec[2]=undo_stack->z;
		translate(vec);

		aux = (UNDO *) malloc(sizeof(UNDO));
		aux = undo_stack;
		undo_stack=undo_stack->previous;
		free(aux);
	}
}

/* Exits translation window, discarding all translations that were made */ 
void cancel_trans_cb(Widget widget, XtPointer client_data, XtPointer call_data){
	discard_trans_button_cb(widget, client_data, call_data);
}

/* Exits translation window */
void ok_trans_cb(Widget widget, XtPointer client_data, XtPointer call_data){
	UNDO *aux=NULL;

	if(undo_stack->previous==NULL){	
		undo_stack->x=0;
		undo_stack->y=0;
		undo_stack->z=0;
	}
	else{
	while(undo_stack->previous){
		aux = (UNDO *) malloc(sizeof(UNDO));
		aux = undo_stack;
		undo_stack=undo_stack->previous;
		free(aux);
	}
	undo_stack->x=0;
	undo_stack->y=0;
	undo_stack->z=0;
	}
}

/* Translates atoms with a given vector */
void translate(double *vec){
  void adjust_current_frame(void);
  void update_bbox(void);
  void update_bond_matrix(Boolean);
  int i,all_moved, counter=0;

  all_moved=1;

  for(i=0;i<no_atoms;i++){
	if(atoms[i].sel == True){
	counter++;
      if((atoms[i].edit == 1)){
          atoms[i].x += vec[0];
          atoms[i].y += vec[1];
          atoms[i].z += vec[2];
          atoms[i].opx += vec[0];
          atoms[i].opy += vec[1];
          atoms[i].opz += vec[2];
        }
      else{
          all_moved = 0;
          atoms_sorted = 0; /* Depths have changed */
        }
      }
    }

  if(counter==0){
  for(i=0;i<no_atoms;i++){
      if((atoms[i].edit == 1)){
          atoms[i].x += vec[0];
          atoms[i].y += vec[1];
          atoms[i].z += vec[2];
          atoms[i].opx += vec[0];
          atoms[i].opy += vec[1];
          atoms[i].opz += vec[2];
        }
      else{
          all_moved = 0;
          atoms_sorted = 0; /* Depths have changed */
        }
    }
  }
	

  if (bbox_available){
      for(i=0;i<8;i++){
          file_bbox.v[i][0] += vec[0];
          file_bbox.v[i][1] += vec[1];
          file_bbox.v[i][2] += vec[2];
        }
    }
  if(all_moved){
    for(i=0;i<3;i++){
      global_vector[i]+=vec[i];
    }
  }
  update_bbox ();
  redraw=1;
  if(!all_moved){
    update_bond_matrix(True);
  }
  deactivate_region();
  clear_message_area();
  canvas_cb(canvas,NULL,NULL);
  update_lengths_dialog(False);
}

/* Creates add atom window */
void make_add_atom_dlg(Widget parent){
 int n;

  XmString title;

  void add_button_cb(Widget, XtPointer, XtPointer);
  void discard_add_button_cb(Widget, XtPointer, XtPointer);
  void undo_add_button_cb(Widget, XtPointer, XtPointer);
  void cancel_add_cb(Widget, XtPointer, XtPointer);
  void ok_add_cb(Widget, XtPointer, XtPointer);

  undoa_stack = (UNDO *) malloc(sizeof(UNDO));

  undoa_stack->previous = NULL;
  undoa_stack->x=0;
  undoa_stack->y=0;
  undoa_stack->z=0;
  undoa_stack->alpha=0;
  strncpy(undoa_stack->label,"null", 4);

 if(add_dialog == NULL)
    {
      title = XmStringCreateLocalized("Add new atom");
      
      n=0;
      XtSetArg(args[n], XmNdialogTitle, title); n++;
  
      add_dialog =
        XmCreateMessageDialog(main_w, "Add new atom" ,args, n);

      ic[0]=XtVaCreateManagedWidget
      ("ic",xmRowColumnWidgetClass,add_dialog,NULL);

      ic[1]=XtVaCreateManagedWidget
      ("epd_ic1",
       xmRowColumnWidgetClass,ic[0],
       XmNorientation, XmHORIZONTAL,
       NULL);

      label = XtVaCreateManagedWidget
      ("Type:",
       xmLabelWidgetClass, ic[1],
       NULL);

      rbx_text = XtVaCreateManagedWidget
      ("rbx_text", xmTextFieldWidgetClass,
       ic[1],
       XmNcolumns, 8,
       NULL);

      ic[1]=XtVaCreateManagedWidget
      ("epd_ic1",
       xmRowColumnWidgetClass,ic[0],
       XmNorientation, XmHORIZONTAL,
       NULL);

      label = XtVaCreateManagedWidget
      ("Coords:",
       xmLabelWidgetClass, ic[1],
       NULL);

      rax_text = XtVaCreateManagedWidget
      ("rax_text", xmTextFieldWidgetClass,
       ic[1],
       XmNcolumns, 8,
       NULL);

      label = XtVaCreateManagedWidget
      ("X",
       xmLabelWidgetClass, ic[1],
       NULL);

      ray_text = XtVaCreateManagedWidget
      ("ray_text", xmTextFieldWidgetClass,
       ic[1],
       XmNcolumns, 8,
       NULL);

      label = XtVaCreateManagedWidget
      ("Y",
       xmLabelWidgetClass, ic[1],
       NULL);

      raz_text = XtVaCreateManagedWidget
      ("raz_text", xmTextFieldWidgetClass,
       ic[1],
       XmNcolumns, 8,
       NULL);

      label = XtVaCreateManagedWidget
      ("Z",
       xmLabelWidgetClass, ic[1],
       NULL);

     add_button = XtVaCreateManagedWidget
      ("Add new atom", xmPushButtonWidgetClass,
       ic[1],
       NULL);

     ic[1]=XtVaCreateManagedWidget
      ("epd_ic1",
       xmRowColumnWidgetClass,ic[0],
       XmNorientation, XmHORIZONTAL,
       NULL);

     discard_add_button = XtVaCreateManagedWidget
      ("Discard all changes", xmPushButtonWidgetClass,
       ic[1],
       NULL);

     undo_add_button = XtVaCreateManagedWidget
      ("Undo last change", xmPushButtonWidgetClass,
       ic[1],
       NULL);

      XtAddCallback(add_button, XmNactivateCallback, add_button_cb, NULL);
      XtAddCallback(discard_add_button, XmNactivateCallback, discard_add_button_cb, NULL);
      XtAddCallback(undo_add_button, XmNactivateCallback, undo_add_button_cb, NULL);
      XtAddCallback(add_dialog,XmNcancelCallback, cancel_add_cb, NULL);
      XtAddCallback(add_dialog,XmNokCallback, ok_add_cb, NULL);

      XmStringFree(title);

      XtUnmanageChild
        (XmMessageBoxGetChild(add_dialog, XmDIALOG_HELP_BUTTON));
  }

  XtManageChild(add_dialog);
  
}

/* Adds a new atom to the molecule */
void add_atom(double x, double y, double z, char tipo[4]){

	struct atoms *aux=NULL;
	int new_no_atoms, i;

	new_no_atoms = no_atoms+1;

	aux = (struct atoms *) malloc(sizeof(struct atoms)*new_no_atoms);

	for(i=0;i<no_atoms;i++) aux[i]=atoms[i];

	aux[i].opx = x;
	aux[i].opy = y;
	aux[i].opz = z;
	aux[i].x = x;
	aux[i].y = y;
	aux[i].z = z;
	aux[i].visi = 1;
	strncpy(aux[i].label, tipo, 4);

	free(atoms);

	no_atoms = new_no_atoms;
	atoms = aux;

/* This method was the only one I managed to get it working, it's obviously horrible performance-wise and also pretty stupid, the correct method would be recalling the functions that parse the atoms data from the file. I hope someone will be able to correct this, I couldn't find the proper functions...*/
	write_file3(".temp.xyz");
	open_file(".temp.xyz", False);
}

/* Parses new atom data and calls for add_atom */
void add_button_cb(Widget widget, XtPointer client_data, XtPointer call_data){

	char *ax=NULL, *ay=NULL, *az=NULL, *tipo=NULL;
	int x, y, z,i;
	UNDO *undo_aux=NULL;
	
	if(!atoms) return;

	XtVaGetValues(rax_text, XmNvalue, &ax, NULL);
	x = (ax)?atof(ax):0;
	XtVaGetValues(ray_text, XmNvalue, &ay, NULL);
	y = (ay)?atof(ay):0;
	XtVaGetValues(raz_text, XmNvalue, &az, NULL);
	z = (az)?atof(az):0;
	XtVaGetValues(rbx_text, XmNvalue, &tipo, NULL);

	add_atom(x, y, z, tipo);

	undo_aux = (UNDO *) malloc(sizeof(UNDO));
	undo_aux->previous = undoa_stack->previous;
	undo_aux->x = undoa_stack->x;
	undo_aux->y = undoa_stack->y;
	undo_aux->z = undoa_stack->z;
	strncpy(undo_aux->label, undoa_stack->label, 4);

	undoa_stack->previous = undo_aux;
	undoa_stack->x = x;
	undoa_stack->y = y;
	undoa_stack->z = z;
	strncpy(undoa_stack->label,tipo,4);

  atoms_sorted=0; /* Depths have changed */
  redraw=1;
  deactivate_region();
  clear_message_area();
  canvas_cb(canvas,NULL,NULL);
  update_lengths_dialog(False);

}

/* Exits new atom window */
void ok_add_cb(Widget widget, XtPointer client_data, XtPointer call_data){
	UNDO *aux=NULL;	

	if(undoa_stack->previous==NULL){
		undoa_stack->x = 0;
		undoa_stack->y = 0;
		undoa_stack->z = 0;
		strncpy(undoa_stack->label,"null",4);
	}else{
		while(undoa_stack->previous){
			aux = (UNDO *) malloc(sizeof(UNDO));
			aux = undoa_stack;
			undoa_stack=undoa_stack->previous;
			free(aux);
		}
		undoa_stack->x=0;
		undoa_stack->y=0;
		undoa_stack->z=0;
		strncpy(undoa_stack->label,"null", 4);
	}
}

/* Discards all new atoms that were added */
void discard_add_button_cb(Widget widget, XtPointer client_data, XtPointer call_data){
	UNDO *aux=NULL;

	if(!strcmp(undoa_stack->label,"null")) return;
	if(undoa_stack->previous==NULL){
		delete_atom(undoa_stack->x, undoa_stack->y, undoa_stack->z);
		undoa_stack->x = 0;
		undoa_stack->y = 0;
		undoa_stack->z = 0;
		strncpy(undoa_stack->label,"null",4);
	}else{
		while(undoa_stack->previous){
			delete_atom(undoa_stack->x, undoa_stack->y, undoa_stack->z);
			aux = (UNDO *) malloc(sizeof(UNDO));
			aux = undoa_stack;
			undoa_stack=undoa_stack->previous;
			free(aux);
		}
	}

  atoms_sorted=0; /* Depths have changed */
  redraw=1;
  deactivate_region();
  clear_message_area();
  canvas_cb(canvas,NULL,NULL);
  update_lengths_dialog(False);
}

/* Discards the last new atom added */
void undo_add_button_cb(Widget widget, XtPointer client_data, XtPointer call_data){

	UNDO *aux=NULL;

	if(!strcmp(undoa_stack->label,"null")) return;
	if(undoa_stack->previous==NULL){
		delete_atom(undoa_stack->x, undoa_stack->y, undoa_stack->z);

		undoa_stack->x = 0;
		undoa_stack->y = 0;
		undoa_stack->z = 0;
		strncpy(undoa_stack->label,"null",4);
	}
	else{
		delete_atom(undoa_stack->x, undoa_stack->y, undoa_stack->z);

		aux = (UNDO *) malloc(sizeof(UNDO));
		aux = undoa_stack;
		undoa_stack=undoa_stack->previous;
		free(aux);
	}

  atoms_sorted=0; /* Depths have changed */
  redraw=1;
  deactivate_region();
  clear_message_area();
  canvas_cb(canvas,NULL,NULL);
  update_lengths_dialog(False);
}

/* Exits new atom window, discarding all new atoms that were added */
void cancel_add_cb(Widget widget, XtPointer client_data, XtPointer call_data){
	discard_add_button_cb(widget,client_data,call_data);
}

/* Creates delete atom window */
void make_delete_dlg(Widget parent){
  int n;

  XmString title;

  void delete_button_cb(Widget, XtPointer, XtPointer);
  void discard_delete_button_cb(Widget, XtPointer, XtPointer);
  void undo_delete_button_cb(Widget, XtPointer, XtPointer);
  void cancel_delete_cb(Widget, XtPointer, XtPointer);
  void ok_delete_cb(Widget, XtPointer, XtPointer);

  undod_stack = (UNDO *) malloc(sizeof(UNDO));

  undod_stack->previous = NULL;
  undod_stack->x=0;
  undod_stack->y=0;
  undod_stack->z=0;
  undod_stack->alpha=0;
  strncpy(undod_stack->label,"null",4);

 if(delete_dialog == NULL)
    {
      title = XmStringCreateLocalized("Delete selected atoms");
      
      n=0;
      XtSetArg(args[n], XmNdialogTitle, title); n++;
  
      delete_dialog =
        XmCreateMessageDialog(main_w, "Delete selected atoms" ,args, n);

      ic[0]=XtVaCreateManagedWidget
      ("ic",xmRowColumnWidgetClass,delete_dialog,NULL);

      ic[1]=XtVaCreateManagedWidget
      ("epd_ic1",
       xmRowColumnWidgetClass,ic[0],
       XmNorientation, XmHORIZONTAL,
       NULL);

     delete_button = XtVaCreateManagedWidget
      ("Delete atom(s)", xmPushButtonWidgetClass,
       ic[1],
       NULL);

     ic[1]=XtVaCreateManagedWidget
      ("epd_ic1",
       xmRowColumnWidgetClass,ic[0],
       XmNorientation, XmHORIZONTAL,
       NULL);

     discard_delete_button = XtVaCreateManagedWidget
      ("Discard all changes", xmPushButtonWidgetClass,
       ic[1],
       NULL);

     undo_delete_button = XtVaCreateManagedWidget
      ("Undo last change", xmPushButtonWidgetClass,
       ic[1],
       NULL);

      XtAddCallback(delete_button, XmNactivateCallback, delete_button_cb, NULL);
      XtAddCallback(discard_delete_button, XmNactivateCallback, discard_delete_button_cb, NULL);
      XtAddCallback(undo_delete_button, XmNactivateCallback, undo_delete_button_cb, NULL);
      XtAddCallback(delete_dialog,XmNcancelCallback, cancel_delete_cb, NULL);
      XtAddCallback(delete_dialog,XmNokCallback, ok_delete_cb, NULL);

      XmStringFree(title);

      XtUnmanageChild
        (XmMessageBoxGetChild(delete_dialog, XmDIALOG_HELP_BUTTON));
  }

  XtManageChild(delete_dialog);
  
}

/* Deletes an atom */
void delete_atom(double x, double y, double z){
	struct atoms *aux;
	int i;

	aux = (struct atoms *) malloc(sizeof(struct atoms)*(no_atoms-1));

	for(i=0;i<no_atoms;i++){
		if(atoms[i].x==x && atoms[i].y==y && atoms[i].z==z){
			no_atoms--;
			while(i<no_atoms){
				atoms[i]=atoms[i+1];
				i++;
			}
		break;
		}
	}

	for(i=0;i<no_atoms;i++) aux[i]=atoms[i];
	free(atoms);
	atoms = aux;

	write_file3(".temp.xyz");
	open_file(".temp.xyz",False);

  atoms_sorted=0; /* Depths have changed */
  redraw=1;
  deactivate_region();
  clear_message_area();
  canvas_cb(canvas,NULL,NULL);
  update_lengths_dialog(False);
}	

/* Parses and delete atom */
void delete_button_cb(Widget widget, XtPointer client_data, XtPointer call_data){

	int new_no_atoms, j, i, counter=0;
	struct atoms *aux;
	UNDO *undo_aux=NULL;

	if(!atoms) return;

	for(i=0;i<no_atoms;i++) if(atoms[i].sel==True) counter++;

	new_no_atoms = no_atoms - counter;
	
	aux = (struct atoms *) malloc(sizeof(struct atoms)*new_no_atoms);

	for(i=0,j=0;i<no_atoms;i++){
		if(atoms[i].sel==False){
			aux[j] = atoms[i];
			j++;
		}else{

	undo_aux = (UNDO *) malloc(sizeof(UNDO));
	undo_aux->previous = undod_stack->previous;
	undo_aux->x = undod_stack->x;
	undo_aux->y = undod_stack->y;
	undo_aux->z = undod_stack->z;
	strncpy(undo_aux->label, undod_stack->label,4);

	undod_stack->previous = undo_aux;
	undod_stack->x = atoms[i].x;
	undod_stack->y = atoms[i].y;
	undod_stack->z = atoms[i].z;
	strncpy(undod_stack->label, atoms[i].label,4);
		}
	}

	free(atoms);
	atoms = aux;

/* Check my comment for this section in add_atom */
	write_file3(".temp.xyz");
	open_file(".temp.xyz",False);

  atoms_sorted=0; /* Depths have changed */
  redraw=1;
  deactivate_region();
  clear_message_area();
  canvas_cb(canvas,NULL,NULL);
  update_lengths_dialog(False);

}

/* Exits delete atom window */
void ok_delete_cb(Widget widget, XtPointer client_data, XtPointer call_data){
	UNDO *aux=NULL;	

	if(undod_stack->previous==NULL){
		undod_stack->x = 0;
		undod_stack->y = 0;
		undod_stack->z = 0;
		strncpy(undod_stack->label,"null",4);
	}else{
		while(undod_stack->previous){
			aux = (UNDO *) malloc(sizeof(UNDO));
			aux = undod_stack;
			undod_stack=undod_stack->previous;
			free(aux);
		}
		undod_stack->x=0;
		undod_stack->y=0;
		undod_stack->z=0;
		strncpy(undod_stack->label,"null",4);
	}
}

/* Discards all atoms that were deleted (adds them back) */
void discard_delete_button_cb(Widget widget, XtPointer client_data, XtPointer call_data){
	UNDO *aux=NULL;

	if(!strcmp(undod_stack->label,"null")) return;
	if(undod_stack->previous==NULL){
		add_atom(undod_stack->x, undod_stack->y, undod_stack->z, undod_stack->label);
		undod_stack->x = 0;
		undod_stack->y = 0;
		undod_stack->z = 0;
		strncpy(undod_stack->label,"null",4);
	}else{
		while(undod_stack->previous){
			add_atom(undod_stack->x, undod_stack->y, undod_stack->z, undod_stack->label);
			aux = (UNDO *) malloc(sizeof(UNDO));
			aux = undod_stack;
			undod_stack=undod_stack->previous;
			free(aux);
		}
	}

  atoms_sorted=0; /* Depths have changed */
  redraw=1;
  deactivate_region();
  clear_message_area();
  canvas_cb(canvas,NULL,NULL);
  update_lengths_dialog(False);
}

/* Discard the last atom deletion */
void undo_delete_button_cb(Widget widget, XtPointer client_data, XtPointer call_data){

	UNDO *aux=NULL;

	if(!strcmp(undod_stack->label,"null")) return;
	if(undod_stack->previous==NULL){
		add_atom(undod_stack->x, undod_stack->y, undod_stack->z, undod_stack->label);

		undod_stack->x = 0;
		undod_stack->y = 0;
		undod_stack->z = 0;
		strncpy(undod_stack->label,"null",4);
	}
	else{
		add_atom(undod_stack->x, undod_stack->y, undod_stack->z, undod_stack->label);

		aux = (UNDO *) malloc(sizeof(UNDO));
		aux = undod_stack;
		undod_stack=undod_stack->previous;
		free(aux);
	}

  atoms_sorted=0; /* Depths have changed */
  redraw=1;
  deactivate_region();
  clear_message_area();
  canvas_cb(canvas,NULL,NULL);
  update_lengths_dialog(False);
}

/* Exits delete atom window and discards all atoms deletion */
void cancel_delete_cb(Widget widget, XtPointer client_data, XtPointer call_data){
	discard_delete_button_cb(widget, client_data, call_data);
} 
