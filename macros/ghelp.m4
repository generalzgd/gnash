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

AC_DEFUN([GNASH_PATH_GHELP],
[
  AC_ARG_ENABLE(ghelp, AC_HELP_STRING([--enable-ghelp], [Enable support for the GNOME help system]),
  [case "${enableval}" in
    yes) ghelp=yes ;;
    no)  ghelp=no ;;
    *)   AC_MSG_ERROR([bad value ${enableval} for enable-ghelp option]) ;;
  esac], ghelp=no)

  if test x"$ghelp" = x"yes" ; then
    AC_PATH_PROG(SCROLLKEEPER, scrollkeeper-config, [],
	[$PATH:/usr/bin/X11:/usr/local/bin/X11:/opt/X11])
    AC_PATH_PROG(SCROLLUPDATE, scrollkeeper-update, [],
	[$PATH:/usr/bin/X11:/usr/local/bin/X11:/opt/X11])
    AC_PATH_PROG(SCROLLINSTALL, scrollkeeper-preinstall, [],
	[$PATH:/usr/bin/X11:/usr/local/bin/X11:/opt/X11])
    if test x"$SCROLLKEEPER" = x -o x"$SCROLLUPDATE" = x -o x"$SCROLLINSTALL" = x ; then
      ghelp=no
      AC_MSG_WARN([You need to install scrollkeeper for gnome help])
    fi
  fi
])
