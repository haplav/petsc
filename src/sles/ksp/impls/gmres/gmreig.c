/*$Id: gmreig.c,v 1.20 2000/08/16 15:18:04 balay Exp bsmith $*/

#include "src/sles/ksp/impls/gmres/gmresp.h"
#include "petscblaslapack.h"

#undef __FUNC__  
#define __FUNC__ /*<a name=""></a>*/"KSPComputeExtremeSingularValues_GMRES"
int KSPComputeExtremeSingularValues_GMRES(KSP ksp,PetscReal *emax,PetscReal *emin)
{
  KSP_GMRES *gmres = (KSP_GMRES*)ksp->data;
  int       n = gmres->it + 1,N = gmres->max_k + 2,ierr,lwork = 5*N,idummy = N,i;
  Scalar    *R = gmres->Rsvd,*work = R + N*N,sdummy;
  PetscReal *realpart = gmres->Dsvd;

  PetscFunctionBegin;
  if (!n) {
    *emax = *emin = 1.0;
    PetscFunctionReturn(0);
  }
  /* copy R matrix to work space */
  ierr = PetscMemcpy(R,gmres->hh_origin,N*N*sizeof(Scalar));CHKERRQ(ierr);

  /* zero below diagonal garbage */
  for (i=0; i<n; i++) {
    R[i*N+i+1] = 0.0;
  }
  
  /* compute Singular Values */
  /*
      The Cray math libraries do not seem to have the DGESVD() lapack routines
  */
#if defined(PETSC_HAVE_MISSING_DGESVD) 
  SETERRQ(PETSC_ERR_SUP,"DGESVD not found on Cray T3D\nNot able to provide singular value estimates.");
#else
#if !defined(PETSC_USE_COMPLEX)
  LAgesvd_("N","N",&n,&n,R,&N,realpart,&sdummy,&idummy,&sdummy,&idummy,work,&lwork,&ierr);
#else
  LAgesvd_("N","N",&n,&n,R,&N,realpart,&sdummy,&idummy,&sdummy,&idummy,work,&lwork,realpart+N,&ierr);
#endif
  if (ierr) SETERRQ(PETSC_ERR_LIB,"Error in SVD Lapack routine");

  *emin = realpart[n-1];
  *emax = realpart[0];

  PetscFunctionReturn(0);
#endif
}
/* ------------------------------------------------------------------------ */
/* ESSL has a different calling sequence for dgeev() and zgeev() than standard LAPACK */
#if defined(PETSC_HAVE_ESSL)
#undef __FUNC__  
#define __FUNC__ /*<a name=""></a>*/"KSPComputeEigenvalues_GMRES"
int KSPComputeEigenvalues_GMRES(KSP ksp,int nmax,PetscReal *r,PetscReal *c,int *neig)
{
  KSP_GMRES *gmres = (KSP_GMRES*)ksp->data;
  int       n = gmres->it + 1,N = gmres->max_k + 1,ierr,lwork = 5*N;
  int       idummy = N,i,*perm,clen,zero;
  Scalar    *R = gmres->Rsvd;
  Scalar    *cwork = R + N*N,sdummy;
  PetscReal *work,*realpart = gmres->Dsvd,*imagpart = realpart + N ;

  PetscFunctionBegin;
  if (nmax < n) SETERRQ(PETSC_ERR_ARG_SIZ,"Not enough room in work space r and c for eigenvalues");
  *neig = n;

  if (!n) {
    PetscFunctionReturn(0);
  }
  /* copy R matrix to work space */
  ierr = PetscMemcpy(R,gmres->hes_origin,N*N*sizeof(Scalar));CHKERRQ(ierr);

  /* compute eigenvalues */

  /* for ESSL version need really cwork of length N (complex), 2N
     (real); already at least 5N of space has been allocated */

  work     = (PetscReal*)PetscMalloc(lwork*sizeof(PetscReal));CHKPTRQ(work);
  zero     = 0;
  LAgeev_(&zero,R,&N,cwork,&sdummy,&idummy,&idummy,&n,work,&lwork);
  ierr = PetscFree(work);CHKERRQ(ierr);

  /* For now we stick with the convention of storing the real and imaginary
     components of evalues separately.  But is this what we really want? */
  perm = (int*)PetscMalloc(n*sizeof(int));CHKPTRQ(perm);

#if !defined(PETSC_USE_COMPLEX)
  for (i=0; i<n; i++) {
    realpart[i] = cwork[2*i];
    perm[i]     = i;
  }
  ierr = PetscSortDoubleWithPermutation(n,realpart,perm);CHKERRQ(ierr);
  for (i=0; i<n; i++) {
    r[i] = cwork[2*perm[i]];
    c[i] = cwork[2*perm[i]+1];
  }
#else
  for (i=0; i<n; i++) {
    realpart[i] = PetscRealPart(cwork[i]);
    perm[i]     = i;
  }
  ierr = PetscSortDoubleWithPermutation(n,realpart,perm);CHKERRQ(ierr);
  for (i=0; i<n; i++) {
    r[i] = PetscRealPart(cwork[perm[i]]);
    c[i] = PetscImaginaryPart(cwork[perm[i]]);
  }
#endif
  ierr = PetscFree(perm);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}
#elif !defined(PETSC_USE_COMPLEX)
#undef __FUNC__  
#define __FUNC__ /*<a name=""></a>*/"KSPComputeEigenvalues_GMRES"
int KSPComputeEigenvalues_GMRES(KSP ksp,int nmax,PetscReal *r,PetscReal *c,int *neig)
{
  KSP_GMRES *gmres = (KSP_GMRES*)ksp->data;
  int       n = gmres->it + 1,N = gmres->max_k + 1,ierr,lwork = 5*N,idummy = N,i,*perm;
  Scalar    *R = gmres->Rsvd,*work = R + N*N;
  Scalar    *realpart = gmres->Dsvd,*imagpart = realpart + N,sdummy;

  PetscFunctionBegin;
  if (nmax < n) SETERRQ(PETSC_ERR_ARG_SIZ,"Not enough room in work space r and c for eigenvalues");
  *neig = n;

  if (!n) {
    PetscFunctionReturn(0);
  }

  /* copy R matrix to work space */
  ierr = PetscMemcpy(R,gmres->hes_origin,N*N*sizeof(Scalar));CHKERRQ(ierr);

  /* compute eigenvalues */
  LAgeev_("N","N",&n,R,&N,realpart,imagpart,&sdummy,&idummy,&sdummy,&idummy,work,&lwork,&ierr);
  if (ierr) SETERRQ(PETSC_ERR_LIB,"Error in LAPACK routine");
  perm = (int*)PetscMalloc(n*sizeof(int));CHKPTRQ(perm);
  for (i=0; i<n; i++) { perm[i] = i;}
  ierr = PetscSortDoubleWithPermutation(n,realpart,perm);CHKERRQ(ierr);
  for (i=0; i<n; i++) {
    r[i] = realpart[perm[i]];
    c[i] = imagpart[perm[i]];
  }
  ierr = PetscFree(perm);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}
#else
#undef __FUNC__  
#define __FUNC__ /*<a name=""></a>*/"KSPComputeEigenvalues_GMRES"
int KSPComputeEigenvalues_GMRES(KSP ksp,int nmax,PetscReal *r,PetscReal *c,int *neig)
{
  KSP_GMRES *gmres = (KSP_GMRES*)ksp->data;
  int       n = gmres->it + 1,N = gmres->max_k + 1,ierr,lwork = 5*N,idummy = N,i,*perm;
  Scalar    *R = gmres->Rsvd,*work = R + N*N,*eigs = work + 5*N,sdummy;

  PetscFunctionBegin;
  if (nmax < n) SETERRQ(PETSC_ERR_ARG_SIZ,"Not enough room in work space r and c for eigenvalues");
  *neig = n;

  if (!n) {
    PetscFunctionReturn(0);
  }
  /* copy R matrix to work space */
  ierr = PetscMemcpy(R,gmres->hes_origin,N*N*sizeof(Scalar));CHKERRQ(ierr);

  /* compute eigenvalues */
  LAgeev_("N","N",&n,R,&N,eigs,&sdummy,&idummy,&sdummy,&idummy,work,&lwork,gmres->Dsvd,&ierr);
  if (ierr) SETERRQ(PETSC_ERR_LIB,"Error in LAPACK routine");
  perm = (int*)PetscMalloc(n*sizeof(int));CHKPTRQ(perm);
  for (i=0; i<n; i++) { perm[i] = i;}
  for (i=0; i<n; i++) { r[i]    = PetscRealPart(eigs[i]);}
  ierr = PetscSortDoubleWithPermutation(n,r,perm);CHKERRQ(ierr);
  for (i=0; i<n; i++) {
    r[i] = PetscRealPart(eigs[perm[i]]);
    c[i] = PetscImaginaryPart(eigs[perm[i]]);
  }
  ierr = PetscFree(perm);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}
#endif




