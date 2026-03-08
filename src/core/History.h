#pragma once

#include <optional>
#include <string>
#include <vector>

namespace lighttex::core {

struct EditOperation {
    size_t start;
    size_t end;
    std::string text;
};

class History {
public:
    History() = default;

    void push(std::vector<EditOperation> ops);
    void pushUndo(std::vector<EditOperation> ops);
    void pushRedo(std::vector<EditOperation> ops);

    std::optional<std::vector<EditOperation>> popUndo();
    std::optional<std::vector<EditOperation>> popRedo();

    std::optional<std::vector<EditOperation>> undo();
    std::optional<std::vector<EditOperation>> redo();

    [[nodiscard]] bool canUndo() const { return !undoStack_.empty(); }
    [[nodiscard]] bool canRedo() const { return !redoStack_.empty(); }

    void clear();

private:
    std::vector<std::vector<EditOperation>> undoStack_;
    std::vector<std::vector<EditOperation>> redoStack_;
};

} // namespace lighttex::core
