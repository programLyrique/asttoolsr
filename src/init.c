/*
 asttoolsr, a R package that can generate different abstract representations of an R program, such as an AST
 Copyright (C) 2022  Pierre Donat-Bouillud

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define R_NO_REMAP

#include <R.h>
#include <R_ext/Rdynload.h>
#include <Rinternals.h>
#include <stdlib.h> // for NULL // Based on evil suggestion

#include "ast.h"

static const R_CallMethodDef callMethods[] = {
  /* name						casted ptr to function			# of args */
  // Generic record related
  {"generate_dot_ast",					(DL_FUNC) &generate_dot_ast,					1},


  // Must have at the end
  {NULL,						NULL,								0}
};

void R_init_sxpdb(DllInfo* dll) {
  R_registerRoutines(dll, NULL, callMethods, NULL, NULL);
  R_useDynamicSymbols(dll, FALSE);
  //R_RegisterCCallable("sxpdb", "add_val", (DL_FUNC) &add_val);
}

