#pragma once

#include <string>
#include <vector>

namespace lighttex::compiler {

enum class Engine { PdfLatex, XeLatex, LuaLatex };

inline const char *engineCommand(Engine e) {
  switch (e) {
  case Engine::PdfLatex:
    return "pdflatex";
  case Engine::XeLatex:
    return "xelatex";
  case Engine::LuaLatex:
    return "lualatex";
  }
  return "pdflatex";
}

inline const char *engineDisplayName(Engine e) {
  switch (e) {
  case Engine::PdfLatex:
    return "pdfLaTeX";
  case Engine::XeLatex:
    return "XeLaTeX";
  case Engine::LuaLatex:
    return "LuaLaTeX";
  }
  return "pdfLaTeX";
}

inline std::vector<std::string> engineDefaultArgs() {
  return {"-synctex=1", "-interaction=nonstopmode", "-file-line-error"};
}

} // namespace lighttex::compiler
