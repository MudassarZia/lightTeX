#pragma once

namespace lighttex::syntax {

enum class TokenKind {
  Command,
  Environment,
  MathDelimiter,
  MathContent,
  Comment,
  Text,
  Bracket,
  Operator,
  Number,
  String,
  Key,
  Error
};

inline const char *tokenKindName(TokenKind kind) {
  switch (kind) {
  case TokenKind::Command:
    return "Command";
  case TokenKind::Environment:
    return "Environment";
  case TokenKind::MathDelimiter:
    return "MathDelimiter";
  case TokenKind::MathContent:
    return "MathContent";
  case TokenKind::Comment:
    return "Comment";
  case TokenKind::Text:
    return "Text";
  case TokenKind::Bracket:
    return "Bracket";
  case TokenKind::Operator:
    return "Operator";
  case TokenKind::Number:
    return "Number";
  case TokenKind::String:
    return "String";
  case TokenKind::Key:
    return "Key";
  case TokenKind::Error:
    return "Error";
  }
  return "Unknown";
}

} // namespace lighttex::syntax
