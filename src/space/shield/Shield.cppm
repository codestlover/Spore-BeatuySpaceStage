
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <Windows.h>
#include <array>
#include <cstdint>
#include <optional>
#include <span>

export module beatuy.feature.shield;

import beatuy.game;

namespace shield {
struct Look {
    std::array<uint32_t, 2> cracks{}, cracked{};
    uint32_t shatter = 0, hit = 0;
};

struct Settings {
    int eliteLevel = 5;
    DWORD thinkMilliseconds = 250;
    DWORD missileMilliseconds = 6000;
    float missileRange = 400.0f;
    uint32_t missile = 0x91b04181;
    int missilesInFlight = 3;
    float missileSpeed = 140.0f, missileTopSpeed = 420.0f, missileChase = 1.35f;
    float missileTurnRadius = 25.0f, missileStrikeRadius = 10.0f, missileStrikeRange = 60.0f;
    DWORD missileHeadStart = 1000, paceMilliseconds = 100, trailMilliseconds = 1500;
    float paceKeep = 0.85f;
    float agileSpeed = 1.5f, agileAcceleration = 2.5f;
    std::array<uint32_t, 4> lockingTools{0xfc220278, 0xfc22027b, 0xfc22027a, 0xfc22027d};
    int patrolJumpChance = 8;
    DWORD patrolGapMilliseconds = 4000;
    float patrolScatter = 0.25f, hearRange = 600.0f;
    int decoyChance = 80;
    DWORD dashCooldownMilliseconds = 900, dashMilliseconds = 260;
    float dashSpeed = 0.27f, dashCost = 0.02f, dashSide = 0.8f, dashAway = 0.6f, reacquireRange = 62.0f,
          misjudgement = 0.2f;
    uint32_t decoy = 0x2ecc4755, decoySound = 0x2ecc4755;
    uint32_t jumpOut = 0x8f7c24f6, jumpTrail = 0x46c66d2e, jumpIn = 0x0914ac5b, jumpSound = 0x544be4b7;
    DWORD decoyBurstMilliseconds = 2500, jumpBurstMilliseconds = 1200;
    DWORD decoySoundMilliseconds = 1200, launchSoundMilliseconds = 1800;
    std::array<uint32_t, game::identityColors> trails{0x9cb36b1f, 0x9cb36b1c, 0x9cb36b1d, 0x9cb36b1a, 0x9cb36b1b,
                                                      0x9cb36b18, 0x9cb36b19, 0x9cb36b16, 0x9cb36b17, 0x9db36cf1,
                                                      0x9db36cf0, 0x9db36cf3, 0x9db36cf2};
    float raiseBelow = 0.6f, shipStrength = 1.5f;
    DWORD shipCooldownMilliseconds = 90000, shipMilliseconds = 90000;
    uint32_t bubble = 0x77846e7d;
    float shipScale = 0.65f;
    float cityStrength = 3000.0f, cityStrengthShare = 1.2f, cityRadius = 60.0f, cityPadding = 1.08f;
    float domeRadius = 64.0f;
    int rockContact = 3;
    float domeBerth = 15.0f;
    std::array<int, 2> raidEscorts{2, 3};
    int mostSorties = 2;
    uint32_t domeRipple = 0xd8d698b8;
    DWORD rippleMilliseconds = 1200;
    DWORD cityMilliseconds = 120000, cityCooldownMilliseconds = 90000;
    std::array<float, 2> stages{2.0f / 3.0f, 1.0f / 3.0f};
    uint32_t dome = 0x53293a51;
    Look bubbleLook{.cracks{0x909e23b2, 0x909e23b1},
                    .cracked{0x62599cc1, 0x62599cc2},
                    .shatter = 0xaeaf0e9d,
                    .hit = 0x82fdeeff};
    Look domeLook{.cracks{0xf5054de8, 0xf5054deb},
                  .cracked{0x00eccb07, 0x00eccb04},
                  .shatter = 0xd7788d83,
                  .hit = 0x4cafe591};
    std::array<std::array<uint32_t, 2>, 2> crackSounds{{{0x909e23b2, 0xda419e61}, {0x909e23b1, 0xda419e62}}};
    std::array<uint32_t, 3> shatterSounds{0x3d2917fc, 0x404b9d2a, 0x162a0793};
    uint32_t hitSound = 0x82fdeeff, launchSound = 0x485a211c;
    DWORD burstMilliseconds = 3000, hitMilliseconds = 350, hitBurstMilliseconds = 700;
    float gaugeRowExtra = 10.0f, gaugeIconSize = 17.0f, gaugeIconGap = 9.0f;
    std::array<game::Key, 3> platePictures{{{.instance = 0x73c765dc, .type = 0x2f7d0004, .group = 0x3064cb38},
                                            {.instance = 0xa1badbf5, .type = 0x2f7d0004, .group = 0x3064cb38},
                                            {.instance = 0x2d353d43, .type = 0x2f7d0004, .group = 0x3064cb38}}};
    game::Rectangle plateArea{0.0f, 0.0f, 150.0f, 28.0f}, plateIcon{8.0f, 5.0f, 25.0f, 22.0f},
        plateTrack{32.0f, 10.0f, 140.0f, 18.0f}, plateFill{1.0f, 2.0f, 107.0f, 6.0f};
    float plateLift = 1.06f, plateGap = 6.0f, plateMargin = 4.0f;
    int rootDepth = 32;
    DWORD fuseMilliseconds = 3600000;
    float shotLookAhead = 2.0f;
    uint32_t takeDamageSlot = 0x18;
    DWORD endMilliseconds = 1500;
    game::Key gaugeIcon{.instance = 0xb17df2dd, .type = 0x2f7d0004, .group = 0x3064cb38};
};
inline constexpr Settings settings{};
inline constexpr int maximumBarriers = 24, maximumParts = 64, maximumBursts = 32, maximumCaptains = 96,
                     maximumWards = 24, maximumMissiles = 8, maximumStrays = 8, maximumHops = 4, maximumShots = 48,
                     maximumBeamPoints = 128, maximumSorties = 32;
}

namespace feature {
export class Shield {
public:
    void tick();
    void onLeavePlanet(bool destroyed);
    bool onDamage(void* combatant, uint32_t politicalId, int type, float damage, void* attacker);
    bool onShipDamage(void* combatant, int type, float damage);
    void onCollisions(char* ship);
    void escort(char* ships, int& fewest, int& most) const;
    bool steer(char* ship, game::Vector& point);
    [[nodiscard]] bool launching() const;
    [[nodiscard]] bool covers(void* city) const;
    struct Cut {
        game::Vector* first = nullptr;
        int count = 0;
        std::array<game::Vector, shield::maximumBeamPoints> kept;
    };
    [[nodiscard]] Cut cutBeam(void* view);
    static void mendBeam(const Cut& cut);
    bool onMissile(void* missile);
    bool onProjectile(void* tool, void* projectile, const game::Vector* origin);
    [[nodiscard]] float speedOf(void* ship) const;
    [[nodiscard]] float accelerationOf(void* ship) const;

private:
    struct Barrier {
        game::Ship ship;
        game::City city;
        std::array<game::GameData, shield::maximumParts> parts{};
        int partCount = 0;
        float strength = 0.0f, left = 0.0f;
        int stage = 0;
        DWORD until = 0, nextHit = 0;
        game::VisualEffect dome, cracks;
    };
    struct Burst {
        game::GameData holder;
        void* spatial = nullptr;
        game::VisualEffect effect;
        float scale = 1.0f;
        DWORD until = 0;
    };
    struct Captain {
        game::Ship ship;
        DWORD nextMissile = 0, shieldReady = 0, nextDash = 0, dashUntil = 0;
        game::Vector dashVelocity{}, landing{};
        bool dashing = false, guest = false;
    };
    struct Sortie {
        game::Ship fighter, bomber;
    };
    struct Ward {
        game::City city;
        DWORD ready = 0;
    };
    struct Clip {
        int track = 0;
        DWORD until = 0;
        bool heard = false;
    };
    struct Hop {
        game::Ship ship;
        game::Vector landing{};
        DWORD until = 0;
    };
    struct Stray {
        game::Projectile projectile;
        game::Ship ship;
    };
    struct Flight {
        game::Missile missile;
        game::GameData target;
        game::VisualEffect trail;
        game::Vector seen{}, last{};
        float pace = 0.0f;
        DWORD sampled = 0, fadeUntil = 0;
        uint32_t shooter = uint32_t(-1);
    };
    struct Moved {
        game::Window window;
        game::Rectangle area{};
    };
    struct Gauge {
        game::Window host, bar, frame, fill, icon;
        game::StandardDrawable look;
        void* image = nullptr;
        Moved plate;
        std::array<Moved, 8> pushed{};
        int pushedCount = 0;
    };
    struct Shooter {
        uint32_t politicalId = uint32_t(-1);
        void* fighter = nullptr;
    };
    struct Shot {
        game::Projectile projectile;
        game::Vector last{};
        Shooter shooter;
    };
    struct Ended {
        game::VisualEffect effect;
        DWORD at = 0;
    };
    struct Plate {
        game::Window root, card, fill;
        std::array<game::Window, 2> parts{};
        std::array<game::StandardDrawable, 4> looks{};
        std::array<void*, 4> images{};
        bool placed = false;
    };

    [[nodiscard]] std::span<Barrier> barriers();
    [[nodiscard]] Barrier* barrierOf(void* combatant);
    bool absorb(void* combatant, int type, float damage, void* attacker);
    static void alert(const Barrier& barrier, void* attacker);
    void strike(Barrier& barrier, float damage, std::optional<game::Vector> at = std::nullopt);
    void ripple(const Barrier& barrier, const game::Vector& at);
    [[nodiscard]] Barrier* domeAhead(const game::Vector& from, const game::Vector& to, uint32_t shooter);
    [[nodiscard]] static Shooter shooterOf(game::Tool tool);
    void intercept();
    void repel();
    void keepOut(game::Ship ship);
    void end(game::VisualEffect effect);
    void clearEnded(DWORD now);
    [[nodiscard]] const Barrier* pointedDome();
    bool buildPlate();
    void showPlate();
    void dropPlate();
    [[nodiscard]] bool elitePlanet() const;
    [[nodiscard]] static float scaleOf(const Barrier& barrier);
    [[nodiscard]] static const shield::Look& lookOf(const Barrier& barrier);
    [[nodiscard]] static void* spatialOf(const Barrier& barrier);
    Barrier* raiseShip(game::Ship ship, DWORD now);
    bool raiseCity(game::City city, DWORD now);
    static void play(Clip& clip, uint32_t sound, DWORD milliseconds);
    static void quiet(Clip& clip);
    void lower(int index, bool broken);
    void crack(Barrier& barrier, bool loud);
    void burst(const Barrier& barrier, uint32_t effect, DWORD milliseconds);
    void flash(game::GameData holder, void* spatial, uint32_t effect, float scale, DWORD milliseconds);
    [[nodiscard]] float agility(void* ship, float factor) const;
    void settle(Barrier& barrier, bool loud);
    bool jump(Captain& captain, const game::Vector& threat, float danger, float incoming);
    void reacquire(const Captain& captain);
    void patrol(game::Ship ship, DWORD now);
    void weave(DWORD now);
    void moveBursts(DWORD now);
    void command(DWORD now);
    void dispatch();
    void recall(game::Ship bomber);
    void fireMissile(Captain& captain, game::GameData target, DWORD now);
    void guide(DWORD now);
    void land(Flight& flight, DWORD now);
    Captain* captainOf(game::Ship ship, bool guest);
    void guardCity(void* combatant, uint32_t politicalId);
    [[nodiscard]] game::City cityOf(game::GameData part) const;
    void releaseAll();
    [[nodiscard]] const Barrier* barrierFor(game::GameData hovered);
    void showGauge();
    void dropGauge();

    std::array<Barrier, shield::maximumBarriers> barrierList{};
    int barrierCount = 0;
    std::array<Burst, shield::maximumBursts> bursts{};
    int burstCount = 0;
    std::array<Captain, shield::maximumCaptains> captains{};
    int captainCount = 0;
    std::array<Sortie, shield::maximumSorties> sorties{};
    int sortieCount = 0;
    std::array<Ward, shield::maximumWards> wards{};
    int wardCount = 0;
    std::array<Flight, shield::maximumMissiles> flights{};
    int flightCount = 0;
    std::array<Stray, shield::maximumStrays> strays{};
    int strayCount = 0;
    std::array<Hop, shield::maximumHops> hops{};
    int hopCount = 0;
    std::array<Shot, shield::maximumShots> shots{};
    int shotCount = 0;
    std::array<Ended, shield::maximumBarriers> ended{};
    int endedCount = 0;
    Plate plate;
    DWORD nextPatrolJump = 0;
    Clip decoyClip;
    std::array<Clip, size_t(shield::settings.missilesInFlight)> launchClips{};
    void* missileTool = nullptr;
    game::Ship launcher;
    game::GameData aimed;
    bool firing = false;
    Gauge gauge;
    DWORD nextThink = 0, lastTick = 0;
    void holdDeadlines(DWORD step);
};
}

