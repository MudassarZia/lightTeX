#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QString>

#include <optional>
#include <string>
#include <vector>

namespace lighttex::lsp {

struct Position {
  int line = 0;
  int character = 0;

  QJsonObject toJson() const {
    return {{"line", line}, {"character", character}};
  }

  static Position fromJson(const QJsonObject &obj) {
    return {obj["line"].toInt(), obj["character"].toInt()};
  }
};

struct Range {
  Position start;
  Position end;

  QJsonObject toJson() const {
    return {{"start", start.toJson()}, {"end", end.toJson()}};
  }

  static Range fromJson(const QJsonObject &obj) {
    return {Position::fromJson(obj["start"].toObject()),
            Position::fromJson(obj["end"].toObject())};
  }
};

struct Location {
  QString uri;
  Range range;

  static Location fromJson(const QJsonObject &obj) {
    Location loc;
    loc.uri = obj["uri"].toString();
    loc.range = Range::fromJson(obj["range"].toObject());
    return loc;
  }
};

struct CompletionItem {
  QString label;
  int kind = 1; // Text
  QString detail;
  QString insertText;
  QString filterText;
  QString sortText;

  static CompletionItem fromJson(const QJsonObject &obj) {
    CompletionItem item;
    item.label = obj["label"].toString();
    item.kind = obj["kind"].toInt(1);
    item.detail = obj["detail"].toString();
    item.insertText = obj["insertText"].toString();
    // texlab puts snippet syntax in textEdit.newText — prefer it
    auto te = obj["textEdit"].toObject();
    QString teNewText = te["newText"].toString();
    if (!teNewText.isEmpty() && teNewText.contains('$')) {
      item.insertText = teNewText;
    }
    if (item.insertText.isEmpty()) {
      item.insertText = item.label;
    }
    item.filterText = obj["filterText"].toString();
    item.sortText = obj["sortText"].toString();
    return item;
  }
};

struct Hover {
  QString contents;

  static Hover fromJson(const QJsonObject &obj) {
    Hover h;
    auto contents = obj["contents"];
    if (contents.isString()) {
      h.contents = contents.toString();
    } else if (contents.isObject()) {
      h.contents = contents.toObject()["value"].toString();
    } else if (contents.isArray()) {
      auto arr = contents.toArray();
      for (const auto &v : arr) {
        if (v.isString()) {
          if (!h.contents.isEmpty())
            h.contents += "\n";
          h.contents += v.toString();
        } else if (v.isObject()) {
          if (!h.contents.isEmpty())
            h.contents += "\n";
          h.contents += v.toObject()["value"].toString();
        }
      }
    }
    return h;
  }
};

enum class DiagnosticSeverity {
  Error = 1,
  Warning = 2,
  Information = 3,
  Hint = 4
};

struct Diagnostic {
  Range range;
  DiagnosticSeverity severity = DiagnosticSeverity::Error;
  QString message;
  QString source;

  static Diagnostic fromJson(const QJsonObject &obj) {
    Diagnostic d;
    d.range = Range::fromJson(obj["range"].toObject());
    d.severity = static_cast<DiagnosticSeverity>(obj["severity"].toInt(1));
    d.message = obj["message"].toString();
    d.source = obj["source"].toString();
    return d;
  }
};

struct TextDocumentIdentifier {
  QString uri;

  QJsonObject toJson() const { return {{"uri", uri}}; }
};

struct TextDocumentPositionParams {
  TextDocumentIdentifier textDocument;
  Position position;

  QJsonObject toJson() const {
    return {{"textDocument", textDocument.toJson()},
            {"position", position.toJson()}};
  }
};

} // namespace lighttex::lsp

Q_DECLARE_METATYPE(lighttex::lsp::CompletionItem)
