
<!-- README.md is generated from README.Rmd. Please edit that file -->

# asttoolsr

<!-- badges: start -->
<!-- badges: end -->

The goal of this package is to generate pretty AST graphs of R
expressions, through the DOT format. It embeds some information in the
graph such as the type of litterals, if the node is control flow or not
and so on.

## Installation

You can install the development version from
[GitHub](https://github.com/) with:

``` r
# install.packages("devtools")
devtools::install_github("programLyrique/asttoolsr")
```

## Example

``` r
library(asttoolsr)

# from a string
string_dot <- generate_dot_ast(parse(text = "1 + 1"))

# from a file
file_dot <- generate_dot_ast(parse(file = "R/asttoolsr.R"))

# from a language expression
lang_dot <- generate_dot_ast(quote(1 + 1))

# generate and directly save the results to a file
file_dot <- ast_to_dot_file(quote(1 + 1), "result.dot")
```

You can then render the graph using dot, directly or through the DOT
package:

``` r
library(DOT)

dot(string_dot)
```
