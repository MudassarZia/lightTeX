#pragma once

#include "lsp/LspTypes.h"

#include <QListWidget>

class QPlainTextEdit;

namespace lighttex::lsp {

enum class TriggerKind { Command, Argument };

class CompletionWidget : public QListWidget {
    Q_OBJECT

public:
    explicit CompletionWidget(QPlainTextEdit* editor);

    void showCompletions(const std::vector<CompletionItem>& items);
    void hideCompletions();
    void startArgumentSession(int triggerPos);
    [[nodiscard]] int triggerPos() const { return triggerPos_; }
    [[nodiscard]] TriggerKind triggerKind() const { return triggerKind_; }

signals:
    void completionRequested();
    void refreshRequested();
    void itemChosen(const CompletionItem& item, int triggerPos, int cursorPos,
                    lighttex::lsp::TriggerKind triggerKind);

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    void positionPopup();

    QPlainTextEdit* editor_;
    std::vector<CompletionItem> items_;
    int triggerPos_ = -1;
    TriggerKind triggerKind_ = TriggerKind::Command;
};

} // namespace lighttex::lsp

Q_DECLARE_METATYPE(lighttex::lsp::TriggerKind)
