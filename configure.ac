#
# Process this file with autoconf to produce a configure script.
#

AC_INIT(Shotgun, 1.0.beta)
AC_CONFIG_AUX_DIR([config])
AC_CANONICAL_HOST
AC_CONFIG_MACRO_DIR([config/m4])
AM_INIT_AUTOMAKE(1.6.3)  dnl Require automake 1.6.3 or better
AM_MAINTAINER_MODE


# This is the version info according to the libtool versioning system.
# It does *not* correspond to the release number, and has a technical
# meaning (CURRENT:REVISION:AGE) that indicates the compatibility of
# different library versions (see the libtool manual):
# 
# Here are a set of rules to help you update your library version information:
# 
#    1. Start with version information of '0:0:0' for each libtool library.
#    2. Update the version information only immediately before a public release of
#       your software. More frequent updates are unnecessary, and only guarantee
#       that the current interface number gets larger faster.
#    3. If the library source code has changed at all since the last update, then
#       increment revision ('c:r:a' becomes 'c:r+1:a').
#    4. If any interfaces have been added, removed, or changed since the last
#       update, increment current, and set revision to 0.
#    5. If any interfaces have been added since the last public release, then
#       increment age.
#    6. If any interfaces have been removed or changed since the last public
#       release, then set age to 0.
#
LIBTOOL_CURRENT=0
LIBTOOL_REVISION=0
LIBTOOL_AGE=0
LIBTOOL_VERSION=$LIBTOOL_CURRENT:$LIBTOOL_REVISION:$LIBTOOL_AGE
AC_SUBST(LIBTOOL_VERSION)

AC_LANG(C++)
AC_PROG_INSTALL
AC_PROG_CXX
AC_PROG_LN_S
AC_PROG_LIBTOOL
AC_PROG_MAKE_SET
AC_CHECK_FUNCS([memset])
AC_C_CONST
AC_HEADER_STDBOOL
AC_PROG_RANLIB
AC_STRUCT_TM
AC_TYPE_SIZE_T

AC_ARG_ENABLE([python],
          [AC_HELP_STRING([--enable-python],
                         [Build Shotgun plugin for Python (default=no)])],
          [sg_build_python=$enableval],
          [sg_build_python=no])
AM_CONDITIONAL([BUILD_PYTHON], [test "x$sg_build_python" != "xno"])


# -------------------------------
# BEGIN Check for xmlrpc libs
#
#
# Note: This is the version we've tested.  If someone can confirm
# that this works with an earlier version of xmlrpc, please update
# accordingly.
#
minimum_xmlrpc_lib_version="1.06.31"
AC_SUBST(minimum_xmlrpc_lib_version)


AC_CHECK_PROGS([XMLRPC_C_CONFIG], [xmlrpc-c-config], "missing")
if test "$XMLRPC_C_CONFIG" = "missing"; then
    AC_MSG_ERROR("Could not find xmlrpc-c-config.  Make sure it's in your path.")
fi

xmlrpc_version=`$XMLRPC_C_CONFIG --version`
AC_MSG_RESULT([xmlrpc-c library version: $xmlrpc_version])
AS_VERSION_COMPARE("$xmlrpc_version", "$minimum_xmlrpc_lib_version",
    [AC_MSG_ERROR([Sorry, you need xmlrpc-c version $minimum_xmlrpc_lib_version or greater.])])

XMLRPC_CFLAGS=`$XMLRPC_C_CONFIG c++2 client --cflags`
AC_MSG_RESULT([xmlrpc-c library cflags: $XMLRPC_CFLAGS])
AC_SUBST(XMLRPC_CFLAGS)

XMLRPC_LDADD=`$XMLRPC_C_CONFIG c++2 client --ldadd`
AC_MSG_RESULT([xmlrpc-c library ldadd: $XMLRPC_LDADD])
AC_SUBST(XMLRPC_LDADD)

XMLRPC_LDFLAGS=`$XMLRPC_C_CONFIG c++2 client --ldflags`
AC_MSG_RESULT([xmlrpc-c library ldflags: $XMLRPC_LDFLAGS])
AC_SUBST(XMLRPC_LDFLAGS)

#
# END Check for xmlrpc libs
# -------------------------------


# -------------------------------
# BEGIN Check for python/sip
#
if test "x$sg_build_python" != "xno"; then
AM_PATH_PYTHON(,,[:])
AS_IF([`$PYTHON -c "import sipconfig" > /dev/null 2>&1`],,
    AC_MSG_ERROR("sip is required for building the Python plugin."))
fi

#
# END Check for python/sip
# -------------------------------


# -------------------------------
# SG_DEFAULT_URL
#
AC_ARG_WITH(default-url,
AS_HELP_STRING([--with-default-url=URL],
               [Specify a default Shotgun URL]),
[
    if test "$withval" = "yes" ; then
      AC_MSG_ERROR([--with-default-url requires an argument.])
    else
      SG_DEFAULT_URL=$withval
    fi
],
[
SG_DEFAULT_URL=""
]
)
AC_DEFINE_UNQUOTED(SG_DEFAULT_URL, "$SG_DEFAULT_URL",
                     [The default Shotgun URL])
#
# SG_DEFAULT_URL
# -------------------------------

# -------------------------------
# SG_AUTHENTICATION_NAME
#
AC_ARG_WITH(authentication-name,
AS_HELP_STRING([--with-authentication-name=NAME],
               [Specify a default Shotgun authentication name]),
[
    if test "$withval" = "yes" ; then
      AC_MSG_ERROR([--with-authentication-name requires an argument.])
    else
      SG_AUTHENTICATION_NAME=$withval
    fi
],
[
SG_AUTHENTICATION_NAME=""
]
)
AC_DEFINE_UNQUOTED(SG_AUTHENTICATION_NAME, "$SG_AUTHENTICATION_NAME",
                     [The default Shotgun authentication name])
#
# SG_AUTHENTICATION_NAME
# -------------------------------

# -------------------------------
# SG_AUTHENTICATION_KEY
#
AC_ARG_WITH(authentication-key,
AS_HELP_STRING([--with-authentication-key=KEY],
               [Specify a default Shotgun authentication key]),
[
    if test "$withval" = "yes" ; then
      AC_MSG_ERROR([--with-authentication-key requires an argument.])
    else
      SG_AUTHENTICATION_KEY=$withval
    fi
],
[
SG_AUTHENTICATION_KEY=""
]
)
AC_DEFINE_UNQUOTED(SG_AUTHENTICATION_KEY, "$SG_AUTHENTICATION_KEY",
                     [The default Shotgun authentication key])
#
# SG_AUTHENTICATION_KEY
# -------------------------------


AC_CONFIG_FILES([Makefile
                 lib/Makefile
                 lib/Shotgun/Makefile
                 plugins/Makefile
                 plugins/python/Makefile
                 plugins/python/test/Makefile
                 test/Makefile
                 doc/Makefile
                 example/Makefile
                 example/lib/Makefile
                 example/lib/SiteShotgun/Makefile
                 example/test/Makefile
                 example/plugins/Makefile
                 example/plugins/python/Makefile
                 example/plugins/python/test/Makefile
                ])
AC_OUTPUT
