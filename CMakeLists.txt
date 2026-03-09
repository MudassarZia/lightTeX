cmake_minimum_required(VERSION 3.21)
project(lightTex VERSION 0.2.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Auto-detect Qt6 on Windows if not specified
if(WIN32 AND NOT DEFINED CMAKE_PREFIX_PATH)
    file(GLOB _qt_hints "C:/Qt/6.*/msvc2022_64" "C:/Qt/6.*/msvc2019_64")
    if(_qt_hints)
        list(SORT _qt_hints ORDER DESCENDING)
        list(GET _qt_hints 0 _qt_best)
        list(APPEND CMAKE_PREFIX_PATH "${_qt_best}")
        message(STATUS "Auto-detected Qt6 at: ${_qt_best}")
    endif()
endif()

include(cmake/CompilerFlags.cmake)
include(cmake/FetchTreeSitter.cmake)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt6 QUIET COMPONENTS Pdf)
if(Qt6Pdf_FOUND)
    message(STATUS "Qt6::Pdf found — PDF rendering via QPdfDocument")
else()
    message(STATUS "Qt6::Pdf not found — will try Poppler or use stub")
endif()
find_package(tomlplusplus CONFIG QUIET)
if(NOT tomlplusplus_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        tomlplusplus
        GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
        GIT_TAG v3.4.0
    )
    FetchContent_MakeAvailable(tomlplusplus)
endif()

# Poppler is optional — PDF rendering is stubbed if not found
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(POPPLER_QT6 poppler-qt6)
endif()

set(LIGHTTEX_SOURCES
    src/main.cpp
    # core
    src/core/PieceTable.cpp
    src/core/LineIndex.cpp
    src/core/Selection.cpp
    src/core/History.cpp
    src/core/Document.cpp
    # syntax
    src/syntax/Highlighter.cpp
    src/syntax/SyntaxHighlighter.cpp
    # compiler
    src/compiler/Engine.cpp
    src/compiler/LogParser.cpp
    src/compiler/SyncTeX.cpp
    src/compiler/Pipeline.cpp
    # pdf
    src/pdf/PdfRenderer.cpp
    src/pdf/PageCache.cpp
    src/pdf/PdfWidget.cpp
    # theme
    src/theme/Theme.cpp
    src/theme/ThemeManager.cpp
    # editor
    src/editor/EditorWidget.cpp
    src/editor/LineNumberArea.cpp
    src/editor/BracketMatcher.cpp
    src/editor/FindReplaceBar.cpp
    # ui
    src/ui/MainWindow.cpp
    src/ui/StatusBar.cpp
    src/ui/CommandPalette.cpp
    src/ui/CompilePanel.cpp
    src/ui/FileTreeWidget.cpp
    # shortcuts
    src/shortcuts/Shortcuts.cpp
    # snippets
    src/snippets/Snippets.cpp
    src/snippets/SnippetSession.cpp
    # lsp
    src/lsp/LspClient.cpp
    src/lsp/CompletionWidget.cpp
    # app
    src/app/AppState.cpp
    src/app/Actions.cpp
)

set(LIGHTTEX_HEADERS
    # core
    src/core/PieceTable.h
    src/core/LineIndex.h
    src/core/Selection.h
    src/core/History.h
    src/core/Document.h
    # syntax
    src/syntax/Highlighter.h
    src/syntax/SyntaxHighlighter.h
    src/syntax/TokenKind.h
    # compiler
    src/compiler/Engine.h
    src/compiler/LogParser.h
    src/compiler/SyncTeX.h
    src/compiler/Pipeline.h
    # pdf
    src/pdf/PdfRenderer.h
    src/pdf/PageCache.h
    src/pdf/PdfWidget.h
    # theme
    src/theme/Theme.h
    src/theme/ThemeManager.h
    # editor
    src/editor/EditorWidget.h
    src/editor/LineNumberArea.h
    src/editor/BracketMatcher.h
    src/editor/FindReplaceBar.h
    # ui
    src/ui/MainWindow.h
    src/ui/StatusBar.h
    src/ui/CommandPalette.h
    src/ui/CompilePanel.h
    src/ui/FileTreeWidget.h
    # shortcuts
    src/shortcuts/Shortcuts.h
    # snippets
    src/snippets/Snippets.h
    src/snippets/SnippetSession.h
    # lsp
    src/lsp/Lsp.h
    src/lsp/JsonRpc.h
    src/lsp/LspTypes.h
    src/lsp/CompletionWidget.h
    # app
    src/app/AppState.h
    src/app/Actions.h
)

# Tree-sitter C sources
set(TREESITTER_SOURCES
    ${treesitter_SOURCE_DIR}/lib/src/lib.c
    ${treesitter_latex_SOURCE_DIR}/src/parser.c
    ${treesitter_latex_SOURCE_DIR}/src/scanner.c
)
set_source_files_properties(${TREESITTER_SOURCES} PROPERTIES LANGUAGE C)

add_executable(lightTex WIN32
    ${LIGHTTEX_SOURCES}
    ${LIGHTTEX_HEADERS}
    ${TREESITTER_SOURCES}
    resources/lighttex.qrc
)

target_include_directories(lightTex PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${treesitter_SOURCE_DIR}/lib/include
    ${treesitter_latex_SOURCE_DIR}/src
)

target_link_libraries(lightTex PRIVATE
    Qt6::Widgets
    tomlplusplus::tomlplusplus
)

if(POPPLER_QT6_FOUND)
    target_compile_definitions(lightTex PRIVATE LIGHTTEX_HAS_POPPLER)
    target_include_directories(lightTex PRIVATE ${POPPLER_QT6_INCLUDE_DIRS})
    target_link_libraries(lightTex PRIVATE ${POPPLER_QT6_LIBRARIES})
elseif(Qt6Pdf_FOUND)
    target_compile_definitions(lightTex PRIVATE LIGHTTEX_HAS_QT_PDF)
    target_link_libraries(lightTex PRIVATE Qt6::Pdf)
endif()

# Tests
option(LIGHTTEX_BUILD_TESTS "Build tests" ON)
if(LIGHTTEX_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Benchmarks
option(LIGHTTEX_BUILD_BENCHMARKS "Build benchmarks" OFF)
if(LIGHTTEX_BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()

# Install
install(TARGETS lightTex RUNTIME DESTINATION bin)
install(DIRECTORY themes/ DESTINATION share/lighttex/themes)
install(DIRECTORY snippets/ DESTINATION share/lighttex/snippets)
