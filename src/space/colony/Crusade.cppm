
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <Windows.h>
#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>

export module beatuy.feature.colony:crusade;

import beatuy.game;

namespace feature {
struct CrusadeSettings {
    float leastWaitSeconds = 3600.0f, mostWaitSeconds = 10800.0f, watchSeconds = 5.0f;
    DWORD longestStepMilliseconds = 100;
    int lowestLevel = 3, highestLevel = 5;
    size_t fewestMembers = 3, clusterStars = 8;
    std::array<int, 2> raidBombers{8, 7};
    std::array<map::identifiers::Relation, 3> missedHelp{map::identifiers::Relation::MissionFailed,
                                                         map::identifiers::Relation::MissionRejected,
                                                         map::identifiers::Relation::MissionAborted};
};

export class Crusade {
public:
    void tick();
    void stop();
    bool pardons(uint32_t who, uint32_t cause, map::identifiers::Relation event, float scale);
    std::span<game::RaidShip> lighten(const void* raid, char* list, std::span<game::RaidShip> changed) const;
    static void restore(std::span<const game::RaidShip> changed);
    void* aim(void* combatant, void* target) const;
    bool steer(char* ship, game::Vector& point) const;

private:
    static constexpr CrusadeSettings settings{};
    static constexpr size_t mostCallers = 256, mostMembers = 16, mostTargets = 128;

    struct Target {
        game::PlanetRecord planet;
        game::StarRecord star;
        uint32_t attacker;
        void* raid;
    };

    struct Census {
        size_t callers;
        uint32_t grox;
    };

    using Members = std::array<uint32_t, mostMembers>;

    Census census(std::span<uint32_t> callers) const;
    void start();
    size_t rally(uint32_t leader, Members& members) const;
    void besiege(std::span<const uint32_t> members);
    void watch();
    bool raiding(game::Ship ship) const;
    game::City siege(game::Ship ship) const;
    void settle();
    void conquer(const Target& target) const;

    std::optional<float> countdown;
    DWORD lastTick = 0;
    uint32_t grox = uint32_t(-1);
    std::array<Target, mostTargets> targets{};
    size_t targetCount = 0;
    bool helped = false;
    float owed = 0.0f;
};
}

