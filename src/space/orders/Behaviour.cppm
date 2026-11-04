
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>

export module beatuy.feature.orders:behaviour;

import :book;
import :mark;
import beatuy.game;

namespace feature {
struct EscortBehaviourSettings {
    uint32_t decisionMilliseconds = 250, memoryMilliseconds = 5000;
    uint32_t longestFrameMilliseconds = 100;
    float nearFactor = 1.0f, reachFactor = 0.9f, switchShare = 0.7f;
    float roseRadius = 50.0f, airRoseShare = 0.6f, leastRoseRadius = 10.0f, airLift = 10.0f;
    float hurryDistance = 2.0f, hurrySpeed = 3.5f, hurryAcceleration = 2.5f;
    std::array<float, 3> fallbackRadius{120.0f, 150.0f, 1.0f};
    size_t maximumShots = 32, maximumCandidates = 192;
};
}

export namespace feature {
class EscortBehaviour {
public:
    void update(const OrderBook& book, uint32_t elapsedLowerWord);
    [[nodiscard]] std::optional<void*> orderedTarget(const OrderBook& book, void* combatant);
    void onHostile(void* combatant, void* attacker);
    bool prepareRose();
    void onRoseTick(void* ship, float* state);
    void onLeavePlanet();
    void reset();

    enum class Kind : uint8_t { None, Ship, Turret, Vehicle, LeftToTheGame };
    static constexpr size_t kindCount = size_t(Kind::LeftToTheGame) + 1;
    struct Mark {
        uint32_t identifier;
        Kind kind;
        friend constexpr bool operator==(const Mark&, const Mark&) = default;
    };
    struct Shot {
        uint32_t victim;
        Mark shooter;
        uint64_t time;
    };
    static constexpr EscortBehaviourSettings settings{};

private:
    struct Aim {
        uint32_t politicalId;
        Order order;
        Mark target;
    };

    [[nodiscard]] bool ordersGiven(const OrderBook& book);
    [[nodiscard]] const Aim* aimOf(uint32_t politicalId) const;
    void decide(const OrderBook& book);
    void apply();

    uint64_t gameTime = 0, nextDecision = 0;
    std::array<Shot, settings.maximumShots> shots{};
    size_t nextShot = 0;
    std::array<Aim, maximumEscort> aims{};
    size_t aimCount = 0;
    std::optional<uint32_t> knownRevision;
    bool ordered = false, decisionDue = true;
    AttackMarks marks;
    std::array<uint32_t, maximumEscort> chasing{};
    size_t chasingCount = 0;
    void follow(game::Ship ship, game::Ship leader);
    void releaseChases();
    struct Rose {
        uint32_t politicalId;
        uint32_t identifier;
        char* object;
        bool airborne;
    };
    [[nodiscard]] Rose* roseOf(uint32_t politicalId);
    void leadRose(game::Ship ship, const Aim* aim, void* target);
    void endRose(game::Ship ship, bool clear);
    void releaseRoses();
    std::array<Rose, maximumEscort> roses{};
    size_t roseCount = 0;
};
}

