/*$Id: f90_common.c,v 1.1 2000/08/29 14:20:11 balay Exp balay $*/

/*-------------------------------------------------------------*/
#undef __FUNC__  
#define __FUNC__ /*<a name=""></a>*/"F90Array1dAccess"
int F90Array1dAccess(F90Array1d ptr,void **array)
{
  PetscFunctionBegin;
  PetscValidPointer(array);
  PetscValidPointer(ptr);
  *array = ptr->addr;
  PetscFunctionReturn(0);
}

#undef __FUNC__  
#define __FUNC__ /*<a name=""></a>*/"F90Array1dDestroy"
int F90Array1dDestroy(F90Array1d ptr)
{
  PetscFunctionBegin;
  PetscValidPointer(ptr);
  ptr->addr = (void *)0;
  PetscFunctionReturn(0);
}
#undef __FUNC__  
#define __FUNC__ /*<a name=""></a>*/"F90Array1dGetNextRecord"
int F90Array1dGetNextRecord(F90Array1d ptr,void **next)
{
  PetscFunctionBegin;
  PetscValidPointer(ptr);
  *next = (void*)(ptr + 1);
  PetscFunctionReturn(0);
}

/*-------------------------------------------------------------*/

#undef __FUNC__  
#define __FUNC__ /*<a name=""></a>*/"F90Array2dAccess"
int F90Array2dAccess(F90Array2d ptr,void **array)
{
  PetscFunctionBegin;
  PetscValidPointer(array);
  PetscValidPointer(ptr);
  *array = ptr->addr;
  PetscFunctionReturn(0);
}

#undef __FUNC__  
#define __FUNC__ /*<a name=""></a>*/"F90Array2dDestroy"
int F90Array2dDestroy(F90Array2d ptr)
{
  PetscFunctionBegin;
  PetscValidPointer(ptr);
  ptr->addr = (void *)0;
  PetscFunctionReturn(0);
}
#undef __FUNC__  
#define __FUNC__ /*<a name=""></a>*/"F90Array1dGetNextRecord"
int F90Array2dGetNextRecord(F90Array2d ptr,void **next)
{
  PetscFunctionBegin;
  PetscValidPointer(ptr);
  *next = (void*)(ptr + 1);
  PetscFunctionReturn(0);
}
/*-------------------------------------------------------------*/
