
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <Windows.h>
#include <array>
#include <cstdint>
#include <optional>

export module beatuy.feature.tribe;

export import :spear;
export import :village;
import beatuy.game;

namespace feature {
namespace tribe {
struct Settings {
    int maximumTribes = 16, maximumDefenders = 24, maximumFlights = 32, maximumStuck = 40, maximumTargets = 9;
    int defenderShare = 40, maximumPerTribe = 4;
    int maximumPerHull = 14;
    int spearTool = 3, spearItem = 3, spearDamage = 2;
    uint32_t throwAnimation = 0x2c39370, grabAnimation = 0x2c39200, spearGetAnimation = 0x56d43de,
             dropPanicAnimation = 0x3a74a0b;
    std::array<uint32_t, 5> taunts = {0x591b041, 0x591b042, 0x591b043, 0x591b031, 0x591b032};
    int tauntChance = 45;
    int runSpeed = 2, turnSpeed = 0;
    DWORD scanMilliseconds = 500, releaseMilliseconds = 450, grabMilliseconds = 1000, regripMilliseconds = 1200,
          spearGetMilliseconds = 700, recoverMilliseconds = 900, advanceMilliseconds = 300, throwMilliseconds = 3500,
          throwJitterMilliseconds = 2000, calmMilliseconds = 6000, flightMilliseconds = 8000;
    float step = 4.0f, stepJitter = 5.0f, closeIn = 12.0f, home = 35.0f;
    float alarm = 220.0f, calm = 330.0f, throwRange = 130.0f, spearHit = 20.0f, lob = 1.6f;
    float hullSlack = 1.25f, hullMargin = 3.0f, embed = 0.35f, defaultFront = 1.0f;
    uint8_t meshCluster = 3;
};
}

export class Tribe {
public:
    [[nodiscard]] bool holds(void* creature);
    [[nodiscard]] int spearTick(void* projectile);
    [[nodiscard]] bool spearAfter(void* projectile, bool alive);
    void tick();

private:
    static constexpr tribe::Settings settings{};

    struct Defender {
        tribe::Citizen citizen;
        tribe::Village village;
        DWORD nextThrow = 0, release = 0, regrip = 0, maneuver = 0;
        int item = 0;
        bool armed = false;
    };
    struct Flight {
        tribe::Spear spear;
        tribe::Citizen thrower;
        game::Ship ship;
        uint32_t politicalId;
        DWORD expires;
        game::Vector heading;
    };
    struct Stuck {
        tribe::Spear spear;
        game::Ship ship;
        game::Vector offset;
        tribe::Quaternion turn;
        DWORD since;
    };
    struct Alert {
        tribe::Village village;
        DWORD calmSince;
        bool calm, rack;
    };
    struct Ships {
        std::array<char*, settings.maximumTargets> ship{};
        std::array<game::Vector, settings.maximumTargets> at{};
        int count = 0;
    };

    static bool spearman(tribe::Citizen citizen);
    static bool ownsSpears(tribe::Village village);
    static bool equip(tribe::Citizen citizen, int item);
    static void pin(const Stuck& lodged);
    static float frontOf(tribe::Spear spear, const tribe::Quaternion& turn, const game::Vector& heading);
    static std::optional<float> pick(const tribe::Mesh& mesh, const game::Vector& from, const game::Vector& to);

    bool disarmed(tribe::Village village) const;
    Ships gather();
    int nearest(const game::Vector& from, float reach) const;
    float closest(const game::Vector& from) const;
    void arm(tribe::Village village);
    bool throwSpear(tribe::Citizen citizen, game::Ship ship, DWORD now);
    void land(int index);
    int flightOf(tribe::Spear spear) const;
    void unstick(int index);
    std::optional<tribe::Mesh> pickable(game::Ship ship);
    void strike(game::Ship ship, const Flight& flight);
    game::Ship hullAt(const game::Vector& point);
    void makeRoom(game::Ship ship);
    void stick(int index, game::Ship ship, const game::Vector& point);
    int fly(int index, const game::Vector& from, const game::Vector& to);
    void maneuver(const Defender& defender);
    void drop(int index, bool restore);
    void disarm(tribe::Village village);
    void standDown(tribe::Village village);
    void reset();
    void watch(game::Array<tribe::Village*> villages, DWORD now);
    void defend(int index, DWORD now);
    void loose(Defender& defender, const game::Vector* at, DWORD now);
    void windUp(Defender& defender, const game::Vector& at, DWORD now);

    game::Planet planet;
    std::array<Alert, settings.maximumTribes> alerts;
    int alertCount = 0;
    std::array<tribe::Village, settings.maximumTribes> disarmedTribes;
    int disarmedCount = 0;
    std::array<Defender, settings.maximumDefenders> defenders;
    int defenderCount = 0;
    DWORD nextScan = 0;
    std::array<Flight, settings.maximumFlights> flights;
    int flightCount = 0;
    std::array<Stuck, settings.maximumStuck> stuck;
    int stuckCount = 0;
    Ships targets, passed;
};
}

