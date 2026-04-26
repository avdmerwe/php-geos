dnl
dnl $ Id: $
dnl vim:se ts=2 sw=2 et:

PHP_ARG_ENABLE(geos, whether to enable geos support,
[  --enable-geos               Enable geos support])

PHP_ARG_WITH(geos-config, for geos-config,
[  --with-geos-config[=DIR]    Use geos-config])

if test -z "$PHP_DEBUG"; then
  AC_ARG_ENABLE(debug,
  [  --enable-debug          compile with debugging symbols],[
    PHP_DEBUG=$enableval
  ],[    PHP_DEBUG=no
  ])
fi

if test "$PHP_GEOS" != "no"; then
  if test "$PHP_GEOS_CONFIG" != "yes"; then
    if test -x "$PHP_GEOS_CONFIG"; then
      GEOS_CONFIG="$PHP_GEOS_CONFIG"
    fi
  else
    AC_PATH_PROG(GEOS_CONFIG, geos-config)
  fi

  if test -n "$GEOS_CONFIG"; then
    GEOS_VERSION=`$GEOS_CONFIG --version`
    GEOS_INCLUDE=`$GEOS_CONFIG --includes`
    GEOS_LDFLAGS=`$GEOS_CONFIG --ldflags`
    AC_MSG_RESULT([Using GEOS version $GEOS_VERSION])
    AC_DEFINE(HAVE_GEOS_C_H,1,[Whether to have geos_c.h])
  fi

  if test -z "$GEOS_INCLUDE"; then
    AC_MSG_ERROR(Cannot find geos_c.h. Please specify correct GEOS installation path)
  fi

  if test -z "$GEOS_LDFLAGS"; then
    AC_MSG_ERROR(Cannot find geos_c.so. Please specify correct GEOS installation path)
  fi

  dnl Hard floor: GEOS >= 3.11.0 (Q5.9 in implementation plan).
  dnl Required for ConcaveHull, PolygonHullSimplify, MakeValid 'structure' mode,
  dnl and the curved-geometry storage that the extended bindings depend on.
  AC_MSG_CHECKING([for GEOS >= 3.11.0])
  AS_VERSION_COMPARE([$GEOS_VERSION], [3.11.0],
    [AC_MSG_ERROR([GEOS >= 3.11.0 is required, found $GEOS_VERSION])],
    [AC_MSG_RESULT([yes ($GEOS_VERSION)])],
    [AC_MSG_RESULT([yes ($GEOS_VERSION)])])

  old_CFLAGS=$CFLAGS
  CFLAGS="-I$GEOS_INCLUDE $CFLAGS"

  old_LDFLAGS=$LDFLAGS
  LDFLAGS="$GEOS_LDFLAGS $LDFLAGS"

  AC_CHECK_HEADER(geos_c.h,, AC_MSG_ERROR(Can't find GEOS includes))
  AC_CHECK_LIB(geos_c, initGEOS_r,, AC_MSG_ERROR([Unable to build the GEOS: a newer libgeos is required]))
  AC_CHECK_LIB(geos_c, finishGEOS_r,, AC_MSG_ERROR([Unable to build the GEOS: a newer libgeos is required]))

  CFLAGS=$old_CFLAGS
  dnl LDFLAGS=$old_LDFLAGS

  PHP_ADD_LIBRARY(geos_c, 1, GEOS_SHARED_LIBADD)
  PHP_ADD_MAKEFILE_FRAGMENT(Makefile.frag)
  PHP_SUBST(GEOS_SHARED_LIBADD)
  PHP_ADD_INCLUDE($GEOS_INCLUDE, 1)
  PHP_NEW_EXTENSION(geos, geos.c, $ext_shared,,)
fi
