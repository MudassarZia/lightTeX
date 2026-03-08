#include "core/Document.h"

#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <algorithm>

namespace lighttex::core {

Document::Document() : selections_({Selection::cursor(0)}) {
#ifdef _WIN32
    lineEnding_ = LineEnding::CrLf;
#else
    lineEnding_ = LineEnding::Lf;
#endif
}

Document Document::open(const std::string& path) {
    QFile file(QString::fromStdString(path));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        throw DocumentError("Cannot open file: " + path);
    }

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    QString content = stream.readAll();
    file.close();

    std::string text = content.toStdString();

    Document doc;
    doc.path_ = path;
    doc.lineEnding_ = detectLineEnding(text);

    // Normalize to LF internally
    std::string normalized;
    normalized.reserve(text.size());
    for (size_t i = 0; i < text.size(); ++i) {
        if (text[i] == '\r' && i + 1 < text.size() && text[i + 1] == '\n') {
            normalized += '\n';
            ++i;
        } else {
            normalized += text[i];
        }
    }

    doc.buffer_ = PieceTable(normalized);
    doc.selections_ = {Selection::cursor(0)};
    doc.modified_ = false;
    return doc;
}

void Document::save() {
    if (!path_) {
        throw DocumentError("No file path set");
    }
    saveAs(*path_);
    modified_ = false;
}

void Document::saveAs(const std::string& path) {
    std::string content = buffer_.text();

    // Convert back to original line ending
    if (lineEnding_ == LineEnding::CrLf) {
        std::string converted;
        converted.reserve(content.size() + content.size() / 10);
        for (char c : content) {
            if (c == '\n') {
                converted += "\r\n";
            } else {
                converted += c;
            }
        }
        content = std::move(converted);
    }

    QFile file(QString::fromStdString(path));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        throw DocumentError("Cannot write file: " + path);
    }

    file.write(content.c_str(), static_cast<qint64>(content.size()));
    file.close();
    path_ = path;
    modified_ = false;
}

std::string Document::text() const {
    return buffer_.text();
}

size_t Document::length() const {
    return buffer_.length();
}

size_t Document::lineCount() const {
    return buffer_.lineCount();
}

bool Document::isEmpty() const {
    return buffer_.empty();
}

std::string Document::line(size_t idx) const {
    return buffer_.line(idx);
}

std::pair<size_t, size_t> Document::charToLineCol(size_t idx) const {
    return buffer_.charToLineCol(idx);
}

size_t Document::lineColToChar(size_t line, size_t col) const {
    return buffer_.lineColToChar(line, col);
}

BufferDelta Document::insert(size_t pos, const std::string& text) {
    auto [line, col] = buffer_.charToLineCol(pos);

    buffer_.insert(pos, text);
    modified_ = true;

    // Record inverse (delete the inserted text)
    history_.push({{pos, pos + text.size(), ""}});

    auto [endLine, endCol] = buffer_.charToLineCol(pos + text.size());
    return makeDelta({{line, col, endLine, endCol, text}});
}

BufferDelta Document::erase(size_t start, size_t end) {
    std::string deleted = buffer_.substr(start, end - start);
    auto [startLine, startCol] = buffer_.charToLineCol(start);

    buffer_.erase(start, end - start);
    modified_ = true;

    // Record inverse (re-insert the deleted text)
    history_.push({{start, start, deleted}});

    return makeDelta({{startLine, startCol, startLine, startCol, ""}});
}

BufferDelta Document::replace(size_t start, size_t end, const std::string& text) {
    std::string deleted = buffer_.substr(start, end - start);
    auto [startLine, startCol] = buffer_.charToLineCol(start);

    buffer_.erase(start, end - start);
    buffer_.insert(start, text);
    modified_ = true;

    // Record inverse (replace new text with old text)
    history_.push({{start, start + text.size(), deleted}});

    auto [endLine, endCol] = buffer_.charToLineCol(start + text.size());
    return makeDelta({{startLine, startCol, endLine, endCol, text}});
}

BufferDelta Document::applyTransaction(const Transaction& txn) {
    std::vector<EditDelta> deltas;
    std::vector<EditOperation> inverseOps;
    int cumulativeShift = 0;

    for (const auto& edit : txn.edits) {
        size_t adjStart = static_cast<size_t>(static_cast<int>(edit.start) + cumulativeShift);
        size_t adjEnd = static_cast<size_t>(static_cast<int>(edit.end) + cumulativeShift);

        std::string deleted;
        if (adjEnd > adjStart) {
            deleted = buffer_.substr(adjStart, adjEnd - adjStart);
        }

        auto [startLine, startCol] = buffer_.charToLineCol(adjStart);

        if (adjEnd > adjStart) {
            buffer_.erase(adjStart, adjEnd - adjStart);
        }
        if (!edit.text.empty()) {
            buffer_.insert(adjStart, edit.text);
        }

        // Inverse op: replace the new text with the old text
        inverseOps.insert(inverseOps.begin(),
            EditOperation{adjStart, adjStart + edit.text.size(), deleted});

        auto [endLine, endCol] = buffer_.charToLineCol(adjStart + edit.text.size());
        deltas.push_back({startLine, startCol, endLine, endCol, edit.text});

        int editDelta = static_cast<int>(edit.text.size()) - static_cast<int>(edit.end - edit.start);
        cumulativeShift += editDelta;
    }

    modified_ = true;
    history_.push(std::move(inverseOps));
    return makeDelta(deltas);
}

std::optional<BufferDelta> Document::undo() {
    auto ops = history_.undo();
    if (!ops) return std::nullopt;

    std::vector<EditDelta> deltas;
    std::vector<EditOperation> redoOps;

    for (const auto& op : *ops) {
        std::string deleted;
        if (op.end > op.start) {
            deleted = buffer_.substr(op.start, op.end - op.start);
        }

        auto [startLine, startCol] = buffer_.charToLineCol(op.start);

        if (op.end > op.start) {
            buffer_.erase(op.start, op.end - op.start);
        }
        if (!op.text.empty()) {
            buffer_.insert(op.start, op.text);
        }

        redoOps.insert(redoOps.begin(),
            EditOperation{op.start, op.start + op.text.size(), deleted});

        auto [endLine, endCol] = buffer_.charToLineCol(op.start + op.text.size());
        deltas.push_back({startLine, startCol, endLine, endCol, op.text});
    }

    modified_ = true;
    return makeDelta(deltas);
}

std::optional<BufferDelta> Document::redo() {
    auto ops = history_.redo();
    if (!ops) return std::nullopt;

    std::vector<EditDelta> deltas;

    for (const auto& op : *ops) {
        std::string deleted;
        if (op.end > op.start) {
            deleted = buffer_.substr(op.start, op.end - op.start);
        }

        auto [startLine, startCol] = buffer_.charToLineCol(op.start);

        if (op.end > op.start) {
            buffer_.erase(op.start, op.end - op.start);
        }
        if (!op.text.empty()) {
            buffer_.insert(op.start, op.text);
        }

        auto [endLine, endCol] = buffer_.charToLineCol(op.start + op.text.size());
        deltas.push_back({startLine, startCol, endLine, endCol, op.text});
    }

    modified_ = true;
    return makeDelta(deltas);
}

std::string Document::displayName() const {
    if (path_) {
        QFileInfo info(QString::fromStdString(*path_));
        return info.fileName().toStdString();
    }
    return "Untitled";
}

LineEnding Document::detectLineEnding(const std::string& text) {
    for (size_t i = 0; i < text.size(); ++i) {
        if (text[i] == '\r' && i + 1 < text.size() && text[i + 1] == '\n') {
            return LineEnding::CrLf;
        }
        if (text[i] == '\n') {
            return LineEnding::Lf;
        }
    }
#ifdef _WIN32
    return LineEnding::CrLf;
#else
    return LineEnding::Lf;
#endif
}

BufferDelta Document::makeDelta(const std::vector<EditDelta>& edits) const {
    return {edits, buffer_.text()};
}

} // namespace lighttex::core
