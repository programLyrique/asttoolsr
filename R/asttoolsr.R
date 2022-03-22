#' @export
#' @param ast expression, coming from a call to parse for instance
#' @return character vector of the resulting DOT representation
generate_dot_ast <- function(ast) {
  stopifnot(is.expression(ast) || is.language(ast) || is.symbol(ast))
  .Call(ASTTOOLSR_generate_dot_ast, ast)
}

#' @export
#' @param ast expression, coming from a call to parse for instance
#' @param character vector file name
#' @return character vector of the resulting DOT representation
ast_to_dot_file <- function(ast, filename) {
  dot <- generate_dot_ast(ast)
  write(dot, file = filename)
}

.onUnload <- function (libpath) {
  library.dynam.unload("asttoolsr", libpath)
}
