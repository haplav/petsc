#ifdef PETSC_RCS_HEADER
"$Id: petscconf.h,v 1.4 2001/03/22 20:27:58 bsmith Exp $"
"Defines the configuration for this machine"
#endif

#if !defined(INCLUDED_PETSCCONF_H)
#define INCLUDED_PETSCCONF_H

#define PARCH_linux
#define PETSC_ARCH_NAME "linux"

#define PETSC_HAVE_POPEN
#define PETSC_HAVE_LIMITS_H
#define PETSC_HAVE_PWD_H 
#define PETSC_HAVE_MALLOC_H 
#define PETSC_HAVE_STRING_H 
#define PETSC_HAVE_GETDOMAINNAME
#define PETSC_HAVE_DRAND48 
#define PETSC_HAVE_UNAME 
#define PETSC_HAVE_UNISTD_H 
#define PETSC_HAVE_SYS_TIME_H 
#define PETSC_HAVE_STDLIB_H
#define PETSC_HAVE_GETCWD
#define PETSC_HAVE_SLEEP
#define PETSC_HAVE_SYS_PARAM_H
#define PETSC_HAVE_SYS_STAT_H

#define PETSC_HAVE_FORTRAN_UNDERSCORE 

#define PETSC_HAVE_READLINK
#define PETSC_HAVE_MEMMOVE

#define PETSC_HAVE_DOUBLE_ALIGN_MALLOC
#define PETSC_HAVE_MEMALIGN
#define PETSC_HAVE_SYS_RESOURCE_H
#define PETSC_SIZEOF_VOID_P 4
#define PETSC_SIZEOF_INT 4
#define PETSC_SIZEOF_DOUBLE 8
#define PETSC_BITS_PER_BYTE 8
#define PETSC_SIZEOF_FLOAT 4
#define PETSC_SIZEOF_LONG 4
#define PETSC_SIZEOF_LONG_LONG 8

#define PETSC_USE_DYNAMIC_LIBRARIES 1
#define PETSC_HAVE_RTLD_GLOBAL 1

#define PETSC_PRINTF_FORMAT_CHECK(a,b) __attribute__ ((format (printf, a,b)))

#define PETSC_HAVE_SYS_UTSNAME_H

#define PETSC_MISSING_SIGSYS
#define PETSC_HAVE_CXX_NAMESPACE
#endif

