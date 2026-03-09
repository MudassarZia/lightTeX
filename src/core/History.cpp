#include "core/History.h"

namespace lighttex::core {

void History::push(std::vector<EditOperation> ops) {
  undoStack_.push_back(std::move(ops));
  redoStack_.clear();
}

void History::pushUndo(std::vector<EditOperation> ops) {
  undoStack_.push_back(std::move(ops));
}

void History::pushRedo(std::vector<EditOperation> ops) {
  redoStack_.push_back(std::move(ops));
}

std::optional<std::vector<EditOperation>> History::popUndo() {
  if (undoStack_.empty())
    return std::nullopt;
  auto ops = std::move(undoStack_.back());
  undoStack_.pop_back();
  return ops;
}

std::optional<std::vector<EditOperation>> History::popRedo() {
  if (redoStack_.empty())
    return std::nullopt;
  auto ops = std::move(redoStack_.back());
  redoStack_.pop_back();
  return ops;
}

std::optional<std::vector<EditOperation>> History::undo() {
  auto ops = popUndo();
  if (ops) {
    redoStack_.push_back(*ops);
  }
  return ops;
}

std::optional<std::vector<EditOperation>> History::redo() {
  auto ops = popRedo();
  if (ops) {
    undoStack_.push_back(*ops);
  }
  return ops;
}

void History::clear() {
  undoStack_.clear();
  redoStack_.clear();
}

} // namespace lighttex::core
