/*$Id: f90_win32.c,v 1.3 2000/07/18 19:48:04 balay Exp balay $*/

#include "petscf90.h"
#include "src/sys/src/f90/f90_win32.h"

#if defined(PETSC_HAVE_WIN32F90)

#undef __FUNC__  
#define __FUNC__ /*<a name=""></a>*/"F90GetID"
int F90GetID(PetscDataType type,int *id)
{
  PetscFunctionBegin;
  if (type == PETSC_INT) {
    *id = F90_INT_ID;
  } else if (type == PETSC_DOUBLE) {
    *id = F90_DOUBLE_ID;
#if defined(PETSC_USE_COMPLEX)
  } else if (type == PETSC_COMPLEX) {
    *id = F90_COMPLEX_ID;
#endif
  } else if (type == PETSC_LONG) {
    *id = F90_INT_ID;
  } else if (type == PETSC_CHAR) {
    *id = F90_CHAR_ID;
  } else {
    SETERRQ(PETSC_ERR_ARG_OUTOFRANGE,1,"Unknown PETSc datatype");
  }
  PetscFunctionReturn(0);
}

#undef __FUNC__  
#define __FUNC__ /*<a name=""></a>*/"F90Array1dCreate"
int F90Array1dCreate(void *array,PetscDataType type,int start,int len,F90Array1d ptr)
{
  int size,ierr,id;

  PetscFunctionBegin;
  PetscValidPointer(array);
  PetscValidPointer(ptr);  
  ierr               = PetscDataTypeGetSize(type,&size);CHKERRQ(ierr);
  ierr               = F90GetID(type,&id);
  ptr->addr          = array;
  ptr->id            = id;
  ptr->sd            = size;
  ptr->ndim          = 1;
  ptr->dim[0].extent = len;
  ptr->dim[0].mult   = size;
  ptr->dim[0].lower  = start;
  ptr->sum_d         = -(ptr->dim[0].lower*ptr->dim[0].mult);

  PetscFunctionReturn(0);
}

#undef __FUNC__  
#define __FUNC__ /*<a name=""></a>*/"F90Array2dCreate"
int F90Array2dCreate(void *array,PetscDataType type,int start1,int len1,int start2,int len2,F90Array2d ptr)
{
  int size,ierr,id;

  PetscFunctionBegin;
  PetscValidPointer(array);
  PetscValidPointer(ptr);
  ierr               = PetscDataTypeGetSize(type,&size);CHKERRQ(ierr);
  ierr               = F90GetID(type,&id);
  ptr->addr          = array;
  ptr->id            = id;
  ptr->sd            = size;
  ptr->ndim          = 2;
  ptr->dim[1].extent = len1;
  ptr->dim[1].mult   = size;
  ptr->dim[1].lower  = start1;
  ptr->dim[0].extent = len2;
  ptr->dim[0].mult   = len1*size;
  ptr->dim[0].lower  = start2;
  ptr->sum_d         = -(ptr->dim[0].lower*ptr->dim[0].mult+ptr->dim[1].lower*ptr->dim[1].mult);

  PetscFunctionReturn(0);
}

#include "src/sys/src/f90/f90_common.c"

#else
/*
     Dummy function so that compilers won't complain about 
  empty files.
*/
int F90_win32_Dummy(int dummy)
{
  return 0;
}

#endif
