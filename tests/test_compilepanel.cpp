#include "ui/CompilePanel.h"

#include <QApplication>
#include <QSignalSpy>
#include <gtest/gtest.h>

using namespace lighttex::ui;
using namespace lighttex::compiler;

class CompilePanelTest : public ::testing::Test {
protected:
    void SetUp() override {
        if (!QApplication::instance()) {
            int argc = 0;
            app_ = new QApplication(argc, nullptr);
        }
        panel_ = new CompilePanel();
    }

    void TearDown() override {
        delete panel_;
    }

    QApplication* app_ = nullptr;
    CompilePanel* panel_ = nullptr;
};

TEST_F(CompilePanelTest, InitiallyEmpty) {
    EXPECT_TRUE(panel_->toPlainText().isEmpty());
    EXPECT_TRUE(panel_->isHidden());
}

TEST_F(CompilePanelTest, SetMessagesShowsPanel) {
    std::vector<CompileMessage> messages = {
        {MessageKind::Error, "test.tex", 42, "Undefined control sequence"}
    };
    panel_->setMessages(messages);
    EXPECT_TRUE(panel_->isVisible());
    EXPECT_FALSE(panel_->toPlainText().isEmpty());
}

TEST_F(CompilePanelTest, ClearMessagesHidesPanel) {
    std::vector<CompileMessage> messages = {
        {MessageKind::Warning, std::nullopt, std::nullopt, "Some warning"}
    };
    panel_->setMessages(messages);
    EXPECT_TRUE(panel_->isVisible());

    panel_->clearMessages();
    EXPECT_TRUE(panel_->isHidden());
    EXPECT_TRUE(panel_->toPlainText().isEmpty());
}

TEST_F(CompilePanelTest, MessageClickedSignalExists) {
    // Verify the signal can be connected
    QSignalSpy spy(panel_, &CompilePanel::messageClicked);
    EXPECT_TRUE(spy.isValid());
}
