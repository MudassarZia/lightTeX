#pragma once

#include "compiler/Pipeline.h"
#include "editor/EditorWidget.h"
#include "pdf/PdfWidget.h"
#include "ui/CommandPalette.h"
#include "ui/CompilePanel.h"
#include "ui/StatusBar.h"

#include <QMainWindow>
#include <QSplitter>
#include <QToolBar>

namespace lighttex::ui {

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

    lighttex::editor::EditorWidget* editor() { return editor_; }
    lighttex::pdf::PdfWidget* pdfWidget() { return pdfWidget_; }
    CompilePanel* compilePanel() { return compilePanel_; }
    LightTexStatusBar* statusBar() { return statusBar_; }
    CommandPalette* commandPalette() { return commandPalette_; }

protected:
    void keyPressEvent(QKeyEvent* event) override;

private:
    void setupUi();
    void setupToolBar();

    lighttex::editor::EditorWidget* editor_;
    lighttex::pdf::PdfWidget* pdfWidget_;
    CompilePanel* compilePanel_;
    LightTexStatusBar* statusBar_;
    CommandPalette* commandPalette_;
    QSplitter* mainSplitter_;
    QSplitter* verticalSplitter_;
    QToolBar* toolBar_;
};

} // namespace lighttex::ui
