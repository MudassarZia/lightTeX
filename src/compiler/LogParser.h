#pragma once

#include <optional>
#include <string>
#include <vector>

namespace lighttex::compiler {

enum class MessageKind { Error, Warning, BadBox, Info };

struct CompileMessage {
  MessageKind kind;
  std::optional<std::string> file;
  std::optional<int> line;
  std::string message;
};

std::vector<CompileMessage> parseLog(const std::string &logContent);

} // namespace lighttex::compiler
