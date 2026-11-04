
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <Windows.h>
#include <array>
#include <cstdint>
#include <optional>
#include <span>

export module beatuy.feature.civilization;

export import :designs;
export import :marker;
export import :render;
export import :settings;
export import :text;
export import :unit;
import beatuy.game;

namespace civilization {
struct Scene {
    int enemies = 0, friends = 0, wounded = 0, fighting = 0, enemyVehicles = 0, enemyTurrets = 0;
    bool hostileShip = false;
};
}

namespace feature {
export class Civilization {
public:
    void tick();
    void onLeavePlanet(bool destroyed);

    void onDamage(void* victim, uint32_t attackerPoliticalId, void* attacker, float damage, int type);
    bool onOrder(void* vehicle);
    bool locked(void* vehicle);
    int inReach(void* combatant, void* other);
    bool playerSide(void* combatant);
    void* lockedTarget(void* vehicle);
    bool ownCall();
    int damageType(void* victim, void* attacker, int type);
    float superHit(void* victim, void* attacker, float damage, int& type);
    void vehicleDesign(void* city, int purpose, int locomotion, game::Key& key);
    uint32_t healerFor(void* vehicleCombatant, uint32_t healer, int type);
    bool blockHeal(void* victim, uint32_t healer, int type);
    void onBeliefLaunch(char* shell);
    bool spared(void* combatant);
    bool blockShot(void* tool, void* source, void* target);
    void beginBelief(char* shell);
    void endBelief();
    const game::Vector* push(void* victim, void* attacker, const game::Vector* direction, game::Vector& out);

private:
    using Unit = civilization::Unit;
    static constexpr const civilization::Settings::Limits& limits = civilization::settings.limits;

    struct Town {
        game::City city;
        uint32_t politicalId;
        game::Vector center;
        float radius;
        bool close;
    };

    struct Pulled {
        Unit vehicle;
        game::GameData target;
        game::Vector point;
        map::identifiers::Order kind;
        int flag;
        bool hadOrder;
        DWORD moveTick;
        game::Vector movePoint;
        float bestGap;
        DWORD bestTick;
        game::Ship prey;
        std::array<game::Tool, 2> weapons;
        std::array<float, 2> baseRange;
        DWORD aloneSince;
    };

    struct Dropped {
        Unit vehicle;
        DWORD tick;
    };

    struct Hall {
        game::PlanetRecord record;
        game::Vector at;
    };

    struct LoiterState {
        uint32_t politicalId;
        float seconds, last;
        int steps, flat;
    };

    struct Penalty {
        uint32_t politicalId;
        DWORD tick;
    };

    struct Help {
        uint32_t defended, enemy;
        game::Vector at;
    };

    enum class Answer { None, Accepted, Closed };

    struct Offer {
        Unit vehicle;
        uint32_t event;
        int amount;
        DWORD shown;
        Answer answer;
    };

    struct Harm {
        uint32_t politicalId, by;
        DWORD tick;
        game::Vector at;
        bool wounded;
    };

    struct Arsenal {
        game::Civilization nation;
        DWORD next;
        bool broken;
        int last;
    };

    struct Shot {
        int ability;
        game::Vector at;
        bool ship;
    };

    struct Command {
        uint32_t politicalId, enemy;
        DWORD rallyUntil, nextDefense, nextOffense, threatTick;
        game::Vector front;
    };

    struct Beam {
        game::Belief shell;
        DWORD launched;
    };

    struct Shove {
        game::Ship ship;
        DWORD tick;
    };

    struct Reach {
        float horizontal, vertical, range;
        game::Vector under;
    };

    class Own {
    public:
        explicit Own(int& counter) : depth(counter) {
            ++depth;
        }

        ~Own() {
            --depth;
        }

        Own(const Own&) = delete;
        Own& operator=(const Own&) = delete;

    private:
        int& depth;
    };

    static char onBribeClick(uint32_t event, void* user);
    static void onBribeRemoved(uint32_t event, void* user);

    [[nodiscard]] static bool identifierIn(std::span<const uint32_t> identifiers, Unit unit);
    [[nodiscard]] bool isSpared(Unit unit) const;
    [[nodiscard]] bool isPulled(Unit unit) const;
    [[nodiscard]] bool isHunting(uint32_t politicalId) const;
    [[nodiscard]] bool isReacting(game::City city) const;
    [[nodiscard]] bool civilizationHere(uint32_t politicalId) const;
    [[nodiscard]] bool hosts(game::Civilization nation) const;
    void refreshCities();
    Town* nearestCity(uint32_t politicalId, const game::Vector& at);
    Town* anyCity(uint32_t politicalId);
    [[nodiscard]] int citiesOf(uint32_t politicalId) const;
    [[nodiscard]] const Pulled* find(Unit unit) const;
    void refreshEscorts();
    [[nodiscard]] game::Ship sideShip(const void* combatant) const;

    [[nodiscard]] bool observing(uint32_t politicalId, DWORD now) const;
    void observe(uint32_t politicalId, DWORD now);
    void unobserve(uint32_t politicalId);
    Penalty* due(std::span<Penalty> list, int& count, uint32_t politicalId, DWORD now, DWORD gap);
    void punish(uint32_t politicalId, Unit unit, DWORD now);
    [[nodiscard]] uint32_t victimOf(Unit unit) const;
    void thank(DWORD now);
    void loiter(uint32_t politicalId, float elapsed);
    void watch(game::Ship ship);
    void makePeace(uint32_t politicalId, float standing);
    bool attacking(uint32_t attacker, uint32_t victim);

    [[nodiscard]] static Reach measure(Unit unit, game::Ship ship);
    [[nodiscard]] static bool within(const Reach& reach);
    void approach(Pulled& pulledUnit, DWORD now, bool force);
    static void boost(Pulled& pulledUnit);
    static void unboost(Pulled& pulledUnit);
    [[nodiscard]] game::Ship choosePrey(Unit unit, game::Ship current) const;
    void lock(Pulled& pulledUnit, game::Ship prey);
    void attack(Pulled& pulledUnit, game::Ship ship);
    [[nodiscard]] bool recentlyDropped(Unit unit, DWORD now) const;
    [[nodiscard]] bool canEngage(Unit unit, game::Ship ship) const;
    void restore(Pulled& pulledUnit, const void* ship);
    void letGo(int index, DWORD now);
    void pull(Unit unit, game::Ship ship);
    void aimTurret(game::Turret turret, game::Ship ship);
    void aimCity(game::City city, game::Ship ship);
    void react(Town& town);
    void huntUnits(uint32_t politicalId, game::Ship ship);
    void fightBack(game::Ship ship, DWORD now);
    void hunt(uint32_t politicalId);
    void war(uint32_t politicalId);
    void remember(const Town& town);
    [[nodiscard]] bool remembered(const Town& town) const;
    void releaseAll(bool issue);
    void maintain(game::Ship ship, DWORD now);

    Command* commandOf(uint32_t politicalId);
    [[nodiscard]] bool available(Unit unit, uint32_t politicalId) const;
    void issue(Unit unit, void* target, map::identifiers::Order kind);
    void rally(const Town& hit, game::Ship playerShip);
    void threat(uint32_t politicalId, uint32_t enemy, const game::Vector& at);
    static int unitFormOf(const void* target, game::Array<Unit*> all);
    void scanOrders(DWORD now);
    void defend(Command& command, DWORD now);
    void offense(Command& command, DWORD now);
    void coordinate(DWORD now);

    void newVisit(game::Planet visited);
    void think(float elapsed);

    void awaitAnswer(DWORD now);
    void accept(DWORD now);
    void ignored(DWORD now);
    void endOffer();
    void cancelOffer();
    void spare(Unit unit, DWORD now);
    void breach(Unit unit);
    void betrayed();
    int allies(Unit unit, uint32_t politicalId);
    void propose(Pulled& pulledUnit, DWORD now);
    void bribes(DWORD now);
    void guard();
    void rings();
    [[nodiscard]] bool automatic(void* tool, game::Ship ship) const;

    [[nodiscard]] static bool wounded(void* combatant, float damage);
    void harmed(uint32_t politicalId, const game::Vector& at, uint32_t by, bool wounded);
    static void shuffle(std::span<Shot> shots);
    [[nodiscard]] bool angry(uint32_t politicalId, uint32_t me) const;
    std::optional<game::Vector> swarm(uint32_t politicalId, const game::Vector& around, float radius);
    std::optional<game::Vector> woundedAround(uint32_t politicalId, const game::Vector& around, float radius);
    [[nodiscard]] uint32_t cardsOf(uint32_t politicalId, game::Civilization nation) const;
    static void consider(std::span<Shot> shots, int& count, uint32_t cards, int ability, const game::Vector& at,
                         bool ship);
    int plan(uint32_t politicalId, game::Civilization nation, const game::Vector& ship, std::span<Shot> shots);
    bool safe(uint32_t politicalId, const Shot& shot);
    struct Launcher {
        game::City city;
        float distance;
    };
    [[nodiscard]] Launcher launcherFor(game::Civilization nation, const game::Vector& at) const;
    [[nodiscard]] civilization::Scene sceneAt(uint32_t politicalId, const game::Vector& at, float radius,
                                              bool atShip) const;
    float worthOf(uint32_t politicalId, game::Civilization nation, Shot& shot) const;
    void watchShip(DWORD now);
    Arsenal* arsenalOf(game::Civilization nation);
    bool ready(game::Civilization nation, Arsenal& arsenal);
    bool launch(game::Civilization nation, const Shot& shot, DWORD now);
    void superweapons(DWORD now);
    void drain(game::Ship ship, int& type);
    void asArtillery(game::Ship ship, int& type);

    DWORD launched(game::Belief shell);
    void picks(DWORD now);

    game::Planet planet;
    bool civilizationPlanet = false;
    std::array<Town, limits.maximumCities> towns{};
    int townCount = 0;

    std::array<Pulled, limits.maximumPulled> pulled{};
    int pulledCount = 0;
    std::array<game::Turret, limits.maximumTurrets> turrets{};
    int turretCount = 0;
    std::array<uint32_t, limits.maximumCivilizations> hunting{};
    int huntCount = 0;
    std::array<game::City, limits.maximumCities> reacting{};
    int reactCount = 0;
    std::array<Hall, limits.maximumHalls> halls{};
    int hallCount = 0;
    std::array<LoiterState, limits.maximumCivilizations> loiterStates{};
    int loiterCount = 0;
    std::array<Dropped, limits.maximumPulled> dropped{};
    int droppedCount = 0;
    std::array<game::Ship, limits.maximumEscorts> escorts{};
    int escortCount = 0;
    std::array<Unit, limits.maximumHits> hits{};
    int hitCount = 0;

    std::array<Penalty, limits.maximumCivilizations> penalties{};
    int penaltyCount = 0;
    std::array<Penalty, limits.maximumCivilizations> observers{};
    int observerCount = 0;
    std::array<Help, limits.maximumHits> helps{};
    int helpCount = 0;
    std::array<Penalty, limits.maximumCivilizations> thanks{};
    int thanksCount = 0;
    map::identifiers::Relation thanksEvent = map::identifiers::Relation::FoughtEnemy;
    std::array<uint32_t, limits.maximumCivilizations> hostile{};
    int hostileCount = 0;

    Offer offer{};
    bool fightingBack = false;
    std::array<Unit, limits.maximumSpared> sparedHere{};
    int sparedHereCount = 0;
    std::array<uint32_t, limits.maximumSpared> sparedIdentifiers{};
    int sparedCount = 0;
    std::array<uint32_t, limits.maximumAsked> asked{};
    int askedCount = 0;
    std::array<uint32_t, limits.maximumStruck> struck{};
    int struckCount = 0, struckNext = 0;
    DWORD nextOffer = 0;
    int breaches = 0;
    DWORD beamTick = 0;

    std::array<Harm, limits.maximumCivilizations> harms{};
    int harmCount = 0;
    std::array<Arsenal, limits.maximumCivilizations> arsenals{};
    int arsenalCount = 0;
    DWORD nextSuper = 0, nextSuperThink = 0, superAtShip = 0;
    game::Vector shipSeen{}, shipVelocity{};
    DWORD shipSeenAt = 0;
    bool shipWatched = false;
    int superTurn = 0;
    bool superDrained = false;

    bool attackPending = false, attackedHall = false;
    uint32_t attackedPoliticalId = 0;
    game::Vector attackedAt{};

    DWORD nextThink = 0, nextAssert = 0, lastThink = 0;

    std::array<Beam, limits.maximumBeams> beams{};
    int beamNext = 0;
    game::Belief belief;
    std::array<Shove, limits.maximumEscorts + 1> shoves{};
    int shoveCount = 0;
    int ownDepth = 0;

    std::array<Command, limits.maximumCivilizations> commands{};
    int commandCount = 0;
    int unitTarget = -1;
    std::array<int, 2> cityTarget{-1, -1};
    DWORD nextOrderScan = 0;

    civilization::Designs designs;
    civilization::Marker marker;
};
}

