
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <span>

export module beatuy.game.objects:empire;

import :view;
import beatuy.game.map;
import beatuy.game.math;
import beatuy.game.memory;

export namespace game {
class Empire : public View {
public:
    using View::View;
    uint32_t colorIdentifier() const {
        return at<uint32_t>(0x10);
    }
    map::identifiers::Archetype& archetype() const {
        return at<map::identifiers::Archetype>(0x58);
    }
    uint32_t& politicalId() const {
        return at<uint32_t>(0x84);
    }
    uint32_t& homeStarIdentifier() const {
        return at<uint32_t>(0xb0);
    }
    uint32_t& homePlanetIdentifier() const {
        return at<uint32_t>(0xb4);
    }
    int& money() const {
        return at<int>(0xd0);
    }
    bool colour(std::span<float, 3> color) const {
        if (!object)
            return false;
        float out[3] = {-1.0f, -1.0f, -1.0f};
        const float* got = map::civilization::empireColor(object, out);
        if (!got)
            return false;
        std::span<const float, 3> channels(got, 3);
        if (!std::ranges::all_of(channels, [](float channel) { return std::isfinite(channel) && channel >= 0.0f; }))
            return false;
        std::ranges::transform(channels, color.begin(), [](float channel) { return std::min(channel, 1.0f); });
        return true;
    }
};
}

