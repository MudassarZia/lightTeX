#include "app/Actions.h"
#include "app/AppState.h"
#include "syntax/SyntaxHighlighter.h"
#include "ui/MainWindow.h"

#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("lightTex");
    app.setApplicationVersion("0.1.0");
    app.setOrganizationName("lightTex");

    lighttex::ui::MainWindow window;
    lighttex::app::AppState state;
    lighttex::app::Actions actions(&state, &window);

    // Attach syntax highlighter to editor's document
    auto* syntaxBridge = new lighttex::syntax::SyntaxHighlighterBridge(
        window.editor()->document());
    syntaxBridge->setTheme(state.themeManager().current());

    // Update syntax highlighting on theme change
    QObject::connect(&state.themeManager(),
                     &lighttex::theme::ThemeManager::themeChanged,
                     syntaxBridge,
                     &lighttex::syntax::SyntaxHighlighterBridge::setTheme);

    // Schedule re-parse on text change (debounced, no recursion)
    QObject::connect(window.editor(), &QPlainTextEdit::textChanged,
                     syntaxBridge,
                     &lighttex::syntax::SyntaxHighlighterBridge::scheduleReparse);

    window.show();

    // Open file from command line argument
    if (argc > 1) {
        try {
            state.openFile(argv[1]);
        } catch (const std::exception&) {
            // Silently fail — user can open via menu
        }
    }

    return app.exec();
}
