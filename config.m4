# Copyright (c) anno Domini nostri Jesu Christi MMXXIV John Boehr & contributors
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

m4_include(m4/ax_require_defined.m4)
m4_include(m4/ax_prepend_flag.m4)
m4_include(m4/ax_compiler_vendor.m4)
m4_include(m4/ax_cflags_warn_all.m4)

PHP_ARG_ENABLE(defer, whether to enable defer support,
[  --enable-defer     Enable defer support])

AC_DEFUN([PHP_DEFER_ADD_SOURCES], [
  PHP_DEFER_SOURCES="$PHP_DEFER_SOURCES $1"
])

if test "$PHP_DEFER" != "no"; then
    AX_CFLAGS_WARN_ALL([WARN_CFLAGS])
    CFLAGS="$WARN_CFLAGS $CFLAGS"

    PHP_DEFER_ADD_SOURCES([
        src/extension.c
        src/process.c
    ])

    PHP_ADD_BUILD_DIR(src)
    PHP_INSTALL_HEADERS([ext/defer], [php_defer.h])
    PHP_NEW_EXTENSION(defer, $PHP_DEFER_SOURCES, $ext_shared, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
    PHP_ADD_EXTENSION_DEP(defer, ast, true)
    PHP_ADD_EXTENSION_DEP(defer, opcache, true)
    PHP_ADD_EXTENSION_DEP(defer, vyrtue, true)
    PHP_SUBST(DEFER_SHARED_LIBADD)
fi
