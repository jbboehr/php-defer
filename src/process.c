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

#include <Zend/zend_API.h>
#include <Zend/zend_ast.h>
#include <Zend/zend_exceptions.h>
#include "ext/vyrtue/php_vyrtue.h"

#include "php_defer.h"

static const char DEFER_KEY[] = "defer_fcall_buffer";
static const char DEFER_HANDLER_FN_NAME[] = "DeferExt\\call_functions";
static const char DEFERRED_FUNCTIONS_REGISTER_NAME[] = "deferred_fns_d18734e4f6a7edefd5c29cc84d3e754d";

static HashTable *get_or_update_scope_ht(struct vyrtue_context *ctx)
{
    HashTable *top = vyrtue_context_scope_stack_top_ht(ctx);
    HashTable *arr = zend_hash_str_find_ptr(top, ZEND_STRL(DEFER_KEY));
    if (NULL == arr) {
        arr = zend_arena_calloc(vyrtue_context_get_arena_ptr(ctx), 1, sizeof(HashTable));
        zend_hash_init(arr, 2, NULL, NULL, 0);
        zend_hash_str_update_ptr(top, ZEND_STRL(DEFER_KEY), arr);
    }
    return arr;
}

static HashTable *get_scope_ht(struct vyrtue_context *ctx)
{
    HashTable *top = vyrtue_context_scope_stack_top_ht(ctx);
    return zend_hash_str_find_ptr(top, ZEND_STRL(DEFER_KEY));
}

DEFER_LOCAL
zend_ast *defer_process_visitor_call_defer_leave(zend_ast *ast, struct vyrtue_context *ctx)
{
    HashTable *arr = get_or_update_scope_ht(ctx);

    if (UNEXPECTED(ast->child[1]->kind != ZEND_AST_ARG_LIST)) {
        zend_error(E_WARNING, "defer: not a arg list");
        return NULL;
    }

    zend_ast_list *args = zend_ast_get_list(ast->child[1]);
    if (args->children != 1) {
        // @todo change to local exception class?
        zend_throw_exception_ex(zend_ce_compile_error, 0, "defer: only accepts one argument");
        return NULL;
    }

    zend_ast *arg = args->child[0];
    zend_ast *closure_ast;

    // Convert to closure
    if (arg->kind != ZEND_AST_CLOSURE && arg->kind != ZEND_AST_ARROW_FUNC) {
#if PHP_VERSION_ID >= 80200
        zend_ast *stmt_list = arg;
#else
        zend_ast *stmt_list = zend_ast_create_list_1(ZEND_AST_STMT_LIST, arg);
#endif

        zend_ast *arg_list = zend_ast_create_list_0(ZEND_AST_ARG_LIST);

        closure_ast = zend_ast_create_decl(
            ZEND_AST_ARROW_FUNC,
            0,
            ast->lineno,
            NULL,
            ZSTR_INIT_LITERAL("{closure}", 0),
            arg_list,
            NULL,
            stmt_list,
            NULL,
            NULL
        );
    } else {
        closure_ast = arg;
    }

    // Convert to array append
    zend_string *str;

    str = zend_string_init_interned(ZEND_STRL(DEFERRED_FUNCTIONS_REGISTER_NAME), 0);
    zend_ast *var_name_ast = zend_ast_create_zval_from_str(str);

    zend_ast *var_ast = zend_ast_create_ex_1(ZEND_AST_VAR, ZEND_ARRAY_SYNTAX_SHORT, var_name_ast);

    zend_ast *dim_ast = zend_ast_create_2(ZEND_AST_DIM, var_ast, NULL);

    zend_ast *assign_ast = zend_ast_create(ZEND_AST_ASSIGN, dim_ast, closure_ast);

    // store the old argument in our array for later
    zval tmp = {0};
    ZVAL_BOOL(&tmp, true);
    zend_hash_next_index_insert(arr, &tmp);

    // blow the old arg away to prevent it from being freed - gross
    args->child[0] = zend_ast_create_zval_from_long(0);

    return assign_ast;
}

static inline zend_ast *prepare_finally_stmts(void)
{
    zend_string *str;

    str = zend_string_init_interned(ZEND_STRL(DEFERRED_FUNCTIONS_REGISTER_NAME), 0);
    zend_ast *var_name_ast = zend_ast_create_zval_from_str(str);

    zend_ast *var_ast = zend_ast_create_1(ZEND_AST_VAR, var_name_ast);

    zend_ast *arg_list_ast = zend_ast_create_list_1(ZEND_AST_ARG_LIST, var_ast);

    str = zend_string_init_interned(ZEND_STRL(DEFER_HANDLER_FN_NAME), 0);
    zend_ast *fn_name_ast = zend_ast_create_zval_from_str(str);

    zend_ast *call_ast = zend_ast_create_2(ZEND_AST_CALL, fn_name_ast, arg_list_ast);

    return zend_ast_create_list_1(ZEND_AST_STMT_LIST, call_ast);
}

DEFER_LOCAL
zend_ast *defer_process_visitor_function_leave(zend_ast *ast, struct vyrtue_context *ctx)
{
    HashTable *arr = get_scope_ht(ctx);

    // nothing to do
    if (NULL == arr || zend_array_count(arr) <= 0) {
        return NULL;
    }

    // this is a top-level defer
    if (ast->kind == ZEND_AST_STMT_LIST) {
        zend_throw_exception_ex(zend_ce_compile_error, 0, "defer: top-level defer not currently supported");
        return NULL;
    }

    // this is a defer inside an arrow function
    if (ast->kind == ZEND_AST_ARROW_FUNC) {
        zend_throw_exception_ex(zend_ce_compile_error, 0, "defer: inside an arrow function currently supported");
        return NULL;
    }

    zend_ast_decl *decl = (zend_ast_decl *) ast;
    zend_ast *stmt = decl->child[2];

    if (stmt->kind != ZEND_AST_STMT_LIST) {
        zend_throw_exception_ex(zend_ce_compile_error, 0, "defer: function body not a statement list");
        return NULL;
    }

    // Wrap in try/finally
    zend_ast *empty_catches_ast = zend_ast_create_list_0(ZEND_AST_STMT_LIST);

    zend_ast *finally_ast = prepare_finally_stmts();

    zend_ast *try_ast = zend_ast_create_3(ZEND_AST_TRY, stmt, empty_catches_ast, finally_ast);

    zend_ast *new_list_ast = zend_ast_create_list_1(ZEND_AST_STMT_LIST, try_ast);

    // Swap
    decl->child[2] = new_list_ast;

    //    fprintf(stderr, "MOOO |%s|\n", zend_ast_export("", decl, "")->val);

    return NULL;
}
