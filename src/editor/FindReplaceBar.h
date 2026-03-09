#pragma once

#include "theme/Theme.h"

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QWidget>

class QPlainTextEdit;

namespace lighttex::editor {

class EditorWidget;

class FindReplaceBar : public QWidget {
    Q_OBJECT

public:
    explicit FindReplaceBar(EditorWidget* editor, QWidget* parent = nullptr);

    void showFind();
    void showFindReplace();
    void setTheme(const lighttex::theme::Theme& theme);

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
    void onSearchChanged(const QString& text);
    void findNext();
    void findPrevious();
    void replaceCurrent();
    void replaceAll();

private:
    void updateHighlights();
    void navigateToMatch(int index);
    void updateMatchLabel();
    void closeBar();

    EditorWidget* editor_;

    QLineEdit* searchInput_;
    QLineEdit* replaceInput_;
    QLabel* matchLabel_;
    QPushButton* regexButton_;
    QPushButton* caseButton_;
    QPushButton* prevButton_;
    QPushButton* nextButton_;
    QPushButton* replaceButton_;
    QPushButton* replaceAllButton_;
    QPushButton* closeButton_;
    QWidget* replaceRow_;

    bool regexMode_ = false;
    bool caseSensitive_ = false;
    int currentMatch_ = -1;
    int totalMatches_ = 0;

    struct MatchInfo {
        int start;
        int length;
    };
    std::vector<MatchInfo> matches_;
};

} // namespace lighttex::editor
