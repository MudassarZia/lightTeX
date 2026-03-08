#pragma once

#include "core/History.h"
#include "core/PieceTable.h"
#include "core/Selection.h"

#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace lighttex::core {

enum class LineEnding { Lf, CrLf };

class DocumentError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

struct EditDelta {
    size_t startLine;
    size_t startCol;
    size_t endLine;
    size_t endCol;
    std::string text;
};

struct BufferDelta {
    std::vector<EditDelta> edits;
    std::string content;
};

struct Edit {
    size_t start;
    size_t end;
    std::string text;
};

struct Transaction {
    std::vector<Edit> edits;
};

class Document {
public:
    Document();

    static Document open(const std::string& path);
    void save();
    void saveAs(const std::string& path);

    [[nodiscard]] std::string text() const;
    [[nodiscard]] size_t length() const;
    [[nodiscard]] size_t lineCount() const;
    [[nodiscard]] bool isEmpty() const;
    [[nodiscard]] std::string line(size_t idx) const;

    [[nodiscard]] std::pair<size_t, size_t> charToLineCol(size_t idx) const;
    [[nodiscard]] size_t lineColToChar(size_t line, size_t col) const;

    BufferDelta insert(size_t pos, const std::string& text);
    BufferDelta erase(size_t start, size_t end);
    BufferDelta replace(size_t start, size_t end, const std::string& text);
    BufferDelta applyTransaction(const Transaction& txn);

    std::optional<BufferDelta> undo();
    std::optional<BufferDelta> redo();

    [[nodiscard]] bool isModified() const { return modified_; }
    void markSaved() { modified_ = false; }

    [[nodiscard]] const std::vector<Selection>& selections() const { return selections_; }
    void setSelections(std::vector<Selection> sels) { selections_ = std::move(sels); }

    [[nodiscard]] std::string displayName() const;
    [[nodiscard]] const std::optional<std::string>& path() const { return path_; }
    [[nodiscard]] LineEnding lineEnding() const { return lineEnding_; }

    static LineEnding detectLineEnding(const std::string& text);

private:
    BufferDelta makeDelta(const std::vector<EditDelta>& edits) const;

    PieceTable buffer_;
    History history_;
    std::vector<Selection> selections_;
    std::optional<std::string> path_;
    LineEnding lineEnding_;
    bool modified_ = false;
};

} // namespace lighttex::core
