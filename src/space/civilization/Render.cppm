
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <cstdint>

export module beatuy.feature.civilization:render;

import beatuy.game;

namespace civilization {
class Renderer : public game::View {
public:
    using View::View;
    void* layer(int index) const {
        return call<void*>(0x58, index);
    }
    void registerLayer(void* newLayer, int index) const {
        call<void>(0x4c, newLayer, index, 0);
    }
};
}

