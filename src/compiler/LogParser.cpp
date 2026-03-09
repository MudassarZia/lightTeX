#include "compiler/LogParser.h"

#include <QRegularExpression>

namespace lighttex::compiler {

static int extractLineFromMessage(const QString &msg) {
  static const QRegularExpression lineRe(R"(on input line (\d+))");
  auto match = lineRe.match(msg);
  if (match.hasMatch()) {
    return match.captured(1).toInt();
  }
  return -1;
}

std::vector<CompileMessage> parseLog(const std::string &logContent) {
  static const QRegularExpression errorRe(R"(^(?:(.+?):(\d+): )?! (.+)$)");
  static const QRegularExpression warningRe(
      R"(^(?:LaTeX |Package (?:\w+) )?Warning[:\s]*(.+)$)",
      QRegularExpression::CaseInsensitiveOption);
  static const QRegularExpression badboxRe(
      R"(^((?:Over|Under)full \\[hv]box .+)$)");

  std::vector<CompileMessage> messages;
  QString content = QString::fromStdString(logContent);
  QStringList lines = content.split('\n');

  for (const auto &line : lines) {
    // Try error pattern first
    auto errorMatch = errorRe.match(line);
    if (errorMatch.hasMatch()) {
      CompileMessage msg;
      msg.kind = MessageKind::Error;
      msg.message = errorMatch.captured(3).toStdString();
      if (!errorMatch.captured(1).isEmpty()) {
        msg.file = errorMatch.captured(1).toStdString();
      }
      if (!errorMatch.captured(2).isEmpty()) {
        msg.line = errorMatch.captured(2).toInt();
      }
      messages.push_back(std::move(msg));
      continue;
    }

    // Try warning pattern
    auto warningMatch = warningRe.match(line);
    if (warningMatch.hasMatch()) {
      CompileMessage msg;
      msg.kind = MessageKind::Warning;
      msg.message = warningMatch.captured(1).toStdString();
      int lineNum = extractLineFromMessage(line);
      if (lineNum > 0) {
        msg.line = lineNum;
      }
      messages.push_back(std::move(msg));
      continue;
    }

    // Try badbox pattern
    auto badboxMatch = badboxRe.match(line);
    if (badboxMatch.hasMatch()) {
      CompileMessage msg;
      msg.kind = MessageKind::BadBox;
      msg.message = badboxMatch.captured(1).toStdString();
      int lineNum = extractLineFromMessage(line);
      if (lineNum > 0) {
        msg.line = lineNum;
      }
      messages.push_back(std::move(msg));
      continue;
    }
  }

  return messages;
}

} // namespace lighttex::compiler
