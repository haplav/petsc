#!/usr/bin/env python

configure_options = [
  '--with-debugging=0',

  '--useThreads=0', # for some reason cmake hangs when invoked from configure on bsd?
  #'-download-fblaslapack=1',
  '--download-mpich=1',
  '--download-cmake=1',
  '--download-metis=1',
  '--download-parmetis=1',
  '--download-triangle=1',
  '--download-superlu=1',
  '--download-superlu_dist=1',
  '--download-scalapack=1',
  '--download-mumps=1',
  # no with-cxx-dialect=C++11 support '--download-elemental=1',
  #'--download-hdf5',
  '--download-sundials=1',
  '--download-hypre=1',
  '--download-umfpack=1',
  '--download-chaco=1',
  '--download-spai=1',
  ]

if __name__ == '__main__':
  import sys,os
  sys.path.insert(0,os.path.abspath('config'))
  import configure
  configure.petsc_configure(configure_options)
