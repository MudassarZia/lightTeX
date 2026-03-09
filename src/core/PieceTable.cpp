#include "core/PieceTable.h"
#include <algorithm>
#include <stdexcept>

namespace lighttex::core {

PieceTable::PieceTable() { lineStartsDirty_ = true; }

PieceTable::PieceTable(const std::string &text) : original_(text) {
  if (!text.empty()) {
    pieces_.push_back({BufferKind::Original, 0, text.size()});
  }
  lineStartsDirty_ = true;
}

PieceTable::PiecePos PieceTable::findPiece(size_t pos) const {
  size_t offset = 0;
  for (size_t i = 0; i < pieces_.size(); ++i) {
    if (pos <= offset + pieces_[i].length) {
      return {i, pos - offset};
    }
    offset += pieces_[i].length;
  }
  return {pieces_.size(), 0};
}

char PieceTable::charAt(size_t pos) const {
  size_t offset = 0;
  for (const auto &p : pieces_) {
    if (pos < offset + p.length) {
      size_t localOff = pos - offset;
      const auto &buf = (p.buffer == BufferKind::Original) ? original_ : added_;
      return buf[p.start + localOff];
    }
    offset += p.length;
  }
  throw std::out_of_range("charAt: position out of range");
}

void PieceTable::insert(size_t pos, const std::string &text) {
  if (text.empty())
    return;

  size_t addStart = added_.size();
  added_ += text;
  Piece newPiece{BufferKind::Added, addStart, text.size()};

  if (pieces_.empty()) {
    pieces_.push_back(newPiece);
  } else {
    auto [pieceIdx, offset] = findPiece(pos);

    if (pieceIdx >= pieces_.size()) {
      pieces_.push_back(newPiece);
    } else if (offset == 0) {
      pieces_.insert(pieces_.begin() + static_cast<ptrdiff_t>(pieceIdx),
                     newPiece);
    } else if (offset == pieces_[pieceIdx].length) {
      pieces_.insert(pieces_.begin() + static_cast<ptrdiff_t>(pieceIdx) + 1,
                     newPiece);
    } else {
      Piece &existing = pieces_[pieceIdx];
      Piece left{existing.buffer, existing.start, offset};
      Piece right{existing.buffer, existing.start + offset,
                  existing.length - offset};
      auto it =
          pieces_.erase(pieces_.begin() + static_cast<ptrdiff_t>(pieceIdx));
      it = pieces_.insert(it, left);
      ++it;
      it = pieces_.insert(it, newPiece);
      ++it;
      pieces_.insert(it, right);
    }
  }
  lineStartsDirty_ = true;
}

void PieceTable::erase(size_t start, size_t length) {
  if (length == 0)
    return;

  size_t end = start + length;
  size_t totalLen = this->length();
  if (end > totalLen) {
    throw std::out_of_range("erase: range exceeds buffer length");
  }

  // Find affected pieces and rebuild
  std::vector<Piece> newPieces;
  size_t offset = 0;

  for (const auto &p : pieces_) {
    size_t pEnd = offset + p.length;

    if (pEnd <= start || offset >= end) {
      // Entirely before or after the deletion range
      newPieces.push_back(p);
    } else {
      // This piece overlaps the deletion range
      if (offset < start) {
        // Keep left portion
        newPieces.push_back({p.buffer, p.start, start - offset});
      }
      if (pEnd > end) {
        // Keep right portion
        size_t rightOffset = end - offset;
        newPieces.push_back(
            {p.buffer, p.start + rightOffset, p.length - rightOffset});
      }
    }
    offset += p.length;
  }

  pieces_ = std::move(newPieces);
  lineStartsDirty_ = true;
}

void PieceTable::replace(size_t start, size_t length, const std::string &text) {
  erase(start, length);
  insert(start, text);
}

std::string PieceTable::text() const {
  std::string result;
  result.reserve(length());
  for (const auto &p : pieces_) {
    const auto &buf = (p.buffer == BufferKind::Original) ? original_ : added_;
    result.append(buf, p.start, p.length);
  }
  return result;
}

std::string PieceTable::substr(size_t start, size_t len) const {
  std::string result;
  result.reserve(len);
  size_t remaining = len;
  size_t offset = 0;

  for (const auto &p : pieces_) {
    if (remaining == 0)
      break;
    size_t pEnd = offset + p.length;

    if (pEnd <= start) {
      offset += p.length;
      continue;
    }

    size_t localStart = (start > offset) ? (start - offset) : 0;
    size_t available = p.length - localStart;
    size_t take = std::min(available, remaining);

    const auto &buf = (p.buffer == BufferKind::Original) ? original_ : added_;
    result.append(buf, p.start + localStart, take);
    remaining -= take;
    offset += p.length;
  }
  return result;
}

size_t PieceTable::length() const {
  size_t total = 0;
  for (const auto &p : pieces_) {
    total += p.length;
  }
  return total;
}

bool PieceTable::empty() const { return length() == 0; }

void PieceTable::rebuildLineStarts() const {
  lineStarts_.clear();
  lineStarts_.push_back(0);
  std::string content = text();
  for (size_t i = 0; i < content.size(); ++i) {
    if (content[i] == '\n') {
      lineStarts_.push_back(i + 1);
    }
  }
  lineStartsDirty_ = false;
}

size_t PieceTable::lineCount() const {
  if (lineStartsDirty_)
    rebuildLineStarts();
  return lineStarts_.size();
}

std::string PieceTable::line(size_t lineIdx) const {
  if (lineStartsDirty_)
    rebuildLineStarts();
  if (lineIdx >= lineStarts_.size())
    return "";

  size_t start = lineStarts_[lineIdx];
  size_t end;
  if (lineIdx + 1 < lineStarts_.size()) {
    end = lineStarts_[lineIdx + 1];
  } else {
    end = length();
  }
  return substr(start, end - start);
}

std::pair<size_t, size_t> PieceTable::charToLineCol(size_t charIdx) const {
  if (lineStartsDirty_)
    rebuildLineStarts();

  size_t total = length();
  if (charIdx > total)
    charIdx = total;

  // Find which line this char is on (binary search)
  size_t lo = 0, hi = lineStarts_.size();
  while (lo + 1 < hi) {
    size_t mid = (lo + hi) / 2;
    if (lineStarts_[mid] <= charIdx) {
      lo = mid;
    } else {
      hi = mid;
    }
  }
  return {lo, charIdx - lineStarts_[lo]};
}

size_t PieceTable::lineColToChar(size_t line, size_t col) const {
  if (lineStartsDirty_)
    rebuildLineStarts();

  if (line >= lineStarts_.size()) {
    return length();
  }

  size_t lineStart = lineStarts_[line];
  size_t lineEnd;
  if (line + 1 < lineStarts_.size()) {
    lineEnd = lineStarts_[line + 1];
  } else {
    lineEnd = length();
  }

  size_t lineLen = lineEnd - lineStart;
  // Clamp col to line length (excluding newline)
  if (lineLen > 0 && line + 1 < lineStarts_.size()) {
    // Line ends with \n, so available columns = lineLen - 1
    lineLen -= 1;
  }
  size_t clampedCol = std::min(col, lineLen);
  return lineStart + clampedCol;
}

} // namespace lighttex::core
