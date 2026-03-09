#pragma once

#include "pdf/PageCache.h"
#include "pdf/PdfRenderer.h"

#include <QLabel>
#include <QScrollArea>
#include <QWidget>

namespace lighttex::pdf {

class PdfWidget : public QScrollArea {
  Q_OBJECT

public:
  explicit PdfWidget(QWidget *parent = nullptr);

  void loadPdf(const std::string &path);
  void clear();
  void setPage(int page);
  void setDpi(int dpi) { dpi_ = dpi; }

  [[nodiscard]] int currentPage() const { return currentPage_; }
  [[nodiscard]] int pageCount() const { return renderer_.pageCount(); }

signals:
  void pageChanged(int page);

private:
  void renderCurrentPage();

  PdfRenderer renderer_;
  PageCache cache_{10};
  QLabel *imageLabel_;
  int currentPage_ = 0;
  int dpi_ = 150;
};

} // namespace lighttex::pdf
