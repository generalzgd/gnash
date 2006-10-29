dnl  
dnl    Copyright (C) 2005, 2006 Free Software Foundation, Inc.
dnl  
dnl  This program is free software; you can redistribute it and/or modify
dnl  it under the terms of the GNU General Public License as published by
dnl  the Free Software Foundation; either version 2 of the License, or
dnl  (at your option) any later version.
dnl  
dnl  This program is distributed in the hope that it will be useful,
dnl  but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl  GNU General Public License for more details.
dnl  You should have received a copy of the GNU General Public License
dnl  along with this program; if not, write to the Free Software
dnl  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

dnl  
dnl 

dnl: $Id: gtkglext.m4,v 1.27 2006/10/29 18:34:10 rsavoye Exp $

AC_DEFUN([GNASH_PATH_GLEXT],
[
  AC_ARG_ENABLE(glext, AC_HELP_STRING([--disable-glext], [Disable support for GTK OpenGL extension]),
  [case "${enableval}" in
    yes) glext=yes ;;
    no)  glext=no ;;
    *)   AC_MSG_ERROR([bad value ${enableval} for disable-glext option]) ;;
  esac], glext=yes)

dnl  if test x"$plugin" = x"no"; then
dnl glext=no
dnl fi

  if test x"$glext" = x"yes"; then
    dnl Look for the header
    AC_ARG_WITH(glext_incl, AC_HELP_STRING([--with-glext-incl], [directory where libglext header is]), with_glext_incl=${withval})

    AC_CACHE_VAL(ac_cv_path_glext_incl,[
    if test x"${with_glext_incl}" != x ; then
      if test -f ${with_glext_incl}/gtk/gtkgl.h ; then
	ac_cv_path_glext_incl="-I`(cd ${with_glext_incl}; pwd)`"
        gnash_glext_topdir=`basename ${with_glext_incl}`
        gnash_glext_version=`echo ${gnash_glext_topdir} | sed -e 's:gtkglext-::'`
      else
	AC_MSG_ERROR([${with_glext_incl} directory doesn't contain gtk/gtkgl.h])
      fi
     ])


      if test x"$PKG_CONFIG" != x -a x"${ac_cv_path_glext_incl}" = x; then
        $PKG_CONFIG --exists gtkglext-1.0 && ac_cv_path_glext_incl=`$PKG_CONFIG --cflags gtkglext-1.0`
        $PKG_CONFIG --exists gtkglext-1.0 && gnash_glext_version=`$PKG_CONFIG --modversion gtkglext-1.0 | cut -d "." -f 1,3`
      fi

dnl Attempt to find the top level directory, which unfortunately has a
dnl version number attached. At least on Debain based systems, this
dnl doesn't seem to get a directory that is unversioned.
    AC_MSG_CHECKING([for the Gtk GL Extensions Version])
    if test x"${gnash_glext_version}" = x ; then
      pathlist="${prefix}/include /sw/include /opt/local/include /usr/local/include /usr/X11R6/include /home/latest/include /opt/include /opt/local/include /usr/include /usr/pkg/include .. ../.."

      gnash_glext_topdir=""
      gnash_glext_version=""
      for i in $pathlist; do
	for j in `ls -dr $i/gtkglext-[[0-9]].[[0-9]] 2>/dev/null`; do
 	  if test -f $j/gtk/gtkgl.h; then
	    gnash_glext_topdir=`basename $j`
	    gnash_glext_version=`echo ${gnash_glext_topdir} | sed -e 's:gtkglext-::'`
 	    break
 	  fi
	done
      done
    fi

    AC_MSG_RESULT([${gnash_glext_version}])

    dnl If the path hasn't been specified, go look for it.
    if test x"${ac_cv_path_glext_incl}" = x; then
        AC_MSG_CHECKING([for gtk/gtkgl.h])
        incllist="${prefix}/include /sw/include /opt/local/include /usr/local/include /usr/X11R6/include /home/latest/include /opt/include /opt/local/include /usr/include /usr/pkg/include .. ../.."

	ac_cv_path_glext_incl=""
        for i in $incllist; do
	  if test -f $i/gtk/gtkgl.h; then
	    if test x"$i" != x"/usr/include"; then
	      ac_cv_path_glext_incl="-I$i"
	      break
            fi
	  else
	    if test -f $i/${gnash_glext_topdir}/gtk/gtkgl.h; then
	      ac_cv_path_glext_incl="-I$i/${gnash_glext_topdir}"
	      break
	    fi
	  fi
        done

        if test x"${ac_cv_path_glext_incl}" = x; then
          AC_MSG_RESULT([not found])
        else
          AC_MSG_RESULT([${ac_cv_path_glext_incl}])
        fi
    fi

     dnl Look for the library
     AC_ARG_WITH(glext_lib, AC_HELP_STRING([--with-glext-lib], [directory where gtkglext library is]), with_glext_lib=${withval})
     AC_CACHE_VAL(ac_cv_path_glext_lib,[
      if test x"${with_glext_lib}" != x ; then
        if test -f ${with_glext_lib}/libgtkglext-x11-${gnash_glext_version}.a -o -f ${with_glext_lib}/libgtkglext-x11-${gnash_glext_version}.so; then
	  ac_cv_path_glext_lib=-L`(cd ${with_glext_lib}; pwd)`
        else
	  AC_MSG_ERROR([${with_glext_lib} directory doesn't contain libgtkglext-x11-${gnash_glext_version}.[a|so]])
        fi
      fi
      ])

      dnl Try with pkg-config
      if test x"$PKG_CONFIG" != x -a x"${ac_cv_path_glext_lib}" = x; then
        $PKG_CONFIG --exists gtkglext-1.0 && ac_cv_path_glext_lib=`$PKG_CONFIG --libs gtkglext-1.0`
      fi

      if test x"${ac_cv_path_glext_lib}" = x; then
        AC_CHECK_LIB(gtkglext-x11-${gnash_glext_version}, gtk_gl_init, [ac_cv_path_glext_lib="-lgtkglext-x11-${gnash_glext_version} -lgdkglext-x11-${gnash_glext_version}"],[
          libslist="${prefix}/lib64 ${prefix}/lib /opt/local/lib /usr/X11R6/lib64 /usr/X11R6/lib /usr/lib /usr/lib64 /sw/lib /opt/local/lib /usr/local/lib /home/latest/lib /opt/lib /usr/pkg/lib .. ../.."
          for i in $libslist; do
	    if test -f $i/libgtkglext-x11-${gnash_glext_version}.a -o -f $i/libgtkglext-x11-${gnash_glext_version}.so; then
	      if test x"$i" != x"/usr/lib"; then
	        ac_cv_path_glext_lib="-L$i -lgtkglext-x11-${gnash_glext_version} -lgdkglext-x11-${gnash_glext_version}"
	        break
              fi
	    else
	      if test -f $i/libgtkglext-x11-${gnash_glext_version}.a -o -f $i/libgtkglext-x11-${gnash_glext_version}.so; then
		ac_cv_path_glext_lib="-L$i/${gnash_glext_topdir} -lgtkglext-x11-${gnash_glext_version} -lgdkglext-x11-${gnash_glext_version}"
		break
              fi
	    fi
          done])
      else
	if test -f $i/libgtkglext-x11-${gnash_glext_version}.a -o -f $i/libgtkglext-x11-${gnash_glext_version}.so; then
          if test x"${ac_cv_path_glext_lib}" != x"/usr/lib"; then
	    ac_cv_path_glext_lib="-L${ac_cv_path_glext_lib} -lgtkglext-x11-${gnash_glext_version} -lgdkglext-x11-${gnash_glext_version}"
           else
	    ac_cv_path_glext_lib=""
          fi
        fi
      fi
    fi
  fi

  
  if test x"${ac_cv_path_glext_incl}" != x ; then
    AC_DEFINE(HAVE_GTK_GTKGL_H, [1], [GTKGLExt header])
    libslist="${prefix}/lib64 ${prefix}/lib /usr/lib /usr/lib64 /opt/local/lib /sw/lib /usr/local/lib /home/latest/lib /opt/lib /usr/pkg/lib .. ../.."

    libslist="${prefix}/lib64 ${prefix}/lib /usr/X11R6/lib64 /usr/X11R6/lib /usr/lib /usr/lib64 /sw/lib /opt/local/lib /usr/local/lib /home/latest/lib /opt/lib /usr/pkg/lib .. ../.."
    ac_cv_path_glext_incl="${ac_cv_path_glext_incl}"
    for i in $libslist; do
      if test -f $i/gtkglext-${gnash_glext_version}/include/gdkglext-config.h; then
        ac_cv_path_glext_incl="${ac_cv_path_glext_incl} -I${i}/gtkglext-${gnash_glext_version}/include"
      fi
    done
    GLEXT_CFLAGS="${ac_cv_path_glext_incl}"
  else
    GLEXT_CFLAGS=""
  fi

  if test x"${ac_cv_path_glext_lib}" != x ; then
    AC_DEFINE(USE_GTKGLEXT,[1], [Use GtkGLExt extension])
    GLEXT_LIBS="${ac_cv_path_glext_lib}"
  else
    GLEXT_LIBS=""
dnl we can't build the plguin without GtkGlExt
    glext=no
  fi

  AC_SUBST(GLEXT_CFLAGS)
  AC_SUBST(GLEXT_LIBS)
])

