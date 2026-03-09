#include "editor/FindReplaceBar.h"
#include "editor/EditorWidget.h"

#include <QHBoxLayout>
#include <QKeyEvent>
#include <QRegularExpression>
#include <QTextDocument>
#include <QVBoxLayout>

namespace lighttex::editor {

FindReplaceBar::FindReplaceBar(EditorWidget* editor, QWidget* parent)
    : QWidget(parent), editor_(editor) {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 4, 8, 4);
    mainLayout->setSpacing(2);

    // Search row
    auto* searchRow = new QHBoxLayout();
    searchRow->setSpacing(4);

    searchInput_ = new QLineEdit();
    searchInput_->setPlaceholderText("Find");
    searchInput_->setMinimumWidth(200);

    caseButton_ = new QPushButton("Aa");
    caseButton_->setCheckable(true);
    caseButton_->setFixedSize(28, 24);
    caseButton_->setToolTip("Match Case");

    regexButton_ = new QPushButton(".*");
    regexButton_->setCheckable(true);
    regexButton_->setFixedSize(28, 24);
    regexButton_->setToolTip("Use Regex");

    matchLabel_ = new QLabel("");
    matchLabel_->setMinimumWidth(50);

    prevButton_ = new QPushButton("Prev");
    prevButton_->setFixedHeight(24);

    nextButton_ = new QPushButton("Next");
    nextButton_->setFixedHeight(24);

    closeButton_ = new QPushButton("x");
    closeButton_->setFixedSize(24, 24);

    searchRow->addWidget(searchInput_);
    searchRow->addWidget(caseButton_);
    searchRow->addWidget(regexButton_);
    searchRow->addWidget(matchLabel_);
    searchRow->addWidget(prevButton_);
    searchRow->addWidget(nextButton_);
    searchRow->addWidget(closeButton_);
    searchRow->addStretch();

    // Replace row
    replaceRow_ = new QWidget();
    auto* replaceLayout = new QHBoxLayout(replaceRow_);
    replaceLayout->setContentsMargins(0, 0, 0, 0);
    replaceLayout->setSpacing(4);

    replaceInput_ = new QLineEdit();
    replaceInput_->setPlaceholderText("Replace");
    replaceInput_->setMinimumWidth(200);

    replaceButton_ = new QPushButton("Replace");
    replaceButton_->setFixedHeight(24);

    replaceAllButton_ = new QPushButton("All");
    replaceAllButton_->setFixedHeight(24);

    replaceLayout->addWidget(replaceInput_);
    replaceLayout->addWidget(replaceButton_);
    replaceLayout->addWidget(replaceAllButton_);
    replaceLayout->addStretch();

    replaceRow_->hide();

    mainLayout->addLayout(searchRow);
    mainLayout->addWidget(replaceRow_);

    // Connections
    connect(searchInput_, &QLineEdit::textChanged,
            this, &FindReplaceBar::onSearchChanged);
    connect(prevButton_, &QPushButton::clicked,
            this, &FindReplaceBar::findPrevious);
    connect(nextButton_, &QPushButton::clicked,
            this, &FindReplaceBar::findNext);
    connect(replaceButton_, &QPushButton::clicked,
            this, &FindReplaceBar::replaceCurrent);
    connect(replaceAllButton_, &QPushButton::clicked,
            this, &FindReplaceBar::replaceAll);
    connect(closeButton_, &QPushButton::clicked,
            this, &FindReplaceBar::closeBar);
    connect(caseButton_, &QPushButton::toggled, this, [this](bool checked) {
        caseSensitive_ = checked;
        onSearchChanged(searchInput_->text());
    });
    connect(regexButton_, &QPushButton::toggled, this, [this](bool checked) {
        regexMode_ = checked;
        onSearchChanged(searchInput_->text());
    });

    searchInput_->installEventFilter(this);
    replaceInput_->installEventFilter(this);
}

void FindReplaceBar::showFind() {
    replaceRow_->hide();
    show();
    searchInput_->setFocus();
    searchInput_->selectAll();
}

void FindReplaceBar::showFindReplace() {
    replaceRow_->show();
    show();
    searchInput_->setFocus();
    searchInput_->selectAll();
}

void FindReplaceBar::setTheme(const lighttex::theme::Theme& theme) {
    QString bg = QString::fromStdString(theme.ui.panelBg);
    QString fg = QString::fromStdString(theme.colors.foreground);
    QString border = QString::fromStdString(theme.ui.panelBorder);

    setStyleSheet(
        QString("FindReplaceBar { background: %1; border-bottom: 1px solid %3; }"
                "QLineEdit { background: %1; color: %2; border: 1px solid %3; "
                "padding: 2px 4px; }"
                "QPushButton { background: transparent; color: %2; border: 1px solid %3; "
                "padding: 2px 6px; }"
                "QPushButton:checked { background: %3; }"
                "QPushButton:hover { background: %3; }"
                "QLabel { color: %2; }")
            .arg(bg, fg, border));
}

bool FindReplaceBar::eventFilter(QObject* obj, QEvent* event) {
    if (event->type() == QEvent::KeyPress) {
        auto* keyEvent = static_cast<QKeyEvent*>(event);

        if (keyEvent->key() == Qt::Key_Escape) {
            closeBar();
            return true;
        }

        if (keyEvent->key() == Qt::Key_Return ||
            keyEvent->key() == Qt::Key_Enter) {
            if (keyEvent->modifiers() & Qt::ShiftModifier) {
                findPrevious();
            } else {
                findNext();
            }
            return true;
        }
    }
    return QWidget::eventFilter(obj, event);
}

void FindReplaceBar::onSearchChanged(const QString& text) {
    matches_.clear();
    currentMatch_ = -1;
    totalMatches_ = 0;

    if (text.isEmpty()) {
        editor_->clearSearchHighlights();
        updateMatchLabel();
        return;
    }

    QTextDocument* doc = editor_->document();

    if (regexMode_) {
        QRegularExpression::PatternOptions opts =
            QRegularExpression::NoPatternOption;
        if (!caseSensitive_) {
            opts |= QRegularExpression::CaseInsensitiveOption;
        }
        QRegularExpression regex(text, opts);
        if (!regex.isValid()) {
            editor_->clearSearchHighlights();
            matchLabel_->setText("Invalid");
            return;
        }

        QString content = doc->toPlainText();
        auto it = regex.globalMatch(content);
        while (it.hasNext()) {
            auto match = it.next();
            matches_.push_back({static_cast<int>(match.capturedStart()),
                                static_cast<int>(match.capturedLength())});
        }
    } else {
        QTextDocument::FindFlags flags;
        if (caseSensitive_) {
            flags |= QTextDocument::FindCaseSensitively;
        }
        QTextCursor cursor(doc);
        while (true) {
            cursor = doc->find(text, cursor, flags);
            if (cursor.isNull()) break;
            matches_.push_back({cursor.selectionStart(),
                                cursor.selectionEnd() - cursor.selectionStart()});
        }
    }

    totalMatches_ = static_cast<int>(matches_.size());
    updateHighlights();

    // Move to nearest match from cursor
    if (!matches_.empty()) {
        int cursorPos = editor_->textCursor().position();
        currentMatch_ = 0;
        for (int i = 0; i < static_cast<int>(matches_.size()); ++i) {
            if (matches_[static_cast<size_t>(i)].start >= cursorPos) {
                currentMatch_ = i;
                break;
            }
        }
        navigateToMatch(currentMatch_);
    }

    updateMatchLabel();
}

void FindReplaceBar::updateHighlights() {
    QList<QTextEdit::ExtraSelection> highlights;

    QColor matchColor(255, 255, 0, 60);    // Yellow tint
    QColor currentColor(255, 200, 0, 120); // Brighter for current

    for (int i = 0; i < static_cast<int>(matches_.size()); ++i) {
        QTextEdit::ExtraSelection sel;
        sel.format.setBackground(
            (i == currentMatch_) ? currentColor : matchColor);
        sel.cursor = QTextCursor(editor_->document());
        sel.cursor.setPosition(matches_[static_cast<size_t>(i)].start);
        sel.cursor.setPosition(
            matches_[static_cast<size_t>(i)].start +
            matches_[static_cast<size_t>(i)].length,
            QTextCursor::KeepAnchor);
        highlights.append(sel);
    }

    editor_->setSearchHighlights(highlights);
}

void FindReplaceBar::navigateToMatch(int index) {
    if (index < 0 || index >= static_cast<int>(matches_.size())) return;

    currentMatch_ = index;
    const auto& m = matches_[static_cast<size_t>(index)];

    QTextCursor cursor = editor_->textCursor();
    cursor.setPosition(m.start);
    cursor.setPosition(m.start + m.length, QTextCursor::KeepAnchor);
    editor_->setTextCursor(cursor);

    updateHighlights();
    updateMatchLabel();
}

void FindReplaceBar::updateMatchLabel() {
    if (totalMatches_ == 0) {
        matchLabel_->setText(searchInput_->text().isEmpty() ? "" : "No results");
    } else {
        matchLabel_->setText(
            QString("%1/%2").arg(currentMatch_ + 1).arg(totalMatches_));
    }
}

void FindReplaceBar::findNext() {
    if (matches_.empty()) return;
    int next = (currentMatch_ + 1) % static_cast<int>(matches_.size());
    navigateToMatch(next);
}

void FindReplaceBar::findPrevious() {
    if (matches_.empty()) return;
    int prev = (currentMatch_ - 1 + static_cast<int>(matches_.size())) %
               static_cast<int>(matches_.size());
    navigateToMatch(prev);
}

void FindReplaceBar::replaceCurrent() {
    if (currentMatch_ < 0 || currentMatch_ >= static_cast<int>(matches_.size()))
        return;

    const auto& m = matches_[static_cast<size_t>(currentMatch_)];
    QTextCursor cursor = editor_->textCursor();
    cursor.setPosition(m.start);
    cursor.setPosition(m.start + m.length, QTextCursor::KeepAnchor);
    cursor.insertText(replaceInput_->text());

    // Re-search after replacement
    onSearchChanged(searchInput_->text());
}

void FindReplaceBar::replaceAll() {
    if (matches_.empty()) return;

    QString replacement = replaceInput_->text();
    QTextCursor cursor = editor_->textCursor();
    cursor.beginEditBlock();

    // Replace from end to start to preserve positions
    for (int i = static_cast<int>(matches_.size()) - 1; i >= 0; --i) {
        const auto& m = matches_[static_cast<size_t>(i)];
        cursor.setPosition(m.start);
        cursor.setPosition(m.start + m.length, QTextCursor::KeepAnchor);
        cursor.insertText(replacement);
    }

    cursor.endEditBlock();
    onSearchChanged(searchInput_->text());
}

void FindReplaceBar::closeBar() {
    editor_->clearSearchHighlights();
    hide();
    editor_->setFocus();
}

} // namespace lighttex::editor
