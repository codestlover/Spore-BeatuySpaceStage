
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <span>

export module beatuy.feature.civilization:text;

namespace civilization {
class Text {
public:
    static const wchar_t* bribeOffer(int amount, std::span<wchar_t> buffer);
};
}

