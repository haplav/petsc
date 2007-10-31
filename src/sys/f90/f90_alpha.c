
/*-------------------------------------------------------------*/

#undef __FUNCT__  
#define __FUNCT__ "F90GetID"
PetscErrorCode F90GetID(PetscDataType type,PetscInt *id)
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
    *id = F90_LONG_ID;   
  } else if (type == PETSC_CHAR) {
    *id = F90_CHAR_ID;
  } else {
    SETERRQ(PETSC_ERR_ARG_OUTOFRANGE,"Unknown PETSc datatype");
  }
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "F90Array1dCreate"
PetscErrorCode F90Array1dCreate(void *array,PetscDataType type,PetscInt start,PetscInt len,F90Array1d *ptr)
{
  size_t size;
  PetscInt id;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  PetscValidPointer(array,1);
  PetscValidPointer(ptr,5);  
  ierr               = PetscDataTypeGetSize(type,&size);CHKERRQ(ierr);
  ierr               = F90GetID(type,&id);
  ptr->addr          = array;
  ptr->id            = (char)id;
  ptr->a             = A_VAL;
  ptr->b             = B_VAL;
  ptr->sd            = size;
  ptr->ndim          = 1;
  ptr->dim[0].upper  = len+start;
  ptr->dim[0].mult   = size;
  ptr->dim[0].lower  = start;
  ptr->addr_d         =  (void*)((long)array - (ptr->dim[0].lower*ptr->dim[0].mult));

  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "F90Array2dCreate"
PetscErrorCode F90Array2dCreate(void *array,PetscDataType type,PetscInt start1,PetscInt len1,PetscInt start2,PetscInt len2,F90Array2d *ptr)
{
  size_t size;
  PetscInt id;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  PetscValidPointer(array,1);
  PetscValidPointer(ptr,7);  
  ierr               = PetscDataTypeGetSize(type,&size);CHKERRQ(ierr);
  ierr               = F90GetID(type,&id);
  ptr->addr          = array;
  ptr->id            = (char)id;
  ptr->a             = A_VAL;
  ptr->b             = B_VAL;
  ptr->sd            = size;
  ptr->ndim          = 2;
  ptr->dim[1].upper  = len1+start1;
  ptr->dim[1].mult   = size;
  ptr->dim[1].lower  = start1;
  ptr->dim[0].upper  = len2+start2;
  ptr->dim[0].mult   = len1*size;
  ptr->dim[0].lower  = start2;
  ptr->addr_d        = (void*)((long)array -(ptr->dim[0].lower*ptr->dim[0].mult+
                                             ptr->dim[1].lower*ptr->dim[1].mult));
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "F90Array3dCreate"
PetscErrorCode F90Array3dCreate(void *array,PetscDataType type,PetscInt start1,PetscInt len1,PetscInt start2,PetscInt len2,PetscInt start3,PetscInt len3,F90Array3d *ptr)
{
  size_t size;
  PetscInt id;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  PetscValidPointer(array,1);
  PetscValidPointer(ptr,9);  
  ierr               = PetscDataTypeGetSize(type,&size);CHKERRQ(ierr);
  ierr               = F90GetID(type,&id);
  ptr->addr          = array;
  ptr->id            = (char)id;
  ptr->a             = A_VAL;
  ptr->b             = B_VAL;
  ptr->sd            = size;
  ptr->ndim          = 3;
  ptr->dim[2].upper  = len1+start1;
  ptr->dim[2].mult   = size;
  ptr->dim[2].lower  = start1;
  ptr->dim[1].upper  = len2+start2;
  ptr->dim[1].mult   = len1*size;
  ptr->dim[1].lower  = start2;
  ptr->dim[0].upper  = len3+start3;
  ptr->dim[0].mult   = len2*len1*size;
  ptr->dim[0].lower  = start3;
  ptr->addr_d        = (void*)((long)array -(ptr->dim[0].lower*ptr->dim[0].mult+
                                             ptr->dim[1].lower*ptr->dim[1].mult+
                                             ptr->dim[2].lower*ptr->dim[2].mult));
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "F90Array4dCreate"
PetscErrorCode F90Array4dCreate(void *array,PetscDataType type,PetscInt start1,PetscInt len1,PetscInt start2,PetscInt len2,PetscInt start3,PetscInt len3,PetscInt start4,PetscInt len4,F90Array4d *ptr)
{
  size_t size;
  PetscInt id;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  PetscValidPointer(array,1);
  PetscValidPointer(ptr,11);  
  ierr               = PetscDataTypeGetSize(type,&size);CHKERRQ(ierr);
  ierr               = F90GetID(type,&id);
  ptr->addr          = array;
  ptr->id            = (char)id;
  ptr->a             = A_VAL;
  ptr->b             = B_VAL;
  ptr->sd            = size;
  ptr->ndim          = 4;
  ptr->dim[3].upper  = len1+start1;
  ptr->dim[3].mult   = size;
  ptr->dim[3].lower  = start1;
  ptr->dim[2].upper  = len2+start2;
  ptr->dim[2].mult   = len1*size;
  ptr->dim[2].lower  = start2;
  ptr->dim[1].upper  = len3+start3;
  ptr->dim[1].mult   = len2*len1*size;
  ptr->dim[1].lower  = start4;
  ptr->dim[0].upper  = len4+start4;
  ptr->dim[0].mult   = len3*len2*len1*size;
  ptr->dim[0].lower  = start4;
  ptr->addr_d        = (void*)((long)array -(ptr->dim[0].lower*ptr->dim[0].mult+
                                             ptr->dim[1].lower*ptr->dim[1].mult+
                                             ptr->dim[2].lower*ptr->dim[2].mult+
                                             ptr->dim[3].lower*ptr->dim[3].mult));
  PetscFunctionReturn(0);
}
/*-------------------------------------------------------------*/

