#include "core/Document.h"
#include <gtest/gtest.h>

#include <QCoreApplication>
#include <QDir>
#include <QTemporaryFile>

using namespace lighttex::core;

// Helper to ensure QCoreApplication exists for Qt file ops
class DocumentTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        if (!QCoreApplication::instance()) {
            static int argc = 1;
            static char arg0[] = "test";
            static char* argv[] = {arg0, nullptr};
            static QCoreApplication app(argc, argv);
        }
    }
};

TEST_F(DocumentTest, NewDocument) {
    Document doc;
    EXPECT_EQ(doc.displayName(), "Untitled");
    EXPECT_TRUE(doc.isEmpty());
    EXPECT_FALSE(doc.isModified());
    EXPECT_FALSE(doc.path().has_value());
}

TEST_F(DocumentTest, OpenNonexistent) {
    EXPECT_THROW(Document::open("/nonexistent/path/file.tex"), DocumentError);
}

TEST_F(DocumentTest, OpenAndSaveRoundtrip) {
    // Create temp file
    QTemporaryFile tmp;
    tmp.setAutoRemove(true);
    ASSERT_TRUE(tmp.open());
    tmp.write("Hello World\n");
    tmp.close();

    // Open
    Document doc = Document::open(tmp.fileName().toStdString());
    EXPECT_EQ(doc.text(), "Hello World\n");
    EXPECT_FALSE(doc.isModified());

    // Insert
    doc.insert(5, " Beautiful");
    EXPECT_EQ(doc.text(), "Hello Beautiful World\n");
    EXPECT_TRUE(doc.isModified());

    // Save
    doc.save();
    EXPECT_FALSE(doc.isModified());

    // Reload and verify
    Document doc2 = Document::open(tmp.fileName().toStdString());
    EXPECT_EQ(doc2.text(), "Hello Beautiful World\n");
}

TEST_F(DocumentTest, LineEndingDetection) {
    EXPECT_EQ(Document::detectLineEnding("hello\nworld"), LineEnding::Lf);
    EXPECT_EQ(Document::detectLineEnding("hello\r\nworld"), LineEnding::CrLf);
}

TEST_F(DocumentTest, DisplayName) {
    Document doc;
    EXPECT_EQ(doc.displayName(), "Untitled");

    // Create temp file with known name
    QTemporaryFile tmp;
    tmp.setFileTemplate(QDir::tempPath() + "/test_XXXXXX.tex");
    tmp.setAutoRemove(true);
    ASSERT_TRUE(tmp.open());
    tmp.write("test");
    tmp.close();

    Document doc2 = Document::open(tmp.fileName().toStdString());
    // Display name should be the filename (not full path)
    EXPECT_FALSE(doc2.displayName().empty());
    EXPECT_NE(doc2.displayName(), "Untitled");
}
