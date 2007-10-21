# DO NOT EDIT! GENERATED AUTOMATICALLY!
# Copyright (C) 2004-2007 Free Software Foundation, Inc.
#
# This file is free software, distributed under the terms of the GNU
# General Public License.  As a special exception to the GNU General
# Public License, this file may be distributed as part of a program
# that contains a configuration script generated by Autoconf, under
# the same distribution terms as the rest of that program.
#
# Generated by gnulib-tool.
#
# This file represents the compiled summary of the specification in
# gnulib-cache.m4. It lists the computed macro invocations that need
# to be invoked from configure.ac.
# In projects using CVS, this file can be treated like other built files.


# This macro should be invoked from ./configure.ac, in the section
# "Checks for programs", right after AC_PROG_CC, and certainly before
# any checks for libraries, header files, types and library functions.
AC_DEFUN([gl_EARLY],
[
  m4_pattern_forbid([^gl_[A-Z]])dnl the gnulib macro namespace
  m4_pattern_allow([^gl_ES$])dnl a valid locale name
  m4_pattern_allow([^gl_LIBOBJS$])dnl a variable
  m4_pattern_allow([^gl_LTLIBOBJS$])dnl a variable
  AC_REQUIRE([AC_PROG_RANLIB])
  AC_REQUIRE([gl_USE_SYSTEM_EXTENSIONS])
])

# This macro should be invoked from ./configure.ac, in the section
# "Check for header files, types and library functions".
AC_DEFUN([gl_INIT],
[
  m4_pushdef([AC_LIBOBJ], m4_defn([gl_LIBOBJ]))
  m4_pushdef([AC_REPLACE_FUNCS], m4_defn([gl_REPLACE_FUNCS]))
  m4_pushdef([AC_LIBSOURCES], m4_defn([gl_LIBSOURCES]))
  AM_CONDITIONAL([GL_COND_LIBTOOL], [false])
  gl_cond_libtool=false
  gl_libdeps=
  gl_ltlibdeps=
  gl_source_base='gnulib/lib'
  gl_FUNC_ALLOCA
  gl_CANONICALIZE_LGPL
  gl_ERROR
  gl_EXITFAIL
  gl_GETOPT
  AC_SUBST([LIBINTL])
  AC_SUBST([LTLIBINTL])
  gl_INLINE
  gl_FUNC_MALLOC_POSIX
  gl_STDLIB_MODULE_INDICATOR([malloc-posix])
  gl_MALLOCA
  gl_PATHMAX
  gl_FUNC_READLINK
  gl_UNISTD_MODULE_INDICATOR([readlink])
  gl_RELOCATABLE([$gl_source_base])
  gl_FUNC_READLINK_SEPARATE
  gl_CANONICALIZE_LGPL_SEPARATE
  gl_MALLOCA
  gl_RELOCATABLE_LIBRARY_SEPARATE
  gl_FUNC_SETENV_SEPARATE
  gl_FUNC_STRERROR_SEPARATE
  gt_TYPE_SSIZE_T
  AM_STDBOOL_H
  gl_STDLIB_H
  gl_FUNC_STRERROR
  gl_STRING_MODULE_INDICATOR([strerror])
  gl_HEADER_STRING_H
  gl_UNISTD_H
  gl_XALLOC
  LIBGNU_LIBDEPS="$gl_libdeps"
  AC_SUBST([LIBGNU_LIBDEPS])
  LIBGNU_LTLIBDEPS="$gl_ltlibdeps"
  AC_SUBST([LIBGNU_LTLIBDEPS])
  m4_popdef([AC_LIBSOURCES])
  m4_popdef([AC_REPLACE_FUNCS])
  m4_popdef([AC_LIBOBJ])
  AC_CONFIG_COMMANDS_PRE([
    gl_libobjs=
    gl_ltlibobjs=
    if test -n "$gl_LIBOBJS"; then
      # Remove the extension.
      sed_drop_objext='s/\.o$//;s/\.obj$//'
      for i in `for i in $gl_LIBOBJS; do echo "$i"; done | sed "$sed_drop_objext" | sort | uniq`; do
        gl_libobjs="$gl_libobjs $i.$ac_objext"
        gl_ltlibobjs="$gl_ltlibobjs $i.lo"
      done
    fi
    AC_SUBST([gl_LIBOBJS], [$gl_libobjs])
    AC_SUBST([gl_LTLIBOBJS], [$gl_ltlibobjs])
  ])
])

# Like AC_LIBOBJ, except that the module name goes
# into gl_LIBOBJS instead of into LIBOBJS.
AC_DEFUN([gl_LIBOBJ], [
  AS_LITERAL_IF([$1], [gl_LIBSOURCES([$1.c])])dnl
  gl_LIBOBJS="$gl_LIBOBJS $1.$ac_objext"
])

# Like AC_REPLACE_FUNCS, except that the module name goes
# into gl_LIBOBJS instead of into LIBOBJS.
AC_DEFUN([gl_REPLACE_FUNCS], [
  m4_foreach_w([gl_NAME], [$1], [AC_LIBSOURCES(gl_NAME[.c])])dnl
  AC_CHECK_FUNCS([$1], , [gl_LIBOBJ($ac_func)])
])

# Like AC_LIBSOURCES, except the directory where the source file is
# expected is derived from the gnulib-tool parametrization,
# and alloca is special cased (for the alloca-opt module).
# We could also entirely rely on EXTRA_lib..._SOURCES.
AC_DEFUN([gl_LIBSOURCES], [
  m4_foreach([_gl_NAME], [$1], [
    m4_if(_gl_NAME, [alloca.c], [], [
      m4_syscmd([test -r gnulib/lib/]_gl_NAME[ || test ! -d gnulib/lib])dnl
      m4_if(m4_sysval, [0], [],
        [AC_FATAL([missing gnulib/lib/]_gl_NAME)])
    ])
  ])
])

# This macro records the list of files which have been installed by
# gnulib-tool and may be removed by future gnulib-tool invocations.
AC_DEFUN([gl_FILE_LIST], [
  build-aux/config.libpath
  build-aux/install-reloc
  build-aux/link-warning.h
  build-aux/reloc-ldflags
  doc/relocatable.texi
  lib/alloca.in.h
  lib/areadlink.c
  lib/areadlink.h
  lib/c-ctype.c
  lib/c-ctype.h
  lib/canonicalize-lgpl.c
  lib/canonicalize.h
  lib/error.c
  lib/error.h
  lib/exitfail.c
  lib/exitfail.h
  lib/getopt.c
  lib/getopt.in.h
  lib/getopt1.c
  lib/getopt_int.h
  lib/gettext.h
  lib/intprops.h
  lib/malloc.c
  lib/malloca.c
  lib/malloca.h
  lib/malloca.valgrind
  lib/pathmax.h
  lib/progname.c
  lib/progname.h
  lib/progreloc.c
  lib/readlink.c
  lib/relocatable.c
  lib/relocatable.h
  lib/relocwrapper.c
  lib/setenv.c
  lib/setenv.h
  lib/stdbool.in.h
  lib/stdlib.in.h
  lib/strerror.c
  lib/string.in.h
  lib/unistd.in.h
  lib/xalloc-die.c
  lib/xalloc.h
  lib/xmalloc.c
  lib/xreadlink.c
  lib/xreadlink.h
  m4/absolute-header.m4
  m4/alloca.m4
  m4/canonicalize-lgpl.m4
  m4/eealloc.m4
  m4/error.m4
  m4/exitfail.m4
  m4/extensions.m4
  m4/getopt.m4
  m4/gnulib-common.m4
  m4/include_next.m4
  m4/inline.m4
  m4/longlong.m4
  m4/malloc.m4
  m4/malloca.m4
  m4/onceonly_2_57.m4
  m4/pathmax.m4
  m4/readlink.m4
  m4/relocatable-lib.m4
  m4/relocatable.m4
  m4/setenv.m4
  m4/ssize_t.m4
  m4/stdbool.m4
  m4/stdlib_h.m4
  m4/strerror.m4
  m4/string_h.m4
  m4/unistd_h.m4
  m4/xalloc.m4
])
