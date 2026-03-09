#pragma once

// Vim mode stub (v0.3)
// C++ state machine: Normal/Insert/Visual/Command/Search modes.

namespace lighttex::vim {

enum class Mode { Normal, Insert, Visual, Command, Search };

class VimMode {
public:
  VimMode() = default;
  // TODO (v0.3): Full vim emulation
};

} // namespace lighttex::vim
