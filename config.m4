dnl $Id$
dnl config.m4 for extension av

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(av, for av support,
dnl Make sure that the comment is aligned:
dnl [  --with-av             Include av support])

dnl Otherwise use enable:

dnl PHP_ARG_ENABLE(av, whether to enable av support,
dnl Make sure that the comment is aligned:
dnl [  --enable-av           Enable av support])

if test "$PHP_AV" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-av -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/av.h"  # you most likely want to change this
  dnl if test -r $PHP_AV/$SEARCH_FOR; then # path given as parameter
  dnl   AV_DIR=$PHP_AV
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for av files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       AV_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$AV_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the av distribution])
  dnl fi

  dnl # --with-av -> add include path
  dnl PHP_ADD_INCLUDE($AV_DIR/include)

  dnl # --with-av -> check for lib and symbol presence
  dnl LIBNAME=av # you may want to change this
  dnl LIBSYMBOL=av # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $AV_DIR/lib, AV_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_AVLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong av lib version or lib not found])
  dnl ],[
  dnl   -L$AV_DIR/lib -lm
  dnl ])
  dnl
  dnl PHP_SUBST(AV_SHARED_LIBADD)

  PHP_NEW_EXTENSION(av, av.c, $ext_shared)
fi
