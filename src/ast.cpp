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

#include "ast.h"

#include <iostream>
#include <fstream>
#include <string>



// Inspired by https://stackoverflow.com/questions/2417588/escaping-a-c-string
std::string const write_escaped(std::string const& s) {
  std::string out;
  out.reserve(s.size());
  for (std::string::const_iterator i = s.begin(), end = s.end(); i != end; ++i) {
    unsigned char c = *i;
    if (' ' <= c and c <= '~' and c != '\\' and c != '"') {
      out += c;
    }
    else {
      out += '\\';
      switch(c) {
      case '"':  out += '"';  break;
      case '\\': out += '\\'; break;
      case '\t': out += 't';  break;
      case '\r': out += 'r';  break;
      case '\n': out += 'n';  break;
      default:
        char const* const hexdig = "0123456789ABCDEF";
      out += 'x';
      out += hexdig[c >> 4];
      out += hexdig[c & 0xF];
      }
    }
  }
  return out;
}


void traverse_ast(std::ostream& stream, const std::string& parent_name, SEXP ast) {
  switch(TYPEOF(ast)) {
  case EXPRSXP: {

    }
    break;

  case LANGSXP: {
    }
    break;
  case SYMSXP: {

    }
    break;
  }
}

SEXP generate_dot_ast(SEXP ast, SEXP filename) {
  // Traverse the AST

  // Generate proper edges and proper nodes

  return R_NilValue;
}
