
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
