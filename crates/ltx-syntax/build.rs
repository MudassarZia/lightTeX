use std::env;
use std::fs;
use std::path::Path;

fn main() {
    // The tree-sitter-latex crate references external scanner symbols
    // but doesn't include the scanner.c file.
    // We provide stub implementations to satisfy the linker.
    let out_dir = env::var("OUT_DIR").unwrap();
    let scanner_path = Path::new(&out_dir).join("scanner_stubs.c");

    fs::write(
        &scanner_path,
        r#"
#include <stdbool.h>
#include <stdlib.h>

void *tree_sitter_latex_external_scanner_create(void) {
    return NULL;
}

void tree_sitter_latex_external_scanner_destroy(void *payload) {
    (void)payload;
}

bool tree_sitter_latex_external_scanner_scan(void *payload, void *lexer, const bool *valid_symbols) {
    (void)payload;
    (void)lexer;
    (void)valid_symbols;
    return false;
}

unsigned tree_sitter_latex_external_scanner_serialize(void *payload, char *buffer) {
    (void)payload;
    (void)buffer;
    return 0;
}

void tree_sitter_latex_external_scanner_deserialize(void *payload, const char *buffer, unsigned length) {
    (void)payload;
    (void)buffer;
    (void)length;
}
"#,
    )
    .expect("Failed to write scanner stubs");

    cc::Build::new()
        .file(&scanner_path)
        .compile("tree_sitter_latex_scanner_stubs");
}
