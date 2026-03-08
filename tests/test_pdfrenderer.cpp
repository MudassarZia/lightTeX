#include "pdf/PdfRenderer.h"
#include <gtest/gtest.h>
#include <QTemporaryFile>

using namespace lighttex::pdf;

TEST(PdfRenderer, NewRenderer) {
    PdfRenderer renderer;
    EXPECT_FALSE(renderer.isLoaded());
    EXPECT_EQ(renderer.pageCount(), 0);
}

TEST(PdfRenderer, OpenNonexistent) {
    PdfRenderer renderer;
    EXPECT_THROW(renderer.open("/nonexistent/path/file.pdf"), PdfError);
}

TEST(PdfRenderer, RenderNoPdf) {
    PdfRenderer renderer;
    EXPECT_THROW(renderer.renderPage(0), PdfError);
}

TEST(PdfRenderer, ReloadNoPdf) {
    PdfRenderer renderer;
    EXPECT_THROW(renderer.reload(), PdfError);
}

TEST(PdfRenderer, OpenInvalidFile) {
    // Create a temp file that is not a PDF
    QTemporaryFile tmp;
    tmp.open();
    tmp.write("This is not a PDF file");
    tmp.flush();
    PdfRenderer renderer;
    EXPECT_THROW(renderer.open(tmp.fileName().toStdString()), PdfError);
}

TEST(PdfRenderer, CloseAndReopen) {
    PdfRenderer renderer;
    EXPECT_FALSE(renderer.isLoaded());
    renderer.close();
    EXPECT_FALSE(renderer.isLoaded());
    EXPECT_EQ(renderer.pageCount(), 0);
}
