#ifndef __TAO_H
#define __TAO_H

#include <petscksp.h>

PetscErrorCode VecFischer(Vec, Vec, Vec, Vec, Vec);
PetscErrorCode VecSFischer(Vec, Vec, Vec, Vec, PetscReal, Vec);
PetscErrorCode MatDFischer(Mat, Vec, Vec, Vec, Vec, Vec, Vec, Vec, Vec);
PetscErrorCode MatDSFischer(Mat, Vec, Vec, Vec, Vec, PetscReal, Vec, Vec, Vec, Vec, Vec);


/*E
  TaoSubsetType - PetscInt representing the way TAO handles active sets

+ TAO_SUBSET_SUBVEC - TAO uses PETSc's MatGetSubMatrix and VecGetSubVector
. TAO_SUBSET_MASK - Matrices are zeroed out corresponding to active set entries
- TAO_SUBSET_MATRIXFREE - Same as TAO_SUBSET_MASK, but can be applied to matrix-free operators

  Options database keys:
. -different_hessian - TAO will use a copy of the hessian operator for masking.  By default
                       TAO will directly alter the hessian operator.
  Level: intermediate

E*/

typedef enum {TAO_SUBSET_SUBVEC,TAO_SUBSET_MASK,TAO_SUBSET_MATRIXFREE} TaoSubsetType;
PETSC_EXTERN const char *const TaoSubsetTypes[];
/*S
     Tao - Abstract PETSc object that manages nonlinear optimization solves

   Level: advanced

.seealso TaoCreate(), TaoDestroy(), TaoSetType(), TaoType
S*/

typedef struct _p_Tao*   Tao;

/*J
        TaoType - String with the name of a TAO method

       Level: beginner

J*/
#define TaoType char*
#define TAOLMVM     "lmvm"
#define TAONLS      "nls"
#define TAONTR      "ntr"
#define TAONTL      "ntl"
#define TAOCG       "cg"
#define TAOTRON     "tron"
#define TAOOWLQN    "owlqn"
#define TAOBMRM     "bmrm"
#define TAOBLMVM    "blmvm"
#define TAOBQPIP    "bqpip"
#define TAOGPCG     "gpcg"
#define TAONM       "nm"
#define TAOPOUNDERS "pounders"
#define TAOLCL      "lcl"
#define TAOSSILS    "ssils"
#define TAOSSFLS    "ssfls"
#define TAOASILS    "asils"
#define TAOASFLS    "asfls"
#define TAOIPM      "ipm"
#define TAOTEST     "test"

PETSC_EXTERN PetscClassId TAO_CLASSID;
PETSC_EXTERN PetscFunctionList TaoList;

/*  Convergence flags.
    Be sure to check that these match the flags in
    include/finclude/petsctao.h
*/
typedef enum {/* converged */
  TAO_CONVERGED_FATOL          =  1, /* f(X)-f(X*) <= fatol */
  TAO_CONVERGED_FRTOL          =  2, /* |F(X) - f(X*)|/|f(X)| < frtol */
  TAO_CONVERGED_GATOL          =  3, /* ||g(X)|| < gatol */
  TAO_CONVERGED_GRTOL          =  4, /* ||g(X)|| / f(X)  < grtol */
  TAO_CONVERGED_GTTOL          =  5, /* ||g(X)|| / ||g(X0)|| < gttol */
  TAO_CONVERGED_STEPTOL        =  6, /* step size small */
  TAO_CONVERGED_MINF          =  7, /* F < F_min */
  TAO_CONVERGED_USER          =  8, /* User defined */
  /* diverged */
  TAO_DIVERGED_MAXITS         = -2,
  TAO_DIVERGED_NAN            = -4,
  TAO_DIVERGED_MAXFCN         = -5,
  TAO_DIVERGED_LS_FAILURE     = -6,
  TAO_DIVERGED_TR_REDUCTION   = -7,
  TAO_DIVERGED_USER           = -8, /* User defined */
  /* keep going */
  TAO_CONTINUE_ITERATING      =  0} TaoConvergedReason;

PETSC_EXTERN const char **TaoConvergedReasons;


PETSC_EXTERN PetscErrorCode TaoInitializePackage(void);
PETSC_EXTERN PetscErrorCode TaoFinalizePackage(void);
PETSC_EXTERN PetscErrorCode TaoCreate(MPI_Comm,Tao*);
PETSC_EXTERN PetscErrorCode TaoSetFromOptions(Tao);
PETSC_EXTERN PetscErrorCode TaoSetFiniteDifferencesOptions(Tao);
PETSC_EXTERN PetscErrorCode TaoSetUp(Tao);
PETSC_EXTERN PetscErrorCode TaoSetType(Tao, const TaoType);
PETSC_EXTERN PetscErrorCode TaoGetType(Tao, const TaoType *);
PETSC_EXTERN PetscErrorCode TaoSetApplicationContext(Tao, void*);
PETSC_EXTERN PetscErrorCode TaoGetApplicationContext(Tao, void*);
PETSC_EXTERN PetscErrorCode TaoDestroy(Tao*);

PETSC_EXTERN PetscErrorCode TaoSetOptionsPrefix(Tao,const char []);
PETSC_EXTERN PetscErrorCode TaoView(Tao, PetscViewer);
PETSC_STATIC_INLINE PetscErrorCode TaoViewFromOptions(Tao A,const char prefix[],const char name[]) {return PetscObjectViewFromOptions((PetscObject)A,prefix,name);}

PETSC_EXTERN PetscErrorCode TaoSolve(Tao);

PETSC_EXTERN PetscErrorCode TaoRegister(const char [],PetscErrorCode (*)(Tao));
PETSC_EXTERN PetscErrorCode TaoRegisterAll(void);
PETSC_EXTERN PetscErrorCode TaoRegisterDestroy(void);

PETSC_EXTERN PetscErrorCode TaoGetConvergedReason(Tao,TaoConvergedReason*);
PETSC_EXTERN PetscErrorCode TaoGetSolutionStatus(Tao, PetscInt*, PetscReal*, PetscReal*, PetscReal*, PetscReal*, TaoConvergedReason*);
PETSC_EXTERN PetscErrorCode TaoSetConvergedReason(Tao,TaoConvergedReason);
PETSC_EXTERN PetscErrorCode TaoSetInitialVector(Tao, Vec);
PETSC_EXTERN PetscErrorCode TaoGetSolutionVector(Tao, Vec*);
PETSC_EXTERN PetscErrorCode TaoGetGradientVector(Tao, Vec*);
PETSC_EXTERN PetscErrorCode TaoSetObjectiveRoutine(Tao, PetscErrorCode(*)(Tao, Vec, PetscReal*,void*), void*);
PETSC_EXTERN PetscErrorCode TaoSetGradientRoutine(Tao, PetscErrorCode(*)(Tao, Vec, Vec, void*), void*);
PETSC_EXTERN PetscErrorCode TaoSetObjectiveAndGradientRoutine(Tao, PetscErrorCode(*)(Tao, Vec, PetscReal*, Vec, void*), void*);
PETSC_EXTERN PetscErrorCode TaoSetHessianRoutine(Tao,Mat,Mat,PetscErrorCode(*)(Tao,Vec, Mat, Mat, void*), void*);
PETSC_EXTERN PetscErrorCode TaoSetSeparableObjectiveRoutine(Tao, Vec, PetscErrorCode(*)(Tao, Vec, Vec, void*), void*);
PETSC_EXTERN PetscErrorCode TaoSetConstraintsRoutine(Tao, Vec, PetscErrorCode(*)(Tao, Vec, Vec, void*), void*);
PETSC_EXTERN PetscErrorCode TaoSetInequalityConstraintsRoutine(Tao, Vec, PetscErrorCode(*)(Tao, Vec, Vec, void*), void*);
PETSC_EXTERN PetscErrorCode TaoSetEqualityConstraintsRoutine(Tao, Vec, PetscErrorCode(*)(Tao, Vec, Vec, void*), void*);
PETSC_EXTERN PetscErrorCode TaoSetJacobianRoutine(Tao,Mat,Mat, PetscErrorCode(*)(Tao,Vec, Mat, Mat, void*), void*);
PETSC_EXTERN PetscErrorCode TaoSetJacobianStateRoutine(Tao,Mat,Mat,Mat, PetscErrorCode(*)(Tao,Vec, Mat, Mat, Mat, void*), void*);
PETSC_EXTERN PetscErrorCode TaoSetJacobianDesignRoutine(Tao,Mat,PetscErrorCode(*)(Tao,Vec, Mat, void*), void*);
PETSC_EXTERN PetscErrorCode TaoSetJacobianInequalityRoutine(Tao,Mat,Mat,PetscErrorCode(*)(Tao,Vec, Mat, Mat, void*), void*);
PETSC_EXTERN PetscErrorCode TaoSetJacobianEqualityRoutine(Tao,Mat,Mat,PetscErrorCode(*)(Tao,Vec, Mat, Mat, void*), void*);

PETSC_EXTERN PetscErrorCode TaoSetStateDesignIS(Tao, IS, IS);

PETSC_EXTERN PetscErrorCode TaoComputeObjective(Tao, Vec, PetscReal*);
PETSC_EXTERN PetscErrorCode TaoComputeSeparableObjective(Tao, Vec, Vec);
PETSC_EXTERN PetscErrorCode TaoComputeGradient(Tao, Vec, Vec);
PETSC_EXTERN PetscErrorCode TaoComputeObjectiveAndGradient(Tao, Vec, PetscReal*, Vec);
PETSC_EXTERN PetscErrorCode TaoComputeConstraints(Tao, Vec, Vec);
PETSC_EXTERN PetscErrorCode TaoComputeInequalityConstraints(Tao, Vec, Vec);
PETSC_EXTERN PetscErrorCode TaoComputeEqualityConstraints(Tao, Vec, Vec);
PETSC_EXTERN PetscErrorCode TaoDefaultComputeGradient(Tao, Vec, Vec, void*);
PETSC_EXTERN PetscErrorCode TaoIsObjectiveDefined(Tao,PetscBool*);
PETSC_EXTERN PetscErrorCode TaoIsGradientDefined(Tao,PetscBool*);
PETSC_EXTERN PetscErrorCode TaoIsObjectiveAndGradientDefined(Tao,PetscBool*);

PETSC_EXTERN PetscErrorCode TaoComputeHessian(Tao, Vec, Mat, Mat);
PETSC_EXTERN PetscErrorCode TaoComputeJacobian(Tao, Vec, Mat, Mat);
PETSC_EXTERN PetscErrorCode TaoComputeJacobianState(Tao, Vec, Mat, Mat, Mat);
PETSC_EXTERN PetscErrorCode TaoComputeJacobianEquality(Tao, Vec, Mat, Mat);
PETSC_EXTERN PetscErrorCode TaoComputeJacobianInequality(Tao, Vec, Mat, Mat);
PETSC_EXTERN PetscErrorCode TaoComputeJacobianDesign(Tao, Vec, Mat);

PETSC_EXTERN PetscErrorCode TaoDefaultComputeHessian(Tao, Vec, Mat, Mat, void*);
PETSC_EXTERN PetscErrorCode TaoDefaultComputeHessianColor(Tao, Vec, Mat, Mat, void*);
PETSC_EXTERN PetscErrorCode TaoComputeDualVariables(Tao, Vec, Vec);
PETSC_EXTERN PetscErrorCode TaoComputeDualVariables(Tao, Vec, Vec);
PETSC_EXTERN PetscErrorCode TaoSetVariableBounds(Tao, Vec, Vec);
PETSC_EXTERN PetscErrorCode TaoGetVariableBounds(Tao, Vec*, Vec*);
PETSC_EXTERN PetscErrorCode TaoGetDualVariables(Tao, Vec*, Vec*);
PETSC_EXTERN PetscErrorCode TaoSetInequalityBounds(Tao, Vec, Vec);
PETSC_EXTERN PetscErrorCode TaoGetInequalityBounds(Tao, Vec*, Vec*);
PETSC_EXTERN PetscErrorCode TaoSetVariableBoundsRoutine(Tao, PetscErrorCode(*)(Tao, Vec, Vec, void*), void*);
PETSC_EXTERN PetscErrorCode TaoComputeVariableBounds(Tao);

PETSC_EXTERN PetscErrorCode TaoGetTolerances(Tao, PetscReal*, PetscReal*, PetscReal*, PetscReal*, PetscReal*);
PETSC_EXTERN PetscErrorCode TaoSetTolerances(Tao, PetscReal, PetscReal, PetscReal, PetscReal, PetscReal);
PETSC_EXTERN PetscErrorCode TaoGetConstraintTolerances(Tao, PetscReal*, PetscReal*);
PETSC_EXTERN PetscErrorCode TaoSetConstraintTolerances(Tao, PetscReal, PetscReal);
PETSC_EXTERN PetscErrorCode TaoSetFunctionLowerBound(Tao, PetscReal);
PETSC_EXTERN PetscErrorCode TaoSetInitialTrustRegionRadius(Tao, PetscReal);
PETSC_EXTERN PetscErrorCode TaoSetMaximumIterations(Tao, PetscInt);
PETSC_EXTERN PetscErrorCode TaoSetMaximumFunctionEvaluations(Tao, PetscInt);
PETSC_EXTERN PetscErrorCode TaoGetFunctionLowerBound(Tao, PetscReal*);
PETSC_EXTERN PetscErrorCode TaoGetInitialTrustRegionRadius(Tao, PetscReal*);
PETSC_EXTERN PetscErrorCode TaoGetCurrentTrustRegionRadius(Tao, PetscReal*);
PETSC_EXTERN PetscErrorCode TaoGetMaximumIterations(Tao, PetscInt*);
PETSC_EXTERN PetscErrorCode TaoGetMaximumFunctionEvaluations(Tao, PetscInt*);
PETSC_EXTERN PetscErrorCode TaoSetOptionsPrefix(Tao, const char p[]);
PETSC_EXTERN PetscErrorCode TaoAppendOptionsPrefix(Tao, const char p[]);
PETSC_EXTERN PetscErrorCode TaoGetOptionsPrefix(Tao, const char *p[]);
PETSC_EXTERN PetscErrorCode TaoResetStatistics(Tao);

PETSC_EXTERN PetscErrorCode TaoGetKSP(Tao, KSP*);
PETSC_EXTERN PetscErrorCode TaoGetLinearSolveIterations(Tao,PetscInt *);

#include <petsctaolinesearch.h>
PETSC_EXTERN PetscErrorCode TaoLineSearchUseTaoRoutines(TaoLineSearch, Tao);
PETSC_EXTERN PetscErrorCode TaoGetLineSearch(Tao, TaoLineSearch*);

PETSC_EXTERN PetscErrorCode TaoSetHistory(Tao,PetscReal*,PetscReal*,PetscReal*,PetscInt,PetscBool);
PETSC_EXTERN PetscErrorCode TaoGetHistory(Tao,PetscReal**,PetscReal**,PetscReal**,PetscInt*);
PETSC_EXTERN PetscErrorCode TaoSetMonitor(Tao, PetscErrorCode (*)(Tao,void*),void *,PetscErrorCode (*)(void**));
PETSC_EXTERN PetscErrorCode TaoCancelMonitors(Tao);
PETSC_EXTERN PetscErrorCode TaoDefaultMonitor(Tao, void*);
PETSC_EXTERN PetscErrorCode TaoDefaultSMonitor(Tao, void*);
PETSC_EXTERN PetscErrorCode TaoDefaultCMonitor(Tao, void*);
PETSC_EXTERN PetscErrorCode TaoSolutionMonitor(Tao, void*);
PETSC_EXTERN PetscErrorCode TaoSeparableObjectiveMonitor(Tao, void*);
PETSC_EXTERN PetscErrorCode TaoGradientMonitor(Tao, void*);
PETSC_EXTERN PetscErrorCode TaoStepDirectionMonitor(Tao, void*);
PETSC_EXTERN PetscErrorCode TaoDrawSolutionMonitor(Tao, void*);
PETSC_EXTERN PetscErrorCode TaoDrawStepMonitor(Tao, void*);
PETSC_EXTERN PetscErrorCode TaoDrawGradientMonitor(Tao, void*);
PETSC_EXTERN PetscErrorCode TaoAddLineSearchCounts(Tao);

PETSC_EXTERN PetscErrorCode TaoDefaultConvergenceTest(Tao,void*);
PETSC_EXTERN PetscErrorCode TaoSetConvergenceTest(Tao, PetscErrorCode (*)(Tao, void*),void *);

PETSC_EXTERN PetscErrorCode TaoSQPCONSetStateDesignIS(Tao, IS, IS);
PETSC_EXTERN PetscErrorCode TaoLCLSetStateDesignIS(Tao, IS, IS);
PETSC_EXTERN PetscErrorCode TaoMonitor(Tao, PetscInt, PetscReal, PetscReal, PetscReal, PetscReal, TaoConvergedReason*);


#endif
