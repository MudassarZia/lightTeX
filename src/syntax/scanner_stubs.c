/*
 * Stub external scanner for tree-sitter-latex.
 * tree-sitter-latex declares an external scanner but some versions
 * don't ship the implementation. This provides empty stubs so linking
 * succeeds.
 */
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

void* tree_sitter_latex_external_scanner_create(void) {
    return NULL;
}

void tree_sitter_latex_external_scanner_destroy(void* payload) {
    (void)payload;
}

unsigned tree_sitter_latex_external_scanner_serialize(void* payload, char* buffer) {
    (void)payload;
    (void)buffer;
    return 0;
}

void tree_sitter_latex_external_scanner_deserialize(void* payload,
                                                     const char* buffer,
                                                     unsigned length) {
    (void)payload;
    (void)buffer;
    (void)length;
}

bool tree_sitter_latex_external_scanner_scan(void* payload,
                                              void* lexer,
                                              const bool* valid_symbols) {
    (void)payload;
    (void)lexer;
    (void)valid_symbols;
    return false;
}
