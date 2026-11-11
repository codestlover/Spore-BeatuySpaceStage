
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <cstdint>

export module beatuy.feature.colony:contact;

import beatuy.game;

namespace feature {
struct ContactSettings {
    const char16_t* tooltipLayout = u"SpacePlanetTooltip";
    uint32_t layoutGroup = 0x40464100, layoutLayer = 0x05b598f6, badgeControl = 0x05e62810;
    float moodShift = 25.0f, badgeSpacing = 18.0f;
};

export class Contact {
public:
    void onHeader(void* screen, uint32_t politicalId);
    void onUnload();
    void tick() const;

private:
    static constexpr ContactSettings settings{};

    bool borrow();
    void move(game::Window into) const;
    void makeRoom(bool aside) const;
    void align() const;

    game::Layout tooltip;
    game::Window badge, home, mood;
    game::Rectangle moodArea{};
    bool tried = false;
};
}

