/**
 * Copyright (C) 2024 John Boehr & contributors
 *
 * This file is part of php-vyrtue.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>

#include "Zend/zend_API.h"
#include "Zend/zend_constants.h"
#include "Zend/zend_exceptions.h"
#include <Zend/zend_interfaces.h>
#include "Zend/zend_ini.h"
#include "Zend/zend_modules.h"
#include "Zend/zend_operators.h"
#include "main/SAPI.h"
#include "main/php.h"
#include "main/php_ini.h"
#include "ext/standard/info.h"
#include "ext/standard/php_string.h"
#include <ext/standard/php_var.h>
#include "ext/vyrtue/php_vyrtue.h"

#include "php_defer.h"

ZEND_DECLARE_MODULE_GLOBALS(defer);

PHP_INI_BEGIN()
PHP_INI_END()

static PHP_RINIT_FUNCTION(defer)
{
#if defined(COMPILE_DL_DEFER) && defined(ZTS)
    ZEND_TSRMLS_CACHE_UPDATE();
#endif

    return SUCCESS;
}

DEFER_LOCAL
zend_ast *defer_process_visitor_call_defer_leave(zend_ast *ast, struct vyrtue_context *ctx);

DEFER_LOCAL
zend_ast *defer_process_visitor_function_leave(zend_ast *ast, struct vyrtue_context *ctx);

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO(call_functions_arginfo, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, functions, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

PHP_FUNCTION(call_functions)
{
    HashTable *functions;
    zval *fn;
    zend_object *exception;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ARRAY_HT(functions)
    ZEND_PARSE_PARAMETERS_END();

    ZEND_HASH_REVERSE_FOREACH_VAL(functions, fn)
    {
        zval retval = {0};
        ZVAL_NULL(&retval);
        call_user_function(NULL, NULL, fn, &retval, 0, NULL);
        zval_ptr_dtor(&retval);

        if (EG(exception)) {
            exception = EG(exception);
            zend_exception_save();
        }
    }
    ZEND_HASH_FOREACH_END();

    if (exception) {
        zend_exception_restore();
    }

    RETURN_NULL();
}

static PHP_MINIT_FUNCTION(defer)
{
    int flags = CONST_CS | CONST_PERSISTENT;
    zend_string *tmp;

    REGISTER_INI_ENTRIES();

    // Register constants
    REGISTER_STRING_CONSTANT("DeferExt\\VERSION", (char *) PHP_DEFER_VERSION, flags);

    // Register vyrtue visitors
    tmp = zend_string_init_interned(ZEND_STRL("DeferExt\\defer"), 1);
    vyrtue_register_function_visitor("defer fcall", tmp, NULL, defer_process_visitor_call_defer_leave);
    zend_string_release(tmp);

    vyrtue_register_kind_visitor("defer fdecl", ZEND_AST_ARROW_FUNC, NULL, defer_process_visitor_function_leave);
    vyrtue_register_kind_visitor("defer fdecl", ZEND_AST_METHOD, NULL, defer_process_visitor_function_leave);
    vyrtue_register_kind_visitor("defer fdecl", ZEND_AST_CLOSURE, NULL, defer_process_visitor_function_leave);
    vyrtue_register_kind_visitor("defer fdecl", ZEND_AST_FUNC_DECL, NULL, defer_process_visitor_function_leave);

    return SUCCESS;
}

static PHP_MSHUTDOWN_FUNCTION(defer)
{
    UNREGISTER_INI_ENTRIES();

    return SUCCESS;
}

static PHP_MINFO_FUNCTION(defer)
{
    php_info_print_table_start();
    php_info_print_table_row(2, "Version", PHP_DEFER_VERSION);
    php_info_print_table_row(2, "Released", PHP_DEFER_RELEASE);
    php_info_print_table_row(2, "Authors", PHP_DEFER_AUTHORS);
    php_info_print_table_end();

    DISPLAY_INI_ENTRIES();
}

static PHP_GINIT_FUNCTION(defer)
{
#if defined(COMPILE_DL_DEFER) && defined(ZTS)
    ZEND_TSRMLS_CACHE_UPDATE();
#endif
    memset(defer_globals, 0, sizeof(zend_defer_globals));
}

// clang-format off
const zend_function_entry defer_functions[] = {
    ZEND_RAW_FENTRY("DeferExt\\call_functions", ZEND_FN(call_functions), call_functions_arginfo, 0)
    PHP_FE_END
};
// clang-format on

static const zend_module_dep defer_deps[] = {
    {"vyrtue",  NULL, NULL, MODULE_DEP_REQUIRED},
    {"ast",     NULL, NULL, MODULE_DEP_OPTIONAL},
    {"opcache", NULL, NULL, MODULE_DEP_OPTIONAL},
    ZEND_MOD_END,
};

zend_module_entry defer_module_entry = {
    STANDARD_MODULE_HEADER_EX,
    NULL,
    defer_deps,                /* Deps */
    PHP_DEFER_NAME,            /* Name */
    defer_functions,           /* Functions */
    PHP_MINIT(defer),          /* MINIT */
    PHP_MSHUTDOWN(defer),      /* MSHUTDOWN */
    PHP_RINIT(defer),          /* RINIT */
    NULL,                      /* RSHUTDOWN */
    PHP_MINFO(defer),          /* MINFO */
    PHP_DEFER_VERSION,         /* Version */
    PHP_MODULE_GLOBALS(defer), /* Globals */
    PHP_GINIT(defer),          /* GINIT */
    NULL,
    NULL,
    STANDARD_MODULE_PROPERTIES_EX,
};

#ifdef COMPILE_DL_DEFER
#if defined(ZTS)
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(defer) // Common for all PHP extensions which are build as shared modules
#endif
