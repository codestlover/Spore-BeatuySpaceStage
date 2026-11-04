
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <Windows.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <optional>
#include <span>

module beatuy.feature.tribe;

import beatuy.core.mod;
import beatuy.game;

namespace feature {
namespace {
struct Pace {
    float forward, lateral;
};
constexpr Pace chasePace{.forward = 1.0f, .lateral = 0.0f}, aroundPace{.forward = 0.5f, .lateral = 0.8f},
    backPace{.forward = -0.3f, .lateral = 0.8f};

constexpr auto axes = std::to_array<game::Vector>({{1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}});

auto inUse(auto& slots, int count) {
    return std::span(slots).first(size_t(count));
}

float hullRadius(game::Ship ship) {
    float radius = ship.body().boundingRadius();
    return radius > 1.0f && radius < 80.0f ? radius : 10.0f;
}
}

bool Tribe::spearman(tribe::Citizen citizen) {
    return citizen.toolKind() == settings.spearTool || citizen.heldItem() == settings.spearItem;
}

bool Tribe::ownsSpears(tribe::Village village) {
    return village.toolKinds() >> settings.spearTool & 1;
}

bool Tribe::disarmed(tribe::Village village) const {
    auto villages = inUse(disarmedTribes, disarmedCount);
    return std::ranges::find(villages, village) != villages.end();
}

Tribe::Ships Tribe::gather() {
    Ships out;
    game::Ship ship = game::playerShip();
    if (ship.alive())
        out.ship[out.count++] = ship.raw();
    out.count += Mod::get().escort.ships(out.ship.data() + out.count, settings.maximumTargets - out.count);
    for (int i = 0; i < out.count; ++i)
        out.at[i] = game::GameData(out.ship[i]).position();
    return out;
}

int Tribe::nearest(const game::Vector& from, float reach) const {
    int best = -1;
    float gap = reach;
    for (int i = 0; i < targets.count; ++i) {
        float distance = game::distance(from, targets.at[i]);
        if (distance <= gap) {
            gap = distance;
            best = i;
        }
    }
    return best;
}

float Tribe::closest(const game::Vector& from) const {
    float gap = 1e30f;
    for (const game::Vector& at : inUse(targets.at, targets.count))
        gap = std::min(gap, game::surfaceDistance(from, at));
    return gap;
}

bool Tribe::equip(tribe::Citizen citizen, int item) {
    if (citizen.grasping() < 0)
        return false;
    if (item == settings.spearItem && !map::tribe::tribeToolData[settings.spearTool])
        return false;
    citizen.setItem(item);
    return true;
}

void Tribe::arm(tribe::Village village) {
    if (disarmed(village))
        return;
    game::Array<tribe::Citizen*> members = village.members();
    if (!members.data())
        return;
    std::array<tribe::Citizen, 64> pool;
    const int room = int(pool.size());
    int count = 0;
    for (tribe::Citizen member : members) {
        if (count >= room)
            break;
        if (member.living() && spearman(member))
            pool[count++] = member;
    }
    bool rack = !count && ownsSpears(village);
    for (tribe::Citizen member : members) {
        if (!member.living() || member.ageStage() < 1)
            continue;
        if (rack && count < room)
            pool[count++] = member;
    }
    if (!count)
        return;
    int take = std::clamp((count * settings.defenderShare + 99) / 100, 1, settings.maximumPerTribe);
    DWORD now = GetTickCount();
    for (int i = 0; i < take && defenderCount < settings.maximumDefenders; ++i) {
        int chosen = i + rand() % (count - i);
        tribe::Citizen citizen = pool[chosen];
        pool[chosen] = pool[i];
        int item = citizen.heldItem();
        citizen.legs().stop();
        bool spear = item == settings.spearItem || equip(citizen, settings.spearItem);
        if (spear && item != settings.spearItem)
            citizen.play(settings.grabAnimation, citizen.grasping());
        citizen.addReference();
        defenders[defenderCount++] = Defender{
            .citizen = citizen,
            .village = village,
            .nextThrow = now + settings.grabMilliseconds + DWORD(rand() % settings.throwJitterMilliseconds),
            .item = item,
        };
    }
}

bool Tribe::throwSpear(tribe::Citizen citizen, game::Ship ship, DWORD now) {
    void* nouns = map::application::nounManager();
    char* world = map::application::planetModel();
    const game::Vector* at = citizen.where();
    if (!nouns || !world || !at || flightCount >= settings.maximumFlights)
        return false;
    float centreDistance = game::length(*at);
    float lift = citizen.place().boundingRadius();
    lift = lift > 0.5f && lift < 20.0f ? lift : 1.5f;
    game::Vector hand = centreDistance > 1e-3f ? *at * (1 + lift / centreDistance) : *at;
    game::Vector target = ship.position(), velocity{0.0f, 0.0f, 0.0f}, still{0.0f, 0.0f, 0.0f};
    float gravity = map::planet::gravity(world);
    float distance = game::distance(hand, target);
    float speed = std::sqrt(std::fabs(gravity) * distance * settings.lob);
    bool reach = speed > 0.0f && map::tribe::ballisticAim(&hand, &target, speed, &velocity, &still) & 0xff;
    tribe::Spear spear = reach ? tribe::Spear::create(nouns) : tribe::Spear();
    if (!spear)
        return false;
    spear.initialize(settings.spearDamage, citizen.fighter().raw());
    spear.body().setPosition(hand);
    spear.setPoliticalId(citizen.askPoliticalId());
    spear.setTarget(ship.fighter());
    spear.damage() = 0.0f;
    spear.launch(velocity);
    spear.body().setScale(1.0f);
    float velocityLength = game::length(velocity);
    game::Vector heading = velocityLength > 1e-3f ? game::dividedBy(velocity, velocityLength) : game::Vector{0, 0, 0};
    spear.addReference();
    citizen.addReference();
    ship.addReference();
    flights[flightCount++] = Flight{
        .spear = spear,
        .thrower = citizen,
        .ship = ship,
        .politicalId = citizen.askPoliticalId(),
        .expires = now + settings.flightMilliseconds,
        .heading = heading,
    };
    return true;
}

void Tribe::land(int index) {
    flights[index].spear.release();
    flights[index].thrower.release();
    flights[index].ship.release();
    flights[index] = flights[--flightCount];
}

int Tribe::flightOf(tribe::Spear spear) const {
    for (int i = 0; i < flightCount; ++i)
        if (flights[i].spear == spear)
            return i;
    return -1;
}

void Tribe::unstick(int index) {
    tribe::Spear spear = stuck[index].spear;
    if (spear.alive()) {
        game::Vector core{0.0f, 0.0f, 0.0f};
        spear.body().setPosition(core);
    }
    spear.release();
    stuck[index].ship.release();
    stuck[index] = stuck[--stuckCount];
}

void Tribe::pin(const Stuck& lodged) {
    game::Spatial hull = lodged.ship.body();
    const game::Vector* at = hull.position();
    if (!at)
        return;
    tribe::Quaternion hullTurn = tribe::Quaternion::of(hull);
    game::Vector position = *at + hullTurn.rotate(lodged.offset);
    tribe::Quaternion turn = hullTurn * lodged.turn;
    lodged.spear.body().setPosition(position);
    lodged.spear.body().setOrientation(turn.data());
}

float Tribe::frontOf(tribe::Spear spear, const tribe::Quaternion& turn, const game::Vector& heading) {
    tribe::Model model(spear.body().model());
    const float* box = model ? model.bounds() : spear.body().extents();
    int axis = 0;
    float along = 0.0f;
    for (int i = 0; i < 3; ++i) {
        float alignment = game::dot(turn.rotate(axes[i]), heading);
        if (std::fabs(alignment) > std::fabs(along)) {
            along = alignment;
            axis = i;
        }
    }
    float front = along >= 0.0f ? box[3 + axis] : -box[axis];
    float length = box[3 + axis] - box[axis];
    return std::isfinite(front) && length > 0.1f && length < 50.0f && front > 0.0f ? front : settings.defaultFront;
}

std::optional<tribe::Mesh> Tribe::pickable(game::Ship ship) {
    game::Spatial hull = ship.body();
    const tribe::Mesh mesh{.model = hull.model(), .world = tribe::ModelWorld(hull.modelWorld())};
    if (!mesh.model || !mesh.world)
        return std::nullopt;
    if (mesh.world.pickFunction() == map::tribe::pickAgainstModel.at())
        return mesh;
    return std::nullopt;
}

std::optional<float> Tribe::pick(const tribe::Mesh& mesh, const game::Vector& from, const game::Vector& to) {
    tribe::ModelFilter filter{0, 0, nullptr, settings.meshCluster, 0};
    float share = 1.0f;
    if (mesh.world.pick(from, to, mesh.model, share, filter) && share >= 0.0f && share <= 1.0f)
        return share;
    return std::nullopt;
}

void Tribe::strike(game::Ship ship, const Flight& flight) {
    game::Vector heading = flight.heading;
    ship.fighter().takeDamage(settings.spearHit, flight.politicalId, settings.spearDamage, &heading,
                              flight.thrower.fighter().raw());
}

game::Ship Tribe::hullAt(const game::Vector& point) {
    const Ships fleet = gather();
    game::Ship best;
    float share = 1e30f;
    for (int i = 0; i < fleet.count; ++i) {
        game::Ship ship(fleet.ship[i]);
        float radius = hullRadius(ship), distance = game::distance(point, fleet.at[i]);
        if (distance <= radius * settings.hullSlack + settings.hullMargin && distance / radius < share) {
            share = distance / radius;
            best = ship;
        }
    }
    return best;
}

void Tribe::makeRoom(game::Ship ship) {
    int onHull = 0, hullOldest = -1, oldest = -1;
    for (int i = 0; i < stuckCount; ++i) {
        if (oldest < 0 || LONG(stuck[i].since - stuck[oldest].since) < 0)
            oldest = i;
        if (stuck[i].ship != ship)
            continue;
        ++onHull;
        if (hullOldest < 0 || LONG(stuck[i].since - stuck[hullOldest].since) < 0)
            hullOldest = i;
    }
    struct Limit {
        int count, limit, oldest;
    };
    const std::array limits{Limit{onHull, settings.maximumPerHull, hullOldest},
                            Limit{stuckCount, settings.maximumStuck, oldest}};
    auto full = std::ranges::find_if(limits, [](const Limit& limit) { return limit.count >= limit.limit; });
    if (full != limits.end())
        unstick(full->oldest);
}

void Tribe::stick(int index, game::Ship ship, const game::Vector& point) {
    Flight flight = flights[index];
    flights[index] = flights[--flightCount];
    makeRoom(ship);
    tribe::Spear spear = flight.spear;
    spear.motion().setLocomotion(nullptr);
    tribe::Quaternion spearTurn = tribe::Quaternion::of(spear.body());
    float front = frontOf(spear, spearTurn, flight.heading);
    game::Vector rest = point + flight.heading * (settings.embed - front);
    game::Spatial hull = ship.body();
    const game::Vector* at = hull.position();
    tribe::Quaternion hullTurn = tribe::Quaternion::of(hull);
    ship.addReference();
    stuck[stuckCount++] = Stuck{
        .spear = spear,
        .ship = ship,
        .offset = at ? hullTurn.conjugate().rotate(rest - *at) : game::Vector{0, 0, 0},
        .turn = hullTurn.conjugate() * spearTurn,
        .since = GetTickCount(),
    };
    pin(stuck[stuckCount - 1]);
    strike(ship, flight);
    flight.thrower.release();
    flight.ship.release();
}

int Tribe::fly(int index, const game::Vector& from, const game::Vector& to) {
    const Ships fleet = gather();
    float step = game::distance(from, to);
    bool close = false;
    game::Ship best;
    float first = 2.0f;
    game::Vector start = from;
    for (int i = 0; i < fleet.count; ++i) {
        game::Ship ship(fleet.ship[i]);
        float reach = hullRadius(ship) * settings.hullSlack + settings.hullMargin + step;
        if (game::distance(to, fleet.at[i]) > reach)
            continue;
        std::optional<tribe::Mesh> mesh = pickable(ship);
        if (!mesh)
            continue;
        close = true;
        game::Vector moved{0.0f, 0.0f, 0.0f};
        for (int past = 0; past < passed.count; ++past)
            if (passed.ship[past] == fleet.ship[i])
                moved = fleet.at[i] - passed.at[past];
        game::Vector back = game::length(moved) < settings.hullMargin * 4.0f ? from + moved : from;
        if (std::optional<float> share = pick(*mesh, back, to); share && *share < first) {
            first = *share;
            best = ship;
            start = back;
        }
    }
    if (best) {
        stick(index, best, start + (to - start) * first);
        return 1;
    }
    if (!close)
        return -1;
    tribe::Spear spear = flights[index].spear;
    const game::Vector* position = spear.body().position();
    if (!position || game::length(*position) <= game::length(game::ground(*position)) + 0.5f)
        return -1;
    spear.lastPosition() = *position;
    return 1;
}

void Tribe::maneuver(const Defender& defender) {
    const game::Vector* at = defender.citizen.where();
    const game::Vector* home = defender.village.alive() ? defender.village.place().position() : nullptr;
    float centreDistance = at ? game::length(*at) : 0.0f;
    int target = at ? nearest(*at, 1e30f) : -1;
    if (!at || !home || centreDistance < 1.0f || target < 0)
        return;
    game::Vector ship = targets.at[target];
    bool distant = game::distance(*at, ship) > settings.throwRange;
    if (!distant && rand() % 100 < settings.tauntChance) {
        defender.citizen.face(ship, settings.turnSpeed);
        defender.citizen.play(settings.taunts[rand() % settings.taunts.size()]);
        return;
    }
    game::Vector to = game::ground(ship) - *at;
    float gap = game::length(to);
    if (gap < 1e-3f)
        return;
    game::Vector ahead = game::dividedBy(to, gap), up = game::dividedBy(*at, centreDistance);
    game::Vector side = game::cross(up, ahead);
    float step = settings.step + settings.stepJitter * float(rand() % 100) / 100.0f;
    const Pace& pace = distant ? chasePace : gap > settings.closeIn ? aroundPace : backPace;
    float forward = step * pace.forward;
    float lateral = pace.lateral != 0.0f ? (rand() & 1 ? step : -step) * pace.lateral : 0.0f;
    game::Vector spot = *at + ahead * forward + side * lateral;
    game::Vector away = spot - *home;
    float fromHome = game::length(away);
    if (fromHome > settings.home)
        spot = *home + game::dividedBy(away * settings.home, fromHome);
    spot = game::ground(spot);
    defender.citizen.walkTo(settings.runSpeed, spot, ahead);
}

void Tribe::drop(int index, bool restore) {
    Defender& defender = defenders[index];
    if (restore && defender.citizen.living() && defender.citizen.heldItem() != defender.item)
        equip(defender.citizen, defender.item);
    defender.citizen.release();
    defenders[index] = defenders[--defenderCount];
}

void Tribe::disarm(tribe::Village village) {
    for (int i = defenderCount - 1; i >= 0; --i) {
        tribe::Citizen citizen = defenders[i].citizen;
        if (defenders[i].village != village)
            continue;
        if (citizen.living()) {
            if (citizen.heldItem())
                equip(citizen, 0);
            citizen.play(settings.dropPanicAnimation);
        }
        drop(i, false);
    }
    if (disarmedCount < settings.maximumTribes)
        disarmedTribes[disarmedCount++] = village;
}

void Tribe::standDown(tribe::Village village) {
    for (int i = defenderCount - 1; i >= 0; --i)
        if (defenders[i].village == village)
            drop(i, true);
}

void Tribe::reset() {
    while (defenderCount)
        drop(defenderCount - 1, false);
    while (flightCount)
        land(flightCount - 1);
    for (const Alert& alert : inUse(alerts, alertCount))
        alert.village.release();
    while (stuckCount)
        unstick(stuckCount - 1);
    alertCount = disarmedCount = 0;
}

void Tribe::watch(game::Array<tribe::Village*> villages, DWORD now) {
    for (tribe::Village village : villages) {
        const game::Vector* at = village.alive() ? village.place().position() : nullptr;
        if (!at)
            continue;
        if (closest(*at) > settings.alarm)
            continue;
        auto active = inUse(alerts, alertCount);
        if (std::ranges::find(active, village, &Alert::village) != active.end() || alertCount >= settings.maximumTribes)
            continue;
        village.addReference();
        alerts[alertCount++] = Alert{
            .village = village,
            .calmSince = now,
            .calm = false,
            .rack = ownsSpears(village),
        };
        arm(village);
    }
    for (int i = alertCount - 1; i >= 0; --i) {
        Alert& alert = alerts[i];
        const game::Vector* at = alert.village.alive() ? alert.village.place().position() : nullptr;
        if (at && alert.rack && !ownsSpears(alert.village)) {
            alert.rack = false;
            disarm(alert.village);
        }
        if (at && closest(*at) <= settings.calm) {
            alert.calm = false;
            continue;
        }
        if (at && !alert.calm) {
            alert.calm = true;
            alert.calmSince = now;
        }
        if (at && LONG(now - alert.calmSince) < LONG(settings.calmMilliseconds))
            continue;
        standDown(alert.village);
        alert.village.release();
        alerts[i] = alerts[--alertCount];
    }
}

void Tribe::defend(int index, DWORD now) {
    Defender& defender = defenders[index];
    if (!defender.citizen.living()) {
        drop(index, false);
        return;
    }
    if (defender.regrip && LONG(now - defender.regrip) >= 0) {
        defender.regrip = 0;
        if (equip(defender.citizen, settings.spearItem))
            defender.citizen.play(settings.spearGetAnimation, defender.citizen.grasping());
    }
    const game::Vector* at = defender.citizen.where();
    if (defender.armed && LONG(now - defender.release) >= 0) {
        loose(defender, at, now);
        return;
    }
    if (!defender.armed && defender.maneuver && LONG(now - defender.maneuver) >= 0) {
        defender.maneuver = 0;
        maneuver(defender);
    }
    if (defender.armed || LONG(now - defender.nextThrow) < 0 || !at)
        return;
    windUp(defender, *at, now);
}

void Tribe::loose(Defender& defender, const game::Vector* at, DWORD now) {
    defender.armed = false;
    int target = at ? nearest(*at, settings.throwRange) : -1;
    if (target >= 0 && throwSpear(defender.citizen, game::Ship(targets.ship[target]), now) &&
        defender.citizen.heldItem() == settings.spearItem && equip(defender.citizen, 0))
        defender.regrip = now + settings.regripMilliseconds;
    defender.maneuver = now + (defender.regrip ? settings.regripMilliseconds + settings.spearGetMilliseconds
                                               : settings.recoverMilliseconds);
}

void Tribe::windUp(Defender& defender, const game::Vector& at, DWORD now) {
    defender.nextThrow = now + settings.throwMilliseconds + DWORD(rand() % settings.throwJitterMilliseconds);
    int target = nearest(at, settings.throwRange);
    int sighted = target >= 0 ? target : nearest(at, 1e30f);
    defender.citizen.legs().stop();
    if (sighted >= 0)
        defender.citizen.face(targets.at[sighted], settings.turnSpeed);
    if (target < 0) {
        defender.maneuver = now + settings.advanceMilliseconds;
        return;
    }
    defender.citizen.play(settings.throwAnimation);
    defender.release = now + settings.releaseMilliseconds;
    defender.armed = true;
}

bool Tribe::holds(void* creature) {
    for (const Defender& defender : inUse(defenders, defenderCount))
        if (defender.citizen.raw() == creature)
            return true;
    return false;
}

int Tribe::spearTick(void* projectile) {
    tribe::Spear spear = tribe::Spear::ofProjectile(projectile);
    for (int i = 0; i < stuckCount; ++i) {
        Stuck& lodged = stuck[i];
        if (lodged.spear != spear)
            continue;
        if (!game::onPlanet() || !lodged.ship.alive()) {
            unstick(i);
            return 0;
        }
        pin(lodged);
        return 1;
    }
    int flightIndex = flightOf(spear);
    if (flightIndex < 0) {
        bool still = !spear.motion().locomotion();
        return still && game::inSpace() ? 0 : -1;
    }
    const game::Vector* position = spear.body().position();
    if (!position || !game::onPlanet())
        return -1;
    game::Vector last = spear.lastPosition();
    game::Vector step = *position - last;
    float stepLength = game::length(step);
    if (stepLength > 1e-3f)
        flights[flightIndex].heading = game::dividedBy(step, stepLength);
    float front = frontOf(spear, tribe::Quaternion::of(spear.body()), flights[flightIndex].heading);
    return fly(flightIndex, last + flights[flightIndex].heading * front,
               *position + flights[flightIndex].heading * front);
}

bool Tribe::spearAfter(void* projectile, bool alive) {
    tribe::Spear spear = tribe::Spear::ofProjectile(projectile);
    int flightIndex = alive ? -1 : flightOf(spear);
    if (flightIndex < 0)
        return alive;
    const game::Vector* position = game::onPlanet() ? spear.body().position() : nullptr;
    game::Ship ship = position && !spear.target() ? hullAt(*position) : game::Ship();
    if (std::optional<tribe::Mesh> mesh = ship ? pickable(ship) : std::nullopt) {
        const game::Vector& heading = flights[flightIndex].heading;
        float radius = hullRadius(ship);
        game::Vector from = *position + heading * -radius, to = *position + heading * (2.0f * radius);
        if (std::optional<float> share = pick(*mesh, from, to)) {
            stick(flightIndex, ship, from + (to - from) * *share);
            return true;
        }
        ship = game::Ship();
    }
    if (!ship) {
        land(flightIndex);
        return false;
    }
    stick(flightIndex, ship, *position);
    return true;
}

void Tribe::tick() {
    DWORD now = GetTickCount();
    game::Planet here = game::onPlanet() && game::planetTechnology() == map::identifiers::Technology::Tribe
                            ? game::activePlanet()
                            : game::Planet();
    if (here != planet) {
        reset();
        planet = here;
    }
    game::Ship ship = game::playerShip();
    if (!planet || !ship)
        return;
    targets = gather();
    if (LONG(now - nextScan) >= 0) {
        nextScan = now + settings.scanMilliseconds;
        if (game::Array<tribe::Village*> villages = game::each<tribe::Village>(map::tribe::tribes); villages.data())
            watch(villages, now);
    }
    for (int i = stuckCount - 1; i >= 0; --i)
        if (stuck[i].ship.alive())
            pin(stuck[i]);
        else
            unstick(i);
    passed = targets;
    for (int i = flightCount - 1; i >= 0; --i)
        if (!flights[i].spear.alive() || LONG(now - flights[i].expires) >= 0)
            land(i);
    for (int i = defenderCount - 1; i >= 0; --i)
        defend(i, now);
}
}

