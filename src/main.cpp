#include "app/Actions.h"
#include "app/AppState.h"
#include "lsp/CompletionWidget.h"
#include "lsp/Lsp.h"
#include "syntax/SyntaxHighlighter.h"
#include "ui/MainWindow.h"

#include <QApplication>
#include <QRegularExpression>
#include <QToolTip>
#include <QUrl>

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  app.setApplicationName("lightTex");
  app.setApplicationVersion("0.2.0-beta");
  app.setOrganizationName("lightTex");

  lighttex::ui::MainWindow window;
  lighttex::app::AppState state;
  lighttex::app::Actions actions(&state, &window);

  // Attach syntax highlighter to editor's document
  auto *syntaxBridge = new lighttex::syntax::SyntaxHighlighterBridge(
      window.editor()->document());
  syntaxBridge->setTheme(state.themeManager().current());

  QObject::connect(&state.themeManager(),
                   &lighttex::theme::ThemeManager::themeChanged, syntaxBridge,
                   &lighttex::syntax::SyntaxHighlighterBridge::setTheme);

  QObject::connect(window.editor(), &QPlainTextEdit::textChanged, syntaxBridge,
                   &lighttex::syntax::SyntaxHighlighterBridge::scheduleReparse);

  // --- LSP ---
  auto *lspClient = new lighttex::lsp::LspClient(&window);
  auto *completionPopup = new lighttex::lsp::CompletionWidget(window.editor());

  // Snippet expansion takes priority over LSP completion on Tab
  completionPopup->setSnippetHandler(
      [&window]() { return window.editor()->tryExpandSnippet(); });

  // After snippet expansion inside {}, trigger LSP argument completion
  QObject::connect(
      window.editor(), &lighttex::editor::EditorWidget::snippetExpandedInBraces,
      completionPopup, &lighttex::lsp::CompletionWidget::startArgumentSession);
  QString currentFileUri;

  auto getFileUri = [&state]() -> QString {
    auto path = state.document().path();
    if (path) {
      return QUrl::fromLocalFile(QString::fromStdString(*path)).toString();
    }
    return {};
  };

  // Helper: send a completion request at the editor's current cursor
  auto requestCompletionAtCursor = [lspClient, &currentFileUri, &window]() {
    if (!lspClient->isRunning() || currentFileUri.isEmpty())
      return;
    QTextCursor cursor = window.editor()->textCursor();
    lspClient->requestCompletion(currentFileUri, cursor.blockNumber(),
                                 cursor.columnNumber());
  };

  if (lighttex::lsp::LspClient::isTexlabAvailable()) {
    // Status
    QObject::connect(lspClient, &lighttex::lsp::LspClient::statusChanged,
                     window.statusBar(),
                     &lighttex::ui::LightTexStatusBar::setLspStatus);

    // On file open: start LSP + send didOpen
    QObject::connect(
        &state, &lighttex::app::AppState::fileOpened, lspClient,
        [lspClient, &state, &currentFileUri](const QString &,
                                             const QString &content) {
          auto path = state.document().path();
          if (!path)
            return;

          currentFileUri =
              QUrl::fromLocalFile(QString::fromStdString(*path)).toString();

          if (!lspClient->isStarted()) {
            QString rootUri =
                QUrl::fromLocalFile(QString::fromStdString(state.projectRoot()))
                    .toString();
            lspClient->start(rootUri.toStdString());
          }
          lspClient->didOpen(currentFileUri, content);
        });

    // didChange on text edits (debounced inside LspClient)
    QObject::connect(window.editor(), &QPlainTextEdit::textChanged, lspClient,
                     [lspClient, &currentFileUri, &window]() {
                       if (!currentFileUri.isEmpty() &&
                           lspClient->isRunning()) {
                         lspClient->didChange(currentFileUri,
                                              window.editor()->toPlainText());
                       }
                     });

    // didSave
    QObject::connect(&state, &lighttex::app::AppState::fileSaved, lspClient,
                     [lspClient, &currentFileUri]() {
                       if (!currentFileUri.isEmpty() &&
                           lspClient->isRunning()) {
                         lspClient->didSave(currentFileUri);
                       }
                     });

    // --- Completion: trigger detected by CompletionWidget event filter ---

    // Backslash typed → send LSP completion request
    QObject::connect(completionPopup,
                     &lighttex::lsp::CompletionWidget::completionRequested,
                     lspClient, requestCompletionAtCursor);

    // User typed more chars while popup visible → re-request
    QObject::connect(completionPopup,
                     &lighttex::lsp::CompletionWidget::refreshRequested,
                     lspClient, requestCompletionAtCursor);

    // Server responded → update popup
    QObject::connect(lspClient, &lighttex::lsp::LspClient::completionReceived,
                     completionPopup,
                     &lighttex::lsp::CompletionWidget::showCompletions);

    // User accepted a completion → insert text
    QObject::connect(
        completionPopup, &lighttex::lsp::CompletionWidget::itemChosen,
        window.editor(),
        [&window, completionPopup](const lighttex::lsp::CompletionItem &item,
                                   int triggerPos, int cursorPos,
                                   lighttex::lsp::TriggerKind triggerKind) {
          auto *editor = window.editor();
          QTextCursor cursor = editor->textCursor();
          cursor.beginEditBlock();
          cursor.setPosition(triggerPos, QTextCursor::MoveAnchor);
          cursor.setPosition(cursorPos, QTextCursor::KeepAnchor);

          if (triggerKind == lighttex::lsp::TriggerKind::Command) {
            // Command completion: \label{}
            QString label = item.label;
            if (label.startsWith('\\')) {
              label = label.mid(1);
            }

            static QRegularExpression alphaOnly(QStringLiteral("^[a-zA-Z]+$"));
            bool wantsBraces = alphaOnly.match(label).hasMatch();

            QString finalText = QStringLiteral("\\") + label;
            if (wantsBraces) {
              finalText += QStringLiteral("{}");
            }

            cursor.insertText(finalText);
            cursor.endEditBlock();

            if (wantsBraces) {
              cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor,
                                  1);
            }
            editor->setTextCursor(cursor);
            editor->setFocus();

            // Auto-trigger argument completion inside braces
            if (wantsBraces) {
              completionPopup->startArgumentSession(cursor.position());
            }
          } else {
            // Argument completion: insert label
            cursor.insertText(item.label);

            // Check if this is inside \begin{} — auto-insert \end{}
            // triggerPos is the position AFTER '{', so -1 to get '{'
            QTextBlock block = cursor.block();
            QString lineText = block.text();
            int braceCol = (triggerPos - 1) - block.position();
            if (braceCol >= 6) {
              QString before = lineText.mid(braceCol - 6, 6);
              if (before == QStringLiteral("\\begin")) {
                // Check if there's a closing '}' after the inserted label
                int cursorCol = cursor.positionInBlock();
                if (cursorCol < lineText.length() &&
                    lineText[cursorCol] == QLatin1Char('}')) {
                  // Move past the existing '}'
                  cursor.movePosition(QTextCursor::Right,
                                      QTextCursor::MoveAnchor, 1);
                } else {
                  // Insert the missing '}'
                  cursor.insertText(QStringLiteral("}"));
                }

                // Re-read block/lineText after possible insertion
                block = cursor.block();
                lineText = block.text();

                // Check next line for existing \end
                QTextBlock nextBlock = block.next();
                bool alreadyHasEnd = false;
                if (nextBlock.isValid()) {
                  QString nextTrimmed = nextBlock.text().trimmed();
                  alreadyHasEnd =
                      nextTrimmed.startsWith(QStringLiteral("\\end{") +
                                             item.label + QStringLiteral("}"));
                }

                if (!alreadyHasEnd) {
                  QString indent;
                  for (int i = 0; i < lineText.length(); ++i) {
                    if (lineText[i] == QLatin1Char(' ') ||
                        lineText[i] == QLatin1Char('\t'))
                      indent += lineText[i];
                    else
                      break;
                  }
                  cursor.insertText(
                      QStringLiteral("\n") + indent + QStringLiteral("\t") +
                      QStringLiteral("\n") + indent + QStringLiteral("\\end{") +
                      item.label + QStringLiteral("}"));
                  cursor.movePosition(QTextCursor::Up);
                  cursor.movePosition(QTextCursor::EndOfBlock);
                }
              }
            }

            cursor.endEditBlock();
            editor->setTextCursor(cursor);
            editor->setFocus();
          }
        });

    // Hover tooltip
    QObject::connect(
        lspClient, &lighttex::lsp::LspClient::hoverReceived, window.editor(),
        [&window](const lighttex::lsp::Hover &hover) {
          if (!hover.contents.isEmpty()) {
            QToolTip::showText(QCursor::pos(), hover.contents, window.editor());
          }
        });

    // Go-to-definition result
    QObject::connect(
        lspClient, &lighttex::lsp::LspClient::definitionReceived,
        window.editor(),
        [&window, &state](const lighttex::lsp::Location &location) {
          auto path = state.document().path();
          QString currentUri;
          if (path) {
            currentUri =
                QUrl::fromLocalFile(QString::fromStdString(*path)).toString();
          }

          if (location.uri == currentUri || location.uri.isEmpty()) {
            auto *editor = window.editor();
            QTextBlock block = editor->document()->findBlockByNumber(
                location.range.start.line);
            QTextCursor cursor(block);
            cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor,
                                location.range.start.character);
            editor->setTextCursor(cursor);
            editor->setFocus();
          } else {
            try {
              QString filePath = QUrl(location.uri).toLocalFile();
              state.openFile(filePath.toStdString());
              auto *editor = window.editor();
              QTextBlock block = editor->document()->findBlockByNumber(
                  location.range.start.line);
              QTextCursor cursor(block);
              cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor,
                                  location.range.start.character);
              editor->setTextCursor(cursor);
            } catch (const std::exception &) {
            }
          }
        });

    // LSP diagnostics → compile panel
    QObject::connect(
        lspClient, &lighttex::lsp::LspClient::diagnosticsReceived, &window,
        [&window](const QString &,
                  const std::vector<lighttex::lsp::Diagnostic> &diags) {
          if (diags.empty())
            return;
          std::vector<lighttex::compiler::CompileMessage> messages;
          for (const auto &d : diags) {
            lighttex::compiler::CompileMessage msg;
            switch (d.severity) {
            case lighttex::lsp::DiagnosticSeverity::Error:
              msg.kind = lighttex::compiler::MessageKind::Error;
              break;
            case lighttex::lsp::DiagnosticSeverity::Warning:
              msg.kind = lighttex::compiler::MessageKind::Warning;
              break;
            default:
              msg.kind = lighttex::compiler::MessageKind::Info;
              break;
            }
            msg.line = d.range.start.line + 1;
            msg.message = d.message.toStdString();
            if (!d.source.isEmpty()) {
              msg.message = "[" + d.source.toStdString() + "] " + msg.message;
            }
            messages.push_back(std::move(msg));
          }
          window.compilePanel()->setMessages(messages);
        });
  } else {
    window.statusBar()->setLspStatus("texlab: not found");
  }

  // F12 → go-to-definition via LSP
  auto *gotoDefAction = new QAction(&window);
  gotoDefAction->setShortcut(state.shortcuts().keySequence("goto_definition"));
  QObject::connect(gotoDefAction, &QAction::triggered, lspClient,
                   [lspClient, &getFileUri, &window]() {
                     QString uri = getFileUri();
                     if (uri.isEmpty() || !lspClient->isRunning())
                       return;
                     QTextCursor cursor = window.editor()->textCursor();
                     lspClient->requestDefinition(uri, cursor.blockNumber(),
                                                  cursor.columnNumber());
                   });
  window.addAction(gotoDefAction);

  // Alt+H → hover info
  auto *hoverAction = new QAction(&window);
  hoverAction->setShortcut(QKeySequence(Qt::ALT | Qt::Key_H));
  QObject::connect(hoverAction, &QAction::triggered, lspClient,
                   [lspClient, &getFileUri, &window]() {
                     QString uri = getFileUri();
                     if (uri.isEmpty() || !lspClient->isRunning())
                       return;
                     QTextCursor cursor = window.editor()->textCursor();
                     lspClient->requestHover(uri, cursor.blockNumber(),
                                             cursor.columnNumber());
                   });
  window.addAction(hoverAction);

  window.show();

  if (argc > 1) {
    try {
      state.openFile(argv[1]);
    } catch (const std::exception &) {
    }
  }

  return app.exec();
}
