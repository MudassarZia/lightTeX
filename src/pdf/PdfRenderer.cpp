#include "pdf/PdfRenderer.h"

#include <QFile>
#include <QFileInfo>

#ifdef LIGHTTEX_HAS_POPPLER
#include <poppler-qt6.h>
#endif

namespace lighttex::pdf {

PdfRenderer::~PdfRenderer() { close(); }

void PdfRenderer::open(const std::string &path) {
  QFileInfo info(QString::fromStdString(path));
  if (!info.exists()) {
    throw PdfError("File not found: " + path);
  }

  // Verify PDF magic bytes
  QFile file(QString::fromStdString(path));
  if (!file.open(QIODevice::ReadOnly)) {
    throw PdfError("Cannot open file: " + path);
  }
  QByteArray magic = file.read(4);
  file.close();

  if (magic.size() < 4 || magic[0] != '%' || magic[1] != 'P' ||
      magic[2] != 'D' || magic[3] != 'F') {
    throw PdfError("Not a valid PDF file: " + path);
  }

  path_ = path;

#ifdef LIGHTTEX_HAS_POPPLER
  auto doc = Poppler::Document::load(QString::fromStdString(path));
  if (!doc) {
    throw PdfError("Failed to open PDF: " + path);
  }
  doc->setRenderHint(Poppler::Document::TextAntialiasing);
  doc->setRenderHint(Poppler::Document::Antialiasing);
  pageCount_ = doc->numPages();
  popplerDoc_ = doc.release();
#elif defined(LIGHTTEX_HAS_QT_PDF)
  if (qtPdfDoc_) {
    delete qtPdfDoc_;
    qtPdfDoc_ = nullptr;
  }
  qtPdfDoc_ = new QPdfDocument();
  auto err = qtPdfDoc_->load(QString::fromStdString(path));
  if (err != QPdfDocument::Error::None) {
    delete qtPdfDoc_;
    qtPdfDoc_ = nullptr;
    path_.clear();
    throw PdfError("Failed to open PDF: " + path);
  }
  pageCount_ = qtPdfDoc_->pageCount();
#else
  // Stub: estimate page count from file size
  pageCount_ = std::max(1, static_cast<int>(info.size() / 50000));
#endif
}

void PdfRenderer::reload() {
  if (path_.empty()) {
    throw PdfError("No PDF loaded");
  }
  std::string p = path_;
  close();
  open(p);
}

void PdfRenderer::close() {
#ifdef LIGHTTEX_HAS_POPPLER
  if (popplerDoc_) {
    delete static_cast<Poppler::Document *>(popplerDoc_);
    popplerDoc_ = nullptr;
  }
#endif
#ifdef LIGHTTEX_HAS_QT_PDF
  if (qtPdfDoc_) {
    delete qtPdfDoc_;
    qtPdfDoc_ = nullptr;
  }
#endif
  path_.clear();
  pageCount_ = 0;
}

RenderedPage PdfRenderer::renderPage(int pageNum, int dpi) {
  if (path_.empty()) {
    throw PdfError("No PDF loaded");
  }
  if (pageNum < 0 || pageNum >= pageCount_) {
    throw PdfError("Page " + std::to_string(pageNum) + " out of range (0-" +
                   std::to_string(pageCount_ - 1) + ")");
  }

#ifdef LIGHTTEX_HAS_POPPLER
  auto *doc = static_cast<Poppler::Document *>(popplerDoc_);
  auto page = doc->page(pageNum);
  if (!page) {
    throw PdfError("Failed to load page " + std::to_string(pageNum));
  }
  QImage img = page->renderToImage(dpi, dpi);
  return {pageNum, img.width(), img.height(), img};
#elif defined(LIGHTTEX_HAS_QT_PDF)
  if (!qtPdfDoc_) {
    throw PdfError("PDF document not loaded");
  }
  QSizeF pageSizePt = qtPdfDoc_->pagePointSize(pageNum);
  double scale = dpi / 72.0;
  int width = static_cast<int>(pageSizePt.width() * scale);
  int height = static_cast<int>(pageSizePt.height() * scale);
  QImage img = qtPdfDoc_->render(pageNum, QSize(width, height));
  if (img.isNull()) {
    throw PdfError("Failed to render page " + std::to_string(pageNum));
  }
  return {pageNum, width, height, img};
#else
  // No PDF renderer available — show placeholder
  int width = static_cast<int>(8.5 * dpi);
  int height = static_cast<int>(11.0 * dpi);
  QImage img(width, height, QImage::Format_ARGB32);
  img.fill(QColor(240, 240, 240));
  return {pageNum, width, height, img};
#endif
}

} // namespace lighttex::pdf
