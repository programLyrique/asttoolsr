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
#include <sstream>
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

inline void write_edge(std::ostream& stream,  uint64_t from_id, uint64_t to_id) {
  stream << " v_" << from_id << " -> v_" << to_id << ";" << std::endl;
}


void traverse_ast(std::ostream& stream, const std::string& role, uint64_t& id, SEXP ast) {
  uint64_t my_id = id;
  //start writing the node
  stream << " v_" << my_id << " [label=\"";

  switch(TYPEOF(ast)) {
  case EXPRSXP: {
      stream << "expression\"];" << std::endl;

      size_t nb_exprs = Rf_xlength(ast);
      for(size_t i = 0 ; i < nb_exprs ; i++) {
        id++;
        write_edge(stream, my_id, id);
        traverse_ast(stream, "", id, VECTOR_ELT(ast, i));
      }
    }
    break;

  case LANGSXP: {
      // TODO: Special treatment for control instructions (if, while, repeat and so on)
      if(role != "") {
        stream << role << ":: ";
      }

      std::string call_name="";
      if(Rf_isSymbol(CAR(ast))) {
        call_name = CHAR(PRINTNAME(CAR(ast)));
        //Rprintf("Call: [%s]\n", call_name.c_str());
      }

      if(call_name == "<-" || call_name == "=") {
        //assignment
        stream << "assign\", color = azure3];" << std::endl;

        id++;
        write_edge(stream, my_id, id);
        traverse_ast(stream, "variable", id, CAR(CDR(ast)));

        id++;
        write_edge(stream, my_id, id);
        traverse_ast(stream, "", id, CAR(CDDR(ast)));
        break;
      }
      else if(call_name == "if") {
          stream << "if\", color = darkgreen];" << std::endl;
          id++;
          write_edge(stream, my_id, id);
          traverse_ast(stream, "condition", id, CAR(CDR(ast)));
          id++;
          write_edge(stream, my_id, id);
          traverse_ast(stream, "true branch", id, CAR(CDDR(ast)));
          if(CDDR(ast) != R_NilValue) {
            id++;
            write_edge(stream, my_id, id);
            traverse_ast(stream, "false branch", id, CAR(CDDDR(ast)));
          }
        break;
      }
      else if(call_name == "for") {
        // for(1 in 1:10) print(i)
        // We get 4 tokens: for, i, 1:10, and print(i)
        stream << "for\", color = darkgreen];" << std::endl;

        id++;
        write_edge(stream, my_id, id);
        traverse_ast(stream, "iteration variable", id, CAR(CDR(ast)));

        id++;
        write_edge(stream, my_id, id);
        traverse_ast(stream, "range", id, CAR(CDDR(ast)));

        id++;
        write_edge(stream, my_id, id);
        traverse_ast(stream, "body", id, CAR(CDDDR(ast)));
        break;
      }
      else if (call_name == "while") {
        stream << "while\", color = darkgreen];" << std::endl;

        id++;
        write_edge(stream, my_id, id);
        traverse_ast(stream, "condition", id, CAR(CDR(ast)));

        id++;
        write_edge(stream, my_id, id);
        traverse_ast(stream, "body", id, CAR(CDDR(ast)));

        break;
      }
      else if (call_name == "repeat") {
        stream << "repeat\", color = darkgreen];" << std::endl;

        id++;
        write_edge(stream, my_id, id);
        traverse_ast(stream, "body", id, CAR(CDR(ast)));
        break;
      }
      else if(call_name == "break") {
        stream << "break\", color = darkgreen];" << std::endl;
        break;
      }
      else if(call_name == "continue") {
        stream << "continue\", color = darkgreen];" << std::endl;
        break;
      }
      else if(call_name == "function") {
        stream << "function\"];" << std::endl;

        id++;
        write_edge(stream, my_id, id);
        traverse_ast(stream, "arguments", id, CAR(CDR(ast)));

        id++;
        write_edge(stream, my_id, id);
        traverse_ast(stream, "body", id, CAR(CDDR(ast)));
        break;
      }

      std::string prefix = "argument ";
      if(call_name == ".Call") {
        stream << "native call\", color = goldenrod2];" << std::endl;
      }
      else if(call_name != "{") {
        stream  << "call\", color = goldenrod2];" << std::endl;
        // call name
        id++;
        write_edge(stream, my_id, id);
        traverse_ast(stream, "call name", id, CAR(ast));
      }
      else {
        stream << "\"];" << std::endl;
        prefix  = "statement ";
      }

      // arguments
      size_t i = 0;
      for(SEXP cons = CDR(ast) ; cons != R_NilValue; cons = CDR(cons), i++) {
        id++;
        std::string arg = prefix + std::to_string(i);
        write_edge(stream, my_id, id);
        traverse_ast(stream, arg, id, CAR(cons) );
      }
    }
    break;
  case LISTSXP: {
   stream << "parameters\"];" << std::endl;
    // the actual parameters
    size_t i = 0;
    for(SEXP cons = ast ; cons != R_NilValue; cons = CDR(cons), i++) {
      id++;
      std::string arg = std::string("param_") + std::to_string(i);
      write_edge(stream, my_id, id);
      traverse_ast(stream, arg, id, TAG(cons) );
    }
  }
    break;
  case SYMSXP: {
      if(role != "") {
        stream << role << ":: ";
      }
      // symbols are red
      stream << CHAR(PRINTNAME(ast)) << "\", color = red];" << std::endl;
    }
    break;
  case NILSXP: {
      //litterals are blue
      stream << "NULL\", color = blue];" << std::endl;
    }
    break;
  case LGLSXP: {
      auto lgl = Rf_asLogical(ast);
      if(lgl == NA_LOGICAL) {
        stream << "NA";
      }
      else if(lgl == 0) {
        stream << "FALSE";
      }
      else {
        stream << "TRUE";
      }
      stream << " : logical\", color = blue];" << std::endl;
    }
    break;
  case INTSXP: {
    stream << Rf_asInteger(ast) << " : int\", color = blue];" << std::endl;
    }
    break;
  case REALSXP: {
    stream << Rf_asReal(ast) << " : real\", color = blue];" << std::endl;
    }
    break;
  case CPLXSXP: {
    auto cplx = Rf_asComplex(ast);
    stream << cplx.r << " + " << cplx.i << " i : complex\", color = blue];" << std::endl;
    }
    break;
  case DOTSXP: {
      stream << "...\", color = darkred];" << std::endl;
    }
    break;
  case BCODESXP: {
    stream << "bytecode\", color = darkred];" << std::endl;
    }
    break;
  case RAWSXP: {
      stream << (Rf_xlength(ast) > 0 ? write_escaped(std::to_string(*RAW(ast))) : "empty") << " : raw\", color = blue];" << std::endl;
    }
    break;
  case STRSXP: {
      std::string str = write_escaped(CHAR(STRING_ELT(ast, 0)));
      // only keep the first 30 characters
      if(str.size() > 33) {
        str.resize(30, ' ');
        str += "...";
      }

      stream << str << " : str\", color = blue];" << std::endl;
    }
    break;
    // the following ones should never happend in an AST
  case EXTPTRSXP: {
    stream << "extptr\", color = darkred];" << std::endl;
    }
    break;
  case WEAKREFSXP: {
    stream << "wearkref\", color = darkred];" << std::endl;
    }
    break;
  case ENVSXP: {
    stream << "environment\", color = darkred];" << std::endl;
    }
    break;
  default: {
    Rf_error("Unexpected SEXP %s\n", Rf_type2char(TYPEOF(ast)));
    }
  }
}

SEXP generate_dot_ast(SEXP ast) {
  std::ostringstream out;
  uint64_t id = 0;
  out << "digraph ast {" << std::endl;
  traverse_ast(out, "", id, ast);
  out << "}" << std::endl;

  return Rf_mkString(out.str().c_str());
}
