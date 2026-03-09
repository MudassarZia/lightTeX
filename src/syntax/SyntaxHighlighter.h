#pragma once

#include "syntax/Highlighter.h"
#include "theme/Theme.h"

#include <QSyntaxHighlighter>
#include <QTextDocument>
#include <QTimer>

namespace lighttex::syntax {

class SyntaxHighlighterBridge : public QSyntaxHighlighter {
  Q_OBJECT

public:
  explicit SyntaxHighlighterBridge(QTextDocument *parent = nullptr);

  void setTheme(const lighttex::theme::Theme &theme);
  void scheduleReparse();

protected:
  void highlightBlock(const QString &text) override;

private slots:
  void doReparse();

private:
  QTextCharFormat formatForKind(TokenKind kind) const;

  Highlighter highlighter_;
  std::vector<HighlightEvent> cachedEvents_;
  QMap<TokenKind, QColor> colorMap_;
  QTimer reparseTimer_;
  bool isHighlighting_ = false;
};

} // namespace lighttex::syntax
