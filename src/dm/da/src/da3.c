/*$Id: da3.c,v 1.126 2001/01/18 17:37:47 bsmith Exp balay $*/

/*
   Code for manipulating distributed regular 3d arrays in parallel.
   File created by Peter Mell  7/14/95
 */

#include "src/dm/da/daimpl.h"     /*I   "petscda.h"    I*/

#if defined (PETSC_HAVE_AMS)
EXTERN_C_BEGIN
EXTERN int AMSSetFieldBlock_DA(AMS_Memory,char *,Vec);
EXTERN_C_END
#endif

#undef __FUNC__  
#define __FUNC__ "DAView_3d"
int DAView_3d(DA da,PetscViewer viewer)
{
  int        rank,ierr;
  PetscTruth isascii,isdraw,isbinary;

  PetscFunctionBegin;
  ierr = MPI_Comm_rank(da->comm,&rank);CHKERRQ(ierr);

  ierr = PetscTypeCompare((PetscObject)viewer,PETSC_VIEWER_ASCII,&isascii);CHKERRQ(ierr);
  ierr = PetscTypeCompare((PetscObject)viewer,PETSC_VIEWER_DRAW,&isdraw);CHKERRQ(ierr);
  ierr = PetscTypeCompare((PetscObject)viewer,PETSC_VIEWER_BINARY,&isbinary);CHKERRQ(ierr);
  if (isascii) {
    ierr = PetscViewerASCIISynchronizedPrintf(viewer,"Processor [%d] M %d N %d P %d m %d n %d p %d w %d s %d\n",
               rank,da->M,da->N,da->P,da->m,da->n,da->p,da->w,da->s);CHKERRQ(ierr);
    ierr = PetscViewerASCIISynchronizedPrintf(viewer,"X range of indices: %d %d, Y range of indices: %d %d, Z range of indices: %d %d\n",
               da->xs,da->xe,da->ys,da->ye,da->zs,da->ze);CHKERRQ(ierr);
#if !defined(PETSC_USE_COMPLEX)
    if (da->coordinates) {
      int    last;
      double *coors;
      ierr = VecGetArray(da->coordinates,&coors);CHKERRQ(ierr);
      ierr = VecGetLocalSize(da->coordinates,&last);CHKERRQ(ierr);
      last = last - 3;
      ierr = PetscViewerASCIISynchronizedPrintf(viewer,"Lower left corner %g %g %g : Upper right %g %g %g\n",
               coors[0],coors[1],coors[2],coors[last],coors[last+1],coors[last+2]);CHKERRQ(ierr);
      ierr = VecRestoreArray(da->coordinates,&coors);CHKERRQ(ierr);
    }
#endif
    ierr = PetscViewerFlush(viewer);CHKERRQ(ierr);
  } else if (isdraw) {
    PetscDraw       draw;
    double     ymin = -1.0,ymax = (double)da->N;
    double     xmin = -1.0,xmax = (double)((da->M+2)*da->P),x,y,ycoord,xcoord;
    int        k,plane,base,*idx;
    char       node[10];
    PetscTruth isnull;

    ierr = PetscViewerDrawGetDraw(viewer,0,&draw);CHKERRQ(ierr);
    ierr = PetscDrawIsNull(draw,&isnull);CHKERRQ(ierr); if (isnull) PetscFunctionReturn(0);
    ierr = PetscDrawSetCoordinates(draw,xmin,ymin,xmax,ymax);CHKERRQ(ierr);
    ierr = PetscDrawSynchronizedClear(draw);CHKERRQ(ierr);

    /* first processor draw all node lines */
    if (!rank) {
      for (k=0; k<da->P; k++) {
        ymin = 0.0; ymax = (double)(da->N - 1);
        for (xmin=(double)(k*(da->M+1)); xmin<(double)(da->M+(k*(da->M+1))); xmin++) {
          ierr = PetscDrawLine(draw,xmin,ymin,xmin,ymax,PETSC_DRAW_BLACK);CHKERRQ(ierr);
        }
      
        xmin = (double)(k*(da->M+1)); xmax = xmin + (double)(da->M - 1);
        for (ymin=0; ymin<(double)da->N; ymin++) {
          ierr = PetscDrawLine(draw,xmin,ymin,xmax,ymin,PETSC_DRAW_BLACK);CHKERRQ(ierr);
        }
      }
    }
    ierr = PetscDrawSynchronizedFlush(draw);CHKERRQ(ierr);
    ierr = PetscDrawPause(draw);CHKERRQ(ierr);

    for (k=0; k<da->P; k++) {  /*Go through and draw for each plane*/
      if ((k >= da->zs) && (k < da->ze)) {
        /* draw my box */
        ymin = da->ys;       
        ymax = da->ye - 1; 
        xmin = da->xs/da->w    + (da->M+1)*k; 
        xmax =(da->xe-1)/da->w + (da->M+1)*k;

        ierr = PetscDrawLine(draw,xmin,ymin,xmax,ymin,PETSC_DRAW_RED);CHKERRQ(ierr);
        ierr = PetscDrawLine(draw,xmin,ymin,xmin,ymax,PETSC_DRAW_RED);CHKERRQ(ierr);
        ierr = PetscDrawLine(draw,xmin,ymax,xmax,ymax,PETSC_DRAW_RED);CHKERRQ(ierr);
        ierr = PetscDrawLine(draw,xmax,ymin,xmax,ymax,PETSC_DRAW_RED);CHKERRQ(ierr); 

        xmin = da->xs/da->w; 
        xmax =(da->xe-1)/da->w;

        /* put in numbers*/
        base = (da->base+(da->xe-da->xs)*(da->ye-da->ys)*(k-da->zs))/da->w;

        /* Identify which processor owns the box */
        sprintf(node,"%d",rank);
        ierr = PetscDrawString(draw,xmin+(da->M+1)*k+.2,ymin+.3,PETSC_DRAW_RED,node);CHKERRQ(ierr);

        for (y=ymin; y<=ymax; y++) {
          for (x=xmin+(da->M+1)*k; x<=xmax+(da->M+1)*k; x++) {
            sprintf(node,"%d",base++);
            ierr = PetscDrawString(draw,x,y,PETSC_DRAW_BLACK,node);CHKERRQ(ierr);
          }
        } 
 
      }
    } 
    ierr = PetscDrawSynchronizedFlush(draw);CHKERRQ(ierr);
    ierr = PetscDrawPause(draw);CHKERRQ(ierr);

    for (k=0-da->s; k<da->P+da->s; k++) {  
      /* Go through and draw for each plane */
      if ((k >= da->Zs) && (k < da->Ze)) {
  
        /* overlay ghost numbers, useful for error checking */
        base = (da->Xe-da->Xs)*(da->Ye-da->Ys)*(k-da->Zs); idx = da->idx;
        plane=k;  
        /* Keep z wrap around points on the dradrawg */
        if (k<0)    { plane=da->P+k; }  
        if (k>=da->P) { plane=k-da->P; }
        ymin = da->Ys; ymax = da->Ye; 
        xmin = (da->M+1)*plane*da->w; 
        xmax = (da->M+1)*plane*da->w+da->M*da->w;
        for (y=ymin; y<ymax; y++) {
          for (x=xmin+da->Xs; x<xmin+da->Xe; x+=da->w) {
            sprintf(node,"%d",idx[base]/da->w);
            ycoord = y;
            /*Keep y wrap around points on drawing */  
            if (y<0)      { ycoord = da->N+y; } 

            if (y>=da->N) { ycoord = y-da->N; }
            xcoord = x;   /* Keep x wrap points on drawing */          

            if (x<xmin)  { xcoord = xmax - (xmin-x); }
            if (x>=xmax) { xcoord = xmin + (x-xmax); }
            ierr = PetscDrawString(draw,xcoord/da->w,ycoord,PETSC_DRAW_BLUE,node);CHKERRQ(ierr);
            base+=da->w;
          }
        }
      }         
    } 
    ierr = PetscDrawSynchronizedFlush(draw);CHKERRQ(ierr);
    ierr = PetscDrawPause(draw);CHKERRQ(ierr);
  } else if (isbinary) {
    ierr = DAView_Binary(da,viewer);CHKERRQ(ierr);
  } else {
    SETERRQ1(1,"Viewer type %s not supported for DA 3d",((PetscObject)viewer)->type_name);
  }
  PetscFunctionReturn(0);
}

EXTERN int DAPublish_Petsc(PetscObject);

#undef __FUNC__  
#define __FUNC__ "DACreate3d"
/*@C
   DACreate3d - Creates an object that will manage the communication of three-dimensional 
   regular array data that is distributed across some processors.

   Collective on MPI_Comm

   Input Parameters:
+  comm - MPI communicator
.  wrap - type of periodicity the array should have, if any.  Use one
          of DA_NONPERIODIC, DA_XPERIODIC, DA_YPERIODIC, DA_XYPERIODIC, DA_XYZPERIODIC, DA_XZPERIODIC, or DA_YZPERIODIC.
.  stencil_type - Type of stencil (DA_STENCIL_STAR or DA_STENCIL_BOX)
.  M,N,P - global dimension in each direction of the array
.  m,n,p - corresponding number of processors in each dimension 
           (or PETSC_DECIDE to have calculated)
.  dof - number of degrees of freedom per node
.  lx, ly, lz - arrays containing the number of nodes in each cell along
          the x, y, and z coordinates, or PETSC_NULL. If non-null, these
          must be of length as m,n,p and the corresponding
          m,n, or p cannot be PETSC_DECIDE. Sum of the lx[] entries must be M, sum of
          the ly[] must n, sum of the lz[] must be P
-  s - stencil width

   Output Parameter:
.  inra - the resulting distributed array object

   Options Database Key:
+  -da_view - Calls DAView() at the conclusion of DACreate3d()
-  -da_noao - do not compute natural to PETSc ordering object

   Level: beginner

   Notes:
   The stencil type DA_STENCIL_STAR with width 1 corresponds to the 
   standard 7-pt stencil, while DA_STENCIL_BOX with width 1 denotes
   the standard 27-pt stencil.

   The array data itself is NOT stored in the DA, it is stored in Vec objects;
   The appropriate vector objects can be obtained with calls to DACreateGlobalVector()
   and DACreateLocalVector() and calls to VecDuplicate() if more are needed.

.keywords: distributed array, create, three-dimensional

.seealso: DADestroy(), DAView(), DACreate1d(), DACreate2d(), DAGlobalToLocalBegin(),
          DAGlobalToLocalEnd(), DALocalToGlobal(), DALocalToLocalBegin(), DALocalToLocalEnd(),
          DAGetInfo(), DACreateGlobalVector(), DACreateLocalVector(), DACreateNaturalVector(), DALoad(), DAView()

@*/
int DACreate3d(MPI_Comm comm,DAPeriodicType wrap,DAStencilType stencil_type,int M,
               int N,int P,int m,int n,int p,int dof,int s,int *lx,int *ly,int *lz,DA *inra)
{
  int           rank,size,ierr,start,end,pm;
  int           xs,xe,ys,ye,zs,ze,x,y,z,Xs,Xe,Ys,Ye,Zs,Ze;
  int           left,up,down,bottom,top,i,j,k,*idx,nn,*flx = 0,*fly = 0,*flz = 0;
  int           n0,n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n14;
  int           n15,n16,n17,n18,n19,n20,n21,n22,n23,n24,n25,n26;
  int           *bases,*ldims,x_t,y_t,z_t,s_t,base,count,s_x,s_y,s_z; 
  int           *gA,*gB,*gAall,*gBall,ict,ldim,gdim;
  int           sn0 = 0,sn1 = 0,sn2 = 0,sn3 = 0,sn5 = 0,sn6 = 0,sn7 = 0;
  int           sn8 = 0,sn9 = 0,sn11 = 0,sn15 = 0,sn24 = 0,sn25 = 0,sn26 = 0;
  int           sn17 = 0,sn18 = 0,sn19 = 0,sn20 = 0,sn21 = 0,sn23 = 0;
  PetscTruth    flg1,flg2;
  DA            da;
  Vec           local,global;
  VecScatter    ltog,gtol;
  IS            to,from;

  PetscFunctionBegin;
  *inra = 0;

  if (dof < 1) SETERRQ1(PETSC_ERR_ARG_OUTOFRANGE,"Must have 1 or more degrees of freedom per node: %d",dof);
  if (s < 0) SETERRQ1(PETSC_ERR_ARG_OUTOFRANGE,"Stencil width cannot be negative: %d",s);
  if (M < 1) SETERRQ(PETSC_ERR_ARG_OUTOFRANGE,"Must have M positive");
  if (N < 1) SETERRQ(PETSC_ERR_ARG_OUTOFRANGE,"Must have N positive");
  if (P < 1) SETERRQ(PETSC_ERR_ARG_OUTOFRANGE,"Must have P positive");

  PetscHeaderCreate(da,_p_DA,struct _DAOps,DA_COOKIE,0,"DA",comm,DADestroy,DAView);
  da->bops->publish           = DAPublish_Petsc;
  da->ops->createglobalvector = DACreateGlobalVector;
  da->ops->getinterpolation   = DAGetInterpolation;
  da->ops->getcoloring        = DAGetColoring;
  da->ops->refine             = DARefine;

  PetscLogObjectCreate(da);
  PetscLogObjectMemory(da,sizeof(struct _p_DA));
  da->dim        = 3;
  da->gtog1      = 0;
  ierr = PetscMalloc(dof*sizeof(char*),&da->fieldname);CHKERRQ(ierr);
  ierr = PetscMemzero(da->fieldname,dof*sizeof(char*));CHKERRQ(ierr);

  ierr = MPI_Comm_size(comm,&size);CHKERRQ(ierr); 
  ierr = MPI_Comm_rank(comm,&rank);CHKERRQ(ierr); 

  if (m != PETSC_DECIDE) {
    if (m < 1) {SETERRQ1(PETSC_ERR_ARG_OUTOFRANGE,"Non-positive number of processors in X direction: %d",m);}
    else if (m > size) {SETERRQ2(PETSC_ERR_ARG_OUTOFRANGE,"Too many processors in X direction: %d %d",m,size);}
  }
  if (n != PETSC_DECIDE) {
    if (n < 1) {SETERRQ1(PETSC_ERR_ARG_OUTOFRANGE,"Non-positive number of processors in Y direction: %d",n);}
    else if (n > size) {SETERRQ2(PETSC_ERR_ARG_OUTOFRANGE,"Too many processors in Y direction: %d %d",n,size);}
  }
  if (p != PETSC_DECIDE) {
    if (p < 1) {SETERRQ1(PETSC_ERR_ARG_OUTOFRANGE,"Non-positive number of processors in Z direction: %d",p);}
    else if (p > size) {SETERRQ2(PETSC_ERR_ARG_OUTOFRANGE,"Too many processors in Z direction: %d %d",p,size);}
  }

  /* Partition the array among the processors */
  if (m == PETSC_DECIDE && n != PETSC_DECIDE && p != PETSC_DECIDE) {
    m = size/(n*p);
  } else if (m != PETSC_DECIDE && n == PETSC_DECIDE && p != PETSC_DECIDE) {
    n = size/(m*p);
  } else if (m != PETSC_DECIDE && n != PETSC_DECIDE && p == PETSC_DECIDE) {
    p = size/(m*n);
  } else if (m == PETSC_DECIDE && n == PETSC_DECIDE && p != PETSC_DECIDE) {
    /* try for squarish distribution */
    m = (int)(0.5 + sqrt(((double)M)*((double)size)/((double)N*p)));
    if (!m) m = 1;
    while (m > 0) {
      n = size/(m*p);
      if (m*n*p == size) break;
      m--;
    }
    if (!m) SETERRQ1(PETSC_ERR_ARG_OUTOFRANGE,"bad p value: p = %d",p);
    if (M > N && m < n) {int _m = m; m = n; n = _m;}
  } else if (m == PETSC_DECIDE && n != PETSC_DECIDE && p == PETSC_DECIDE) {
    /* try for squarish distribution */
    m = (int)(0.5 + sqrt(((double)M)*((double)size)/((double)P*n)));
    if (!m) m = 1;
    while (m > 0) {
      p = size/(m*n);
      if (m*n*p == size) break;
      m--;
    }
    if (!m) SETERRQ1(PETSC_ERR_ARG_OUTOFRANGE,"bad n value: n = %d",n);
    if (M > P && m < p) {int _m = m; m = p; p = _m;}
  } else if (m != PETSC_DECIDE && n == PETSC_DECIDE && p == PETSC_DECIDE) {
    /* try for squarish distribution */
    n = (int)(0.5 + sqrt(((double)N)*((double)size)/((double)P*m)));
    if (!n) n = 1;
    while (n > 0) {
      p = size/(m*n);
      if (m*n*p == size) break;
      n--;
    }
    if (!n) SETERRQ1(PETSC_ERR_ARG_OUTOFRANGE,"bad m value: m = %d",n);
    if (N > P && n < p) {int _n = n; n = p; p = _n;}
  } else if (m == PETSC_DECIDE && n == PETSC_DECIDE && p == PETSC_DECIDE) {
    /* try for squarish distribution */
    n = (int)(0.5 + pow(((double)N*N)*((double)size)/((double)P*M),1./3.));
    if (!n) n = 1;
    while (n > 0) {
      pm = size/n;
      if (n*pm == size) break;
      n--;
    }   
    if (!n) n = 1; 
    m = (int)(0.5 + sqrt(((double)M)*((double)size)/((double)P*n)));
    if (!m) m = 1;
    while (m > 0) {
      p = size/(m*n);
      if (m*n*p == size) break;
      m--;
    }
    if (M > P && m < p) {int _m = m; m = p; p = _m;}
  } else if (m*n*p != size) SETERRQ(PETSC_ERR_ARG_OUTOFRANGE,"Given Bad partition"); 

  if (m*n*p != size) SETERRQ(PETSC_ERR_PLIB,"Could not find good partition");  
  if (M < m) SETERRQ2(PETSC_ERR_ARG_OUTOFRANGE,"Partition in x direction is too fine! %d %d",M,m);
  if (N < n) SETERRQ2(PETSC_ERR_ARG_OUTOFRANGE,"Partition in y direction is too fine! %d %d",N,n);
  if (P < p) SETERRQ2(PETSC_ERR_ARG_OUTOFRANGE,"Partition in z direction is too fine! %d %d",P,p);

  ierr = PetscOptionsHasName(PETSC_NULL,"-da_partition_nodes_at_end",&flg2);CHKERRQ(ierr);
  /* 
     Determine locally owned region 
     [x, y, or z]s is the first local node number, [x, y, z] is the number of local nodes 
  */
  if (lx) { /* user decided distribution */
    x  = lx[rank % m];
    xs = 0;
    for (i=0; i<(rank%m); i++) { xs += lx[i];}
    if (x < s) SETERRQ2(PETSC_ERR_ARG_OUTOFRANGE,"Column width is too thin for stencil! %d %d",x,s);
  } else if (flg2) { 
    SETERRQ(PETSC_ERR_SUP,"-da_partition_nodes_at_end not supported");
  } else { /* Normal PETSc distribution */
    x = M/m + ((M % m) > (rank % m));
    if (x < s) SETERRQ2(PETSC_ERR_ARG_OUTOFRANGE,"Column width is too thin for stencil! %d %d",x,s);
    if ((M % m) > (rank % m)) { xs = (rank % m)*x; }
    else                      { xs = (M % m)*(x+1) + ((rank % m)-(M % m))*x; }
    ierr = PetscMalloc(m*sizeof(int),&lx);CHKERRQ(ierr);
    flx = lx;
    for (i=0; i<m; i++) {
      lx[i] = M/m + ((M % m) > (i % m));
    }
  }
  if (ly) { /* user decided distribution */
    y  = ly[(rank % (m*n))/m];
    if (y < s) SETERRQ2(PETSC_ERR_ARG_OUTOFRANGE,"Row width is too thin for stencil! %d %d",y,s);      
    ys = 0;
    for (i=0; i<(rank % (m*n))/m; i++) { ys += ly[i];}
  } else if (flg2) { 
    SETERRQ(PETSC_ERR_SUP,"-da_partition_nodes_at_end not supported");
  } else { /* Normal PETSc distribution */
    y = N/n + ((N % n) > ((rank % (m*n)) /m)); 
    if (y < s) SETERRQ2(PETSC_ERR_ARG_OUTOFRANGE,"Row width is too thin for stencil! %d %d",y,s);
    if ((N % n) > ((rank % (m*n)) /m)) {ys = ((rank % (m*n))/m)*y;}
    else                               {ys = (N % n)*(y+1) + (((rank % (m*n))/m)-(N % n))*y;}
    ierr = PetscMalloc(n*sizeof(int),&ly);CHKERRQ(ierr);
    fly = ly;
    for (i=0; i<n; i++) {
      ly[i] = N/n + ((N % n) > (i % n));
    }
  }
  if (lz) { /* user decided distribution */
    z  = lz[rank/(m*n)];
    if (z < s) SETERRQ2(PETSC_ERR_ARG_OUTOFRANGE,"Plane width is too thin for stencil! %d %d",z,s);      
    zs = 0;
    for (i=0; i<(rank/(m*n)); i++) { zs += lz[i];}
  } else if (flg2) { 
    SETERRQ(PETSC_ERR_SUP,"-da_partition_nodes_at_end not supported");
  } else { /* Normal PETSc distribution */
    z = P/p + ((P % p) > (rank / (m*n)));
    if (z < s) SETERRQ2(PETSC_ERR_ARG_OUTOFRANGE,"Plane width is too thin for stencil! %d %d",z,s);
    if ((P % p) > (rank / (m*n))) {zs = (rank/(m*n))*z;}
    else                          {zs = (P % p)*(z+1) + ((rank/(m*n))-(P % p))*z;}
    ierr = PetscMalloc(p*sizeof(int),&lz);CHKERRQ(ierr);
    flz = lz;
    for (i=0; i<p; i++) {
      lz[i] = P/p + ((P % p) > (i % p));
    }
  }
  ye = ys + y;
  xe = xs + x;
  ze = zs + z;

  /* determine ghost region */
  /* Assume No Periodicity */
  if (xs-s > 0) Xs = xs - s; else Xs = 0; 
  if (ys-s > 0) Ys = ys - s; else Ys = 0;
  if (zs-s > 0) Zs = zs - s; else Zs = 0;
  if (xe+s <= M) Xe = xe + s; else Xe = M; 
  if (ye+s <= N) Ye = ye + s; else Ye = N;
  if (ze+s <= P) Ze = ze + s; else Ze = P;

  /* X Periodic */
  if (DAXPeriodic(wrap)){
    Xs = xs - s; 
    Xe = xe + s; 
  }

  /* Y Periodic */
  if (DAYPeriodic(wrap)){
    Ys = ys - s;
    Ye = ye + s;
  }

  /* Z Periodic */
  if (DAZPeriodic(wrap)){
    Zs = zs - s;
    Ze = ze + s;
  }

  /* Resize all X parameters to reflect w */
  x   *= dof;
  xs  *= dof;
  xe  *= dof;
  Xs  *= dof;
  Xe  *= dof;
  s_x  = s*dof;
  s_y  = s;
  s_z  = s;

  /* determine starting point of each processor */
  nn = x*y*z;
  ierr = PetscMalloc((2*size+1)*sizeof(int),&bases);CHKERRQ(ierr);
  ldims = (int*)(bases+size+1);
  ierr = MPI_Allgather(&nn,1,MPI_INT,ldims,1,MPI_INT,comm);CHKERRQ(ierr);
  bases[0] = 0;
  for (i=1; i<=size; i++) {
    bases[i] = ldims[i-1];
  }
  for (i=1; i<=size; i++) {
    bases[i] += bases[i-1];
  }

  /* allocate the base parallel and sequential vectors */
  ierr = VecCreateMPI(comm,x*y*z,PETSC_DECIDE,&global);CHKERRQ(ierr);
  ierr = VecSetBlockSize(global,dof);CHKERRQ(ierr);
  ierr = VecCreateSeq(MPI_COMM_SELF,(Xe-Xs)*(Ye-Ys)*(Ze-Zs),&local);CHKERRQ(ierr);
  ierr = VecSetBlockSize(local,dof);CHKERRQ(ierr);

  /* generate appropriate vector scatters */
  /* local to global inserts non-ghost point region into global */
  VecGetOwnershipRange(global,&start,&end);
  ierr = ISCreateStride(comm,x*y*z,start,1,&to);CHKERRQ(ierr);

  left   = xs - Xs; 
  bottom = ys - Ys; top = bottom + y;
  down   = zs - Zs; up  = down + z;
  count  = x*(top-bottom)*(up-down);
  ierr = PetscMalloc(count*sizeof(int),&idx);CHKERRQ(ierr);
  count  = 0;
  for (i=down; i<up; i++) {
    for (j=bottom; j<top; j++) {
      for (k=0; k<x; k++) {
        idx[count++] = (left+j*(Xe-Xs))+i*(Xe-Xs)*(Ye-Ys) + k;
      }
    }
  }
  ierr = ISCreateGeneral(comm,count,idx,&from);CHKERRQ(ierr);
  ierr = PetscFree(idx);CHKERRQ(ierr);

  ierr = VecScatterCreate(local,from,global,to,&ltog);CHKERRQ(ierr);
  PetscLogObjectParent(da,to);
  PetscLogObjectParent(da,from);
  PetscLogObjectParent(da,ltog);
  ierr = ISDestroy(from);CHKERRQ(ierr);
  ierr = ISDestroy(to);CHKERRQ(ierr);

  /* global to local must include ghost points */
  if (stencil_type == DA_STENCIL_BOX) {
    ierr = ISCreateStride(comm,(Xe-Xs)*(Ye-Ys)*(Ze-Zs),0,1,&to); 
  } else {
    /* This is way ugly! We need to list the funny cross type region */
    /* the bottom chunck */
    left   = xs - Xs; 
    bottom = ys - Ys; top = bottom + y;
    down   = zs - Zs;   up  = down + z;
    count  = down*(top-bottom)*x +
             (up-down)*(bottom*x  + (top-bottom)*(Xe-Xs) + (Ye-Ys-top)*x) +
             (Ze-Zs-up)*(top-bottom)*x;
    ierr = PetscMalloc(count*sizeof(int),&idx);CHKERRQ(ierr);
    count  = 0;
    for (i=0; i<down; i++) {
      for (j=bottom; j<top; j++) {
        for (k=0; k<x; k++) idx[count++] = left+j*(Xe-Xs)+i*(Xe-Xs)*(Ye-Ys)+k;
      }
    }
    /* the middle piece */
    for (i=down; i<up; i++) {
      /* front */
      for (j=0; j<bottom; j++) {
        for (k=0; k<x; k++) idx[count++] = left+j*(Xe-Xs)+i*(Xe-Xs)*(Ye-Ys)+k;
      }
      /* middle */
      for (j=bottom; j<top; j++) {
        for (k=0; k<Xe-Xs; k++) idx[count++] = j*(Xe-Xs)+i*(Xe-Xs)*(Ye-Ys)+k;
      }
      /* back */
      for (j=top; j<Ye-Ys; j++) {
        for (k=0; k<x; k++) idx[count++] = left+j*(Xe-Xs)+i*(Xe-Xs)*(Ye-Ys)+k;
      }
    }
    /* the top piece */
    for (i=up; i<Ze-Zs; i++) {
      for (j=bottom; j<top; j++) {
        for (k=0; k<x; k++) idx[count++] = left+j*(Xe-Xs)+i*(Xe-Xs)*(Ye-Ys)+k;
      }
    }
    ierr = ISCreateGeneral(comm,count,idx,&to);CHKERRQ(ierr);
    ierr = PetscFree(idx);CHKERRQ(ierr);
  }

  /* determine who lies on each side of use stored in    n24 n25 n26
                                                         n21 n22 n23
                                                         n18 n19 n20

                                                         n15 n16 n17
                                                         n12     n14
                                                         n9  n10 n11

                                                         n6  n7  n8
                                                         n3  n4  n5
                                                         n0  n1  n2
  */
  
  /* Solve for X,Y, and Z Periodic Case First, Then Modify Solution */
 
  /* Assume Nodes are Internal to the Cube */
 
  n0  = rank - m*n - m - 1;
  n1  = rank - m*n - m;
  n2  = rank - m*n - m + 1;
  n3  = rank - m*n -1;
  n4  = rank - m*n;
  n5  = rank - m*n + 1;
  n6  = rank - m*n + m - 1;
  n7  = rank - m*n + m;
  n8  = rank - m*n + m + 1;

  n9  = rank - m - 1;
  n10 = rank - m;
  n11 = rank - m + 1;
  n12 = rank - 1;
  n14 = rank + 1;
  n15 = rank + m - 1;
  n16 = rank + m;
  n17 = rank + m + 1;

  n18 = rank + m*n - m - 1;
  n19 = rank + m*n - m;
  n20 = rank + m*n - m + 1;
  n21 = rank + m*n - 1;
  n22 = rank + m*n;
  n23 = rank + m*n + 1;
  n24 = rank + m*n + m - 1;
  n25 = rank + m*n + m;
  n26 = rank + m*n + m + 1;

  /* Assume Pieces are on Faces of Cube */

  if (xs == 0) { /* First assume not corner or edge */
    n0  = rank       -1 - (m*n);
    n3  = rank + m   -1 - (m*n);
    n6  = rank + 2*m -1 - (m*n);
    n9  = rank       -1;
    n12 = rank + m   -1;
    n15 = rank + 2*m -1;
    n18 = rank       -1 + (m*n);
    n21 = rank + m   -1 + (m*n);
    n24 = rank + 2*m -1 + (m*n);
   }

  if (xe == M*dof) { /* First assume not corner or edge */
    n2  = rank -2*m +1 - (m*n);
    n5  = rank - m  +1 - (m*n);
    n8  = rank      +1 - (m*n);      
    n11 = rank -2*m +1;
    n14 = rank - m  +1;
    n17 = rank      +1;
    n20 = rank -2*m +1 + (m*n);
    n23 = rank - m  +1 + (m*n);
    n26 = rank      +1 + (m*n);
  }

  if (ys==0) { /* First assume not corner or edge */
    n0  = rank + m * (n-1) -1 - (m*n);
    n1  = rank + m * (n-1)    - (m*n);
    n2  = rank + m * (n-1) +1 - (m*n);
    n9  = rank + m * (n-1) -1;
    n10 = rank + m * (n-1);
    n11 = rank + m * (n-1) +1;
    n18 = rank + m * (n-1) -1 + (m*n);
    n19 = rank + m * (n-1)    + (m*n);
    n20 = rank + m * (n-1) +1 + (m*n);
  }

  if (ye == N) { /* First assume not corner or edge */
    n6  = rank - m * (n-1) -1 - (m*n);
    n7  = rank - m * (n-1)    - (m*n);
    n8  = rank - m * (n-1) +1 - (m*n);
    n15 = rank - m * (n-1) -1;
    n16 = rank - m * (n-1);
    n17 = rank - m * (n-1) +1;
    n24 = rank - m * (n-1) -1 + (m*n);
    n25 = rank - m * (n-1)    + (m*n);
    n26 = rank - m * (n-1) +1 + (m*n);
  }
 
  if (zs == 0) { /* First assume not corner or edge */
    n0 = size - (m*n) + rank - m - 1;
    n1 = size - (m*n) + rank - m;
    n2 = size - (m*n) + rank - m + 1;
    n3 = size - (m*n) + rank - 1;
    n4 = size - (m*n) + rank;
    n5 = size - (m*n) + rank + 1;
    n6 = size - (m*n) + rank + m - 1;
    n7 = size - (m*n) + rank + m ;
    n8 = size - (m*n) + rank + m + 1;
  }

  if (ze == P) { /* First assume not corner or edge */
    n18 = (m*n) - (size-rank) - m - 1;
    n19 = (m*n) - (size-rank) - m;
    n20 = (m*n) - (size-rank) - m + 1;
    n21 = (m*n) - (size-rank) - 1;
    n22 = (m*n) - (size-rank);
    n23 = (m*n) - (size-rank) + 1;
    n24 = (m*n) - (size-rank) + m - 1;
    n25 = (m*n) - (size-rank) + m;
    n26 = (m*n) - (size-rank) + m + 1; 
  }

  if ((xs==0) && (zs==0)) { /* Assume an edge, not corner */
    n0 = size - m*n + rank + m-1 - m;
    n3 = size - m*n + rank + m-1;
    n6 = size - m*n + rank + m-1 + m;
  }
 
  if ((xs==0) && (ze==P)) { /* Assume an edge, not corner */
    n18 = m*n - (size - rank) + m-1 - m;
    n21 = m*n - (size - rank) + m-1;
    n24 = m*n - (size - rank) + m-1 + m;
  }

  if ((xs==0) && (ys==0)) { /* Assume an edge, not corner */
    n0  = rank + m*n -1 - m*n;
    n9  = rank + m*n -1;
    n18 = rank + m*n -1 + m*n;
  }

  if ((xs==0) && (ye==N)) { /* Assume an edge, not corner */
    n6  = rank - m*(n-1) + m-1 - m*n;
    n15 = rank - m*(n-1) + m-1;
    n24 = rank - m*(n-1) + m-1 + m*n;
  }

  if ((xe==M*dof) && (zs==0)) { /* Assume an edge, not corner */
    n2 = size - (m*n-rank) - (m-1) - m;
    n5 = size - (m*n-rank) - (m-1);
    n8 = size - (m*n-rank) - (m-1) + m;
  }

  if ((xe==M*dof) && (ze==P)) { /* Assume an edge, not corner */
    n20 = m*n - (size - rank) - (m-1) - m;
    n23 = m*n - (size - rank) - (m-1);
    n26 = m*n - (size - rank) - (m-1) + m;
  }

  if ((xe==M*dof) && (ys==0)) { /* Assume an edge, not corner */
    n2  = rank + m*(n-1) - (m-1) - m*n;
    n11 = rank + m*(n-1) - (m-1);
    n20 = rank + m*(n-1) - (m-1) + m*n;
  }

  if ((xe==M*dof) && (ye==N)) { /* Assume an edge, not corner */
    n8  = rank - m*n +1 - m*n;
    n17 = rank - m*n +1;
    n26 = rank - m*n +1 + m*n;
  }

  if ((ys==0) && (zs==0)) { /* Assume an edge, not corner */
    n0 = size - m + rank -1;
    n1 = size - m + rank;
    n2 = size - m + rank +1;
  }

  if ((ys==0) && (ze==P)) { /* Assume an edge, not corner */
    n18 = m*n - (size - rank) + m*(n-1) -1;
    n19 = m*n - (size - rank) + m*(n-1);
    n20 = m*n - (size - rank) + m*(n-1) +1;
  }

  if ((ye==N) && (zs==0)) { /* Assume an edge, not corner */
    n6 = size - (m*n-rank) - m * (n-1) -1;
    n7 = size - (m*n-rank) - m * (n-1);
    n8 = size - (m*n-rank) - m * (n-1) +1;
  }

  if ((ye==N) && (ze==P)) { /* Assume an edge, not corner */
    n24 = rank - (size-m) -1;
    n25 = rank - (size-m);
    n26 = rank - (size-m) +1;
  }

  /* Check for Corners */
  if ((xs==0)   && (ys==0) && (zs==0)) { n0  = size -1;}
  if ((xs==0)   && (ys==0) && (ze==P)) { n18 = m*n-1;}    
  if ((xs==0)   && (ye==N) && (zs==0)) { n6  = (size-1)-m*(n-1);}
  if ((xs==0)   && (ye==N) && (ze==P)) { n24 = m-1;}
  if ((xe==M*dof) && (ys==0) && (zs==0)) { n2  = size-m;}
  if ((xe==M*dof) && (ys==0) && (ze==P)) { n20 = m*n-m;}
  if ((xe==M*dof) && (ye==N) && (zs==0)) { n8  = size-m*n;}
  if ((xe==M*dof) && (ye==N) && (ze==P)) { n26 = 0;}

  /* Check for when not X,Y, and Z Periodic */

  /* If not X periodic */
  if ((wrap != DA_XPERIODIC)  && (wrap != DA_XYPERIODIC) && 
     (wrap != DA_XZPERIODIC) && (wrap != DA_XYZPERIODIC)) {
    if (xs==0)   {n0  = n3  = n6  = n9  = n12 = n15 = n18 = n21 = n24 = -2;}
    if (xe==M*dof) {n2  = n5  = n8  = n11 = n14 = n17 = n20 = n23 = n26 = -2;}
  }

  /* If not Y periodic */
  if ((wrap != DA_YPERIODIC)  && (wrap != DA_XYPERIODIC) && 
      (wrap != DA_YZPERIODIC) && (wrap != DA_XYZPERIODIC)) {
    if (ys==0)   {n0  = n1  = n2  = n9  = n10 = n11 = n18 = n19 = n20 = -2;}
    if (ye==N)   {n6  = n7  = n8  = n15 = n16 = n17 = n24 = n25 = n26 = -2;}
  }

  /* If not Z periodic */
  if ((wrap != DA_ZPERIODIC)  && (wrap != DA_XZPERIODIC) && 
      (wrap != DA_YZPERIODIC) && (wrap != DA_XYZPERIODIC)) {
    if (zs==0)   {n0  = n1  = n2  = n3  = n4  = n5  = n6  = n7  = n8  = -2;}
    if (ze==P)   {n18 = n19 = n20 = n21 = n22 = n23 = n24 = n25 = n26 = -2;}
  }

  /* If star stencil then delete the corner neighbors */
  if (stencil_type == DA_STENCIL_STAR) { 
     /* save information about corner neighbors */
     sn0 = n0; sn1 = n1; sn2 = n2; sn3 = n3; sn5 = n5; sn6 = n6; sn7 = n7;
     sn8 = n8; sn9 = n9; sn11 = n11; sn15 = n15; sn17 = n17; sn18 = n18;
     sn19 = n19; sn20 = n20; sn21 = n21; sn23 = n23; sn24 = n24; sn25 = n25;
     sn26 = n26;
     n0  = n1  = n2  = n3  = n5  = n6  = n7  = n8  = n9  = n11 = 
     n15 = n17 = n18 = n19 = n20 = n21 = n23 = n24 = n25 = n26 = -1;
  }


  ierr = PetscMalloc((Xe-Xs)*(Ye-Ys)*(Ze-Zs)*sizeof(int),&idx);CHKERRQ(ierr);
  PetscLogObjectMemory(da,(Xe-Xs)*(Ye-Ys)*(Ze-Zs)*sizeof(int));

  nn = 0;

  /* Bottom Level */
  for (k=0; k<s_z; k++) {  
    for (i=1; i<=s_y; i++) {
      if (n0 >= 0) { /* left below */
        x_t = lx[n0 % m]*dof; 
        y_t = ly[(n0 % (m*n))/m]; 
        z_t = lz[n0 / (m*n)]; 
        s_t = bases[n0] + x_t*y_t*z_t - (s_y-i)*x_t - s_x - (s_z-k-1)*x_t*y_t;
        for (j=0; j<s_x; j++) { idx[nn++] = s_t++;}
      }
      if (n1 >= 0) { /* directly below */
        x_t = x;
        y_t = ly[(n1 % (m*n))/m];
        z_t = lz[n1 / (m*n)];
        s_t = bases[n1] + x_t*y_t*z_t - (s_y+1-i)*x_t - (s_z-k-1)*x_t*y_t;
        for (j=0; j<x_t; j++) { idx[nn++] = s_t++;}
      }
      if (n2 >= 0) { /* right below */
        x_t = lx[n2 % m]*dof;
        y_t = ly[(n2 % (m*n))/m];
        z_t = lz[n2 / (m*n)];
        s_t = bases[n2] + x_t*y_t*z_t - (s_y+1-i)*x_t - (s_z-k-1)*x_t*y_t;
        for (j=0; j<s_x; j++) { idx[nn++] = s_t++;}
      }
    }

    for (i=0; i<y; i++) {
      if (n3 >= 0) { /* directly left */
        x_t = lx[n3 % m]*dof;
        y_t = y;
        z_t = lz[n3 / (m*n)];
        s_t = bases[n3] + (i+1)*x_t - s_x + x_t*y_t*z_t - (s_z-k)*x_t*y_t;
        for (j=0; j<s_x; j++) { idx[nn++] = s_t++;}
      }

      if (n4 >= 0) { /* middle */
        x_t = x;
        y_t = y;
        z_t = lz[n4 / (m*n)];
        s_t = bases[n4] + i*x_t + x_t*y_t*z_t - (s_z-k)*x_t*y_t;
        for (j=0; j<x_t; j++) { idx[nn++] = s_t++;}
      }

      if (n5 >= 0) { /* directly right */
        x_t = lx[n5 % m]*dof;
        y_t = y;
        z_t = lz[n5 / (m*n)];
        s_t = bases[n5] + i*x_t + x_t*y_t*z_t - (s_z-k)*x_t*y_t;
        for (j=0; j<s_x; j++) { idx[nn++] = s_t++;}
      }
    }

    for (i=1; i<=s_y; i++) {
      if (n6 >= 0) { /* left above */
        x_t = lx[n6 % m]*dof;
        y_t = ly[(n6 % (m*n))/m];
        z_t = lz[n6 / (m*n)];
        s_t = bases[n6] + i*x_t - s_x + x_t*y_t*z_t - (s_z-k)*x_t*y_t;
        for (j=0; j<s_x; j++) { idx[nn++] = s_t++;}
      }
      if (n7 >= 0) { /* directly above */
        x_t = x;
        y_t = ly[(n7 % (m*n))/m];
        z_t = lz[n7 / (m*n)];
        s_t = bases[n7] + (i-1)*x_t + x_t*y_t*z_t - (s_z-k)*x_t*y_t;
        for (j=0; j<x_t; j++) { idx[nn++] = s_t++;}
      }
      if (n8 >= 0) { /* right above */
        x_t = lx[n8 % m]*dof;
        y_t = ly[(n8 % (m*n))/m];
        z_t = lz[n8 / (m*n)];
        s_t = bases[n8] + (i-1)*x_t + x_t*y_t*z_t - (s_z-k)*x_t*y_t;
        for (j=0; j<s_x; j++) { idx[nn++] = s_t++;}
      }
    }
  }

  /* Middle Level */
  for (k=0; k<z; k++) {  
    for (i=1; i<=s_y; i++) {
      if (n9 >= 0) { /* left below */
        x_t = lx[n9 % m]*dof;
        y_t = ly[(n9 % (m*n))/m];
        z_t = z;
        s_t = bases[n9] - (s_y-i)*x_t -s_x + (k+1)*x_t*y_t;
        for (j=0; j<s_x; j++) { idx[nn++] = s_t++;}
      }
      if (n10 >= 0) { /* directly below */
        x_t = x;
        y_t = ly[(n10 % (m*n))/m]; 
        z_t = z;
        s_t = bases[n10] - (s_y+1-i)*x_t + (k+1)*x_t*y_t;
        for (j=0; j<x_t; j++) { idx[nn++] = s_t++;}
      }
      if (n11 >= 0) { /* right below */
        x_t = lx[n11 % m]*dof;
        y_t = ly[(n11 % (m*n))/m];
        z_t = z;
        s_t = bases[n11] - (s_y+1-i)*x_t + (k+1)*x_t*y_t;
        for (j=0; j<s_x; j++) { idx[nn++] = s_t++;}
      }
    }

    for (i=0; i<y; i++) {
      if (n12 >= 0) { /* directly left */
        x_t = lx[n12 % m]*dof;
        y_t = y;
        z_t = z;
        s_t = bases[n12] + (i+1)*x_t - s_x + k*x_t*y_t;
        for (j=0; j<s_x; j++) { idx[nn++] = s_t++;}
      }

      /* Interior */
      s_t = bases[rank] + i*x + k*x*y;
      for (j=0; j<x; j++) { idx[nn++] = s_t++;}

      if (n14 >= 0) { /* directly right */
        x_t = lx[n14 % m]*dof;
        y_t = y;
        z_t = z;
        s_t = bases[n14] + i*x_t + k*x_t*y_t;
        for (j=0; j<s_x; j++) { idx[nn++] = s_t++;}
      }
    }

    for (i=1; i<=s_y; i++) {
      if (n15 >= 0) { /* left above */
        x_t = lx[n15 % m]*dof; 
        y_t = ly[(n15 % (m*n))/m];
        z_t = z;
        s_t = bases[n15] + i*x_t - s_x + k*x_t*y_t;
        for (j=0; j<s_x; j++) { idx[nn++] = s_t++;}
      }
      if (n16 >= 0) { /* directly above */
        x_t = x;
        y_t = ly[(n16 % (m*n))/m];
        z_t = z;
        s_t = bases[n16] + (i-1)*x_t + k*x_t*y_t;
        for (j=0; j<x_t; j++) { idx[nn++] = s_t++;}
      }
      if (n17 >= 0) { /* right above */
        x_t = lx[n17 % m]*dof;
        y_t = ly[(n17 % (m*n))/m]; 
        z_t = z;
        s_t = bases[n17] + (i-1)*x_t + k*x_t*y_t;
        for (j=0; j<s_x; j++) { idx[nn++] = s_t++;}
      }
    } 
  }
 
  /* Upper Level */
  for (k=0; k<s_z; k++) {  
    for (i=1; i<=s_y; i++) {
      if (n18 >= 0) { /* left below */
        x_t = lx[n18 % m]*dof;
        y_t = ly[(n18 % (m*n))/m]; 
        z_t = lz[n18 / (m*n)]; 
        s_t = bases[n18] - (s_y-i)*x_t -s_x + (k+1)*x_t*y_t;
        for (j=0; j<s_x; j++) { idx[nn++] = s_t++;}
      }
      if (n19 >= 0) { /* directly below */
        x_t = x;
        y_t = ly[(n19 % (m*n))/m]; 
        z_t = lz[n19 / (m*n)]; 
        s_t = bases[n19] - (s_y+1-i)*x_t + (k+1)*x_t*y_t;
        for (j=0; j<x_t; j++) { idx[nn++] = s_t++;}
      }
      if (n20 >= 0) { /* right below */
        x_t = lx[n20 % m]*dof;
        y_t = ly[(n20 % (m*n))/m];
        z_t = lz[n20 / (m*n)];
        s_t = bases[n20] - (s_y+1-i)*x_t + (k+1)*x_t*y_t;
        for (j=0; j<s_x; j++) { idx[nn++] = s_t++;}
      }
    }

    for (i=0; i<y; i++) {
      if (n21 >= 0) { /* directly left */
        x_t = lx[n21 % m]*dof;
        y_t = y;
        z_t = lz[n21 / (m*n)];
        s_t = bases[n21] + (i+1)*x_t - s_x + k*x_t*y_t;
        for (j=0; j<s_x; j++) { idx[nn++] = s_t++;}
      }

      if (n22 >= 0) { /* middle */
        x_t = x;
        y_t = y;
        z_t = lz[n22 / (m*n)];
        s_t = bases[n22] + i*x_t + k*x_t*y_t;
        for (j=0; j<x_t; j++) { idx[nn++] = s_t++;}
      }

      if (n23 >= 0) { /* directly right */
        x_t = lx[n23 % m]*dof;
        y_t = y;
        z_t = lz[n23 / (m*n)];
        s_t = bases[n23] + i*x_t + k*x_t*y_t;
        for (j=0; j<s_x; j++) { idx[nn++] = s_t++;}
      }
    }

    for (i=1; i<=s_y; i++) {
      if (n24 >= 0) { /* left above */
        x_t = lx[n24 % m]*dof;
        y_t = ly[(n24 % (m*n))/m]; 
        z_t = lz[n24 / (m*n)]; 
        s_t = bases[n24] + i*x_t - s_x + k*x_t*y_t;
        for (j=0; j<s_x; j++) { idx[nn++] = s_t++;}
      }
      if (n25 >= 0) { /* directly above */
        x_t = x;
        y_t = ly[(n25 % (m*n))/m];
        z_t = lz[n25 / (m*n)];
        s_t = bases[n25] + (i-1)*x_t + k*x_t*y_t;
        for (j=0; j<x_t; j++) { idx[nn++] = s_t++;}
      }
      if (n26 >= 0) { /* right above */
        x_t = lx[n26 % m]*dof;
        y_t = ly[(n26 % (m*n))/m]; 
        z_t = lz[n26 / (m*n)];
        s_t = bases[n26] + (i-1)*x_t + k*x_t*y_t;
        for (j=0; j<s_x; j++) { idx[nn++] = s_t++;}
      }
    }
  }  
  base = bases[rank];
  ierr = ISCreateGeneral(comm,nn,idx,&from);CHKERRQ(ierr);
  ierr = VecScatterCreate(global,from,local,to,&gtol);CHKERRQ(ierr);
  PetscLogObjectParent(da,gtol);
  PetscLogObjectParent(da,to);
  PetscLogObjectParent(da,from);
  ierr = ISDestroy(to);CHKERRQ(ierr);
  ierr = ISDestroy(from);CHKERRQ(ierr);
  da->stencil_type = stencil_type;
  da->M  = M;  da->N  = N; da->P = P; 
  da->m  = m;  da->n  = n; da->p = p;
  da->w  = dof;  da->s  = s;
  da->xs = xs; da->xe = xe; da->ys = ys; da->ye = ye; da->zs = zs; da->ze = ze;
  da->Xs = Xs; da->Xe = Xe; da->Ys = Ys; da->Ye = Ye; da->Zs = Zs; da->Ze = Ze;

  PetscLogObjectParent(da,global);
  PetscLogObjectParent(da,local);

  if (stencil_type == DA_STENCIL_STAR) { 
    /*
        Recompute the local to global mappings, this time keeping the 
      information about the cross corner processor numbers.
    */
    n0  = sn0;  n1  = sn1;  n2  = sn2;  n3  = sn3;  n5  = sn5;  n6  = sn6; n7 = sn7;
    n8  = sn8;  n9  = sn9;  n11 = sn11; n15 = sn15; n17 = sn17; n18 = sn18;
    n19 = sn19; n20 = sn20; n21 = sn21; n23 = sn23; n24 = sn24; n25 = sn25;
    n26 = sn26;

    nn = 0;

    /* Bottom Level */
    for (k=0; k<s_z; k++) {  
      for (i=1; i<=s_y; i++) {
        if (n0 >= 0) { /* left below */
          x_t = lx[n0 % m]*dof; 
          y_t = ly[(n0 % (m*n))/m]; 
          z_t = lz[n0 / (m*n)]; 
          s_t = bases[n0] + x_t*y_t*z_t - (s_y-i)*x_t - s_x - (s_z-k-1)*x_t*y_t;
          for (j=0; j<s_x; j++) { idx[nn++] = s_t++;}
        }
        if (n1 >= 0) { /* directly below */
          x_t = x;
          y_t = ly[(n1 % (m*n))/m];
          z_t = lz[n1 / (m*n)];
          s_t = bases[n1] + x_t*y_t*z_t - (s_y+1-i)*x_t - (s_z-k-1)*x_t*y_t;
          for (j=0; j<x_t; j++) { idx[nn++] = s_t++;}
        }
        if (n2 >= 0) { /* right below */
          x_t = lx[n2 % m]*dof;
          y_t = ly[(n2 % (m*n))/m];
          z_t = lz[n2 / (m*n)];
          s_t = bases[n2] + x_t*y_t*z_t - (s_y+1-i)*x_t - (s_z-k-1)*x_t*y_t;
          for (j=0; j<s_x; j++) { idx[nn++] = s_t++;}
        }
      }

      for (i=0; i<y; i++) {
        if (n3 >= 0) { /* directly left */
          x_t = lx[n3 % m]*dof;
          y_t = y;
          z_t = lz[n3 / (m*n)];
          s_t = bases[n3] + (i+1)*x_t - s_x + x_t*y_t*z_t - (s_z-k)*x_t*y_t;
          for (j=0; j<s_x; j++) { idx[nn++] = s_t++;}
        }

        if (n4 >= 0) { /* middle */
          x_t = x;
          y_t = y;
          z_t = lz[n4 / (m*n)];
          s_t = bases[n4] + i*x_t + x_t*y_t*z_t - (s_z-k)*x_t*y_t;
          for (j=0; j<x_t; j++) { idx[nn++] = s_t++;}
        }

        if (n5 >= 0) { /* directly right */
          x_t = lx[n5 % m]*dof;
          y_t = y;
          z_t = lz[n5 / (m*n)];
          s_t = bases[n5] + i*x_t + x_t*y_t*z_t - (s_z-k)*x_t*y_t;
          for (j=0; j<s_x; j++) { idx[nn++] = s_t++;}
        }
      }

      for (i=1; i<=s_y; i++) {
        if (n6 >= 0) { /* left above */
          x_t = lx[n6 % m]*dof;
          y_t = ly[(n6 % (m*n))/m];
          z_t = lz[n6 / (m*n)];
          s_t = bases[n6] + i*x_t - s_x + x_t*y_t*z_t - (s_z-k)*x_t*y_t;
          for (j=0; j<s_x; j++) { idx[nn++] = s_t++;}
        }
        if (n7 >= 0) { /* directly above */
          x_t = x;
          y_t = ly[(n7 % (m*n))/m];
          z_t = lz[n7 / (m*n)];
          s_t = bases[n7] + (i-1)*x_t + x_t*y_t*z_t - (s_z-k)*x_t*y_t;
          for (j=0; j<x_t; j++) { idx[nn++] = s_t++;}
        }
        if (n8 >= 0) { /* right above */
          x_t = lx[n8 % m]*dof;
          y_t = ly[(n8 % (m*n))/m];
          z_t = lz[n8 / (m*n)];
          s_t = bases[n8] + (i-1)*x_t + x_t*y_t*z_t - (s_z-k)*x_t*y_t;
          for (j=0; j<s_x; j++) { idx[nn++] = s_t++;}
        }
      }
    }

    /* Middle Level */
    for (k=0; k<z; k++) {  
      for (i=1; i<=s_y; i++) {
        if (n9 >= 0) { /* left below */
          x_t = lx[n9 % m]*dof;
          y_t = ly[(n9 % (m*n))/m];
          z_t = z;
          s_t = bases[n9] - (s_y-i)*x_t -s_x + (k+1)*x_t*y_t;
          for (j=0; j<s_x; j++) { idx[nn++] = s_t++;}
        }
        if (n10 >= 0) { /* directly below */
          x_t = x;
          y_t = ly[(n10 % (m*n))/m]; 
          z_t = z;
          s_t = bases[n10] - (s_y+1-i)*x_t + (k+1)*x_t*y_t;
          for (j=0; j<x_t; j++) { idx[nn++] = s_t++;}
        }
        if (n11 >= 0) { /* right below */
          x_t = lx[n11 % m]*dof;
          y_t = ly[(n11 % (m*n))/m];
          z_t = z;
          s_t = bases[n11] - (s_y+1-i)*x_t + (k+1)*x_t*y_t;
          for (j=0; j<s_x; j++) { idx[nn++] = s_t++;}
        }
      }

      for (i=0; i<y; i++) {
        if (n12 >= 0) { /* directly left */
          x_t = lx[n12 % m]*dof;
          y_t = y;
          z_t = z;
          s_t = bases[n12] + (i+1)*x_t - s_x + k*x_t*y_t;
          for (j=0; j<s_x; j++) { idx[nn++] = s_t++;}
        }

        /* Interior */
        s_t = bases[rank] + i*x + k*x*y;
        for (j=0; j<x; j++) { idx[nn++] = s_t++;}

        if (n14 >= 0) { /* directly right */
          x_t = lx[n14 % m]*dof;
          y_t = y;
          z_t = z;
          s_t = bases[n14] + i*x_t + k*x_t*y_t;
          for (j=0; j<s_x; j++) { idx[nn++] = s_t++;}
        }
      }

      for (i=1; i<=s_y; i++) {
        if (n15 >= 0) { /* left above */
          x_t = lx[n15 % m]*dof; 
          y_t = ly[(n15 % (m*n))/m];
          z_t = z;
          s_t = bases[n15] + i*x_t - s_x + k*x_t*y_t;
          for (j=0; j<s_x; j++) { idx[nn++] = s_t++;}
        }
        if (n16 >= 0) { /* directly above */
          x_t = x;
          y_t = ly[(n16 % (m*n))/m];
          z_t = z;
          s_t = bases[n16] + (i-1)*x_t + k*x_t*y_t;
          for (j=0; j<x_t; j++) { idx[nn++] = s_t++;}
        }
        if (n17 >= 0) { /* right above */
          x_t = lx[n17 % m]*dof;
          y_t = ly[(n17 % (m*n))/m]; 
          z_t = z;
          s_t = bases[n17] + (i-1)*x_t + k*x_t*y_t;
          for (j=0; j<s_x; j++) { idx[nn++] = s_t++;}
        }
      } 
    }
 
    /* Upper Level */
    for (k=0; k<s_z; k++) {  
      for (i=1; i<=s_y; i++) {
        if (n18 >= 0) { /* left below */
          x_t = lx[n18 % m]*dof;
          y_t = ly[(n18 % (m*n))/m]; 
          z_t = lz[n18 / (m*n)]; 
          s_t = bases[n18] - (s_y-i)*x_t -s_x + (k+1)*x_t*y_t;
          for (j=0; j<s_x; j++) { idx[nn++] = s_t++;}
        }
        if (n19 >= 0) { /* directly below */
          x_t = x;
          y_t = ly[(n19 % (m*n))/m]; 
          z_t = lz[n19 / (m*n)]; 
          s_t = bases[n19] - (s_y+1-i)*x_t + (k+1)*x_t*y_t;
          for (j=0; j<x_t; j++) { idx[nn++] = s_t++;}
        }
        if (n20 >= 0) { /* right below */
          x_t = lx[n20 % m]*dof;
          y_t = ly[(n20 % (m*n))/m];
          z_t = lz[n20 / (m*n)];
          s_t = bases[n20] - (s_y+1-i)*x_t + (k+1)*x_t*y_t;
          for (j=0; j<s_x; j++) { idx[nn++] = s_t++;}
        }
      }

      for (i=0; i<y; i++) {
        if (n21 >= 0) { /* directly left */
          x_t = lx[n21 % m]*dof;
          y_t = y;
          z_t = lz[n21 / (m*n)];
          s_t = bases[n21] + (i+1)*x_t - s_x + k*x_t*y_t;
          for (j=0; j<s_x; j++) { idx[nn++] = s_t++;}
        }

        if (n22 >= 0) { /* middle */
          x_t = x;
          y_t = y;
          z_t = lz[n22 / (m*n)];
          s_t = bases[n22] + i*x_t + k*x_t*y_t;
          for (j=0; j<x_t; j++) { idx[nn++] = s_t++;}
        }

        if (n23 >= 0) { /* directly right */
          x_t = lx[n23 % m]*dof;
          y_t = y;
          z_t = lz[n23 / (m*n)];
          s_t = bases[n23] + i*x_t + k*x_t*y_t;
          for (j=0; j<s_x; j++) { idx[nn++] = s_t++;}
        }
      }

      for (i=1; i<=s_y; i++) {
        if (n24 >= 0) { /* left above */
          x_t = lx[n24 % m]*dof;
          y_t = ly[(n24 % (m*n))/m]; 
          z_t = lz[n24 / (m*n)]; 
          s_t = bases[n24] + i*x_t - s_x + k*x_t*y_t;
          for (j=0; j<s_x; j++) { idx[nn++] = s_t++;}
        }
        if (n25 >= 0) { /* directly above */
          x_t = x;
          y_t = ly[(n25 % (m*n))/m];
          z_t = lz[n25 / (m*n)];
          s_t = bases[n25] + (i-1)*x_t + k*x_t*y_t;
          for (j=0; j<x_t; j++) { idx[nn++] = s_t++;}
        }
        if (n26 >= 0) { /* right above */
          x_t = lx[n26 % m]*dof;
          y_t = ly[(n26 % (m*n))/m]; 
          z_t = lz[n26 / (m*n)];
          s_t = bases[n26] + (i-1)*x_t + k*x_t*y_t;
          for (j=0; j<s_x; j++) { idx[nn++] = s_t++;}
        }
      }
    }  
  }
  da->global    = global; 
  da->local     = local; 
  da->gtol      = gtol;
  da->ltog      = ltog;
  da->idx       = idx;
  da->Nl        = nn;
  da->base      = base;
  da->ops->view = DAView_3d;
  da->wrap      = wrap;
  *inra = da;

  /* 
     Set the local to global ordering in the global vector, this allows use
     of VecSetValuesLocal().
  */
  ierr  = ISLocalToGlobalMappingCreate(comm,nn,idx,&da->ltogmap);CHKERRQ(ierr);
  ierr  = VecSetLocalToGlobalMapping(da->global,da->ltogmap);CHKERRQ(ierr);
  ierr = ISLocalToGlobalMappingBlock(da->ltogmap,da->w,&da->ltogmapb);CHKERRQ(ierr);
  ierr = VecSetLocalToGlobalMappingBlock(da->global,da->ltogmapb);CHKERRQ(ierr);
  PetscLogObjectParent(da,da->ltogmap);

  /* redo idx to include "missing" ghost points */
  /* Solve for X,Y, and Z Periodic Case First, Then Modify Solution */
 
  /* Assume Nodes are Internal to the Cube */
 
  n0  = rank - m*n - m - 1;
  n1  = rank - m*n - m;
  n2  = rank - m*n - m + 1;
  n3  = rank - m*n -1;
  n4  = rank - m*n;
  n5  = rank - m*n + 1;
  n6  = rank - m*n + m - 1;
  n7  = rank - m*n + m;
  n8  = rank - m*n + m + 1;

  n9  = rank - m - 1;
  n10 = rank - m;
  n11 = rank - m + 1;
  n12 = rank - 1;
  n14 = rank + 1;
  n15 = rank + m - 1;
  n16 = rank + m;
  n17 = rank + m + 1;

  n18 = rank + m*n - m - 1;
  n19 = rank + m*n - m;
  n20 = rank + m*n - m + 1;
  n21 = rank + m*n - 1;
  n22 = rank + m*n;
  n23 = rank + m*n + 1;
  n24 = rank + m*n + m - 1;
  n25 = rank + m*n + m;
  n26 = rank + m*n + m + 1;

  /* Assume Pieces are on Faces of Cube */

  if (xs == 0) { /* First assume not corner or edge */
    n0  = rank       -1 - (m*n);
    n3  = rank + m   -1 - (m*n);
    n6  = rank + 2*m -1 - (m*n);
    n9  = rank       -1;
    n12 = rank + m   -1;
    n15 = rank + 2*m -1;
    n18 = rank       -1 + (m*n);
    n21 = rank + m   -1 + (m*n);
    n24 = rank + 2*m -1 + (m*n);
   }

  if (xe == M*dof) { /* First assume not corner or edge */
    n2  = rank -2*m +1 - (m*n);
    n5  = rank - m  +1 - (m*n);
    n8  = rank      +1 - (m*n);      
    n11 = rank -2*m +1;
    n14 = rank - m  +1;
    n17 = rank      +1;
    n20 = rank -2*m +1 + (m*n);
    n23 = rank - m  +1 + (m*n);
    n26 = rank      +1 + (m*n);
  }

  if (ys==0) { /* First assume not corner or edge */
    n0  = rank + m * (n-1) -1 - (m*n);
    n1  = rank + m * (n-1)    - (m*n);
    n2  = rank + m * (n-1) +1 - (m*n);
    n9  = rank + m * (n-1) -1;
    n10 = rank + m * (n-1);
    n11 = rank + m * (n-1) +1;
    n18 = rank + m * (n-1) -1 + (m*n);
    n19 = rank + m * (n-1)    + (m*n);
    n20 = rank + m * (n-1) +1 + (m*n);
  }

  if (ye == N) { /* First assume not corner or edge */
    n6  = rank - m * (n-1) -1 - (m*n);
    n7  = rank - m * (n-1)    - (m*n);
    n8  = rank - m * (n-1) +1 - (m*n);
    n15 = rank - m * (n-1) -1;
    n16 = rank - m * (n-1);
    n17 = rank - m * (n-1) +1;
    n24 = rank - m * (n-1) -1 + (m*n);
    n25 = rank - m * (n-1)    + (m*n);
    n26 = rank - m * (n-1) +1 + (m*n);
  }
 
  if (zs == 0) { /* First assume not corner or edge */
    n0 = size - (m*n) + rank - m - 1;
    n1 = size - (m*n) + rank - m;
    n2 = size - (m*n) + rank - m + 1;
    n3 = size - (m*n) + rank - 1;
    n4 = size - (m*n) + rank;
    n5 = size - (m*n) + rank + 1;
    n6 = size - (m*n) + rank + m - 1;
    n7 = size - (m*n) + rank + m ;
    n8 = size - (m*n) + rank + m + 1;
  }

  if (ze == P) { /* First assume not corner or edge */
    n18 = (m*n) - (size-rank) - m - 1;
    n19 = (m*n) - (size-rank) - m;
    n20 = (m*n) - (size-rank) - m + 1;
    n21 = (m*n) - (size-rank) - 1;
    n22 = (m*n) - (size-rank);
    n23 = (m*n) - (size-rank) + 1;
    n24 = (m*n) - (size-rank) + m - 1;
    n25 = (m*n) - (size-rank) + m;
    n26 = (m*n) - (size-rank) + m + 1; 
  }

  if ((xs==0) && (zs==0)) { /* Assume an edge, not corner */
    n0 = size - m*n + rank + m-1 - m;
    n3 = size - m*n + rank + m-1;
    n6 = size - m*n + rank + m-1 + m;
  }
 
  if ((xs==0) && (ze==P)) { /* Assume an edge, not corner */
    n18 = m*n - (size - rank) + m-1 - m;
    n21 = m*n - (size - rank) + m-1;
    n24 = m*n - (size - rank) + m-1 + m;
  }

  if ((xs==0) && (ys==0)) { /* Assume an edge, not corner */
    n0  = rank + m*n -1 - m*n;
    n9  = rank + m*n -1;
    n18 = rank + m*n -1 + m*n;
  }

  if ((xs==0) && (ye==N)) { /* Assume an edge, not corner */
    n6  = rank - m*(n-1) + m-1 - m*n;
    n15 = rank - m*(n-1) + m-1;
    n24 = rank - m*(n-1) + m-1 + m*n;
  }

  if ((xe==M*dof) && (zs==0)) { /* Assume an edge, not corner */
    n2 = size - (m*n-rank) - (m-1) - m;
    n5 = size - (m*n-rank) - (m-1);
    n8 = size - (m*n-rank) - (m-1) + m;
  }

  if ((xe==M*dof) && (ze==P)) { /* Assume an edge, not corner */
    n20 = m*n - (size - rank) - (m-1) - m;
    n23 = m*n - (size - rank) - (m-1);
    n26 = m*n - (size - rank) - (m-1) + m;
  }

  if ((xe==M*dof) && (ys==0)) { /* Assume an edge, not corner */
    n2  = rank + m*(n-1) - (m-1) - m*n;
    n11 = rank + m*(n-1) - (m-1);
    n20 = rank + m*(n-1) - (m-1) + m*n;
  }

  if ((xe==M*dof) && (ye==N)) { /* Assume an edge, not corner */
    n8  = rank - m*n +1 - m*n;
    n17 = rank - m*n +1;
    n26 = rank - m*n +1 + m*n;
  }

  if ((ys==0) && (zs==0)) { /* Assume an edge, not corner */
    n0 = size - m + rank -1;
    n1 = size - m + rank;
    n2 = size - m + rank +1;
  }

  if ((ys==0) && (ze==P)) { /* Assume an edge, not corner */
    n18 = m*n - (size - rank) + m*(n-1) -1;
    n19 = m*n - (size - rank) + m*(n-1);
    n20 = m*n - (size - rank) + m*(n-1) +1;
  }

  if ((ye==N) && (zs==0)) { /* Assume an edge, not corner */
    n6 = size - (m*n-rank) - m * (n-1) -1;
    n7 = size - (m*n-rank) - m * (n-1);
    n8 = size - (m*n-rank) - m * (n-1) +1;
  }

  if ((ye==N) && (ze==P)) { /* Assume an edge, not corner */
    n24 = rank - (size-m) -1;
    n25 = rank - (size-m);
    n26 = rank - (size-m) +1;
  }

  /* Check for Corners */
  if ((xs==0)   && (ys==0) && (zs==0)) { n0  = size -1;}
  if ((xs==0)   && (ys==0) && (ze==P)) { n18 = m*n-1;}    
  if ((xs==0)   && (ye==N) && (zs==0)) { n6  = (size-1)-m*(n-1);}
  if ((xs==0)   && (ye==N) && (ze==P)) { n24 = m-1;}
  if ((xe==M*dof) && (ys==0) && (zs==0)) { n2  = size-m;}
  if ((xe==M*dof) && (ys==0) && (ze==P)) { n20 = m*n-m;}
  if ((xe==M*dof) && (ye==N) && (zs==0)) { n8  = size-m*n;}
  if ((xe==M*dof) && (ye==N) && (ze==P)) { n26 = 0;}

  /* Check for when not X,Y, and Z Periodic */

  /* If not X periodic */
  if (!DAXPeriodic(wrap)){
    if (xs==0)   {n0  = n3  = n6  = n9  = n12 = n15 = n18 = n21 = n24 = -2;}
    if (xe==M*dof) {n2  = n5  = n8  = n11 = n14 = n17 = n20 = n23 = n26 = -2;}
  }

  /* If not Y periodic */
  if (!DAYPeriodic(wrap)){
    if (ys==0)   {n0  = n1  = n2  = n9  = n10 = n11 = n18 = n19 = n20 = -2;}
    if (ye==N)   {n6  = n7  = n8  = n15 = n16 = n17 = n24 = n25 = n26 = -2;}
  }

  /* If not Z periodic */
  if (!DAZPeriodic(wrap)){
    if (zs==0)   {n0  = n1  = n2  = n3  = n4  = n5  = n6  = n7  = n8  = -2;}
    if (ze==P)   {n18 = n19 = n20 = n21 = n22 = n23 = n24 = n25 = n26 = -2;}
  }

  nn = 0;

  /* Bottom Level */
  for (k=0; k<s_z; k++) {  
    for (i=1; i<=s_y; i++) {
      if (n0 >= 0) { /* left below */
        x_t = lx[n0 % m]*dof;
        y_t = ly[(n0 % (m*n))/m];
        z_t = lz[n0 / (m*n)];
        s_t = bases[n0] + x_t*y_t*z_t - (s_y-i)*x_t -s_x - (s_z-k-1)*x_t*y_t;
        for (j=0; j<s_x; j++) { idx[nn++] = s_t++;}
      }
      if (n1 >= 0) { /* directly below */
        x_t = x;
        y_t = ly[(n1 % (m*n))/m];
        z_t = lz[n1 / (m*n)];
        s_t = bases[n1] + x_t*y_t*z_t - (s_y+1-i)*x_t - (s_z-k-1)*x_t*y_t;
        for (j=0; j<x_t; j++) { idx[nn++] = s_t++;}
      }
      if (n2 >= 0) { /* right below */
        x_t = lx[n2 % m]*dof;
        y_t = ly[(n2 % (m*n))/m];
        z_t = lz[n2 / (m*n)];
        s_t = bases[n2] + x_t*y_t*z_t - (s_y+1-i)*x_t - (s_z-k-1)*x_t*y_t;
        for (j=0; j<s_x; j++) { idx[nn++] = s_t++;}
      }
    }

    for (i=0; i<y; i++) {
      if (n3 >= 0) { /* directly left */
        x_t = lx[n3 % m]*dof;
        y_t = y;
        z_t = lz[n3 / (m*n)];
        s_t = bases[n3] + (i+1)*x_t - s_x + x_t*y_t*z_t - (s_z-k)*x_t*y_t;
        for (j=0; j<s_x; j++) { idx[nn++] = s_t++;}
      }

      if (n4 >= 0) { /* middle */
        x_t = x;
        y_t = y;
        z_t = lz[n4 / (m*n)];
        s_t = bases[n4] + i*x_t + x_t*y_t*z_t - (s_z-k)*x_t*y_t;
        for (j=0; j<x_t; j++) { idx[nn++] = s_t++;}
      }

      if (n5 >= 0) { /* directly right */
        x_t = lx[n5 % m]*dof;
        y_t = y;
        z_t = lz[n5 / (m*n)];
        s_t = bases[n5] + i*x_t + x_t*y_t*z_t - (s_z-k)*x_t*y_t;
        for (j=0; j<s_x; j++) { idx[nn++] = s_t++;}
      }
    }

    for (i=1; i<=s_y; i++) {
      if (n6 >= 0) { /* left above */
        x_t = lx[n6 % m]*dof;
        y_t = ly[(n6 % (m*n))/m]; 
        z_t = lz[n6 / (m*n)];
        s_t = bases[n6] + i*x_t - s_x + x_t*y_t*z_t - (s_z-k)*x_t*y_t;
        for (j=0; j<s_x; j++) { idx[nn++] = s_t++;}
      }
      if (n7 >= 0) { /* directly above */
        x_t = x;
        y_t = ly[(n7 % (m*n))/m];
        z_t = lz[n7 / (m*n)];
        s_t = bases[n7] + (i-1)*x_t + x_t*y_t*z_t - (s_z-k)*x_t*y_t;
        for (j=0; j<x_t; j++) { idx[nn++] = s_t++;}
      }
      if (n8 >= 0) { /* right above */
        x_t = lx[n8 % m]*dof;
        y_t = ly[(n8 % (m*n))/m];
        z_t = lz[n8 / (m*n)];
        s_t = bases[n8] + (i-1)*x_t + x_t*y_t*z_t - (s_z-k)*x_t*y_t;
        for (j=0; j<s_x; j++) { idx[nn++] = s_t++;}
      }
    }
  }

  /* Middle Level */
  for (k=0; k<z; k++) {  
    for (i=1; i<=s_y; i++) {
      if (n9 >= 0) { /* left below */
        x_t = lx[n9 % m]*dof;
        y_t = ly[(n9 % (m*n))/m];
        z_t = z;
        s_t = bases[n9] - (s_y-i)*x_t -s_x + (k+1)*x_t*y_t;
        for (j=0; j<s_x; j++) { idx[nn++] = s_t++;}
      }
      if (n10 >= 0) { /* directly below */
        x_t = x;
        y_t = ly[(n10 % (m*n))/m];
        z_t = z;
        s_t = bases[n10] - (s_y+1-i)*x_t + (k+1)*x_t*y_t;
        for (j=0; j<x_t; j++) { idx[nn++] = s_t++;}
      }
      if (n11 >= 0) { /* right below */
        x_t = lx[n11 % m]*dof;
        y_t = ly[(n11 % (m*n))/m];
        z_t = z;
        s_t = bases[n11] - (s_y+1-i)*x_t + (k+1)*x_t*y_t;
        for (j=0; j<s_x; j++) { idx[nn++] = s_t++;}
      }
    }

    for (i=0; i<y; i++) {
      if (n12 >= 0) { /* directly left */
        x_t = lx[n12 % m]*dof;
        y_t = y;
        z_t = z;
        s_t = bases[n12] + (i+1)*x_t - s_x + k*x_t*y_t;
        for (j=0; j<s_x; j++) { idx[nn++] = s_t++;}
      }

      /* Interior */
      s_t = bases[rank] + i*x + k*x*y;
      for (j=0; j<x; j++) { idx[nn++] = s_t++;}

      if (n14 >= 0) { /* directly right */
        x_t = lx[n14 % m]*dof;
        y_t = y;
        z_t = z;
        s_t = bases[n14] + i*x_t + k*x_t*y_t;
        for (j=0; j<s_x; j++) { idx[nn++] = s_t++;}
      }
    }

    for (i=1; i<=s_y; i++) {
      if (n15 >= 0) { /* left above */
        x_t = lx[n15 % m]*dof;
        y_t = ly[(n15 % (m*n))/m];
        z_t = z;
        s_t = bases[n15] + i*x_t - s_x + k*x_t*y_t;
        for (j=0; j<s_x; j++) { idx[nn++] = s_t++;}
      }
      if (n16 >= 0) { /* directly above */
        x_t = x;
        y_t = ly[(n16 % (m*n))/m];
        z_t = z;
        s_t = bases[n16] + (i-1)*x_t + k*x_t*y_t;
        for (j=0; j<x_t; j++) { idx[nn++] = s_t++;}
      }
      if (n17 >= 0) { /* right above */
        x_t = lx[n17 % m]*dof;
        y_t = ly[(n17 % (m*n))/m];
        z_t = z;
        s_t = bases[n17] + (i-1)*x_t + k*x_t*y_t;
        for (j=0; j<s_x; j++) { idx[nn++] = s_t++;}
      }
    } 
  }
 
  /* Upper Level */
  for (k=0; k<s_z; k++) {  
    for (i=1; i<=s_y; i++) {
      if (n18 >= 0) { /* left below */
        x_t = lx[n18 % m]*dof;
        y_t = ly[(n18 % (m*n))/m];
        z_t = lz[n18 / (m*n)];
        s_t = bases[n18] - (s_y-i)*x_t -s_x + (k+1)*x_t*y_t;
        for (j=0; j<s_x; j++) { idx[nn++] = s_t++;}
      }
      if (n19 >= 0) { /* directly below */
        x_t = x;
        y_t = ly[(n19 % (m*n))/m];
        z_t = lz[n19 / (m*n)];
        s_t = bases[n19] - (s_y+1-i)*x_t + (k+1)*x_t*y_t;
        for (j=0; j<x_t; j++) { idx[nn++] = s_t++;}
      }
      if (n20 >= 0) { /* right belodof */
        x_t = lx[n20 % m]*dof;
        y_t = ly[(n20 % (m*n))/m];
        z_t = lz[n20 / (m*n)];
        s_t = bases[n20] - (s_y+1-i)*x_t + (k+1)*x_t*y_t;
        for (j=0; j<s_x; j++) { idx[nn++] = s_t++;}
      }
    }

    for (i=0; i<y; i++) {
      if (n21 >= 0) { /* directly left */
        x_t = lx[n21 % m]*dof;
        y_t = y;
        z_t = lz[n21 / (m*n)];
        s_t = bases[n21] + (i+1)*x_t - s_x + k*x_t*y_t;
        for (j=0; j<s_x; j++) { idx[nn++] = s_t++;}
      }

      if (n22 >= 0) { /* middle */
        x_t = x;
        y_t = y;
        z_t = lz[n22 / (m*n)];
        s_t = bases[n22] + i*x_t + k*x_t*y_t;
        for (j=0; j<x_t; j++) { idx[nn++] = s_t++;}
      }

      if (n23 >= 0) { /* directly right */
        x_t = lx[n23 % m]*dof;
        y_t = y;
        z_t = lz[n23 / (m*n)];
        s_t = bases[n23] + i*x_t + k*x_t*y_t;
        for (j=0; j<s_x; j++) { idx[nn++] = s_t++;}
      }
    }

    for (i=1; i<=s_y; i++) {
      if (n24 >= 0) { /* left above */
        x_t = lx[n24 % m]*dof;
        y_t = ly[(n24 % (m*n))/m];
        z_t = lz[n24 / (m*n)];
        s_t = bases[n24] + i*x_t - s_x + k*x_t*y_t;
        for (j=0; j<s_x; j++) { idx[nn++] = s_t++;}
      }
      if (n25 >= 0) { /* directly above */
        x_t = x;
        y_t = ly[(n25 % (m*n))/m];
        z_t = lz[n25 / (m*n)];
        s_t = bases[n25] + (i-1)*x_t + k*x_t*y_t;
        for (j=0; j<x_t; j++) { idx[nn++] = s_t++;}
      }
      if (n26 >= 0) { /* right above */
        x_t = lx[n26 % m]*dof;
        y_t = ly[(n26 % (m*n))/m];
        z_t = lz[n26 / (m*n)];
        s_t = bases[n26] + (i-1)*x_t + k*x_t*y_t;
        for (j=0; j<s_x; j++) { idx[nn++] = s_t++;}
      }
    }
  }

  /* construct the local to local scatter context */
  /* 
      We simply remap the values in the from part of 
    global to local to read from an array with the ghost values 
    rather then from the plan array.
  */
  ierr = VecScatterCopy(gtol,&da->ltol);CHKERRQ(ierr);
  PetscLogObjectParent(da,da->ltol);
  left   = xs - Xs; 
  bottom = ys - Ys; top = bottom + y;
  down   = zs - Zs; up  = down + z;
  count  = x*(top-bottom)*(up-down);
  ierr = PetscMalloc(count*sizeof(int),&idx);CHKERRQ(ierr);
  count  = 0;
  for (i=down; i<up; i++) {
    for (j=bottom; j<top; j++) {
      for (k=0; k<x; k++) {
        idx[count++] = (left+j*(Xe-Xs))+i*(Xe-Xs)*(Ye-Ys) + k;
      }
    }
  }
  ierr = VecScatterRemap(da->ltol,idx,PETSC_NULL);CHKERRQ(ierr); 
  ierr = PetscFree(idx);CHKERRQ(ierr);


  /* 
     Build the natural ordering to PETSc ordering mappings.
  */
  ierr = PetscOptionsHasName(PETSC_NULL,"-da_noao",&flg1);CHKERRQ(ierr);
  if (!flg1) {
    IS  ispetsc,isnatural;
    int *lidx,lict = 0;
    int Nlocal = (da->xe-da->xs)*(da->ye-da->ys)*(da->ze-da->zs);

    ierr = ISCreateStride(comm,Nlocal,da->base,1,&ispetsc);CHKERRQ(ierr);

    ierr = PetscMalloc(Nlocal*sizeof(int),&lidx);CHKERRQ(ierr);
    for (k=zs; k<ze; k++) {
      for (j=ys; j<ye; j++) {
        for (i=xs; i<xe; i++) {
          lidx[lict++] = i + j*M*dof + k*M*N*dof;
        }
      }
    }
    ierr = ISCreateGeneral(comm,Nlocal,lidx,&isnatural);CHKERRQ(ierr);
    ierr = PetscFree(lidx);CHKERRQ(ierr);

    ierr = AOCreateBasicIS(isnatural,ispetsc,&da->ao);CHKERRQ(ierr);
    PetscLogObjectParent(da,da->ao);
    ierr = ISDestroy(ispetsc);CHKERRQ(ierr);
    ierr = ISDestroy(isnatural);CHKERRQ(ierr);
  } else {
    da->ao = PETSC_NULL;
  }

  if (!flx) {
    ierr = PetscMalloc(m*sizeof(int),&flx);CHKERRQ(ierr);
    ierr = PetscMemcpy(flx,lx,m*sizeof(int));CHKERRQ(ierr);
  }
  if (!fly) {
    ierr = PetscMalloc(n*sizeof(int),&fly);CHKERRQ(ierr);
    ierr = PetscMemcpy(fly,ly,n*sizeof(int));CHKERRQ(ierr);
  }
  if (!flz) {
    ierr = PetscMalloc(p*sizeof(int),&flz);CHKERRQ(ierr);
    ierr = PetscMemcpy(flz,lz,p*sizeof(int));CHKERRQ(ierr);
  }
  da->lx = flx;
  da->ly = fly;
  da->lz = flz;

  /*
     Note the following will be removed soon. Since the functionality 
    is replaced by the above. */

  /* Construct the mapping from current global ordering to global
     ordering that would be used if only 1 processor were employed.
     This mapping is intended only for internal use by discrete
     function and matrix viewers.

     Note: At this point, x has already been adjusted for multiple
     degrees of freedom per node.
   */
  ldim = x*y*z;
  ierr = VecGetSize(global,&gdim);CHKERRQ(ierr);
  ierr = PetscMalloc(gdim*sizeof(int),&da->gtog1);CHKERRQ(ierr);
  PetscLogObjectMemory(da,gdim*sizeof(int));
  ierr = PetscMalloc((2*(gdim+ldim))*sizeof(int),&gA);CHKERRQ(ierr);
  gB        = (int *)(gA + ldim);
  gAall     = (int *)(gB + ldim);
  gBall     = (int *)(gAall + gdim);
  /* Compute local parts of global orderings */
  ict = 0;
  for (k=zs; k<ze; k++) {
    for (j=ys; j<ye; j++) {
      for (i=xs; i<xe; i++) {
        /* gA = global number for 1 proc; gB = current global number */
        gA[ict] = i + j*M*dof + k*M*N*dof;
        gB[ict] = start + ict;
        ict++;
      }
    }
  }
  /* Broadcast the orderings */
  ierr = MPI_Allgatherv(gA,ldim,MPI_INT,gAall,ldims,bases,MPI_INT,comm);CHKERRQ(ierr);
  ierr = MPI_Allgatherv(gB,ldim,MPI_INT,gBall,ldims,bases,MPI_INT,comm);CHKERRQ(ierr);
  for (i=0; i<gdim; i++) da->gtog1[gBall[i]] = gAall[i];
  ierr = PetscFree(gA);CHKERRQ(ierr);
  ierr = PetscFree(bases);CHKERRQ(ierr);

  ierr = PetscOptionsHasName(PETSC_NULL,"-da_view",&flg1);CHKERRQ(ierr);
  if (flg1) {ierr = DAView(da,PETSC_VIEWER_STDOUT_(da->comm));CHKERRQ(ierr);}
  ierr = PetscOptionsHasName(PETSC_NULL,"-da_view_draw",&flg1);CHKERRQ(ierr);
  if (flg1) {ierr = DAView(da,PETSC_VIEWER_DRAW_(da->comm));CHKERRQ(ierr);}
  ierr = PetscOptionsHasName(PETSC_NULL,"-help",&flg1);CHKERRQ(ierr);
  if (flg1) {ierr = DAPrintHelp(da);CHKERRQ(ierr);}
  ierr = PetscPublishAll(da);CHKERRQ(ierr);

#if defined(PETSC_HAVE_AMS)
  ierr = PetscObjectComposeFunctionDynamic((PetscObject)global,"AMSSetFieldBlock_C",
         "AMSSetFieldBlock_DA",AMSSetFieldBlock_DA);CHKERRQ(ierr);
  ierr = PetscObjectComposeFunctionDynamic((PetscObject)local,"AMSSetFieldBlock_C",
         "AMSSetFieldBlock_DA",AMSSetFieldBlock_DA);CHKERRQ(ierr);
  if (((PetscObject)global)->amem > -1) {
    ierr = AMSSetFieldBlock_DA(((PetscObject)global)->amem,"values",global);CHKERRQ(ierr);
  }
#endif
  ierr = VecSetOperation(global,VECOP_VIEW,(void*)VecView_MPI_DA);CHKERRQ(ierr);
  ierr = VecSetOperation(global,VECOP_LOADINTOVECTOR,(void*)VecLoadIntoVector_Binary_DA);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

