/*$Id: zf90ksp.c,v 1.1 2000/09/28 14:59:15 bsmith Exp bsmith $*/

#include "petscksp.h"
#include "petscf90.h"

#ifdef PETSC_HAVE_FORTRAN_CAPS
#define kspgetresidualhistoryf90_     KSPGETRESIDUALHISTORYF90
#elif !defined(PETSC_HAVE_FORTRAN_UNDERSCORE)
#define kspgetresidualhistoryf90_     kspgetresidualhistoryf90
#endif

EXTERN_C_BEGIN
void PETSC_STDCALL kspgetresidualhistoryf90_(KSP *ksp,F90Array1d *indices,int *n,int *ierr)
{
  PetscReal *hist;
  *ierr = KSPGetResidualHistory(*ksp,&hist,n); if (*ierr) return;
  *ierr = F90Array1dCreate(hist,PETSC_DOUBLE,1,*n,indices);
}
EXTERN_C_END
