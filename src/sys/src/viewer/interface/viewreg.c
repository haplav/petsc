/*$Id: viewreg.c,v 1.31 2000/09/22 20:41:53 bsmith Exp bsmith $*/

#include "src/sys/src/viewer/viewerimpl.h"  /*I "petsc.h" I*/  

FList ViewerList              = 0;

#undef __FUNC__  
#define __FUNC__ /*<a name="ViewerCreate"></a>*/"ViewerCreate" 
/*@C
   ViewerCreate - Creates a viewing context

   Collective on MPI_Comm

   Input Parameter:
.  comm - MPI communicator

   Output Parameter:
.  inviewer - location to put the Viewer context

   Level: advanced

   Concepts: graphics^creating Viewer
   Concepts: file input/output^creating Viewer
   Concepts: sockets^creating Viewer

.seealso: ViewerDestroy(), ViewerSetType()

@*/
int ViewerCreate(MPI_Comm comm,Viewer *inviewer)
{
  Viewer viewer;

  PetscFunctionBegin;
  *inviewer = 0;
  PetscHeaderCreate(viewer,_p_Viewer,struct _ViewerOps,VIEWER_COOKIE,-1,"Viewer",comm,ViewerDestroy,0);
  PLogObjectCreate(viewer);
  *inviewer           = viewer;
  viewer->type        = -1;
  viewer->data        = 0;
  PetscFunctionReturn(0);
}
 
#undef __FUNC__  
#define __FUNC__ /*<a name="ViewerSetType"></a>*/"ViewerSetType" 
/*@C
   ViewerSetType - Builds viewer for a particular implementation.

   Collective on Viewer

   Input Parameter:
+  viewer      - the viewer context
-  type        - for example, "ASCII"

   Options Database Command:
.  -draw_type  <type> - Sets the type; use -help for a list 
    of available methods (for instance, ascii)

   Level: advanced

   Notes:  
   See "include/petscviewer.h" for available methods (for instance,
   SOCKET_VIEWER)

.seealso: ViewerCreate(), ViewerGetType()
@*/
int ViewerSetType(Viewer viewer,ViewerType type)
{
  int        ierr,(*r)(Viewer);
  PetscTruth match;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(viewer,VIEWER_COOKIE);
  PetscValidCharPointer(type);

  ierr = PetscTypeCompare((PetscObject)viewer,type,&match);CHKERRQ(ierr);
  if (match) PetscFunctionReturn(0);

  if (viewer->data) {
    /* destroy the old private Viewer context */
    ierr = (*viewer->ops->destroy)(viewer);CHKERRQ(ierr);
    viewer->data      = 0;
  }
  /* Get the function pointers for the graphics method requested */
  if (!ViewerList) SETERRQ(1,"No viewer implementations registered");

  ierr =  FListFind(viewer->comm,ViewerList,type,(int (**)(void *)) &r);CHKERRQ(ierr);

  if (!r) SETERRQ1(1,"Unknown Viewer type given: %s",type);

  viewer->data        = 0;
  ierr = PetscMemzero(viewer->ops,sizeof(struct _ViewerOps));CHKERRQ(ierr);
  ierr = (*r)(viewer);CHKERRQ(ierr);

  ierr = PetscObjectChangeTypeName((PetscObject)viewer,type);CHKERRQ(ierr);

  PetscFunctionReturn(0);
}

#undef __FUNC__  
#define __FUNC__ /*<a name="ViewerRegisterDestroy"></a>*/"ViewerRegisterDestroy" 
/*@C
   ViewerRegisterDestroy - Frees the list of Viewer methods that were
   registered by ViewerRegisterDynamic().

   Not Collective

   Level: developer

.seealso: ViewerRegisterDynamic(), ViewerRegisterAll()
@*/
int ViewerRegisterDestroy(void)
{
  int ierr;

  PetscFunctionBegin;
  if (ViewerList) {
    ierr = FListDestroy(&ViewerList);CHKERRQ(ierr);
    ViewerList = 0;
  }
  PetscFunctionReturn(0);
}

/*MC
   ViewerRegisterDynamic - Adds a method to the Krylov subspace solver package.

   Synopsis:
   ViewerRegisterDynamic(char *name_solver,char *path,char *name_create,int (*routine_create)(Viewer))

   Not Collective

   Input Parameters:
+  name_solver - name of a new user-defined solver
.  path - path (either absolute or relative) the library containing this solver
.  name_create - name of routine to create method context
-  routine_create - routine to create method context

   Level: developer

   Notes:
   ViewerRegisterDynamic() may be called multiple times to add several user-defined solvers.

   If dynamic libraries are used, then the fourth input argument (routine_create)
   is ignored.

   Sample usage:
.vb
   ViewerRegisterDynamic("my_viewer_type",/home/username/my_lib/lib/libO/solaris/mylib.a,
               "MyViewerCreate",MyViewerCreate);
.ve

   Then, your solver can be chosen with the procedural interface via
$     ViewerSetType(ksp,"my_viewer_type")
   or at runtime via the option
$     -viewer_type my_viewer_type

  Concepts: registering^Viewers

.seealso: ViewerRegisterAll(), ViewerRegisterDestroy()
M*/

#undef __FUNC__  
#define __FUNC__ /*<a name="ViewerRegister"></a>*/"ViewerRegister" 
int ViewerRegister(char *sname,char *path,char *name,int (*function)(Viewer))
{
  int ierr;
  char fullname[256];

  PetscFunctionBegin;
  ierr = FListConcat(path,name,fullname);CHKERRQ(ierr);
  ierr = FListAdd(&ViewerList,sname,fullname,(int (*)(void*))function);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNC__  
#define __FUNC__ /*<a name="ViewerSetFromOptions"></a>*/"ViewerSetFromOptions" 
/*@C
   ViewerSetFromOptions - Sets the graphics type from the options database.
      Defaults to a PETSc X windows graphics.

   Collective on Viewer

   Input Parameter:
.     viewer - the graphics context

   Level: intermediate

   Notes: 
    Must be called after ViewerCreate() before the Viewer is used.

  Concepts: Viewer^setting options

.seealso: ViewerCreate(), ViewerSetType()

@*/
int ViewerSetFromOptions(Viewer viewer)
{
  int        ierr;
  char       vtype[256];
  PetscTruth flg;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(viewer,VIEWER_COOKIE);

  if (!ViewerList) SETERRQ(1,"No viewer implementations registered");
  ierr = OptionsBegin(viewer->comm,viewer->prefix,"Viewer options","Viewer");CHKERRQ(ierr);
    ierr = OptionsList("-viewer_type","Type of viewer","None",ViewerList,(char *)(viewer->type_name?viewer->type_name:ASCII_VIEWER),vtype,256,&flg);CHKERRQ(ierr);
    if (flg) {
      ierr = ViewerSetType(viewer,vtype);CHKERRQ(ierr);
    }
    /* type has not been set? */
    if (!viewer->type_name) {
      ierr = ViewerSetType(viewer,ASCII_VIEWER);CHKERRQ(ierr);
    }
    if (viewer->ops->setfromoptions) {
      ierr = (*viewer->ops->setfromoptions)(viewer);CHKERRQ(ierr);
    }
  ierr = OptionsEnd();CHKERRQ(ierr);

  PetscFunctionReturn(0);
}
