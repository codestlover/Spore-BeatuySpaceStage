
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <Windows.h>
#include <array>
#include <cstdint>

export module beatuy.feature.escort;

import beatuy.game;

namespace feature {
struct EscortSettings {
    int maximumShips = 64, maximumFormer = 64;
    DWORD watchMilliseconds = 500;
};

export class Escort {
public:
    [[nodiscard]] bool blocks(void* target);
    int ships(char** out, int maximum);
    void maskFormer();
    void unmask();
    void tick();

private:
    struct Masked {
        game::Ship ship;
        uint32_t politicalId;
    };
    static constexpr EscortSettings settings{};

    void watch(game::Ship playerShip);

    DWORD nextWatch = 0;
    std::array<uint32_t, settings.maximumShips> escortIdentifiers{};
    std::array<uint32_t, settings.maximumFormer> formerIdentifiers{};
    int escortIdentifierCount = 0, formerCount = 0;
    std::array<Masked, settings.maximumShips> masked{};
    int maskedCount = 0;
};
}

