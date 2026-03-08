#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace lighttex::core {

enum class BufferKind { Original, Added };

struct Piece {
    BufferKind buffer;
    size_t start;
    size_t length;
};

class PieceTable {
public:
    PieceTable();
    explicit PieceTable(const std::string& text);

    void insert(size_t pos, const std::string& text);
    void erase(size_t start, size_t length);
    void replace(size_t start, size_t length, const std::string& text);

    [[nodiscard]] std::string text() const;
    [[nodiscard]] std::string substr(size_t start, size_t length) const;
    [[nodiscard]] size_t length() const;
    [[nodiscard]] bool empty() const;

    [[nodiscard]] size_t lineCount() const;
    [[nodiscard]] std::string line(size_t lineIdx) const;
    [[nodiscard]] std::pair<size_t, size_t> charToLineCol(size_t charIdx) const;
    [[nodiscard]] size_t lineColToChar(size_t line, size_t col) const;

    [[nodiscard]] const std::vector<Piece>& pieces() const { return pieces_; }

private:
    struct PiecePos {
        size_t pieceIdx;
        size_t offset;
    };

    PiecePos findPiece(size_t pos) const;
    char charAt(size_t pos) const;
    void rebuildLineStarts() const;

    std::string original_;
    std::string added_;
    std::vector<Piece> pieces_;
    mutable std::vector<size_t> lineStarts_;
    mutable bool lineStartsDirty_ = true;
};

} // namespace lighttex::core
