/*$Id: zf90snes.c,v 1.1 2000/09/28 14:59:34 bsmith Exp bsmith $*/

#include "petscsnes.h"
#include "petscf90.h"

#ifdef PETSC_HAVE_FORTRAN_CAPS
#define snesgetconvergencehistoryf90_     SNESGETCONVERGENCEHISTORYF90
#elif !defined(PETSC_HAVE_FORTRAN_UNDERSCORE)
#define snesgetconvergencehistoryf90_     snesgetconvergencehistoryf90
#endif

EXTERN_C_BEGIN
void PETSC_STDCALL snesgetconvergencehistoryf90_(SNES *snes,F90Array1d *r,F90Array1d *fits,int *n,int *ierr)
{
  PetscReal *hist;
  int       *its;
  *ierr = SNESGetConvergenceHistory(*snes,&hist,&its,n); if (*ierr) return;
  *ierr = F90Array1dCreate(hist,PETSC_DOUBLE,1,*n,r); if (*ierr) return;
  *ierr = F90Array1dCreate(its,PETSC_INT,1,*n,fits);
}
EXTERN_C_END
