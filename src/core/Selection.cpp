#include "core/Selection.h"
#include <algorithm>

namespace lighttex::core {

void normalizeSelections(std::vector<Selection>& selections) {
    if (selections.size() <= 1) return;

    std::sort(selections.begin(), selections.end(),
              [](const Selection& a, const Selection& b) {
                  return a.start() < b.start();
              });

    std::vector<Selection> merged;
    merged.push_back(selections[0]);

    for (size_t i = 1; i < selections.size(); ++i) {
        if (merged.back().overlaps(selections[i]) ||
            merged.back().end() >= selections[i].start()) {
            merged.back() = merged.back().merge(selections[i]);
        } else {
            merged.push_back(selections[i]);
        }
    }

    selections = std::move(merged);
}

} // namespace lighttex::core
