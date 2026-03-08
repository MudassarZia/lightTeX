include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.15.2
    GIT_SHALLOW ON
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

find_package(Qt6 REQUIRED COMPONENTS Widgets Test)

# Shared sources for tests (no main.cpp, no UI assembly)
set(LIGHTTEX_LIB_SOURCES
    ${CMAKE_SOURCE_DIR}/src/core/PieceTable.cpp
    ${CMAKE_SOURCE_DIR}/src/core/LineIndex.cpp
    ${CMAKE_SOURCE_DIR}/src/core/Selection.cpp
    ${CMAKE_SOURCE_DIR}/src/core/History.cpp
    ${CMAKE_SOURCE_DIR}/src/core/Document.cpp
    ${CMAKE_SOURCE_DIR}/src/syntax/Highlighter.cpp
    ${CMAKE_SOURCE_DIR}/src/syntax/SyntaxHighlighter.cpp
    ${CMAKE_SOURCE_DIR}/src/compiler/Engine.cpp
    ${CMAKE_SOURCE_DIR}/src/compiler/LogParser.cpp
    ${CMAKE_SOURCE_DIR}/src/compiler/SyncTeX.cpp
    ${CMAKE_SOURCE_DIR}/src/compiler/Pipeline.cpp
    ${CMAKE_SOURCE_DIR}/src/pdf/PdfRenderer.cpp
    ${CMAKE_SOURCE_DIR}/src/pdf/PageCache.cpp
    ${CMAKE_SOURCE_DIR}/src/pdf/PdfWidget.cpp
    ${CMAKE_SOURCE_DIR}/src/theme/Theme.cpp
    ${CMAKE_SOURCE_DIR}/src/theme/ThemeManager.cpp
    ${CMAKE_SOURCE_DIR}/src/editor/EditorWidget.cpp
    ${CMAKE_SOURCE_DIR}/src/editor/LineNumberArea.cpp
    ${CMAKE_SOURCE_DIR}/src/editor/BracketMatcher.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/MainWindow.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/StatusBar.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/CommandPalette.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/CompilePanel.cpp
    ${CMAKE_SOURCE_DIR}/src/app/AppState.cpp
    ${CMAKE_SOURCE_DIR}/src/app/Actions.cpp
)

set(TREESITTER_TEST_SOURCES
    ${treesitter_SOURCE_DIR}/lib/src/lib.c
    ${treesitter_latex_SOURCE_DIR}/src/parser.c
    ${treesitter_latex_SOURCE_DIR}/src/scanner.c
)
set_source_files_properties(${TREESITTER_TEST_SOURCES} PROPERTIES LANGUAGE C)

# Create a static library for testing
add_library(lighttex_lib STATIC
    ${LIGHTTEX_LIB_SOURCES}
    ${TREESITTER_TEST_SOURCES}
)
set_target_properties(lighttex_lib PROPERTIES AUTOMOC ON)
target_include_directories(lighttex_lib PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${treesitter_SOURCE_DIR}/lib/include
    ${treesitter_latex_SOURCE_DIR}/src
)
target_link_libraries(lighttex_lib PUBLIC
    Qt6::Widgets
    Qt6::Test
    tomlplusplus::tomlplusplus
)

# Define test fixture path
set(FIXTURE_DIR "${CMAKE_SOURCE_DIR}/tests/fixtures")

# Helper function to add a test
function(add_lighttex_test name)
    add_executable(${name} ${name}.cpp)
    target_link_libraries(${name} PRIVATE lighttex_lib GTest::gtest_main)
    target_compile_definitions(${name} PRIVATE
        FIXTURE_DIR="${FIXTURE_DIR}"
    )
    add_test(NAME ${name} COMMAND ${name})
    # For widget tests on CI
    set_tests_properties(${name} PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
    )
endfunction()

# Logic tests (Google Test)
add_lighttex_test(test_piecetable)
add_lighttex_test(test_selection)
add_lighttex_test(test_history)
add_lighttex_test(test_document)
add_lighttex_test(test_highlighter)
add_lighttex_test(test_logparser)
add_lighttex_test(test_synctex)
add_lighttex_test(test_pagecache)
add_lighttex_test(test_pdfrenderer)
add_lighttex_test(test_theme)

# Widget tests (QTest via Google Test wrapper)
add_lighttex_test(test_commandpalette)
add_lighttex_test(test_editorwidget)
