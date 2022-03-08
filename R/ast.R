#' @export
#' @param ast expression, coming from a call to parse for instance
#' @return NULL, usfeul for the side effect of creating the DOT file
generate_dot_ast <- function(ast, dot_name) {
  stopifnot(is.expression(ast))
  .Call(ASTTOOLSR_generate_dot_ast, ast)
}
