#pragma once

#include <optional>
#include <string>

namespace lighttex::compiler {

struct SourcePosition {
  std::string file;
  int line;
  int column;
};

struct PdfPosition {
  int page;
  double x;
  double y;
};

std::optional<PdfPosition> parseForwardOutput(const std::string &output);
std::optional<SourcePosition> parseInverseOutput(const std::string &output);

class SyncTeX {
public:
  SyncTeX() = default;

  bool load(const std::string &pdfPath);
  [[nodiscard]] bool isLoaded() const { return !synctexPath_.empty(); }

  // These are async in Qt via QProcess — see Pipeline for actual usage
  [[nodiscard]] const std::string &path() const { return synctexPath_; }

private:
  std::string synctexPath_;
};

} // namespace lighttex::compiler
