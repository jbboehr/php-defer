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

static void ast_dtor(zval *value)
{
    if (EXPECTED(Z_TYPE_P(value) == IS_PTR)) {
        zend_ast_destroy(Z_PTR_P(value));
        ZVAL_NULL(value);
    }
}

static HashTable *get_or_update_scope_ht(struct vyrtue_context *ctx)
{
    HashTable *top = vyrtue_context_scope_stack_top_ht(ctx);
    HashTable *arr = zend_hash_str_find_ptr(top, ZEND_STRL(DEFER_KEY));
    if (NULL == arr) {
        arr = zend_arena_calloc(vyrtue_context_get_arena_ptr(ctx), 1, sizeof(HashTable));
        zend_hash_init(arr, 2, NULL, ast_dtor, 0);
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
    zend_ast *scope = vyrtue_context_scope_stack_top_ast(ctx);
    HashTable *arr = get_or_update_scope_ht(ctx);

    fprintf(stderr, "TOP AST KIND: %d (ZEND_AST_FUNC_DECL=%d)\n", scope->kind, ZEND_AST_FUNC_DECL);

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
        zend_ast *stmt_list = zend_ast_create_list_1(ZEND_AST_STMT_LIST, arg);

        closure_ast = zend_ast_create_decl(
            ZEND_AST_ARROW_FUNC,
            0,
            ast->lineno,
            NULL,
            ZSTR_INIT_LITERAL("{closure}", 0),
            NULL,
            NULL,
            stmt_list,
            NULL,
            NULL
        );
    } else {
        closure_ast = arg;
    }

    // Convert to array_push
    zend_string *str;

    str = zend_string_init_interned(ZEND_STRL("deferred_fns"), 0);
    zend_ast *register_ast = zend_ast_create_zval_from_str(str);
    register_ast->lineno = ast->lineno;

    zend_ast *var_ast = zend_ast_create_1(ZEND_AST_VAR, register_ast);

    zend_ast *arg_list_ast = zend_ast_create_list_2(ZEND_AST_ARG_LIST, register_ast, closure_ast);

    str = zend_string_init_interned(ZEND_STRL("\\array_push"), 0);
    zend_ast *function_ast = zend_ast_create_zval_from_str(str);

    zend_ast *call_ast = zend_ast_create_2(ZEND_AST_CALL, function_ast, arg_list_ast);

    // store the old argument in our array for later
    zend_hash_next_index_insert_ptr(arr, arg);

    return call_ast;
}

DEFER_LOCAL
zend_ast *defer_process_visitor_function_leave(zend_ast *ast, struct vyrtue_context *ctx)
{
    HashTable *arr = get_scope_ht(ctx);

    fprintf(stderr, "LEAVING SCOPE kind=%d, count=%d\n", ast->kind, arr ? zend_array_count(arr) : 0);

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

    fprintf(stderr, "MooO? %d %d\n", ast->kind, ZEND_AST_FUNC_DECL);

    zend_ast_decl *decl = (zend_ast_decl *) ast;
    zend_ast *stmt = decl->child[2];

    if (stmt->kind != ZEND_AST_STMT_LIST) {
        zend_throw_exception_ex(zend_ce_compile_error, 0, "defer: function body not a statement list");
        return NULL;
    }

    //    if (stmt->kind != ZEND_AST_STMT_LIST) {
    //        zend_throw_exception_ex(zend_ce_compile_error, 0, "defer: function body not a statement list");
    //        return NULL;
    //    }

    fprintf(stderr, "AST |||%s|||\n", zend_ast_export("", ast, "")->val);

    return NULL;
}
