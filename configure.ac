AC_INIT([quartz-engine], [0.1])
AC_PREREQ(2.59)

AC_CONFIG_SRCDIR([README])
AC_CONFIG_HEADER(config.h)

AM_INIT_AUTOMAKE(1.9 dist-bzip2 no-define)
AM_MAINTAINER_MODE

AC_PROG_CC
AM_PROG_CC_STDC
AC_PROG_INSTALL
AC_PROG_MAKE_SET

AM_DISABLE_STATIC
AM_PROG_LIBTOOL

PKG_CHECK_MODULES(GTK, gtk+-2.0 >= 2.10.0,,
                  AC_MSG_ERROR([GTK+ 2.10 is required to compile quartz-engine]))

GTK_VERSION=`$PKG_CONFIG --variable=gtk_binary_version gtk+-2.0`
AC_SUBST(GTK_VERSION)

AC_OUTPUT([
  Makefile
  src/Makefile
  theme/Makefile
]) 
