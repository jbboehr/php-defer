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
#include "ext/vyrtue/php_vyrtue.h"

#include "php_defer.h"

DEFER_LOCAL
zend_ast *defer_process_visitor_call_defer_leave(zend_ast *ast, struct vyrtue_context *ctx)
{
    return NULL;
}

DEFER_LOCAL
zend_ast *defer_process_visitor_function_leave(zend_ast *ast, struct vyrtue_context *ctx)
{
    return NULL;
}
