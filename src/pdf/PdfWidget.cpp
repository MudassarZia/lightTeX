#include "pdf/PdfWidget.h"

#include <QPixmap>
#include <QVBoxLayout>

namespace lighttex::pdf {

PdfWidget::PdfWidget(QWidget *parent) : QScrollArea(parent) {
  imageLabel_ = new QLabel(this);
  imageLabel_->setAlignment(Qt::AlignCenter);
  imageLabel_->setText("Compile a .tex file to see the preview here");
  imageLabel_->setStyleSheet("QLabel { color: #888; padding: 40px; }");
  setWidget(imageLabel_);
  setWidgetResizable(true);
  // White background so the PDF page looks like real paper
  setStyleSheet("QScrollArea { background: white; } "
                "QLabel { background: white; }");
}

void PdfWidget::loadPdf(const std::string &path) {
  try {
    renderer_.open(path);
    cache_.clear();
    currentPage_ = 0;
    renderCurrentPage();
  } catch (const PdfError &e) {
    imageLabel_->setText(QString("Error loading PDF: %1").arg(e.what()));
  }
}

void PdfWidget::clear() {
  renderer_.close();
  cache_.clear();
  currentPage_ = 0;
  imageLabel_->setPixmap(QPixmap());
  imageLabel_->setText("Compile a .tex file to see the preview here");
}

void PdfWidget::setPage(int page) {
  if (page < 0 || page >= renderer_.pageCount())
    return;
  currentPage_ = page;
  renderCurrentPage();
  emit pageChanged(currentPage_);
}

void PdfWidget::renderCurrentPage() {
  if (!renderer_.isLoaded())
    return;

  const RenderedPage *cached = cache_.get(currentPage_);
  if (cached) {
    imageLabel_->setPixmap(QPixmap::fromImage(cached->image));
    return;
  }

  try {
    auto page = renderer_.renderPage(currentPage_, dpi_);
    imageLabel_->setPixmap(QPixmap::fromImage(page.image));
    cache_.insert(std::move(page));
  } catch (const PdfError &e) {
    imageLabel_->setText(QString("Error rendering page: %1").arg(e.what()));
  }
}

} // namespace lighttex::pdf
