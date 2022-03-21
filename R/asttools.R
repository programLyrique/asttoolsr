#' @export
#' @param ast expression, coming from a call to parse for instance
#' @return NULL, usfeul for the side effect of creating the DOT file
generate_dot_ast <- function(ast) {
  stopifnot(is.expression(ast))
  .Call(ASTTOOLSR_generate_dot_ast, ast)
}

.onUnload <- function (libpath) {
  library.dynam.unload("asttoolsr", libpath)
}
