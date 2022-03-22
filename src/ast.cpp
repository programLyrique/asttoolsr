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

inline void start_attributes(std::ostream& stream) {
  stream << " [";
}

inline void end_attributes(std::ostream& stream) {
  stream << " ];" << std::endl;
}

inline void set_label(std::ostream& stream, const std::string& sem1, const std::string& sem2, const std::string& type, const std::string& source) {
  std::string semantics=sem1;
  if(sem1 != "" && sem2 != "") {
    semantics += "; ";
    semantics += sem2;
  }
  else {
    semantics = sem2;
  }
  stream << "label\"[" <<  semantics << "](" << type<< "){" << source << "}\" ";
}

inline void set_color(std::ostream& stream, const std::string& color) {
  stream << "color = " << color;
}

void traverse_ast(std::ostream& stream, const std::string& role, uint64_t& id, SEXP ast) {
  uint64_t my_id = id;
  //start writing the node
  stream << " v_" << my_id;

  start_attributes(stream);

  switch(TYPEOF(ast)) {
  case EXPRSXP: {
      set_label(stream, "", "", "", "expression");
      end_attributes(stream);

      size_t nb_exprs = Rf_xlength(ast);
      for(size_t i = 0 ; i < nb_exprs ; i++) {
        id++;
        write_edge(stream, my_id, id);
        traverse_ast(stream, "", id, VECTOR_ELT(ast, i));
      }
    }
    break;

  case LANGSXP: {
      std::string call_name="";
      if(Rf_isSymbol(CAR(ast))) {
        call_name = CHAR(PRINTNAME(CAR(ast)));
      }

      if(call_name == "<-" || call_name == "=") {
        //assignment
        set_label(stream, role, "assign", "", call_name);
        set_color(stream, "azure3");
        end_attributes(stream);

        id++;
        write_edge(stream, my_id, id);
        traverse_ast(stream, "variable", id, CAR(CDR(ast)));

        id++;
        write_edge(stream, my_id, id);
        traverse_ast(stream, "", id, CAR(CDDR(ast)));
        break;
      }
      else if(call_name == "if") {
          set_label(stream, role, "control flow", "", "if");
          set_color(stream, "darkgreen");
          end_attributes(stream);

          id++;
          write_edge(stream, my_id, id);
          traverse_ast(stream, "condition", id, CAR(CDR(ast)));

          id++;
          write_edge(stream, my_id, id);
          traverse_ast(stream, "true branch", id, CAR(CDDR(ast)));
          if(CDDDR(ast) != R_NilValue) {
            id++;
            write_edge(stream, my_id, id);
            traverse_ast(stream, "false branch", id, CAR(CDDDR(ast)));
          }
        break;
      }
      else if(call_name == "for") {
        // for(1 in 1:10) print(i)
        // We get 4 tokens: for, i, 1:10, and print(i)
        set_label(stream, role, "control flow", "", "for");
        set_color(stream, "darkgreen");
        end_attributes(stream);

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
        set_label(stream, role, "control flow", "", "while");
        set_color(stream, "darkgreen");
        end_attributes(stream);

        id++;
        write_edge(stream, my_id, id);
        traverse_ast(stream, "condition", id, CAR(CDR(ast)));

        id++;
        write_edge(stream, my_id, id);
        traverse_ast(stream, "body", id, CAR(CDDR(ast)));

        break;
      }
      else if (call_name == "repeat") {
        set_label(stream, role, "control flow", "", "repeat");
        set_color(stream, "darkgreen");
        end_attributes(stream);

        id++;
        write_edge(stream, my_id, id);
        traverse_ast(stream, "body", id, CAR(CDR(ast)));
        break;
      }
      else if(call_name == "break") {
        set_label(stream, role, "control flow", "", "break");
        set_color(stream, "darkgreen");
        end_attributes(stream);
        break;
      }
      else if(call_name == "continue") {
        set_label(stream, role, "control flow", "", "continue");
        set_color(stream, "darkgreen");
        end_attributes(stream);
        break;
      }
      else if(call_name == "function") {
        set_label(stream, role, "", "", "function");
        end_attributes(stream);

        id++;
        write_edge(stream, my_id, id);
        traverse_ast(stream, "parameters", id, CAR(CDR(ast)));

        id++;
        write_edge(stream, my_id, id);
        traverse_ast(stream, "body", id, CAR(CDDR(ast)));
        break;
      }

      std::string prefix = "argument ";
      SEXP start_sexp = R_NilValue;
      if(call_name == ".Call") {
        set_label(stream, role, "call", "", ".Call");
        set_color(stream, "goldenrod2");
        end_attributes(stream);

        // call name
        id++;
        write_edge(stream, my_id, id);
        traverse_ast(stream, "call name", id, CADR(ast));

        start_sexp = CDDR(ast);
      }
      else if(call_name != "{") {
        set_label(stream, role, "call", "", "");
        set_color(stream, "goldenrod2");
        end_attributes(stream);
        // call name
        id++;
        write_edge(stream, my_id, id);
        traverse_ast(stream, "call name", id, CAR(ast));
        start_sexp = CDR(ast);
      }
      else {
        set_label(stream, role, "block", "", "");// should be { here for the value
        set_color(stream, "darkred");
        end_attributes(stream);
        start_sexp = CDR(ast);
        prefix  = "statement ";
      }

      // arguments
      size_t i = 0;
      for(SEXP cons = start_sexp ; cons != R_NilValue; cons = CDR(cons), i++) {
        id++;
        std::string arg = prefix + std::to_string(i);
        write_edge(stream, my_id, id);
        traverse_ast(stream, arg, id, CAR(cons) );
      }
    }
    break;
  case LISTSXP: {
    set_label(stream, role, "", "pairlist", "");
    end_attributes(stream);
    // the actual parameters
    size_t i = 0;
    for(SEXP cons = ast ; cons != R_NilValue; cons = CDR(cons), i++) {
      id++;
      std::string param = std::string("param_") + std::to_string(i);
      write_edge(stream, my_id, id);
      traverse_ast(stream, param, id, TAG(cons) );
    }
  }
    break;
  case SYMSXP: {
      set_label(stream, role, "", "symbol", CHAR(PRINTNAME(ast)));
      set_color(stream, "red");
      end_attributes(stream);
    }
    break;
  case NILSXP: {
      //litterals are blue
      set_label(stream, role, "litteral", "NULL", "NULL");
      set_color(stream, "blue");
      end_attributes(stream);    }
    break;
  case LGLSXP: {
      auto lgl = Rf_asLogical(ast);
      std::string value = "";
      if(lgl == NA_LOGICAL) {
        value =  "NA";
      }
      else if(lgl == 0) {
        value = "FALSE";
      }
      else {
        value = "TRUE";
      }
      set_label(stream, role, "litteral", "logical", value);
      set_color(stream, "blue");
      end_attributes(stream);
    }
    break;
  case INTSXP: {
    set_label(stream, role, "litteral", "integer", std::to_string(Rf_asInteger(ast)));
    set_color(stream, "blue");
    end_attributes(stream);
    }
    break;
  case REALSXP: {
    set_label(stream, role, "litteral", "real", std::to_string(Rf_asReal(ast)));
    set_color(stream, "blue");
    end_attributes(stream);
    }
    break;
  case CPLXSXP: {
    auto cplx = Rf_asComplex(ast);
    set_label(stream, role, "litteral", "complex", std::to_string(cplx.r) + " + " + std::to_string(cplx.i) + " i");
    set_color(stream, "blue");
    end_attributes(stream);
    }
    break;
  case DOTSXP: {
    set_label(stream, role, "variable number of parameters", "dots", "...");
    set_color(stream, "darkred");
    end_attributes(stream);
    }
    break;
  case BCODESXP: {
    set_label(stream, role, "", "bytecode", "");
    set_color(stream, "darkred");
    end_attributes(stream);
    }
    break;
  case RAWSXP: {
      set_label(stream, role, "litteral", "raw", Rf_xlength(ast) > 0 ? write_escaped(std::to_string(*RAW(ast))) : "empty");
      set_color(stream, "blue");
      end_attributes(stream);
    }
    break;
  case STRSXP: {
      std::string str = write_escaped(CHAR(STRING_ELT(ast, 0)));
      // only keep the first 30 characters
      if(str.size() > 33) {
        str.resize(30, ' ');
        str += "...";
      }
      set_label(stream, role, "litteral", "str", str);
      set_color(stream, "blue");
      end_attributes(stream);
    }
    break;
    // the following ones should never happen in an AST
  case EXTPTRSXP: {
    set_label(stream, role, "", "extptr", "");
    set_color(stream, "darkred");
    end_attributes(stream);
    }
    break;
  case WEAKREFSXP: {
    set_label(stream, role, "", "weakref", "");
    set_color(stream, "darkred");
    end_attributes(stream);
    }
    break;
  case ENVSXP: {
    set_label(stream, role, "", "environment", "");
    set_color(stream, "darkred");
    end_attributes(stream);
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
