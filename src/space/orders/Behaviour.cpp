
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>

module beatuy.feature.orders;

import beatuy.core.hooks;
import beatuy.game;

namespace feature::behaviour {
namespace {
using map::identifiers::ShipKind;
using map::identifiers::Type;
using Kind = EscortBehaviour::Kind;
using Mark = EscortBehaviour::Mark;
using Shot = EscortBehaviour::Shot;
constexpr const auto& settings = EscortBehaviour::settings;

game::Array<game::Ship*> spaceShips() {
    return game::each<game::Ship>(map::space::ships);
}

bool inPosse(uint32_t politicalId) {
    game::PlayerShipManager manager = game::shipManager();
    return manager && manager.escort().has(politicalId);
}

bool fights(game::Ship ship) {
    return ship && ship.alive() && !ship.fighter().destroyed() && ship.specialState() != 3 && !ship.withdrawn();
}

bool ordersShip(game::Ship ship, uint32_t playerPoliticalId) {
    return ship.kind() == ShipKind::Escort && ship.politicalId() != playerPoliticalId && inPosse(ship.politicalId());
}

size_t ledShips(std::span<game::Ship> out) {
    const game::Ship playerShip = game::playerShip();
    const uint32_t playerPoliticalId = game::playerPoliticalId();
    size_t count = 0;
    for (game::Ship ship : spaceShips())
        if (count < out.size() && ship && ship != playerShip && fights(ship) && ordersShip(ship, playerPoliticalId))
            out[count++] = ship;
    return count;
}

bool usableTurret(game::Turret turret) {
    return turret.alive() && turret.cast(Type::Turret) == turret.raw() && !turret.fighter().destroyed();
}

bool usableVehicle(game::Vehicle vehicle) {
    return vehicle.alive() && !vehicle.dead() && !vehicle.fighter().destroyed();
}

struct KindOfType {
    Type type;
    Kind kind;
};
constexpr std::array kindsOfTypes{KindOfType{Type::Ship, Kind::Ship}, KindOfType{Type::Turret, Kind::Turret},
                                  KindOfType{Type::Vehicle, Kind::Vehicle}};

Mark markOf(game::GameData object) {
    if (!object)
        return Mark{};
    const auto found = std::ranges::find_if(
        kindsOfTypes, [&](const KindOfType& entry) { return object.cast(entry.type) != nullptr; });
    return found != kindsOfTypes.end() ? Mark{object.identifier(), found->kind} : Mark{};
}

class ShotLog {
public:
    ShotLog(std::span<const Shot> remembered, uint64_t moment) : remembered(remembered), moment(moment) {}

    [[nodiscard]] bool shot(const Mark& shooter) const {
        return std::ranges::any_of(remembered, [&](const Shot& entry) { return recent(entry, shooter); });
    }
    [[nodiscard]] bool shot(const Mark& shooter, uint32_t victim) const {
        return std::ranges::any_of(remembered,
                                   [&](const Shot& entry) { return entry.victim == victim && recent(entry, shooter); });
    }

private:
    [[nodiscard]] bool recent(const Shot& entry, const Mark& shooter) const {
        return entry.shooter.kind != Kind::None && entry.shooter == shooter &&
               moment - entry.time <= settings.memoryMilliseconds;
    }

    std::span<const Shot> remembered;
    uint64_t moment;
};

struct Candidate {
    Mark mark;
    game::GameData object;
    game::Combatant fighter;
    uint32_t politicalId;
    game::Vector position;
};

Candidate candidateOf(const Mark& mark, game::GameData object, game::Combatant fighter) {
    return Candidate{mark, object, fighter, object.politicalId(), object.position()};
}

template <class Visit> void eachShip(Visit&& visit) {
    const game::Ship playerShip = game::playerShip();
    const uint32_t playerPoliticalId = game::playerPoliticalId();
    for (game::Ship ship : spaceShips())
        if (ship && ship != playerShip && ship.kind() != ShipKind::Escort && fights(ship) &&
            ship.politicalId() != playerPoliticalId)
            visit(Mark{ship.identifier(), Kind::Ship}, game::GameData(ship), ship.fighter());
}

template <class Visit> void eachTurret(Visit&& visit) {
    if (!game::onPlanet())
        return;
    const uint32_t playerPoliticalId = game::playerPoliticalId();
    for (game::City city : game::each<game::City>(map::civilization::cities))
        for (game::Turret turret : city.turrets())
            if (turret && usableTurret(turret) && turret.politicalId() != playerPoliticalId)
                visit(Mark{turret.identifier(), Kind::Turret}, game::GameData(turret), turret.fighter());
}

template <class Visit> void eachVehicle(Visit&& visit) {
    if (!game::onPlanet())
        return;
    const uint32_t playerPoliticalId = game::playerPoliticalId();
    for (game::Vehicle vehicle : game::each<game::Vehicle>(map::civilization::vehicles))
        if (vehicle && usableVehicle(vehicle) && vehicle.politicalId() != playerPoliticalId)
            visit(Mark{vehicle.identifier(), Kind::Vehicle}, game::GameData(vehicle), vehicle.fighter());
}

template <class Each> Candidate findAmong(uint32_t identifier, Each each) {
    Candidate found{};
    each([&](const Mark& mark, game::GameData object, game::Combatant fighter) {
        if (found.mark.kind == Kind::None && mark.identifier == identifier)
            found = candidateOf(mark, object, fighter);
    });
    return found;
}

Candidate findNothing(uint32_t) {
    return Candidate{};
}

Candidate findShip(uint32_t identifier) {
    return findAmong(identifier, [](auto&& visit) { eachShip(visit); });
}

Candidate findTurret(uint32_t identifier) {
    return findAmong(identifier, [](auto&& visit) { eachTurret(visit); });
}

Candidate findVehicle(uint32_t identifier) {
    return findAmong(identifier, [](auto&& visit) { eachVehicle(visit); });
}

using FindFunction = Candidate (*)(uint32_t identifier);
constexpr std::array<FindFunction, EscortBehaviour::kindCount> findByKind{findNothing, findShip, findTurret,
                                                                          findVehicle, findNothing};
static_assert(std::ranges::none_of(findByKind, [](FindFunction function) { return function == nullptr; }));

class Candidates {
public:
    void add(const Candidate& candidate) {
        if (count < entries.size())
            entries[count++] = candidate;
    }
    [[nodiscard]] std::span<const Candidate> all() const {
        return std::span(entries).first(count);
    }

private:
    std::array<Candidate, settings.maximumCandidates> entries{};
    size_t count = 0;
};

void gather(Candidates& candidates) {
    const auto add = [&](const Mark& mark, game::GameData object, game::Combatant fighter) {
        candidates.add(candidateOf(mark, object, fighter));
    };
    eachShip(add);
    eachTurret(add);
    eachVehicle(add);
}

bool sideCombatant(const char* combatant) {
    const game::Ship playerShip = game::playerShip();
    if (!combatant || !playerShip)
        return false;
    return combatant == playerShip.fighter().raw() || std::ranges::any_of(spaceShips(), [&](game::Ship ship) {
               return ship && ship.kind() == ShipKind::Escort && ship.fighter().raw() == combatant;
           });
}

struct Judge {
    game::Ship escortShip;
    ShotLog log;
    uint32_t playerPoliticalId;
};

Judge judgeOf(game::Ship escortShip, std::span<const Shot> shots, uint64_t moment) {
    return Judge{escortShip, ShotLog(shots, moment), game::playerPoliticalId()};
}

bool hostileShip(const Candidate& candidate, const Judge& judge) {
    const bool war = game::onPlanet() && (judge.escortShip.decisionFlags() & game::Ship::warOnPlanetFlag) != 0;
    return map::space::shipHostile(candidate.object.raw(), judge.escortShip.raw(), war) ||
           judge.log.shot(candidate.mark);
}

bool hostileGround(const Candidate& candidate, const Judge& judge) {
    return game::atWar(candidate.politicalId, judge.playerPoliticalId) || sideCombatant(candidate.fighter.target()) ||
           judge.log.shot(candidate.mark);
}

bool neverHostile(const Candidate&, const Judge&) {
    return false;
}

using HostileFunction = bool (*)(const Candidate&, const Judge&);
constexpr std::array<HostileFunction, EscortBehaviour::kindCount> hostileByKind{neverHostile, hostileShip,
                                                                                hostileGround, hostileGround,
                                                                                neverHostile};
static_assert(std::ranges::none_of(hostileByKind, [](HostileFunction function) { return function == nullptr; }));

bool hostile(const Candidate& candidate, const Judge& judge) {
    return hostileByKind[size_t(candidate.mark.kind)](candidate, judge);
}

void* resolve(const Mark& mark, const Judge& judge) {
    const Candidate found = findByKind[size_t(mark.kind)](mark.identifier);
    return found.mark.kind != Kind::None && hostile(found, judge) ? found.fighter.raw() : nullptr;
}

struct SideShip {
    game::Ship ship;
    uint32_t politicalId;
    game::Vector position;
};

SideShip sideShipOf(game::Ship ship) {
    return SideShip{ship, ship.politicalId(), ship.position()};
}

struct Fight {
    SideShip player;
    std::span<const SideShip> escort;
    std::span<const Candidate> candidates;
    ShotLog log;
    uint32_t playerPoliticalId;

    [[nodiscard]] Judge judge(const SideShip& escortShip) const {
        return Judge{escortShip.ship, log, playerPoliticalId};
    }
    [[nodiscard]] const SideShip* member(uint32_t politicalId) const {
        const auto found = std::ranges::find(escort, politicalId, &SideShip::politicalId);
        return found != escort.end() ? &*found : nullptr;
    }
};

struct Threat {
    const Candidate* candidate;
    float fromVictim, fromEscort;
};

bool before(const Threat& first, const Threat& second) {
    return first.fromVictim < second.fromVictim ||
           (first.fromVictim == second.fromVictim && first.fromEscort < second.fromEscort);
}

std::optional<Threat> nearestThreat(const Fight& fight, const SideShip& escortShip, const SideShip& victim) {
    const Judge judge = fight.judge(escortShip);
    const char* victimFighter = victim.ship.fighter().raw();
    const char* lastAttacker = victim.ship.lastShipAttacker();
    const uint32_t victimIdentifier = victim.ship.identifier();
    std::optional<Threat> nearest;
    for (const Candidate& candidate : fight.candidates) {
        const bool threatens = candidate.fighter.target() == victimFighter || candidate.object.raw() == lastAttacker ||
                               fight.log.shot(candidate.mark, victimIdentifier);
        if (!threatens || !hostile(candidate, judge))
            continue;
        const Threat threat{&candidate, game::distance(candidate.position, victim.position),
                            game::distance(candidate.position, escortShip.position)};
        if (!nearest || before(threat, *nearest))
            nearest = threat;
    }
    return nearest;
}

Mark targetOf(const std::optional<Threat>& threat) {
    return threat ? threat->candidate->mark : Mark{};
}

using Chooser = Mark (*)(const Fight& fight, const SideShip& escortShip, const Standing& standing);

Mark leaveToTheGame(const Fight&, const SideShip&, const Standing&) {
    return Mark{};
}

Mark protectPlayer(const Fight& fight, const SideShip& escortShip, const Standing&) {
    return targetOf(nearestThreat(fight, escortShip, fight.player));
}

Mark protectEscort(const Fight& fight, const SideShip& escortShip, const Standing& standing) {
    std::optional<Threat> nearest;
    bool guarding = false;
    const auto answer = [&](const SideShip& guarded) {
        guarding = true;
        const std::optional<Threat> threat = nearestThreat(fight, escortShip, guarded);
        if (threat && (!nearest || before(*threat, *nearest)))
            nearest = threat;
    };
    for (const SideShip& member : fight.escort)
        if (standing.guards(member.politicalId))
            answer(member);
    const bool gone = std::ranges::any_of(standing.guardedShips(),
                                          [&](uint32_t politicalId) { return !fight.member(politicalId); });
    if (standing.guardsAll || gone || !guarding)
        answer(fight.player);
    return targetOf(nearest);
}

Mark protectSelf(const Fight& fight, const SideShip& escortShip, const Standing&) {
    return targetOf(nearestThreat(fight, escortShip, escortShip));
}

float lookout(const SideShip& escortShip, float factor) {
    const float reach = std::max(game::Tool(escortShip.ship.airWeapon()).range(),
                                 game::Tool(escortShip.ship.groundWeapon()).range());
    if (reach > 0.0f)
        return reach * factor;
    const int context = int(game::context());
    return context >= 0 && size_t(context) < settings.fallbackRadius.size() ? settings.fallbackRadius[size_t(context)]
                                                                            : 0.0f;
}

float reachAt(const SideShip& escortShip, const Candidate& candidate) {
    const bool airborne = candidate.mark.kind == Kind::Ship ||
                          (candidate.mark.kind == Kind::Vehicle &&
                           game::Vehicle(candidate.object.raw()).locomotion() == map::identifiers::Locomotion::Air);
    return game::Tool(std::array{escortShip.ship.groundWeapon(), escortShip.ship.airWeapon()}[size_t(airborne)])
        .range();
}

bool roseReady = false;

const Candidate* aimedCandidate(const Fight& fight, const Judge& judge) {
    const game::PlayerShipManager manager = game::shipManager();
    const char* aimed = manager ? manager.aimed() : nullptr;
    if (!aimed)
        return nullptr;
    const auto found = std::ranges::find(fight.candidates, aimed, [](const Candidate& candidate) -> const char* {
        return candidate.fighter.raw();
    });
    return found != fight.candidates.end() && hostile(*found, judge) ? &*found : nullptr;
}

Mark attackNearby(const Fight& fight, const SideShip& escortShip, const Standing&) {
    const game::PlayerShipManager manager = game::shipManager();
    if (game::onPlanet() && manager && manager.attackedCity())
        return Mark{0, Kind::LeftToTheGame};
    const Judge judge = fight.judge(escortShip);
    if (const Candidate* aimed = aimedCandidate(fight, judge))
        return aimed->mark;
    std::optional<Threat> threat = nearestThreat(fight, escortShip, fight.player);
    for (const SideShip& member : fight.escort)
        if (const std::optional<Threat> other = nearestThreat(fight, escortShip, member);
            other && (!threat || before(*other, *threat)))
            threat = other;
    if (threat)
        return threat->candidate->mark;
    const float around = lookout(escortShip, settings.nearFactor);
    const Candidate* nearest = nullptr;
    float nearestDistance = 0.0f;
    bool nearestShip = false;
    for (const Candidate& candidate : fight.candidates) {
        const bool ship = candidate.mark.kind == Kind::Ship;
        const float distance = game::distance(candidate.position, fight.player.position);
        const bool reachable = ship || roseReady || game::distance(candidate.position, escortShip.position) <=
                                                        reachAt(escortShip, candidate) * settings.reachFactor;
        const bool better = !nearest || (ship && !nearestShip) || (ship == nearestShip && distance < nearestDistance);
        if (better && reachable && distance <= around && hostile(candidate, judge)) {
            nearest = &candidate;
            nearestDistance = distance;
            nearestShip = ship;
        }
    }
    return nearest ? nearest->mark : Mark{};
}

constexpr PerOrder<Chooser> choosers{leaveToTheGame, protectPlayer, protectEscort, protectSelf, attackNearby};
static_assert(std::ranges::none_of(choosers, [](Chooser chooser) { return chooser == nullptr; }));
}
}

namespace feature {
bool EscortBehaviour::ordersGiven(const OrderBook& book) {
    if (knownRevision != book.revision()) {
        knownRevision = book.revision();
        ordered = std::ranges::any_of(book.standings(),
                                      [](const Standing& standing) { return standing.order != Order::Free; });
        decisionDue = true;
    }
    return ordered;
}

const EscortBehaviour::Aim* EscortBehaviour::aimOf(uint32_t politicalId) const {
    const std::span<const Aim> kept = std::span(aims).first(aimCount);
    const auto found = std::ranges::find(kept, politicalId, &Aim::politicalId);
    return found != kept.end() ? &*found : nullptr;
}

void EscortBehaviour::update(const OrderBook& book, uint32_t elapsedLowerWord) {
    if (!game::inSpace() || !game::playerShip()) {
        marks.clear();
        return;
    }
    const bool paused = game::paused();
    gameTime += paused ? 0u : std::min(elapsedLowerWord, settings.longestFrameMilliseconds);
    if (!ordersGiven(book) && !aimCount) {
        marks.clear();
        releaseChases();
        releaseRoses();
        return;
    }
    if (decisionDue || gameTime >= nextDecision) {
        decisionDue = false;
        nextDecision = gameTime + settings.decisionMilliseconds;
        decide(book);
    }
    apply();
}

void EscortBehaviour::decide(const OrderBook& book) {
    std::array<game::Ship, maximumEscort> led{};
    const size_t escortCount = behaviour::ledShips(led);
    std::array<behaviour::SideShip, maximumEscort> escort{};
    std::ranges::transform(std::span(led).first(escortCount), escort.begin(), behaviour::sideShipOf);
    behaviour::Candidates candidates;
    behaviour::gather(candidates);
    const behaviour::Fight fight{
        .player = behaviour::sideShipOf(game::playerShip()),
        .escort = std::span<const behaviour::SideShip>(escort).first(escortCount),
        .candidates = candidates.all(),
        .log = behaviour::ShotLog(shots, gameTime),
        .playerPoliticalId = game::playerPoliticalId(),
    };
    const auto steadyTarget = [&](const Aim& aim, const behaviour::SideShip& member) {
        const Aim* was = aimOf(aim.politicalId);
        const auto real = [](const Mark& mark) { return mark.kind != Kind::None && mark.kind != Kind::LeftToTheGame; };
        if (aim.order != Order::AttackNearby || !was || was->order != aim.order || was->target == aim.target ||
            !real(aim.target) || !real(was->target))
            return aim.target;
        const behaviour::Judge judge = fight.judge(member);
        const behaviour::Candidate* aimed = behaviour::aimedCandidate(fight, judge);
        const behaviour::Candidate old = behaviour::findByKind[size_t(was->target.kind)](was->target.identifier);
        const behaviour::Candidate now = behaviour::findByKind[size_t(aim.target.kind)](aim.target.identifier);
        if ((aimed && aimed->mark == aim.target) || old.mark.kind == Kind::None || now.mark.kind == Kind::None ||
            !behaviour::hostile(old, judge))
            return aim.target;
        const float oldDistance = game::distance(old.position, member.position);
        const float newDistance = game::distance(now.position, member.position);
        return newDistance >= oldDistance * settings.switchShare ? was->target : aim.target;
    };
    std::array<Aim, maximumEscort> chosen{};
    size_t chosenCount = 0;
    for (const behaviour::SideShip& member : fight.escort) {
        const Standing standing = book.standingOf(member.politicalId);
        if (standing.order == Order::Free)
            continue;
        Aim aim{member.politicalId, standing.order,
                behaviour::choosers[size_t(standing.order)](fight, member, standing)};
        aim.target = steadyTarget(aim, member);
        chosen[chosenCount++] = aim;
    }
    const std::span<const Aim> kept = std::span(chosen).first(chosenCount);
    for (const Aim& was : std::span(aims).first(aimCount))
        if (const behaviour::SideShip* member = fight.member(was.politicalId);
            member && std::ranges::find(kept, was.politicalId, &Aim::politicalId) == kept.end())
            member->ship.fighter().setTarget(nullptr);
    aims = chosen;
    aimCount = chosenCount;
}

void EscortBehaviour::apply() {
    std::array<game::Ship, maximumEscort> led{};
    const std::span<game::Ship> ships = std::span(led).first(behaviour::ledShips(led));
    const bool planet = game::onPlanet();
    if (planet)
        releaseChases();
    std::array<uint32_t, maximumEscort> going{};
    size_t goingCount = 0;
    marks.begin();
    for (const game::Ship ship : ships) {
        const Aim* aim = aimOf(ship.politicalId());
        void* target = aim ? behaviour::resolve(aim->target, behaviour::judgeOf(ship, shots, gameTime)) : nullptr;
        if (aim && aim->target.kind != Kind::LeftToTheGame)
            ship.fighter().setTarget(target);
        if (planet)
            leadRose(ship, aim, target);
        if (!aim || aim->order != Order::AttackNearby || !target)
            continue;
        marks.show(ship.politicalId(), target);
        if (planet || aim->target.kind != Kind::Ship)
            continue;
        follow(ship, game::Ship::ofFighter(target));
        going[goingCount++] = ship.politicalId();
    }
    marks.end();
    const std::span<const uint32_t> goingNow = std::span(going).first(goingCount);
    for (const game::Ship ship : ships) {
        const std::span<const uint32_t> listed = std::span(chasing).first(chasingCount);
        const uint32_t politicalId = ship.politicalId();
        if (std::ranges::find(listed, politicalId) != listed.end() &&
            std::ranges::find(goingNow, politicalId) == goingNow.end())
            follow(ship, game::playerShip());
    }
}

void EscortBehaviour::releaseChases() {
    if (!chasingCount)
        return;
    std::array<game::Ship, maximumEscort> led{};
    for (const game::Ship ship : std::span(led).first(behaviour::ledShips(led))) {
        const std::span<const uint32_t> listed = std::span(chasing).first(chasingCount);
        if (std::ranges::find(listed, ship.politicalId()) != listed.end())
            follow(ship, game::playerShip());
    }
    chasingCount = 0;
}

void EscortBehaviour::follow(game::Ship ship, game::Ship leader) {
    char*& field = ship.leader();
    const std::span<uint32_t> listed = std::span(chasing).first(chasingCount);
    const bool chases = leader != game::playerShip();
    chasingCount -= std::ranges::remove(listed, ship.politicalId()).size();
    if (chases && chasingCount < chasing.size())
        chasing[chasingCount++] = ship.politicalId();
    if (field == leader.raw())
        return;
    const game::GameData before(field);
    leader.addReference();
    field = leader.raw();
    before.release();
}

std::optional<void*> EscortBehaviour::orderedTarget(const OrderBook& book, void* combatant) {
    if (!combatant || !ordersGiven(book))
        return std::nullopt;
    const game::Ship ship = game::Ship::ofFighter(combatant);
    if (ship.kind() != map::identifiers::ShipKind::Escort)
        return std::nullopt;
    const uint32_t politicalId = ship.politicalId();
    if (politicalId == game::playerPoliticalId() || book.orderOf(politicalId) == Order::Free || !game::inSpace() ||
        !behaviour::inPosse(politicalId))
        return std::nullopt;
    const Aim* aim = aimOf(politicalId);
    if (aim && aim->target.kind == Kind::LeftToTheGame)
        return std::nullopt;
    return aim ? behaviour::resolve(aim->target, behaviour::judgeOf(ship, shots, gameTime)) : nullptr;
}

void EscortBehaviour::onHostile(void* combatant, void* attacker) {
    if (!combatant || !attacker)
        return;
    const game::Ship victim = game::Ship::ofFighter(combatant);
    if (victim.kind() != map::identifiers::ShipKind::Escort && victim != game::playerShip())
        return;
    const Mark shooter = behaviour::markOf(game::Combatant(attacker).owner());
    if (shooter.kind == Kind::None)
        return;
    const uint32_t victimIdentifier = victim.identifier();
    const auto same = std::ranges::find_if(
        shots, [&](const Shot& shot) { return shot.victim == victimIdentifier && shot.shooter == shooter; });
    if (same != shots.end()) {
        same->time = gameTime;
        return;
    }
    shots[nextShot] = Shot{victimIdentifier, shooter, gameTime};
    nextShot = (nextShot + 1) % shots.size();
}

void EscortBehaviour::reset() {
    marks.clear();
    chasingCount = 0;
    roseCount = 0;
    shots.fill(Shot{});
    nextShot = 0;
    aimCount = 0;
    gameTime = nextDecision = 0;
    decisionDue = true;
}

namespace {
EscortBehaviour* roseLeader = nullptr;
std::array<void*, 4> escortRose{};
constexpr std::array<uint8_t, 4> roseGateJump{0x14, 0x00, 0x00, 0x00};
constexpr uint32_t roseGateOffset = 2;

bool __cdecl roseTickHook(void* ship, uint32_t lowTime, uint32_t highTime, uint32_t flags, uint32_t parameter,
                          float* state, void* selection, float elapsed) {
    if (roseLeader)
        roseLeader->onRoseTick(ship, state);
    return map::space::roseTick(ship, lowTime, highTime, flags, parameter, state, selection, elapsed);
}

void ownPace(game::Ship ship) {
    ship.speedFactor() = 1.0f;
    ship.accelerationFactor() = 1.0f;
}
}

bool EscortBehaviour::prepareRose() {
    roseLeader = this;
    void** record = map::space::roseBehaviour.at<void**>();
    const bool known = *map::space::escortRoseBehaviour == record && map::space::roseBehaviour.matches() &&
                       map::space::roseEscortGate.matches();
    if (!known)
        return false;
    escortRose = {record[0], record[1], std::bit_cast<void*>(&roseTickHook), record[3]};
    if (!hooks::patch(map::space::roseEscortGate, roseGateOffset, roseGateJump))
        return false;
    *map::space::escortRoseBehaviour = escortRose.data();
    behaviour::roseReady = true;
    return true;
}

EscortBehaviour::Rose* EscortBehaviour::roseOf(uint32_t politicalId) {
    const std::span<Rose> kept = std::span(roses).first(roseCount);
    const auto found = std::ranges::find(kept, politicalId, &Rose::politicalId);
    return found != kept.end() ? &*found : nullptr;
}

void EscortBehaviour::leadRose(game::Ship ship, const Aim* aim, void* target) {
    const uint32_t politicalId = ship.politicalId();
    Rose* rose = roseOf(politicalId);
    char* current = ship.aiTarget();
    if (rose && (current != rose->object || game::GameData(current).identifier() != rose->identifier)) {
        endRose(ship, false);
        rose = nullptr;
    }
    const game::GameData object = behaviour::roseReady && aim && aim->order != Order::Free && target
                                      ? game::Combatant(target).owner()
                                      : game::GameData();
    if (!object || !object.alive()) {
        if (rose)
            endRose(ship, true);
        return;
    }
    if (current != object.raw())
        map::space::shipSetAiTarget(ship.raw(), object.raw());
    const bool airborne = aim->target.kind == Kind::Ship ||
                          (aim->target.kind == Kind::Vehicle &&
                           game::Vehicle(object.raw()).locomotion() == map::identifiers::Locomotion::Air);
    const Rose led{politicalId, object.identifier(), object.raw(), airborne};
    if (rose) {
        *rose = led;
        return;
    }
    if (roseCount < roses.size())
        roses[roseCount++] = led;
}

void EscortBehaviour::endRose(game::Ship ship, bool clear) {
    const Rose* rose = roseOf(ship.politicalId());
    if (!rose)
        return;
    if (clear && ship.aiTarget() == rose->object)
        map::space::shipClearAiTarget(ship.raw());
    ownPace(ship);
    const std::span<Rose> kept = std::span(roses).first(roseCount);
    roseCount -= std::ranges::remove(kept, ship.politicalId(), &Rose::politicalId).size();
}

void EscortBehaviour::releaseRoses() {
    if (!roseCount)
        return;
    std::array<game::Ship, maximumEscort> led{};
    for (const game::Ship ship : std::span(led).first(behaviour::ledShips(led)))
        endRose(ship, true);
    roseCount = 0;
}

void EscortBehaviour::onLeavePlanet() {
    releaseRoses();
}

void EscortBehaviour::onRoseTick(void* shipObject, float* state) {
    if (!shipObject || !state || !roseCount)
        return;
    const game::Ship ship(shipObject);
    if (ship.kind() != map::identifiers::ShipKind::Escort)
        return;
    const Rose* rose = roseOf(ship.politicalId());
    if (!rose || ship.aiTarget() != rose->object)
        return;
    const game::GameData target(rose->object);
    const float reach = game::Tool(ship.airWeapon()).range() * settings.airRoseShare;
    const float airRadius = std::clamp(reach, settings.leastRoseRadius, settings.roseRadius);
    state[2] = std::array{settings.roseRadius, airRadius}[size_t(rose->airborne)];
    ownPace(ship);
    if (rose->airborne) {
        ship.flightRadius() = game::length(target.position()) + settings.airLift;
        const bool behind = game::distance(target.position(), ship.position()) > settings.hurryDistance * airRadius;
        ship.speedFactor() = std::array{1.0f, settings.hurrySpeed}[size_t(behind)];
        ship.accelerationFactor() = settings.hurryAcceleration;
    }
    if (state[0] == 0.0f) {
        const std::span<const Rose> kept = std::span(roses).first(roseCount);
        const auto same = [rose](const Rose& other) { return other.object == rose->object; };
        const size_t place = size_t(std::ranges::count_if(kept.first(size_t(rose - kept.data())), same));
        const size_t sharing = size_t(std::ranges::count_if(kept, same));
        state[0] = float(place) * 2.2f * state[1] / float(std::max<size_t>(sharing, 1));
    }
}
}

