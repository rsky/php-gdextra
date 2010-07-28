dnl
dnl $ Id: $
dnl

PHP_ARG_ENABLE(gdextra, [whether to enable gdextra functions],
[  --enable-gdextra        Enable gdextra functions support], yes, yes)

PHP_ARG_WITH(gdextra-lqr, [whether to enable liquid rescaling support],
[  --with-gdextra-lqr      Enable liquid rescaling support.], no, no)

PHP_ARG_WITH(gdextra-magick, [whether to enable ImageMagick image loader support],
[  --with-gdextra-magick[[=PATH]]    Enable ImageMagick image loader support.
                                  PATH is the optional pathname to Wand-config], no, no)

if test "$PHP_GDEXTRA" != "no"; then

  if test -z "$AWK"; then
    AC_PATH_PROGS(AWK, awk gawk nawk, [no])
  fi
  if test -z "$SED"; then
    AC_PATH_PROGS(SED, sed gsed, [no])
  fi

  dnl
  dnl Check for Liquid Rescale Library support
  dnl
  if test "$PHP_GDEXTRA_LQR" != "no"; then
    dnl
    dnl Check the location of pkg-config
    dnl
    if test -z "$PKG_CONFIG"; then
      AC_PATH_PROGS(PKG_CONFIG, pkg-config, [no])
    fi

    dnl
    dnl Get the version number, CFLAGS and LIBS by pkg-config
    dnl
    AC_MSG_CHECKING([for Liquid Rescale Library version])

    LQR_VERSION=`$PKG_CONFIG --modversion lqr-1 2> /dev/null`
    LQR_INCLINE=`$PKG_CONFIG --cflags-only-I lqr-1 2> /dev/null`
    LQR_LIBLINE=`$PKG_CONFIG --libs lqr-1 2> /dev/null`

    if test -z "$LQR_VERSION"; then
      AC_MSG_ERROR([lqr-1.pc not found in PKG_CONFIG_PATH])
    fi

    AC_MSG_RESULT([$LQR_VERSION (ok)])

    AC_DEFINE(PHP_GDEXTRA_WITH_LQR, 1, [enable liquid rescaling support])
    AC_DEFINE_UNQUOTED(PHP_GDEXTRA_LIBLQR_VERSION_STRING, "$LQR_VERSION", [Liquid Rescale Library version])
  fi

  dnl
  dnl Check for ImageMagick support
  dnl
  if test "$PHP_GDEXTRA_MAGICK" != "no"; then
    dnl
    dnl Check the location of Wand-config
    dnl
    if test "$PHP_GDEXTRA_MAGICK" != "yes"; then
      AC_MSG_CHECKING([for Wand-config])
      if test -f "$PHP_GDEXTRA_MAGICK"; then
        WAND_CONFIG="$PHP_GDEXTRA_MAGICK"
      elif test -f "$PHP_GDEXTRA_MAGICK/Wand-config"; then
        WAND_CONFIG="$PHP_GDEXTRA_MAGICK/Wand-config"
      elif test -f "$PHP_GDEXTRA_MAGICK/bin/Wand-config"; then
        WAND_CONFIG="$PHP_GDEXTRA_MAGICK/bin/Wand-config"
      else
        AC_MSG_ERROR([not found])
      fi
      AC_MSG_RESULT([$WAND_CONFIG])
    else
      AC_PATH_PROGS(WAND_CONFIG, Wand-config, [])
      if test -z "$WAND_CONFIG"; then
        AC_MSG_ERROR([Wand-config not found])
      fi
    fi

    dnl
    dnl Get the version number, CFLAGS and LIBS by Wand-config
    dnl
    AC_MSG_CHECKING([for ImageMagick library version])

    IMAGICK_VERSION=`$WAND_CONFIG --version 2> /dev/null`
    IMAGICK_INCLINE=`$WAND_CONFIG --cppflags 2> /dev/null`
    IMAGICK_LIBLINE=`$WAND_CONFIG --ldflags --libs 2> /dev/null`

    if test -z "$IMAGICK_VERSION"; then
      AC_MSG_ERROR([invalid Wand-config passed to --with-gdextra-magick])
    fi

    IMAGICK_VERSION_NUMBER=`echo $IMAGICK_VERSION | $AWK -F. '{ printf "%d", ($1 * 1000 + $2) * 1000 + $3 }'`

    if test "$IMAGICK_VERSION_NUMBER" -lt 6002004; then
      AC_MSG_RESULT([$IMAGICK_VERSION])
      AC_MSG_ERROR([ImageMagick version 6.2.4 or later is required])
    fi

    AC_MSG_RESULT([$IMAGICK_VERSION (ok)])

    AC_DEFINE(PHP_GDEXTRA_WITH_MAGICK, 1, [enable ImageMagick support])
    AC_DEFINE_UNQUOTED(PHP_GDEXTRA_IMAGICK_VERSION_STRING, "$IMAGICK_VERSION", [ImageMagick library version])
  fi

  dnl
  dnl Check for PHP version
  dnl
  export OLD_CPPFLAGS="$CPPFLAGS"
  export CPPFLAGS="$CPPFLAGS $INCLUDES"
  AC_MSG_CHECKING([PHP version])
  AC_TRY_COMPILE([#include <php_version.h>], [
#if ((PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION < 1)) || ((PHP_MAJOR_VERSION == 4) && (PHP_MINOR_VERSION < 4))
#error  this extension requires at least PHP4 version 4.4.0 or PHP5 version 5.1.0
#endif
],
    [AC_MSG_RESULT([ok])],
    [AC_MSG_ERROR([need at least PHP4 4.4.0 or PHP5 5.1.0])])
  export CPPFLAGS="$OLD_CPPFLAGS"

  dnl
  dnl Check for libGD header of the PHP GD extension
  dnl
  export OLD_CPPFLAGS="$CPPFLAGS"
  export CPPFLAGS="$CPPFLAGS $INCLUDES"
  AC_CHECK_HEADER([ext/gd/libgd/gd.h], [], AC_MSG_ERROR(['ext/gd/libgd/gd.h' header not found]))
  export CPPFLAGS="$OLD_CPPFLAGS"

  GDEXTRA_SOURCES="gdextra.c gdex_bmp.c gdex_channel.c gdex_color.c gdex_correct.c gdex_geom.c"

  dnl
  dnl Check for Liquid Rescale Library header
  dnl
  if test "$PHP_GDEXTRA_LQR" != "no"; then
    export OLD_CPPFLAGS="$CPPFLAGS"
    export CPPFLAGS="$CPPFLAGS $LQR_INCLINE"
    AC_CHECK_HEADER([lqr.h], [], AC_MSG_ERROR(['lqr.h' header not found]))
    export CPPFLAGS="$OLD_CPPFLAGS"

    PHP_EVAL_INCLINE($LQR_INCLINE)
    PHP_EVAL_LIBLINE($LQR_LIBLINE, GDEXTRA_SHARED_LIBADD)

    GDEXTRA_SOURCES="$GDEXTRA_SOURCES gdex_lqr.c"
  fi

  dnl
  dnl Check for ImageMagick header
  dnl
  if test "$PHP_GDEXTRA_MAGICK" != "no"; then
    export OLD_CPPFLAGS="$CPPFLAGS"
    export CPPFLAGS="$CPPFLAGS $IMAGICK_INCLINE"
    AC_CHECK_HEADER([wand/MagickWand.h], [], AC_MSG_ERROR(['wand/MagickWand.h' header not found]))
    export CPPFLAGS="$OLD_CPPFLAGS"

    PHP_EVAL_INCLINE($IMAGICK_INCLINE)
    PHP_EVAL_LIBLINE($IMAGICK_LIBLINE, GDEXTRA_SHARED_LIBADD)

    GDEXTRA_SOURCES="$GDEXTRA_SOURCES gdex_magick.c"
  fi

  PHP_SUBST(GDEXTRA_SHARED_LIBADD)
  AC_DEFINE(HAVE_GDEXTRA, 1, [ ])

  GDEXTRA_PHP_VERNUM=`"$PHP_CONFIG" --version | $AWK -F. '{ printf "%d", ($1 * 100 + $2) * 100 }'`
  if test "$GDEXTRA_PHP_VERNUM" -ge 50300; then
     GDEXTRA_SOURCES="$GDEXTRA_SOURCES gdex_wrappers.c"
  fi

  GDEXTRA_SOURCES="$GDEXTRA_SOURCES spline.c"
  PHP_NEW_EXTENSION(gdextra, $GDEXTRA_SOURCES, $ext_shared)
fi
