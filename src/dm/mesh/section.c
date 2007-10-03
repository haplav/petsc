#include "private/meshimpl.h"   /*I      "petscmesh.h"   I*/
#include <petscmesh_viewers.hh>

/* Logging support */
PetscCookie PETSCDM_DLLEXPORT SECTIONREAL_COOKIE = 0;
PetscEvent  SectionReal_View = 0;
PetscCookie PETSCDM_DLLEXPORT SECTIONINT_COOKIE = 0;
PetscEvent  SectionInt_View = 0;
PetscCookie PETSCDM_DLLEXPORT SECTIONPAIR_COOKIE = 0;
PetscEvent  SectionPair_View = 0;

#undef __FUNCT__  
#define __FUNCT__ "SectionRealView_Sieve"
PetscErrorCode SectionRealView_Sieve(SectionReal section, PetscViewer viewer)
{
  PetscTruth     iascii, isbinary, isdraw;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  ierr = PetscTypeCompare((PetscObject) viewer, PETSC_VIEWER_ASCII, &iascii);CHKERRQ(ierr);
  ierr = PetscTypeCompare((PetscObject) viewer, PETSC_VIEWER_BINARY, &isbinary);CHKERRQ(ierr);
  ierr = PetscTypeCompare((PetscObject) viewer, PETSC_VIEWER_DRAW, &isdraw);CHKERRQ(ierr);

  if (iascii){
    ALE::Obj<ALE::Mesh::real_section_type> s;
    ALE::Obj<ALE::Mesh>                    b;
    const char                                   *name;

    ierr = SectionRealGetSection(section, s);CHKERRQ(ierr);
    ierr = SectionRealGetBundle(section, b);CHKERRQ(ierr);
    ierr = PetscObjectGetName((PetscObject) section, &name);CHKERRQ(ierr);
    ierr = SectionView_Sieve_Ascii(b, s, name, viewer);CHKERRQ(ierr);
  } else if (isbinary) {
    SETERRQ(PETSC_ERR_SUP, "Binary viewer not implemented for Section");
  } else if (isdraw){ 
    SETERRQ(PETSC_ERR_SUP, "Draw viewer not implemented for Section");
  } else {
    SETERRQ1(PETSC_ERR_SUP,"Viewer type %s not supported by this section object", viewer->type_name);
  }
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "SectionRealView"
/*@C
   SectionRealView - Views a Section object. 

   Collective on Section

   Input Parameters:
+  section - the Section
-  viewer - an optional visualization context

   Notes:
   The available visualization contexts include
+     PETSC_VIEWER_STDOUT_SELF - standard output (default)
-     PETSC_VIEWER_STDOUT_WORLD - synchronized standard
         output where only the first processor opens
         the file.  All other processors send their 
         data to the first processor to print. 

   You can change the format the section is printed using the 
   option PetscViewerSetFormat().

   The user can open alternative visualization contexts with
+    PetscViewerASCIIOpen() - Outputs section to a specified file
.    PetscViewerBinaryOpen() - Outputs section in binary to a
         specified file; corresponding input uses SectionLoad()
.    PetscViewerDrawOpen() - Outputs section to an X window display

   The user can call PetscViewerSetFormat() to specify the output
   format of ASCII printed objects (when using PETSC_VIEWER_STDOUT_SELF,
   PETSC_VIEWER_STDOUT_WORLD and PetscViewerASCIIOpen).  Available formats include
+    PETSC_VIEWER_ASCII_DEFAULT - default, prints section information
-    PETSC_VIEWER_ASCII_VTK - outputs a VTK file describing the section

   Level: beginner

   Concepts: section^printing
   Concepts: section^saving to disk

.seealso: VecView(), PetscViewerASCIIOpen(), PetscViewerDrawOpen(), PetscViewerBinaryOpen(), PetscViewerCreate()
@*/
PetscErrorCode SectionRealView(SectionReal section, PetscViewer viewer)
{
  PetscErrorCode ierr;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(section, SECTIONREAL_COOKIE, 1);
  PetscValidType(section, 1);
  if (!viewer) {
    ierr = PetscViewerASCIIGetStdout(section->comm,&viewer);CHKERRQ(ierr);
  }
  PetscValidHeaderSpecific(viewer, PETSC_VIEWER_COOKIE, 2);
  PetscCheckSameComm(section, 1, viewer, 2);

  ierr = PetscLogEventBegin(SectionReal_View,0,0,0,0);CHKERRQ(ierr);
  ierr = (*section->ops->view)(section, viewer);CHKERRQ(ierr);
  ierr = PetscLogEventEnd(SectionReal_View,0,0,0,0);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "SectionRealDuplicate"
/*@C
  SectionRealDuplicate - Create an equivalent Section object

  Not collective

  Input Parameter:
. section - the section object

  Output Parameter:
. newSection - the duplicate
 
  Level: advanced

.seealso SectionRealCreate(), SectionRealSetSection()
@*/
PetscErrorCode PETSCDM_DLLEXPORT SectionRealDuplicate(SectionReal section, SectionReal *newSection)
{
  PetscErrorCode ierr;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(section, SECTIONREAL_COOKIE, 1);
  PetscValidPointer(newSection, 2);
  const ALE::Obj<ALE::Mesh::real_section_type>& s = section->s;
  ALE::Obj<ALE::Mesh::real_section_type>        t = new ALE::Mesh::real_section_type(s->comm(), s->debug());

  t->setAtlas(s->getAtlas());
  t->allocateStorage();
  t->copyBC(s);
  ierr = SectionRealCreate(s->comm(), newSection);CHKERRQ(ierr);
  ierr = SectionRealSetSection(*newSection, t);CHKERRQ(ierr);
  ierr = SectionRealSetBundle(*newSection, section->b);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "SectionRealGetSection"
/*@C
  SectionRealGetSection - Gets the internal section object

  Not collective

  Input Parameter:
. section - the section object

  Output Parameter:
. s - the internal section object
 
  Level: advanced

.seealso SectionRealCreate(), SectionRealSetSection()
@*/
PetscErrorCode PETSCDM_DLLEXPORT SectionRealGetSection(SectionReal section, ALE::Obj<ALE::Mesh::real_section_type>& s)
{
  PetscFunctionBegin;
  PetscValidHeaderSpecific(section, SECTIONREAL_COOKIE, 1);
  s = section->s;
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "SectionRealSetSection"
/*@C
  SectionRealSetSection - Sets the internal section object

  Not collective

  Input Parameters:
+ section - the section object
- s - the internal section object
 
  Level: advanced

.seealso SectionRealCreate(), SectionRealGetSection()
@*/
PetscErrorCode PETSCDM_DLLEXPORT SectionRealSetSection(SectionReal section, const ALE::Obj<ALE::Mesh::real_section_type>& s)
{
  PetscErrorCode ierr;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(section, SECTIONREAL_COOKIE, 1);
  if (!s.isNull()) {ierr = PetscObjectSetName((PetscObject) section, s->getName().c_str());CHKERRQ(ierr);}
  section->s = s;
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "SectionRealGetBundle"
/*@C
  SectionRealGetBundle - Gets the section bundle

  Not collective

  Input Parameter:
. section - the section object

  Output Parameter:
. b - the section bundle
 
  Level: advanced

.seealso SectionRealCreate(), SectionRealGetSection(), SectionRealSetSection()
@*/
PetscErrorCode PETSCDM_DLLEXPORT SectionRealGetBundle(SectionReal section, ALE::Obj<ALE::Mesh>& b)
{
  PetscFunctionBegin;
  PetscValidHeaderSpecific(section, SECTIONREAL_COOKIE, 1);
  b = section->b;
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "SectionRealSetBundle"
/*@C
  SectionRealSetBundle - Sets the section bundle

  Not collective

  Input Parameters:
+ section - the section object
- b - the section bundle
 
  Level: advanced

.seealso SectionRealCreate(), SectionRealGetSection(), SectionRealSetSection()
@*/
PetscErrorCode PETSCDM_DLLEXPORT SectionRealSetBundle(SectionReal section, const ALE::Obj<ALE::Mesh>& b)
{
  PetscFunctionBegin;
  PetscValidHeaderSpecific(section, SECTIONREAL_COOKIE, 1);
  section->b = b;
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "SectionRealCreate"
/*@C
  SectionRealCreate - Creates a Section object, used to manage data for an unstructured problem
  described by a Sieve.

  Collective on MPI_Comm

  Input Parameter:
. comm - the processors that will share the global section

  Output Parameters:
. section - the section object

  Level: advanced

.seealso SectionRealDestroy(), SectionRealView()
@*/
PetscErrorCode PETSCDM_DLLEXPORT SectionRealCreate(MPI_Comm comm, SectionReal *section)
{
  PetscErrorCode ierr;
  SectionReal    s;

  PetscFunctionBegin;
  PetscValidPointer(section,2);
  *section = PETSC_NULL;

  ierr = PetscHeaderCreate(s,_p_SectionReal,struct _SectionRealOps,SECTIONREAL_COOKIE,0,"SectionReal",comm,SectionRealDestroy,0);CHKERRQ(ierr);
  s->ops->view     = SectionRealView_Sieve;
  s->ops->restrict = SectionRealRestrict;
  s->ops->update   = SectionRealUpdate;

  ierr = PetscObjectChangeTypeName((PetscObject) s, "sieve");CHKERRQ(ierr);

  s->s             = new ALE::Mesh::real_section_type(comm);
  *section = s;
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "SectionRealDestroy"
/*@C
  SectionRealDestroy - Destroys a section.

  Collective on Section

  Input Parameter:
. section - the section object

  Level: advanced

.seealso SectionRealCreate(), SectionRealView()
@*/
PetscErrorCode PETSCDM_DLLEXPORT SectionRealDestroy(SectionReal section)
{
  PetscErrorCode ierr;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(section, SECTIONREAL_COOKIE, 1);
  if (--((PetscObject)section)->refct > 0) PetscFunctionReturn(0);
  section->s = PETSC_NULL;
  ierr = PetscHeaderDestroy(section);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "SectionRealDistribute"
/*@C
  SectionRealDistribute - Distributes the sections.

  Not Collective

  Input Parameters:
+ serialSection - The original Section object
- parallelMesh - The parallel Mesh

  Output Parameter:
. parallelSection - The distributed Section object

  Level: intermediate

.keywords: mesh, section, distribute
.seealso: MeshCreate()
@*/
PetscErrorCode SectionRealDistribute(SectionReal serialSection, Mesh parallelMesh, SectionReal *parallelSection)
{
  ALE::Obj<ALE::Mesh::real_section_type> oldSection;
  ALE::Obj<ALE::Mesh>               m;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  ierr = SectionRealGetSection(serialSection, oldSection);CHKERRQ(ierr);
  ierr = MeshGetMesh(parallelMesh, m);CHKERRQ(ierr);
  ierr = SectionRealCreate(oldSection->comm(), parallelSection);CHKERRQ(ierr);
  ALE::Obj<ALE::Mesh::real_section_type> newSection = ALE::Distribution<ALE::Mesh>::distributeSection(oldSection, m, m->getDistSendOverlap(), m->getDistRecvOverlap());
  ierr = SectionRealSetSection(*parallelSection, newSection);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "SectionRealRestrict"
/*@C
  SectionRealRestrict - Restricts the SectionReal to a subset of the topology, returning an array of values.

  Not collective

  Input Parameters:
+ section - the section object
- point - the Sieve point

  Output Parameter:
. values - The values associated with the submesh

  Level: advanced

.seealso SectionUpdate(), SectionCreate(), SectionView()
@*/
PetscErrorCode PETSCDM_DLLEXPORT SectionRealRestrict(SectionReal section, PetscInt point, PetscScalar *values[])
{
  PetscFunctionBegin;
  PetscValidHeaderSpecific(section, SECTIONREAL_COOKIE, 1);
  PetscValidScalarPointer(values,3);
  *values = (PetscScalar *) section->b->restrict(section->s, point);
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "SectionRealUpdate"
/*@C
  SectionRealUpdate - Updates the array of values associated to a subset of the topology in this Section.

  Not collective

  Input Parameters:
+ section - the section object
. point - the Sieve point
- values - The values associated with the submesh

  Level: advanced

.seealso SectionRealRestrict(), SectionRealCreate(), SectionRealView()
@*/
PetscErrorCode PETSCDM_DLLEXPORT SectionRealUpdate(SectionReal section, PetscInt point, const PetscScalar values[])
{
  PetscFunctionBegin;
  PetscValidHeaderSpecific(section, SECTIONREAL_COOKIE, 1);
  PetscValidScalarPointer(values,3);
  section->b->update(section->s, point, values);
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "SectionRealUpdateAdd"
/*@C
  SectionRealUpdateAdd - Updates the array of values associated to a subset of the topology in this Section.

  Not collective

  Input Parameters:
+ section - the section object
. point - the Sieve point
- values - The values associated with the submesh

  Level: advanced

.seealso SectionRealRestrict(), SectionRealCreate(), SectionRealView()
@*/
PetscErrorCode PETSCDM_DLLEXPORT SectionRealUpdateAdd(SectionReal section, PetscInt point, const PetscScalar values[])
{
  PetscFunctionBegin;
  PetscValidHeaderSpecific(section, SECTIONREAL_COOKIE, 1);
  PetscValidScalarPointer(values,3);
  section->b->updateAdd(section->s, point, values);
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "SectionRealComplete"
/*@C
  SectionRealComplete - Exchanges data across the mesh overlap.

  Not collective

  Input Parameter:
. section - the section object

  Level: advanced

.seealso SectionRealRestrict(), SectionRealCreate(), SectionRealView()
@*/
PetscErrorCode SectionRealComplete(SectionReal section)
{
  Obj<ALE::Mesh::real_section_type> s;
  Obj<ALE::Mesh>                    b;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  ierr = SectionRealGetSection(section, s);CHKERRQ(ierr);
  ierr = SectionRealGetBundle(section, b);CHKERRQ(ierr);
  ALE::Distribution<ALE::Mesh>::completeSection(b, s);
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "SectionRealZero"
/*@C
  SectionRealZero - Zero out the entries

  Not collective

  Input Parameter:
. section - the section object

  Level: advanced

.seealso SectionRealRestrict(), SectionRealCreate(), SectionRealView()
@*/
PetscErrorCode SectionRealZero(SectionReal section)
{
  Obj<ALE::Mesh::real_section_type> s;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  ierr = SectionRealGetSection(section, s);CHKERRQ(ierr);
  s->zero();CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "SectionRealSetFiberDimension"
/*@C
  SectionRealSetFiberDimension - Set the size of the vector space attached to the point

  Not collective

  Input Parameters:
+ section - the section object
. point - the Sieve point
- size - The fiber dimension

  Level: advanced

.seealso SectionRealRestrict(), SectionRealCreate(), SectionRealView()
@*/
PetscErrorCode PETSCDM_DLLEXPORT SectionRealSetFiberDimension(SectionReal section, PetscInt point, const PetscInt size)
{
  PetscFunctionBegin;
  PetscValidHeaderSpecific(section, SECTIONPAIR_COOKIE, 1);
  section->s->setFiberDimension(point, size);
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "SectionRealAllocate"
/*@C
  SectionRealAllocate - Allocate storage for this section

  Not collective

  Input Parameter:
. section - the section object

  Level: advanced

.seealso SectionRealRestrict(), SectionRealCreate(), SectionRealView()
@*/
PetscErrorCode PETSCDM_DLLEXPORT SectionRealAllocate(SectionReal section)
{
  PetscFunctionBegin;
  PetscValidHeaderSpecific(section, SECTIONPAIR_COOKIE, 1);
  section->b->allocate(section->s);
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "SectionRealCreateLocalVector"
/*@C
  SectionRealCreateLocalVector - Creates a vector with the local piece of the Section

  Collective on Mesh

  Input Parameter:
. section - the Section  

  Output Parameter:
. localVec - the local vector

  Level: advanced

.seealso MeshDestroy(), MeshCreate()
@*/
PetscErrorCode PETSCDM_DLLEXPORT SectionRealCreateLocalVector(SectionReal section, Vec *localVec)
{
  ALE::Obj<ALE::Mesh::real_section_type> s;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  ierr = SectionRealGetSection(section, s);CHKERRQ(ierr);
  ierr = VecCreateSeqWithArray(PETSC_COMM_SELF, s->getStorageSize(), s->restrict(), localVec);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "SectionRealToVec"
/*@C
  SectionRealToVec - Maps the given section to a Vec

  Collective on Section

  Input Parameters:
+ section - the real Section
- mesh - The Mesh

  Output Parameter:
. vec - the Vec 

  Level: intermediate

.seealso VecCreate(), SectionRealCreate()
@*/
PetscErrorCode PETSCDM_DLLEXPORT SectionRealToVec(SectionReal section, Mesh mesh, ScatterMode mode, Vec vec)
{
  Vec            localVec;
  VecScatter     scatter;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(section, SECTIONREAL_COOKIE, 1);
  ierr = SectionRealCreateLocalVector(section, &localVec);CHKERRQ(ierr);
  ierr = MeshGetGlobalScatter(mesh, &scatter);CHKERRQ(ierr);
  if (mode == SCATTER_FORWARD) {
    ierr = VecScatterBegin(scatter, localVec, vec, INSERT_VALUES, mode);CHKERRQ(ierr);
    ierr = VecScatterEnd(scatter, localVec, vec, INSERT_VALUES, mode);CHKERRQ(ierr);
  } else {
    ierr = VecScatterBegin(scatter, vec, localVec, INSERT_VALUES, mode);CHKERRQ(ierr);
    ierr = VecScatterEnd(scatter, vec, localVec, INSERT_VALUES, mode);CHKERRQ(ierr);
  }
  ierr = VecDestroy(localVec);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

PetscErrorCode PETSCDM_DLLEXPORT SectionRealToVec(SectionReal section, Mesh mesh, VecScatter scatter, ScatterMode mode, Vec vec)
{
  Vec            localVec;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(section, SECTIONREAL_COOKIE, 1);
  ierr = SectionRealCreateLocalVector(section, &localVec);CHKERRQ(ierr);
  if (mode == SCATTER_FORWARD) {
    ierr = VecScatterBegin(scatter, localVec, vec, INSERT_VALUES, mode);CHKERRQ(ierr);
    ierr = VecScatterEnd(scatter, localVec, vec, INSERT_VALUES, mode);CHKERRQ(ierr);
  } else {
    ierr = VecScatterBegin(scatter, vec, localVec, INSERT_VALUES, mode);CHKERRQ(ierr);
    ierr = VecScatterEnd(scatter, vec, localVec, INSERT_VALUES, mode);CHKERRQ(ierr);
  }
  ierr = VecDestroy(localVec);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "SectionRealClear"
/*@C
  SectionRealClear - Dellocate storage for this section

  Not collective

  Input Parameter:
. section - the section object

  Level: advanced

.seealso SectionRealRestrict(), SectionRealCreate(), SectionRealView()
@*/
PetscErrorCode PETSCDM_DLLEXPORT SectionRealClear(SectionReal section)
{
  PetscFunctionBegin;
  PetscValidHeaderSpecific(section, SECTIONREAL_COOKIE, 1);
  section->s->clear();
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "SectionRealNorm"
/*@C
  SectionRealNorm - Computes the vector norm.

  Collective on Section

  Input Parameters:
+  section - the real Section
-  type - one of NORM_1, NORM_2, NORM_INFINITY.  Also available
          NORM_1_AND_2, which computes both norms and stores them
          in a two element array.

  Output Parameter:
. val - the norm 

  Notes:
$     NORM_1 denotes sum_i |x_i|
$     NORM_2 denotes sqrt(sum_i (x_i)^2)
$     NORM_INFINITY denotes max_i |x_i|

  Level: intermediate

.seealso VecNorm(), SectionRealCreate()
@*/
PetscErrorCode PETSCDM_DLLEXPORT SectionRealNorm(SectionReal section, Mesh mesh, NormType type, PetscReal *val)
{
  Obj<ALE::Mesh> m;
  Obj<ALE::Mesh::real_section_type> s;
  Vec            v;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(section, SECTIONREAL_COOKIE, 1);
  ierr = MeshGetMesh(mesh, m);CHKERRQ(ierr);
  ierr = SectionRealGetSection(section, s);CHKERRQ(ierr);
  const ALE::Obj<ALE::Mesh::order_type>& order = m->getFactory()->getGlobalOrder(m, s->getName(), s);
  ierr = VecCreate(m->comm(), &v);CHKERRQ(ierr);
  ierr = VecSetSizes(v, order->getLocalSize(), order->getGlobalSize());CHKERRQ(ierr);
  ierr = VecSetFromOptions(v);CHKERRQ(ierr);
  ierr = SectionRealToVec(section, mesh, SCATTER_FORWARD, v);CHKERRQ(ierr);
  ierr = VecNorm(v, type, val);CHKERRQ(ierr);
  ierr = VecDestroy(v);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "SectionRealAXPY"
/*@C
  SectionRealAXPY - 

  Collective on Section

  Input Parameters:
+  section - the real Section
.  alpha - a scalar
-  X - the other real Section

  Output Parameter:
. section - the difference 

  Level: intermediate

.seealso VecNorm(), SectionRealCreate()
@*/
PetscErrorCode PETSCDM_DLLEXPORT SectionRealAXPY(SectionReal section, Mesh mesh, PetscScalar alpha, SectionReal X)
{
  Obj<ALE::Mesh> m;
  Obj<ALE::Mesh::real_section_type> s;
  Obj<ALE::Mesh::real_section_type> sX;
  Vec            v, x;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(section, SECTIONREAL_COOKIE, 1);
  ierr = MeshGetMesh(mesh, m);CHKERRQ(ierr);
  ierr = SectionRealGetSection(section, s);CHKERRQ(ierr);
  ierr = SectionRealGetSection(X, sX);CHKERRQ(ierr);
  const ALE::Obj<ALE::Mesh::order_type>& order = m->getFactory()->getGlobalOrder(m, s->getName(), s);
  ierr = VecCreate(m->comm(), &v);CHKERRQ(ierr);
  ierr = VecSetSizes(v, order->getLocalSize(), order->getGlobalSize());CHKERRQ(ierr);
  ierr = VecSetFromOptions(v);CHKERRQ(ierr);
  ierr = VecDuplicate(v, &x);CHKERRQ(ierr);
  ierr = SectionRealToVec(section, mesh, SCATTER_FORWARD, v);CHKERRQ(ierr);
  ierr = SectionRealToVec(X,       mesh, SCATTER_FORWARD, x);CHKERRQ(ierr);
  ierr = VecAXPY(v, alpha, x);CHKERRQ(ierr);
  ierr = SectionRealToVec(section, mesh, SCATTER_REVERSE, v);CHKERRQ(ierr);
  ierr = VecDestroy(v);CHKERRQ(ierr);
  ierr = VecDestroy(x);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "MeshGetVertexSectionReal"
/*@C
  MeshGetVertexSectionReal - Create a Section over the vertices with the specified fiber dimension

  Collective on Mesh

  Input Parameters:
+ mesh - The Mesh object
- fiberDim - The section name

  Output Parameter:
. section - The section

  Level: intermediate

.keywords: mesh, section, vertex
.seealso: MeshCreate(), SectionRealCreate()
@*/
PetscErrorCode MeshGetVertexSectionReal(Mesh mesh, PetscInt fiberDim, SectionReal *section)
{
  ALE::Obj<ALE::Mesh> m;
  ALE::Obj<ALE::Mesh::real_section_type> s;
  PetscErrorCode      ierr;

  PetscFunctionBegin;
  ierr = MeshGetMesh(mesh, m);CHKERRQ(ierr);
  ierr = SectionRealCreate(m->comm(), section);CHKERRQ(ierr);
  ierr = SectionRealSetBundle(*section, m);CHKERRQ(ierr);
  ierr = SectionRealGetSection(*section, s);CHKERRQ(ierr);
  s->setFiberDimension(m->depthStratum(0), fiberDim);
  m->allocate(s);
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "MeshGetCellSectionReal"
/*@C
  MeshGetCellSectionReal - Create a Section over the cells with the specified fiber dimension

  Collective on Mesh

  Input Parameters:
+ mesh - The Mesh object
- fiberDim - The section name

  Output Parameter:
. section - The section

  Level: intermediate

.keywords: mesh, section, cell
.seealso: MeshCreate(), SectionRealCreate()
@*/
PetscErrorCode MeshGetCellSectionReal(Mesh mesh, PetscInt fiberDim, SectionReal *section)
{
  ALE::Obj<ALE::Mesh> m;
  ALE::Obj<ALE::Mesh::real_section_type> s;
  PetscErrorCode      ierr;

  PetscFunctionBegin;
  ierr = MeshGetMesh(mesh, m);CHKERRQ(ierr);
  ierr = SectionRealCreate(m->comm(), section);CHKERRQ(ierr);
  ierr = SectionRealSetBundle(*section, m);CHKERRQ(ierr);
  ierr = SectionRealGetSection(*section, s);CHKERRQ(ierr);
  s->setFiberDimension(m->heightStratum(0), fiberDim);
  m->allocate(s);
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "MeshCreateGlobalRealVector"
/*@C
    MeshCreateGlobalRealVector - Creates a vector of the correct size to be gathered into 
        by the mesh.

    Collective on Mesh

    Input Parameters:
+    mesh - the mesh object
-    section - The SectionReal

    Output Parameters:
.   gvec - the global vector

    Level: advanced

.seealso MeshDestroy(), MeshCreate(), MeshCreateGlobalVector()

@*/
PetscErrorCode PETSCDM_DLLEXPORT MeshCreateGlobalRealVector(Mesh mesh, SectionReal section, Vec *gvec)
{
  ALE::Obj<ALE::Mesh> m;
  ALE::Obj<ALE::Mesh::real_section_type> s;
  const char    *name;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  ierr = MeshGetMesh(mesh, m);CHKERRQ(ierr);
  ierr = SectionRealGetSection(section, s);CHKERRQ(ierr);
  ierr = PetscObjectGetName((PetscObject) section, &name);CHKERRQ(ierr);
  const ALE::Obj<ALE::Mesh::order_type>& order = m->getFactory()->getGlobalOrder(m, name, s);

  ierr = VecCreate(m->comm(), gvec);CHKERRQ(ierr);
  ierr = VecSetSizes(*gvec, order->getLocalSize(), order->getGlobalSize());CHKERRQ(ierr);
  ierr = VecSetFromOptions(*gvec);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "SectionIntView_Sieve"
PetscErrorCode SectionIntView_Sieve(SectionInt section, PetscViewer viewer)
{
  PetscTruth     iascii, isbinary, isdraw;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  ierr = PetscTypeCompare((PetscObject) viewer, PETSC_VIEWER_ASCII, &iascii);CHKERRQ(ierr);
  ierr = PetscTypeCompare((PetscObject) viewer, PETSC_VIEWER_BINARY, &isbinary);CHKERRQ(ierr);
  ierr = PetscTypeCompare((PetscObject) viewer, PETSC_VIEWER_DRAW, &isdraw);CHKERRQ(ierr);

  if (iascii){
    ALE::Obj<ALE::Mesh::int_section_type> s;
    ALE::Obj<ALE::Mesh>                   b;
    const char                                  *name;

    ierr = SectionIntGetSection(section, s);CHKERRQ(ierr);
    ierr = SectionIntGetBundle(section, b);CHKERRQ(ierr);
    ierr = PetscObjectGetName((PetscObject) section, &name);CHKERRQ(ierr);
    ierr = SectionView_Sieve_Ascii(b, s, name, viewer);CHKERRQ(ierr);
  } else if (isbinary) {
    SETERRQ(PETSC_ERR_SUP, "Binary viewer not implemented for Section");
  } else if (isdraw){ 
    SETERRQ(PETSC_ERR_SUP, "Draw viewer not implemented for Section");
  } else {
    SETERRQ1(PETSC_ERR_SUP,"Viewer type %s not supported by this section object", viewer->type_name);
  }
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "SectionIntView"
/*@C
   SectionIntView - Views a Section object. 

   Collective on Section

   Input Parameters:
+  section - the Section
-  viewer - an optional visualization context

   Notes:
   The available visualization contexts include
+     PETSC_VIEWER_STDOUT_SELF - standard output (default)
-     PETSC_VIEWER_STDOUT_WORLD - synchronized standard
         output where only the first processor opens
         the file.  All other processors send their 
         data to the first processor to print. 

   You can change the format the section is printed using the 
   option PetscViewerSetFormat().

   The user can open alternative visualization contexts with
+    PetscViewerASCIIOpen() - Outputs section to a specified file
.    PetscViewerBinaryOpen() - Outputs section in binary to a
         specified file; corresponding input uses SectionLoad()
.    PetscViewerDrawOpen() - Outputs section to an X window display

   The user can call PetscViewerSetFormat() to specify the output
   format of ASCII printed objects (when using PETSC_VIEWER_STDOUT_SELF,
   PETSC_VIEWER_STDOUT_WORLD and PetscViewerASCIIOpen).  Available formats include
+    PETSC_VIEWER_ASCII_DEFAULT - default, prints section information
-    PETSC_VIEWER_ASCII_VTK - outputs a VTK file describing the section

   Level: beginner

   Concepts: section^printing
   Concepts: section^saving to disk

.seealso: VecView(), PetscViewerASCIIOpen(), PetscViewerDrawOpen(), PetscViewerBinaryOpen(), PetscViewerCreate()
@*/
PetscErrorCode SectionIntView(SectionInt section, PetscViewer viewer)
{
  PetscErrorCode ierr;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(section, SECTIONINT_COOKIE, 1);
  PetscValidType(section, 1);
  if (!viewer) {
    ierr = PetscViewerASCIIGetStdout(section->comm,&viewer);CHKERRQ(ierr);
  }
  PetscValidHeaderSpecific(viewer, PETSC_VIEWER_COOKIE, 2);
  PetscCheckSameComm(section, 1, viewer, 2);

  ierr = PetscLogEventBegin(SectionInt_View,0,0,0,0);CHKERRQ(ierr);
  ierr = (*section->ops->view)(section, viewer);CHKERRQ(ierr);
  ierr = PetscLogEventEnd(SectionInt_View,0,0,0,0);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "SectionIntGetSection"
/*@C
  SectionIntGetSection - Gets the internal section object

  Not collective

  Input Parameter:
. section - the section object

  Output Parameter:
. s - the internal section object
 
  Level: advanced

.seealso SectionIntCreate(), SectionIntSetSection()
@*/
PetscErrorCode PETSCDM_DLLEXPORT SectionIntGetSection(SectionInt section, ALE::Obj<ALE::Mesh::int_section_type>& s)
{
  PetscFunctionBegin;
  PetscValidHeaderSpecific(section, SECTIONINT_COOKIE, 1);
  s = section->s;
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "SectionIntSetSection"
/*@C
  SectionIntSetSection - Sets the internal section object

  Not collective

  Input Parameters:
+ section - the section object
- s - the internal section object
 
  Level: advanced

.seealso SectionIntCreate(), SectionIntGetSection()
@*/
PetscErrorCode PETSCDM_DLLEXPORT SectionIntSetSection(SectionInt section, const ALE::Obj<ALE::Mesh::int_section_type>& s)
{
  PetscErrorCode ierr;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(section, SECTIONINT_COOKIE, 1);
  if (!s.isNull()) {ierr = PetscObjectSetName((PetscObject) section, s->getName().c_str());CHKERRQ(ierr);}
  section->s = s;
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "SectionIntGetBundle"
/*@C
  SectionIntGetBundle - Gets the section bundle

  Not collective

  Input Parameter:
. section - the section object

  Output Parameter:
. b - the section bundle
 
  Level: advanced

.seealso SectionIntCreate(), SectionIntGetSection(), SectionIntSetSection()
@*/
PetscErrorCode PETSCDM_DLLEXPORT SectionIntGetBundle(SectionInt section, ALE::Obj<ALE::Mesh>& b)
{
  PetscFunctionBegin;
  PetscValidHeaderSpecific(section, SECTIONINT_COOKIE, 1);
  b = section->b;
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "SectionIntSetBundle"
/*@C
  SectionIntSetBundle - Sets the section bundle

  Not collective

  Input Parameters:
+ section - the section object
- b - the section bundle
 
  Level: advanced

.seealso SectionIntCreate(), SectionIntGetSection(), SectionIntSetSection()
@*/
PetscErrorCode PETSCDM_DLLEXPORT SectionIntSetBundle(SectionInt section, const ALE::Obj<ALE::Mesh>& b)
{
  PetscFunctionBegin;
  PetscValidHeaderSpecific(section, SECTIONINT_COOKIE, 1);
  section->b = b;
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "SectionIntCreate"
/*@C
  SectionIntCreate - Creates a Section object, used to manage data for an unstructured problem
  described by a Sieve.

  Collective on MPI_Comm

  Input Parameter:
. comm - the processors that will share the global section

  Output Parameters:
. section - the section object

  Level: advanced

.seealso SectionIntDestroy(), SectionIntView()
@*/
PetscErrorCode PETSCDM_DLLEXPORT SectionIntCreate(MPI_Comm comm, SectionInt *section)
{
  PetscErrorCode ierr;
  SectionInt    s;

  PetscFunctionBegin;
  PetscValidPointer(section,2);
  *section = PETSC_NULL;

  ierr = PetscHeaderCreate(s,_p_SectionInt,struct _SectionIntOps,SECTIONINT_COOKIE,0,"SectionInt",comm,SectionIntDestroy,0);CHKERRQ(ierr);
  s->ops->view     = SectionIntView_Sieve;
  s->ops->restrict = SectionIntRestrict;
  s->ops->update   = SectionIntUpdate;

  ierr = PetscObjectChangeTypeName((PetscObject) s, "sieve");CHKERRQ(ierr);

  s->s             = new ALE::Mesh::int_section_type(comm);
  *section = s;
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "SectionIntDestroy"
/*@C
  SectionIntDestroy - Destroys a section.

  Collective on Section

  Input Parameter:
. section - the section object

  Level: advanced

.seealso SectionIntCreate(), SectionIntView()
@*/
PetscErrorCode PETSCDM_DLLEXPORT SectionIntDestroy(SectionInt section)
{
  PetscErrorCode ierr;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(section, SECTIONINT_COOKIE, 1);
  if (--((PetscObject)section)->refct > 0) PetscFunctionReturn(0);
  section->s = PETSC_NULL;
  ierr = PetscHeaderDestroy(section);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "SectionIntDistribute"
/*@C
  SectionIntDistribute - Distributes the sections.

  Not Collective

  Input Parameters:
+ serialSection - The original Section object
- parallelMesh - The parallel Mesh

  Output Parameter:
. parallelSection - The distributed Section object

  Level: intermediate

.keywords: mesh, section, distribute
.seealso: MeshCreate()
@*/
PetscErrorCode SectionIntDistribute(SectionInt serialSection, Mesh parallelMesh, SectionInt *parallelSection)
{
  ALE::Obj<ALE::Mesh::int_section_type> oldSection;
  ALE::Obj<ALE::Mesh> m;
  PetscErrorCode      ierr;

  PetscFunctionBegin;
  ierr = SectionIntGetSection(serialSection, oldSection);CHKERRQ(ierr);
  ierr = MeshGetMesh(parallelMesh, m);CHKERRQ(ierr);
  ierr = SectionIntCreate(oldSection->comm(), parallelSection);CHKERRQ(ierr);
  ALE::Obj<ALE::Mesh::int_section_type> newSection = ALE::Distribution<ALE::Mesh>::distributeSection(oldSection, m, m->getDistSendOverlap(), m->getDistRecvOverlap());
  ierr = SectionIntSetSection(*parallelSection, newSection);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "SectionIntRestrict"
/*@C
  SectionIntRestrict - Restricts the SectionInt to a subset of the topology, returning an array of values.

  Not collective

  Input Parameters:
+ section - the section object
- point - the Sieve point

  Output Parameter:
. values - The values associated with the submesh

  Level: advanced

.seealso SectionIntUpdate(), SectionIntCreate(), SectionIntView()
@*/
PetscErrorCode PETSCDM_DLLEXPORT SectionIntRestrict(SectionInt section, PetscInt point, PetscInt *values[])
{
  PetscFunctionBegin;
  PetscValidHeaderSpecific(section, SECTIONINT_COOKIE, 1);
  PetscValidPointer(values,3);
  *values = (PetscInt *) section->b->restrict(section->s, point);
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "SectionIntUpdate"
/*@C
  SectionIntUpdate - Updates the array of values associated to a subset of the topology in this Section.

  Not collective

  Input Parameters:
+ section - the section object
. point - the Sieve point
- values - The values associated with the submesh

  Level: advanced

.seealso SectionIntRestrict(), SectionIntCreate(), SectionIntView()
@*/
PetscErrorCode PETSCDM_DLLEXPORT SectionIntUpdate(SectionInt section, PetscInt point, const PetscInt values[])
{
  PetscFunctionBegin;
  PetscValidHeaderSpecific(section, SECTIONINT_COOKIE, 1);
  PetscValidIntPointer(values,3);
  section->b->update(section->s, point, values);
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "SectionIntUpdateAdd"
/*@C
  SectionIntUpdateAdd - Updates the array of values associated to a subset of the topology in this Section.

  Not collective

  Input Parameters:
+ section - the section object
. point - the Sieve point
- values - The values associated with the submesh

  Level: advanced

.seealso SectionIntRestrict(), SectionIntCreate(), SectionIntView()
@*/
PetscErrorCode PETSCDM_DLLEXPORT SectionIntUpdateAdd(SectionInt section, PetscInt point, const PetscInt values[])
{
  PetscFunctionBegin;
  PetscValidHeaderSpecific(section, SECTIONREAL_COOKIE, 1);
  PetscValidScalarPointer(values,3);
  section->b->updateAdd(section->s, point, values);
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "SectionIntComplete"
/*@C
  SectionIntComplete - Exchanges data across the mesh overlap.

  Not collective

  Input Parameter:
. section - the section object

  Level: advanced

.seealso SectionIntRestrict(), SectionIntCreate(), SectionIntView()
@*/
PetscErrorCode SectionIntComplete(SectionInt section)
{
  Obj<ALE::Mesh::int_section_type> s;
  Obj<ALE::Mesh>                   b;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  ierr = SectionIntGetSection(section, s);CHKERRQ(ierr);
  ierr = SectionIntGetBundle(section, b);CHKERRQ(ierr);
  ALE::Distribution<ALE::Mesh>::completeSection(b, s);
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "SectionIntSetFiberDimension"
/*@C
  SectionIntSetFiberDimension - Set the size of the vector space attached to the point

  Not collective

  Input Parameters:
+ section - the section object
. point - the Sieve point
- size - The fiber dimension

  Level: advanced

.seealso SectionIntRestrict(), SectionIntCreate(), SectionIntView()
@*/
PetscErrorCode PETSCDM_DLLEXPORT SectionIntSetFiberDimension(SectionInt section, PetscInt point, const PetscInt size)
{
  PetscFunctionBegin;
  PetscValidHeaderSpecific(section, SECTIONPAIR_COOKIE, 1);
  section->s->setFiberDimension(point, size);
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "SectionIntAllocate"
/*@C
  SectionIntAllocate - Allocate storage for this section

  Not collective

  Input Parameter:
. section - the section object

  Level: advanced

.seealso SectionIntRestrict(), SectionIntCreate(), SectionIntView()
@*/
PetscErrorCode PETSCDM_DLLEXPORT SectionIntAllocate(SectionInt section)
{
  PetscFunctionBegin;
  PetscValidHeaderSpecific(section, SECTIONPAIR_COOKIE, 1);
  section->b->allocate(section->s);
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "SectionIntClear"
/*@C
  SectionIntClear - Dellocate storage for this section

  Not collective

  Input Parameter:
. section - the section object

  Level: advanced

.seealso SectionIntRestrict(), SectionIntCreate(), SectionIntView()
@*/
PetscErrorCode PETSCDM_DLLEXPORT SectionIntClear(SectionInt section)
{
  PetscFunctionBegin;
  PetscValidHeaderSpecific(section, SECTIONPAIR_COOKIE, 1);
  section->s->clear();
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "MeshGetVertexSectionInt"
/*@C
  MeshGetVertexSectionInt - Create a Section over the vertices with the specified fiber dimension

  Collective on Mesh

  Input Parameters:
+ mesh - The Mesh object
- fiberDim - The section name

  Output Parameter:
. section - The section

  Level: intermediate

.keywords: mesh, section, vertex
.seealso: MeshCreate(), SectionIntCreate()
@*/
PetscErrorCode MeshGetVertexSectionInt(Mesh mesh, PetscInt fiberDim, SectionInt *section)
{
  ALE::Obj<ALE::Mesh> m;
  ALE::Obj<ALE::Mesh::int_section_type> s;
  PetscErrorCode      ierr;

  PetscFunctionBegin;
  ierr = MeshGetMesh(mesh, m);CHKERRQ(ierr);
  ierr = SectionIntCreate(m->comm(), section);CHKERRQ(ierr);
  ierr = SectionIntSetBundle(*section, m);CHKERRQ(ierr);
  ierr = SectionIntGetSection(*section, s);CHKERRQ(ierr);
  s->setFiberDimension(m->depthStratum(0), fiberDim);
  m->allocate(s);
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "MeshGetCellSectionInt"
/*@C
  MeshGetCellSectionInt - Create a Section over the cells with the specified fiber dimension

  Collective on Mesh

  Input Parameters:
+ mesh - The Mesh object
- fiberDim - The section name

  Output Parameter:
. section - The section

  Level: intermediate

.keywords: mesh, section, cell
.seealso: MeshCreate(), SectionIntCreate()
@*/
PetscErrorCode MeshGetCellSectionInt(Mesh mesh, PetscInt fiberDim, SectionInt *section)
{
  ALE::Obj<ALE::Mesh> m;
  ALE::Obj<ALE::Mesh::int_section_type> s;
  PetscErrorCode      ierr;

  PetscFunctionBegin;
  ierr = MeshGetMesh(mesh, m);CHKERRQ(ierr);
  ierr = SectionIntCreate(m->comm(), section);CHKERRQ(ierr);
  ierr = SectionIntSetBundle(*section, m);CHKERRQ(ierr);
  ierr = SectionIntGetSection(*section, s);CHKERRQ(ierr);
  s->setFiberDimension(m->heightStratum(0), fiberDim);
  m->allocate(s);
  PetscFunctionReturn(0);
}
