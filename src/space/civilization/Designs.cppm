
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <Windows.h>
#include <array>
#include <cstdint>
#include <optional>

export module beatuy.feature.civilization:designs;

import :settings;
import beatuy.game;

namespace civilization {
class Designs {
public:
    struct Job {
        uint32_t politicalId;
        game::Civilization nation;
        int index;
    };

    void choose(game::City city, int purpose, int locomotion, game::Key& key);
    [[nodiscard]] std::optional<Job> due(DWORD now);
    void pick(const Job& job, DWORD now);
    void forget();

private:
    static constexpr int slots = 9;

    struct Pool {
        std::array<game::Key, settings.design.poolSize> keys;
        int count;
    };

    void queue(uint32_t politicalId, game::Civilization nation, int index);

    std::array<Pool, slots> pools{};
    std::array<Job, settings.limits.maximumJobs> jobs{};
    int jobCount = 0;
    std::array<uint32_t, settings.limits.maximumPicked> picked{};
    int pickedCount = 0;
    DWORD nextPick = 0;
};
}

