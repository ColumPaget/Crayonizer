AC_INIT([main.c])

VERSION="0.0.8" 
AC_SUBST(VERSION) 

AC_CONFIG_SUBDIRS([libUseful-2.3])

AC_PROG_CC 
AC_LANG_C 
AC_PROG_MAKE_SET 
AC_PROG_INSTALL 
AC_HEADER_STDC 

AC_CHECK_LIB(z,deflate,,)
cf_have_libz=$ac_cv_lib_z_deflate

dnl read Makefile.in and write Makefile 
AC_OUTPUT(Makefile)
