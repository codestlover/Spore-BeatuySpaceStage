
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <Windows.h>
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

export module beatuy.feature.escort:hunt;

import beatuy.game;

namespace feature {
struct HuntSettings {
    DWORD thinkMilliseconds = 250;
    std::array<float, 2> searchReaches{400.0f, 2000.0f};
    float turretReach = 400.0f, neighbourReach = 20.0f;
    uint32_t interceptReachProperty = 0x055e7dad;
};

export class Hunt {
public:
    bool prepare();
    void tick(std::span<char* const> escort);
    void stop();
    [[nodiscard]] bool isHunter(void* ship) const;
    [[nodiscard]] bool isCalled(void* ship) const;
    [[nodiscard]] bool attacks(void* ship, void* target) const;
    [[nodiscard]] void* targetFor(void* ship, bool distant, void* found) const;

private:
    static constexpr HuntSettings settings{};
    static constexpr size_t mostSquads = 8;

    void spot(std::span<char* const> escort);
    bool begin();
    void aimTurrets() const;
    void leadReinforcements() const;
    void callSquads();
    void callSquad(uint32_t politicalId);

    bool decisionReady = false, hunting = false;
    uint32_t colonyOwner = uint32_t(-1);
    game::Ship grox;
    std::array<uint32_t, mostSquads> squads{};
    int squadCount = 0;
    DWORD nextThink = 0;
};
}

