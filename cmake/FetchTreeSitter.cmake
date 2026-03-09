include(FetchContent)

FetchContent_Declare(
    treesitter
    GIT_REPOSITORY https://github.com/tree-sitter/tree-sitter.git
    GIT_TAG v0.25.3
    GIT_SHALLOW ON
)

FetchContent_Declare(
    treesitter_latex
    GIT_REPOSITORY https://github.com/latex-lsp/tree-sitter-latex.git
    GIT_TAG v0.3.0
    GIT_SHALLOW ON
)

FetchContent_MakeAvailable(treesitter treesitter_latex)
