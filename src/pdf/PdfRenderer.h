#pragma once

#include <QImage>
#include <optional>
#include <stdexcept>
#include <string>

#ifdef LIGHTTEX_HAS_QT_PDF
#include <QPdfDocument>
#endif

namespace lighttex::pdf {

class PdfError : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

struct RenderedPage {
  int pageNum;
  int width;
  int height;
  QImage image;
};

class PdfRenderer {
public:
  PdfRenderer() = default;
  ~PdfRenderer();

  void open(const std::string &path);
  void reload();
  void close();

  [[nodiscard]] int pageCount() const { return pageCount_; }
  [[nodiscard]] bool isLoaded() const { return !path_.empty(); }
  [[nodiscard]] const std::string &path() const { return path_; }

  RenderedPage renderPage(int pageNum, int dpi = 150);

private:
  std::string path_;
  int pageCount_ = 0;

#ifdef LIGHTTEX_HAS_POPPLER
  void *popplerDoc_ = nullptr;
#endif
#ifdef LIGHTTEX_HAS_QT_PDF
  QPdfDocument *qtPdfDoc_ = nullptr;
#endif
};

} // namespace lighttex::pdf
