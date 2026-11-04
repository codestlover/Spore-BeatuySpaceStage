
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <Windows.h>
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <optional>
#include <ranges>
#include <span>
#include <utility>

module beatuy.feature.civilization;

import beatuy.core.mod;
import beatuy.game;

namespace feature {
namespace {
using civilization::settings;
using map::identifiers::Type;

enum Ability : int {
    Megabomb = 0,
    Technobomb = 1,
    Repair = 3,
    BlackCloud = 4,
    Madness = 6,
    Propaganda = 7,
    Static = 9,
    Invulnerable = 10,
    Ambassador = 11
};

constexpr auto blasts = std::to_array({40.0f, 95.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 64.0f, 0.0f, 0.0f});

constexpr int abilities = int(blasts.size());

constexpr uint32_t bit(Ability ability) {
    return 1u << ability;
}

constexpr auto specialtyCards = std::to_array<uint32_t>({
    bit(Technobomb) | bit(Megabomb) | bit(Static),
    bit(BlackCloud) | bit(Invulnerable),
    bit(Propaganda) | bit(Ambassador) | bit(Madness),
});

constexpr auto beliefDamage = std::to_array({3, 4, 0xc, 0xe, 0xf, 0x10});

constexpr auto armed = std::to_array({Type::Vehicle, Type::Turret, Type::Creature});
constexpr auto cityParts = std::to_array({Type::Turret, Type::Building, Type::City});

using UnitAim = char* (*)(game::Vehicle);

constexpr auto unitForms = std::to_array<UnitAim>({
    [](game::Vehicle vehicle) { return vehicle.raw(); },
    [](game::Vehicle vehicle) { return vehicle.fighter().raw(); },
    [](game::Vehicle vehicle) { return vehicle.body().raw(); },
});

using CityAim = char* (*)(game::City);

constexpr auto cityForms = std::to_array<CityAim>({
    [](game::City city) { return city.raw(); },
    [](game::City city) -> char* { return city.hall(); },
});

constexpr auto cityOrders = std::to_array({map::identifiers::Order::AttackCity, map::identifiers::Order::ConvertCity});

auto used(auto& list, int count) {
    return std::span(list).first(size_t(count));
}

bool contains(std::ranges::input_range auto&& list, const auto& value) {
    return std::ranges::find(list, value) != std::ranges::end(list);
}

bool contains(std::ranges::input_range auto&& list, const auto& value, auto projection) {
    return std::ranges::find(list, value, projection) != std::ranges::end(list);
}

bool castsTo(game::GameData data, std::span<const Type> types) {
    return std::ranges::any_of(types, [data](Type type) { return data.cast(type) != nullptr; });
}

int formOf(const auto& forms, const void* target, auto subject) {
    auto form = std::ranges::find_if(forms, [&](const auto& aim) { return target == aim(subject); });
    return form != std::ranges::end(forms) ? int(form - std::ranges::begin(forms)) : -1;
}

int cityFormOf(const void* target, game::City city) {
    return formOf(cityForms, target, city);
}

int cityOrderOf(map::identifiers::Order kind) {
    auto order = std::ranges::find(cityOrders, kind);
    return order != cityOrders.end() ? int(order - cityOrders.begin()) : -1;
}

float blastOf(int ability) {
    return ability >= 0 && ability < abilities ? blasts[ability] : 0.0f;
}

using Valuation = float (*)(const civilization::Scene& scene);
float spared(const civilization::Scene& scene, float worth) {
    return scene.friends > 0 ? 0.0f : worth;
}
constexpr std::array<Valuation, abilities> valuations{
    [](const civilization::Scene& scene) {
        return spared(scene, float(scene.enemies + scene.enemyTurrets) + 3.0f * float(scene.hostileShip));
    },
    [](const civilization::Scene& scene) {
        return spared(scene, float(scene.enemies + 2 * scene.enemyTurrets) + 3.0f * float(scene.hostileShip));
    },
    [](const civilization::Scene&) { return 0.0f; },
    [](const civilization::Scene& scene) { return 2.0f * float(scene.wounded); },
    [](const civilization::Scene& scene) { return 0.75f * float(scene.enemies + scene.enemyTurrets); },
    [](const civilization::Scene&) { return 0.0f; },
    [](const civilization::Scene& scene) {
        return scene.enemyVehicles >= 2 ? 1.5f * float(scene.enemyVehicles) : 0.0f;
    },
    [](const civilization::Scene&) { return 3.0f; },
    [](const civilization::Scene&) { return 0.0f; },
    [](const civilization::Scene& scene) {
        return spared(scene, float(scene.enemyVehicles) + 4.0f * float(scene.hostileShip));
    },
    [](const civilization::Scene& scene) { return scene.fighting >= 2 ? float(scene.fighting) : 0.0f; },
    [](const civilization::Scene&) { return 3.0f; },
};
}

bool Civilization::identifierIn(std::span<const uint32_t> identifiers, Unit unit) {
    uint32_t identifier = unit.identifier();
    return contains(identifiers, identifier);
}

bool Civilization::isSpared(Unit unit) const {
    return sparedCount && identifierIn(used(sparedIdentifiers, sparedCount), unit);
}

bool Civilization::observing(uint32_t politicalId, DWORD now) const {
    std::span<const Penalty> watching = used(observers, observerCount);
    auto entry = std::ranges::find(watching, politicalId, &Penalty::politicalId);
    return entry != watching.end() && LONG(now - entry->tick) < LONG(settings.mood.observeMilliseconds);
}

void Civilization::observe(uint32_t politicalId, DWORD now) {
    std::span<Penalty> watching = used(observers, observerCount);
    if (auto entry = std::ranges::find(watching, politicalId, &Penalty::politicalId); entry != watching.end()) {
        entry->tick = now;
        return;
    }
    if (observerCount >= limits.maximumCivilizations)
        return;
    observers[observerCount++] = Penalty{politicalId, now};
    for (int i = pulledCount - 1; i >= 0; --i)
        if (pulled[i].vehicle.politicalId() == politicalId)
            letGo(i, now);
}

void Civilization::unobserve(uint32_t politicalId) {
    std::span<Penalty> watching = used(observers, observerCount);
    auto entry = std::ranges::find(watching, politicalId, &Penalty::politicalId);
    if (entry == watching.end())
        return;
    *entry = observers[--observerCount];
}

bool Civilization::isPulled(Unit unit) const {
    return contains(used(pulled, pulledCount), unit, &Pulled::vehicle);
}

bool Civilization::isHunting(uint32_t politicalId) const {
    return contains(used(hunting, huntCount), politicalId);
}

bool Civilization::isReacting(game::City city) const {
    return contains(used(reacting, reactCount), city);
}

bool Civilization::civilizationHere(uint32_t politicalId) const {
    return contains(used(towns, townCount), politicalId, &Town::politicalId);
}

bool Civilization::hosts(game::Civilization nation) const {
    auto ruled = [nation](const Town& town) { return town.city.civilization() == nation; };
    return std::ranges::any_of(used(towns, townCount), ruled);
}

void Civilization::refreshCities() {
    townCount = 0;
    game::Array<game::City*> all = game::each<game::City>(map::civilization::cities);
    uint32_t me = game::playerPoliticalId();
    game::Vector ship = game::shipPosition();
    for (game::City city : all) {
        if (townCount >= limits.maximumCities)
            break;
        if (!city || !city.alive())
            continue;
        uint32_t politicalId = city.politicalId();
        if (politicalId == me || politicalId == uint32_t(-1))
            continue;
        const game::Vector* center = city.place().position();
        if (!center)
            continue;
        game::Walls walls = city.walls();
        float outer = walls ? walls.outerRadius() : 0.0f;
        float radius = std::clamp(outer > 1.0f ? outer * 1.6f : 90.0f, 60.0f, 220.0f);
        towns[townCount++] = Town{city, politicalId, *center, radius, game::surfaceDistance(ship, *center) <= radius};
    }
}

Civilization::Town* Civilization::nearestCity(uint32_t politicalId, const game::Vector& at) {
    Town* best = nullptr;
    float bestDistance = 1e30f;
    for (Town& town : used(towns, townCount)) {
        float distance = game::surfaceDistance(at, town.center);
        if (town.politicalId == politicalId && distance < bestDistance) {
            best = &town;
            bestDistance = distance;
        }
    }
    return best;
}

const Civilization::Pulled* Civilization::find(Unit unit) const {
    std::span<const Pulled> inUse = used(pulled, pulledCount);
    auto entry = std::ranges::find(inUse, unit, &Pulled::vehicle);
    return entry != inUse.end() ? &*entry : nullptr;
}

void Civilization::refreshEscorts() {
    escortCount = 0;
    for (game::Ship ship : game::each<game::Ship>(map::space::ships)) {
        if (escortCount >= limits.maximumEscorts)
            break;
        if (ship.alive() && ship.kind() == map::identifiers::ShipKind::Escort)
            escorts[escortCount++] = ship;
    }
}

game::Ship Civilization::sideShip(const void* combatant) const {
    game::Ship ship = game::playerShip();
    if (!combatant || !ship)
        return game::Ship();
    if (combatant == ship.fighter().raw())
        return ship;
    for (game::Ship escort : used(escorts, escortCount))
        if (combatant == escort.fighter().raw())
            return escort;
    return game::Ship();
}

Civilization::Reach Civilization::measure(Unit unit, game::Ship ship) {
    game::Vector at = unit.where();
    const game::Vector* target = ship.body().position();
    Reach reach{1e30f, 0.0f, game::Tool(unit.fighter().weapon()).range(), at};
    float ground = game::length(at);
    if (!target || ground < 1.0f)
        return reach;
    float height = game::length(*target);
    reach.vertical = height - ground;
    reach.under = {target->x * ground / height, target->y * ground / height, target->z * ground / height};
    reach.horizontal = game::distance(at, reach.under);
    return reach;
}

bool Civilization::within(const Reach& reach) {
    float up = reach.vertical > 0.0f ? reach.vertical * settings.hunt.verticalReach : 0.0f;
    return reach.range > 0.0f && reach.horizontal * reach.horizontal + up * up <= reach.range * reach.range;
}

void Civilization::approach(Pulled& pulledUnit, DWORD now, bool force) {
    Reach reach = measure(pulledUnit.vehicle, pulledUnit.prey);
    if (reach.range <= 0.0f || reach.horizontal <= reach.range * settings.hunt.standOff)
        return;
    game::Vector point = game::toward(reach.under, pulledUnit.vehicle.where(), reach.range * settings.hunt.approach);
    if (!force && LONG(now - pulledUnit.moveTick) < LONG(settings.hunt.moveMilliseconds) &&
        game::distance(point, pulledUnit.movePoint) < settings.hunt.replan)
        return;
    Own own(ownDepth);
    game::clearOrders(pulledUnit.vehicle);
    game::addOrderAt(pulledUnit.vehicle, point, 0);
    game::startOrders(pulledUnit.vehicle);
    pulledUnit.moveTick = now;
    pulledUnit.movePoint = point;
}

void Civilization::boost(Pulled& pulledUnit) {
    const std::array<char*, 2> list{pulledUnit.vehicle.weapon(), pulledUnit.vehicle.airWeapon()};
    for (int i = 0; i < 2; ++i) {
        pulledUnit.weapons[i] = game::Tool(list[i] && (i == 0 || list[1] != list[0]) ? list[i] : nullptr);
        if (!pulledUnit.weapons[i])
            continue;
        pulledUnit.baseRange[i] = pulledUnit.weapons[i].rangeMultiplier();
        pulledUnit.weapons[i].rangeMultiplier() = pulledUnit.baseRange[i] * settings.hunt.shipReach;
    }
}

void Civilization::unboost(Pulled& pulledUnit) {
    const std::array<char*, 2> list{pulledUnit.vehicle.weapon(), pulledUnit.vehicle.airWeapon()};
    for (int i = 0; i < 2; ++i)
        if (pulledUnit.weapons[i] && (list[0] == pulledUnit.weapons[i].raw() || list[1] == pulledUnit.weapons[i].raw()))
            pulledUnit.weapons[i].rangeMultiplier() = pulledUnit.baseRange[i];
}

game::Ship Civilization::choosePrey(Unit unit, game::Ship current) const {
    game::Ship best = game::playerShip();
    if (!best)
        return current;
    Reach reach = measure(unit, best);
    bool bestIn = within(reach);
    float bestGap = reach.horizontal;
    for (game::Ship escort : used(escorts, escortCount)) {
        Reach escortReach = measure(unit, escort);
        bool in = within(escortReach);
        if ((in && !bestIn) || (in == bestIn && escortReach.horizontal < bestGap)) {
            best = escort;
            bestIn = in;
            bestGap = escortReach.horizontal;
        }
    }
    return best;
}

void Civilization::lock(Pulled& pulledUnit, game::Ship prey) {
    pulledUnit.prey = prey;
    Own own(ownDepth);
    pulledUnit.vehicle.fighter().setTarget(prey.fighter().raw());
}

void Civilization::attack(Pulled& pulledUnit, game::Ship ship) {
    lock(pulledUnit, ship);
    boost(pulledUnit);
    lock(pulledUnit, choosePrey(pulledUnit.vehicle, ship));
    approach(pulledUnit, GetTickCount(), true);
}

bool Civilization::recentlyDropped(Unit unit, DWORD now) const {
    return std::ranges::any_of(used(dropped, droppedCount), [&](const Dropped& entry) {
        return entry.vehicle == unit && LONG(now - entry.tick) < LONG(settings.hunt.repullMilliseconds);
    });
}

bool Civilization::canEngage(Unit unit, game::Ship ship) const {
    if (unit.flies())
        return true;
    Reach reach = measure(unit, ship);
    if (reach.horizontal > settings.hunt.detect)
        return false;
    return !recentlyDropped(unit, GetTickCount()) || within(reach);
}

void Civilization::restore(Pulled& pulledUnit, const void* ship) {
    if (pulledUnit.vehicle.usable()) {
        unboost(pulledUnit);
        game::Combatant combatant = pulledUnit.vehicle.fighter();
        game::clearOrders(pulledUnit.vehicle);
        if (pulledUnit.hadOrder) {
            if (pulledUnit.kind == map::identifiers::Order::Move)
                game::addOrderAt(pulledUnit.vehicle, pulledUnit.point, pulledUnit.flag);
            else if (pulledUnit.target.alive())
                game::addOrder(pulledUnit.vehicle, pulledUnit.target.raw(), pulledUnit.kind, pulledUnit.flag);
        }
        if (ship && (combatant.target() == ship || sideShip(combatant.target())))
            combatant.setTarget(nullptr);
        game::startOrders(pulledUnit.vehicle);
    }
    pulledUnit.target.release();
    pulledUnit.vehicle.release();
}

void Civilization::letGo(int index, DWORD now) {
    Pulled& pulledUnit = pulled[index];
    {
        Own own(ownDepth);
        restore(pulledUnit, game::playerShip().fighter().raw());
    }
    int slot = 0;
    while (slot < droppedCount && dropped[slot].vehicle != pulledUnit.vehicle &&
           LONG(now - dropped[slot].tick) < LONG(settings.hunt.repullMilliseconds))
        ++slot;
    if (slot == droppedCount && droppedCount < limits.maximumPulled)
        ++droppedCount;
    if (slot < droppedCount)
        dropped[slot] = Dropped{pulledUnit.vehicle, now};
    pulled[index] = pulled[--pulledCount];
}

void Civilization::pull(Unit unit, game::Ship ship) {
    if (pulledCount >= limits.maximumPulled || isPulled(unit) || (!fightingBack && isSpared(unit)))
        return;
    Pulled pulledUnit{};
    pulledUnit.vehicle = unit;
    unit.addReference();
    game::Order order = unit.current();
    if (order) {
        pulledUnit.hadOrder = true;
        pulledUnit.target = game::GameData(order.target());
        pulledUnit.target.addReference();
        pulledUnit.point = order.point();
        pulledUnit.kind = order.kind();
        pulledUnit.flag = unit.orderFlags();
    }
    pulledUnit.bestGap = 1e30f;
    pulledUnit.bestTick = GetTickCount();
    pulled[pulledCount] = pulledUnit;
    attack(pulled[pulledCount++], ship);
}

void Civilization::aimTurret(game::Turret turret, game::Ship ship) {
    if (!contains(used(turrets, turretCount), turret)) {
        if (turretCount >= limits.maximumTurrets)
            return;
        turret.addReference();
        turrets[turretCount++] = turret;
    }
    turret.fighter().setTarget(ship.fighter().raw());
}

void Civilization::aimCity(game::City city, game::Ship ship) {
    for (game::Turret turret : city.turrets())
        if (turret.alive() && turret.cast(Type::Turret) == turret.raw())
            aimTurret(turret, ship);
}

void Civilization::react(Town& town) {
    game::Ship ship = game::playerShip();
    if (!ship || isReacting(town.city) || reactCount >= limits.maximumCities)
        return;
    reacting[reactCount++] = town.city;
    int specialty = town.city.specialty();
    aimCity(town.city, ship);
    rally(town, ship);
    if (specialty == int(map::identifiers::Purpose::Economic))
        return;
    civilization::Nearest<settings.hunt.pullPerCity> pick(settings.hunt.pullRadius);
    for (Unit unit : game::each<Unit>(map::civilization::vehicles)) {
        if (!unit.usable() || unit.politicalId() != town.politicalId || !unit.fights() || isPulled(unit) ||
            recentlyDropped(unit, GetTickCount()))
            continue;
        pick.offer(unit, game::surfaceDistance(unit.where(), town.center));
    }
    for (Unit unit : pick.units)
        if (unit)
            pull(unit, ship);
}

Civilization::Command* Civilization::commandOf(uint32_t politicalId) {
    std::span<Command> inUse = used(commands, commandCount);
    if (auto command = std::ranges::find(inUse, politicalId, &Command::politicalId); command != inUse.end())
        return &*command;
    if (commandCount >= limits.maximumCivilizations)
        return nullptr;
    DWORD now = GetTickCount();
    commands[commandCount] = Command{
        .politicalId = politicalId,
        .enemy = uint32_t(-1),
        .rallyUntil = 0,
        .nextDefense = 0,
        .nextOffense = now + settings.command.offenseFirstMilliseconds +
                       DWORD(rand() % settings.command.offenseJitterMilliseconds),
        .threatTick = 0,
        .front = {},
    };
    return &commands[commandCount++];
}

int Civilization::citiesOf(uint32_t politicalId) const {
    return int(std::ranges::count(used(towns, townCount), politicalId, &Town::politicalId));
}

bool Civilization::available(Unit unit, uint32_t politicalId) const {
    return unit.usable() && unit.politicalId() == politicalId && unit.fights() && !isPulled(unit) &&
           !recentlyDropped(unit, GetTickCount());
}

void Civilization::issue(Unit unit, void* target, map::identifiers::Order kind) {
    Own own(ownDepth);
    game::clearOrders(unit);
    game::addOrder(unit, target, kind, 0);
    game::startOrders(unit);
}

void Civilization::rally(const Town& hit, game::Ship playerShip) {
    Command* command = citiesOf(hit.politicalId) > 1 ? commandOf(hit.politicalId) : nullptr;
    DWORD now = GetTickCount();
    if (!command || LONG(now - command->rallyUntil) < 0)
        return;
    command->rallyUntil = now + settings.command.rallyMilliseconds;
    game::Vector ship = game::shipPosition();
    game::Array<Unit*> all = game::each<Unit>(map::civilization::vehicles);
    bool nouns = all.data() != nullptr;
    for (const Town& sister : used(towns, townCount)) {
        if (sister.politicalId != hit.politicalId || sister.city == hit.city)
            continue;
        if (game::surfaceDistance(sister.center, ship) <= settings.command.rallyTurrets)
            aimCity(sister.city, playerShip);
        if (!nouns || sister.city.specialty() == int(map::identifiers::Purpose::Economic))
            continue;
        civilization::Nearest<settings.command.rallyPerCity> pick(settings.command.rallyRadius);
        for (Unit unit : all) {
            if (!available(unit, hit.politicalId) || isSpared(unit))
                continue;
            pick.offer(unit, game::surfaceDistance(unit.where(), sister.center));
        }
        for (Unit unit : pick.units)
            if (unit && !isPulled(unit))
                pull(unit, playerShip);
    }
}

void Civilization::threat(uint32_t politicalId, uint32_t enemy, const game::Vector& at) {
    Command* command = citiesOf(politicalId) > 1 ? commandOf(politicalId) : nullptr;
    if (!command)
        return;
    command->enemy = enemy;
    command->front = at;
    command->threatTick = GetTickCount();
}

int Civilization::unitFormOf(const void* target, game::Array<Unit*> all) {
    game::Vehicle ship(game::playerShip().raw());
    for (Unit each : all)
        for (game::Vehicle vehicle : std::array<game::Vehicle, 2>{each, ship}) {
            if (!vehicle)
                continue;
            if (int form = formOf(unitForms, target, vehicle); form >= 0)
                return form;
        }
    return -1;
}

void Civilization::scanOrders(DWORD now) {
    if ((unitTarget >= 0 && cityTarget[0] >= 0 && cityTarget[1] >= 0) || LONG(now - nextOrderScan) < 0)
        return;
    nextOrderScan = now + settings.command.orderScanMilliseconds;
    game::Array<Unit*> all = game::each<Unit>(map::civilization::vehicles);
    for (Unit unit : all) {
        if (!unit.usable() || unit.idle())
            continue;
        game::Order order = unit.current();
        map::identifiers::Order kind = order.kind();
        void* target = order.target();
        if (!target)
            continue;
        if (kind == map::identifiers::Order::Attack && unitTarget < 0) {
            unitTarget = unitFormOf(target, all);
            continue;
        }
        int against = cityOrderOf(kind);
        if (against < 0)
            continue;
        int& form = cityTarget[against];
        for (int i = 0; i < townCount && form < 0; ++i)
            form = cityFormOf(target, towns[i].city);
    }
}

void Civilization::defend(Command& command, DWORD now) {
    if (unitTarget < 0 || command.enemy == uint32_t(-1) ||
        LONG(now - command.threatTick) > LONG(settings.command.threatMilliseconds) ||
        LONG(now - command.nextDefense) < 0 || !game::atWar(command.politicalId, command.enemy))
        return;
    Unit raider;
    float best = settings.command.raidRadius;
    civilization::Nearest<settings.command.defenders> pick(settings.command.defendRadius);
    for (Unit unit : game::each<Unit>(map::civilization::vehicles)) {
        if (!unit.usable())
            continue;
        float distance = game::surfaceDistance(unit.where(), command.front);
        if (unit.politicalId() == command.enemy) {
            if (distance < best) {
                best = distance;
                raider = unit;
            }
            continue;
        }
        if (!available(unit, command.politicalId) || !unit.idle())
            continue;
        pick.offer(unit, distance);
    }
    if (!raider || !pick.units[0])
        return;
    command.nextDefense = now + settings.command.defenseGapMilliseconds;
    void* aim = unitForms[unitTarget](raider);
    for (Unit unit : pick.units)
        if (unit)
            issue(unit, aim, map::identifiers::Order::Attack);
}

void Civilization::offense(Command& command, DWORD now) {
    if (LONG(now - command.nextOffense) < 0)
        return;
    command.nextOffense =
        now + settings.command.offenseMilliseconds + DWORD(rand() % settings.command.offenseJitterMilliseconds);
    if (command.threatTick && LONG(now - command.threatTick) < LONG(settings.command.threatMilliseconds))
        return;
    game::Vector home{0, 0, 0};
    int own = 0;
    for (const Town& town : used(towns, townCount))
        if (town.politicalId == command.politicalId) {
            home = home + town.center;
            ++own;
        }
    if (own < 2)
        return;
    home = {home.x / own, home.y / own, home.z / own};
    Town* target = nullptr;
    float best = 1e30f;
    for (Town& town : used(towns, townCount)) {
        float distance = game::surfaceDistance(town.center, home);
        if (town.politicalId != command.politicalId && game::atWar(command.politicalId, town.politicalId) &&
            distance < best) {
            best = distance;
            target = &town;
        }
    }
    if (!target)
        return;
    game::Array<Unit*> all = game::each<Unit>(map::civilization::vehicles);
    if (!all.data())
        return;
    struct Assault {
        Unit unit;
        map::identifiers::Order kind;
        void* aim;
    };
    std::array<Assault, settings.command.wave> wave;
    int waveCount = 0;
    for (Unit unit : all) {
        if (waveCount >= settings.command.wave)
            break;
        if (!available(unit, command.politicalId) || !unit.idle() ||
            unit.locomotion() == map::identifiers::Locomotion::Water)
            continue;
        bool convert = unit.purpose() == map::identifiers::Purpose::Cultural;
        int form = cityTarget[convert];
        void* aim = form >= 0 ? cityForms[form](target->city) : nullptr;
        if (!aim)
            continue;
        wave[waveCount++] = Assault{unit, cityOrders[convert], aim};
    }
    if (waveCount < settings.command.waveMinimum)
        return;
    for (const Assault& assault : used(wave, waveCount))
        issue(assault.unit, assault.aim, assault.kind);
}

void Civilization::coordinate(DWORD now) {
    scanOrders(now);
    for (int i = 0; i < townCount; ++i) {
        bool repeat = contains(used(towns, i), towns[i].politicalId, &Town::politicalId);
        Command* command = repeat || citiesOf(towns[i].politicalId) < 2 ? nullptr : commandOf(towns[i].politicalId);
        if (!command)
            continue;
        defend(*command, now);
        offense(*command, now);
    }
}

void Civilization::huntUnits(uint32_t politicalId, game::Ship ship) {
    for (Unit unit : game::each<Unit>(map::civilization::vehicles))
        if (unit.usable() && unit.politicalId() == politicalId && unit.fights() && !isPulled(unit) &&
            canEngage(unit, ship))
            pull(unit, ship);
}

Civilization::Town* Civilization::anyCity(uint32_t politicalId) {
    std::span<Town> inUse = used(towns, townCount);
    auto town = std::ranges::find(inUse, politicalId, &Town::politicalId);
    return town != inUse.end() ? &*town : nullptr;
}

Civilization::Penalty* Civilization::due(std::span<Penalty> list, int& count, uint32_t politicalId, DWORD now,
                                         DWORD gap) {
    std::span<Penalty> inUse = list.first(size_t(count));
    auto found = std::ranges::find(inUse, politicalId, &Penalty::politicalId);
    Penalty* entry = found != inUse.end() ? &*found : nullptr;
    if (entry && LONG(now - entry->tick) < LONG(gap))
        return nullptr;
    if (!entry) {
        if (count >= limits.maximumCivilizations)
            return nullptr;
        entry = &list[count++];
        entry->politicalId = politicalId;
    }
    entry->tick = now;
    return entry;
}

void Civilization::punish(uint32_t politicalId, Unit unit, DWORD now) {
    unobserve(politicalId);
    if (!due(penalties, penaltyCount, politicalId, now, settings.mood.hitPenaltyMilliseconds))
        return;
    uint32_t me = game::playerPoliticalId();
    float delta;
    {
        Own own(ownDepth);
        delta = game::apply(politicalId, me, map::identifiers::Relation::Attacked, settings.mood.hitScale);
    }
    Town* town = anyCity(politicalId);
    game::Vector at = unit.where();
    if (town)
        game::showMood(town->city, delta, &at);
}

uint32_t Civilization::victimOf(Unit unit) const {
    const Pulled* pulledUnit = pulledCount ? find(unit) : nullptr;
    void* target = pulledUnit ? (pulledUnit->hadOrder ? pulledUnit->target.raw() : nullptr) : nullptr;
    if (!pulledUnit) {
        void* targetCombatant = unit.fighter().target();
        target = game::Combatant(targetCombatant).owner().raw();
        if (!target)
            target = unit.currentTarget();
    }
    return target ? game::GameData(target).politicalId() : uint32_t(-1);
}

void Civilization::thank(DWORD now) {
    uint32_t me = game::playerPoliticalId();
    for (int i = 0; i < helpCount; ++i) {
        const Help& help = helps[i];
        if (!civilizationHere(help.defended))
            continue;
        observe(help.defended, now);
        if (!due(thanks, thanksCount, help.defended, now, settings.mood.thanksMilliseconds))
            continue;
        float delta;
        {
            Own own(ownDepth);
            delta = game::apply(help.defended, me, thanksEvent, settings.mood.thanksScale);
            if (delta <= 0.0f && thanksEvent == map::identifiers::Relation::FoughtEnemy) {
                thanksEvent = map::identifiers::Relation::GoodToolUse;
                delta = game::apply(help.defended, me, thanksEvent, settings.mood.thanksScale);
            }
        }
        Town* town = nearestCity(help.defended, help.at);
        if (town)
            game::showMood(town->city, delta, &town->center);
    }
    helpCount = 0;
}

void Civilization::fightBack(game::Ship ship, DWORD now) {
    for (int i = 0; i < hitCount; ++i) {
        Unit unit = hits[i];
        const uint32_t politicalId = unit.politicalId();
        if (civilizationHere(politicalId))
            punish(politicalId, unit, now);
        if (unit.usable() && unit.fights() && !isPulled(unit)) {
            fightingBack = true;
            pull(unit, ship);
            fightingBack = false;
            game::Vector at = unit.where();
            for (Unit other : game::each<Unit>(map::civilization::vehicles))
                if (other != unit && other.usable() && other.politicalId() == politicalId && other.fights() &&
                    !isPulled(other) && game::surfaceDistance(other.where(), at) <= settings.hunt.squadRadius)
                    pull(other, ship);
        }
        unit.release();
    }
    hitCount = 0;
}

void Civilization::hunt(uint32_t politicalId) {
    game::Ship ship = game::playerShip();
    if (!ship || isHunting(politicalId) || huntCount >= limits.maximumCivilizations)
        return;
    hunting[huntCount++] = politicalId;
    for (const Town& town : used(towns, townCount))
        if (town.politicalId == politicalId) {
            aimCity(town.city, ship);
            if (!isReacting(town.city) && reactCount < limits.maximumCities)
                reacting[reactCount++] = town.city;
        }
    huntUnits(politicalId, ship);
}

void Civilization::war(uint32_t politicalId) {
    uint32_t me = game::playerPoliticalId();
    if (!game::atWar(politicalId, me)) {
        Own own(ownDepth);
        game::declareWar(politicalId, me);
    }
    hunt(politicalId);
}

void Civilization::remember(const Town& town) {
    game::PlanetRecord record = game::activeRecord();
    auto same = [&](const Hall& hall) {
        return hall.record == record && game::distance(hall.at, town.center) < settings.hunt.hallMatch;
    };
    if (std::ranges::any_of(used(halls, hallCount), same))
        return;
    if (hallCount < limits.maximumHalls)
        halls[hallCount++] = Hall{record, town.center};
}

bool Civilization::remembered(const Town& town) const {
    game::PlanetRecord record = game::activeRecord();
    auto same = [&](const Hall& hall) {
        return hall.record == record && game::distance(hall.at, town.center) < settings.hunt.hallMatch;
    };
    return std::ranges::any_of(used(halls, hallCount), same);
}

void Civilization::releaseAll(bool issue) {
    cancelOffer();
    if (!pulledCount && !turretCount && !huntCount && !reactCount && !hitCount && !helpCount && !hostileCount &&
        !observerCount)
        return;
    Own own(ownDepth);
    void* shipCombatant = game::playerShip().fighter().raw();
    for (Pulled& pulledUnit : used(pulled, pulledCount))
        if (issue)
            restore(pulledUnit, shipCombatant);
    for (game::Turret turret : used(turrets, turretCount)) {
        game::Combatant combatant = turret.fighter();
        if (issue && turret.alive() && shipCombatant && combatant.target() == shipCombatant)
            combatant.setTarget(nullptr);
        if (issue)
            turret.release();
    }
    for (Unit hit : used(hits, hitCount))
        hit.release();
    pulledCount = turretCount = huntCount = reactCount = droppedCount = hitCount = helpCount = hostileCount = 0;
    observerCount = 0;
}

void Civilization::newVisit(game::Planet visited) {
    planet = visited;
    map::identifiers::Technology technology = game::planetTechnology();
    civilizationPlanet = visited && (technology == map::identifiers::Technology::City ||
                                     technology == map::identifiers::Technology::Civilization);
    loiterCount = 0;
    attackPending = false;
    harmCount = arsenalCount = commandCount = 0;
    designs.forget();
    nextSuper = GetTickCount() + settings.super.graceMilliseconds;
    superAtShip = 0;
    if (civilizationPlanet)
        refreshCities();
}

bool Civilization::attacking(uint32_t attacker, uint32_t victim) {
    for (Unit unit : game::each<Unit>(map::civilization::vehicles)) {
        if (!unit.usable() || unit.politicalId() != attacker)
            continue;
        game::GameData target(unit.currentTarget());
        if (target.alive() && target.politicalId() == victim)
            return true;
    }
    return false;
}

void Civilization::loiter(uint32_t politicalId, float elapsed) {
    std::span<Town> inUse = used(towns, townCount);
    auto nearby = std::ranges::find_if(
        inUse, [politicalId](const Town& town) { return town.politicalId == politicalId && town.close; });
    if (nearby == inUse.end())
        return;
    Town* close = &*nearby;
    uint32_t me = game::playerPoliticalId();
    float standing = game::standing(politicalId, me);
    if (standing > 0.0001f || observing(politicalId, GetTickCount()))
        return;
    std::span<LoiterState> loiters = used(loiterStates, loiterCount);
    auto found = std::ranges::find(loiters, politicalId, &LoiterState::politicalId);
    LoiterState* entry = found != loiters.end() ? &*found : nullptr;
    if (!entry) {
        if (loiterCount >= limits.maximumCivilizations)
            return;
        entry = &loiterStates[loiterCount++];
        *entry = LoiterState{.politicalId = politicalId, .seconds = 0.0f, .last = standing, .steps = 0, .flat = 0};
    }
    entry->seconds += elapsed;
    if (entry->seconds < settings.mood.loiterStepSeconds)
        return;
    entry->seconds = 0.0f;
    {
        Own own(ownDepth);
        game::apply(politicalId, me, map::identifiers::Relation::BeNasty, settings.mood.loiterScale);
    }
    float after = game::standing(politicalId, me);
    ++entry->steps;
    entry->flat = after >= entry->last - 0.0001f ? entry->flat + 1 : 0;
    entry->last = after;
    game::showMood(close->city, after);
    bool floor = after < 0.0f && (entry->flat >= 2 || after <= settings.mood.warFloor);
    if ((entry->steps >= settings.mood.loiterMinimumSteps && floor) || entry->steps >= settings.mood.loiterMaximumSteps)
        war(politicalId);
}

void Civilization::watch(game::Ship ship) {
    uint32_t me = game::playerPoliticalId();
    std::array<uint32_t, limits.maximumCivilizations> seen;
    int seenCount = 0;
    for (int i = 0; i < townCount && seenCount < limits.maximumCivilizations; ++i) {
        uint32_t politicalId = towns[i].politicalId;
        if (contains(used(seen, seenCount), politicalId))
            continue;
        seen[seenCount++] = politicalId;
        if (isHunting(politicalId))
            continue;
        float standing = game::standing(politicalId, me);
        bool angry = standing <= settings.mood.hostileStanding;
        int slot = 0;
        while (slot < hostileCount && hostile[slot] != politicalId)
            ++slot;
        bool was = slot < hostileCount;
        if (angry && !was && hostileCount < limits.maximumCivilizations)
            hostile[hostileCount++] = politicalId;
        else if (!angry && was)
            hostile[slot] = hostile[--hostileCount];
        if (angry && !observing(politicalId, GetTickCount()))
            huntUnits(politicalId, ship);
    }
}

void Civilization::maintain(game::Ship ship, DWORD now) {
    refreshEscorts();
    guard();
    for (uint32_t politicalId : used(hunting, huntCount))
        huntUnits(politicalId, ship);
    watch(ship);
    coordinate(now);
    for (int i = pulledCount - 1; i >= 0; --i) {
        Pulled& pulledUnit = pulled[i];
        if (!pulledUnit.vehicle.usable())
            continue;
        game::Ship prey = choosePrey(pulledUnit.vehicle, pulledUnit.prey);
        if (prey != pulledUnit.prey || pulledUnit.vehicle.fighter().target() != prey.fighter().raw())
            lock(pulledUnit, prey);
        if (pulledUnit.vehicle.flies())
            continue;
        Reach reach = measure(pulledUnit.vehicle, pulledUnit.prey);
        if (reach.horizontal < pulledUnit.bestGap - settings.hunt.closer) {
            pulledUnit.bestGap = reach.horizontal;
            pulledUnit.bestTick = now;
        }
        if (!within(reach) && LONG(now - pulledUnit.bestTick) > LONG(settings.hunt.outrunMilliseconds))
            letGo(i, now);
    }
    for (Pulled& pulledUnit : used(pulled, pulledCount))
        if (pulledUnit.vehicle.usable())
            approach(pulledUnit, now, false);
    for (game::Turret turret : used(turrets, turretCount)) {
        if (!turret.alive())
            continue;
        const game::Vector* at = turret.body().position();
        game::Ship best = ship;
        float bestDistance = at ? game::distance(*at, game::shipPosition()) : 0.0f;
        if (at)
            for (game::Ship escort : used(escorts, escortCount)) {
                const game::Vector* there = escort.body().position();
                float distance = there ? game::distance(*at, *there) : 1e30f;
                if (distance < bestDistance) {
                    best = escort;
                    bestDistance = distance;
                }
            }
        turret.fighter().setTarget(best.fighter().raw());
    }
}

void Civilization::makePeace(uint32_t politicalId, float standing) {
    uint32_t me = game::playerPoliticalId();
    game::declarePeace(politicalId, me);
    for (int i = 0; i < huntCount; ++i)
        if (hunting[i] == politicalId) {
            hunting[i] = hunting[--huntCount];
            break;
        }
    for (int i = 0; i < hostileCount; ++i)
        if (hostile[i] == politicalId) {
            hostile[i] = hostile[--hostileCount];
            break;
        }
    DWORD now = GetTickCount();
    for (int i = pulledCount - 1; i >= 0; --i)
        if (pulled[i].vehicle.politicalId() == politicalId)
            letGo(i, now);
    for (int i = turretCount - 1; i >= 0; --i) {
        game::Turret turret = turrets[i];
        if (turret.politicalId() != politicalId)
            continue;
        game::Combatant combatant = turret.fighter();
        if (turret.alive() && sideShip(combatant.target())) {
            Own own(ownDepth);
            combatant.setTarget(nullptr);
        }
        turret.release();
        turrets[i] = turrets[--turretCount];
    }
    game::PlanetRecord record = game::activeRecord();
    for (const Town& town : used(towns, townCount)) {
        if (town.politicalId != politicalId)
            continue;
        for (int i = reactCount - 1; i >= 0; --i)
            if (reacting[i] == town.city)
                reacting[i] = reacting[--reactCount];
        for (int i = hallCount - 1; i >= 0; --i)
            if (halls[i].record == record && game::distance(halls[i].at, town.center) < settings.hunt.hallMatch)
                halls[i] = halls[--hallCount];
    }
    Town* town = anyCity(politicalId);
    if (town)
        game::showMood(town->city, standing, &town->center);
}

char Civilization::onBribeClick(uint32_t event, void*) {
    Offer& bribe = Mod::get().civilization.offer;
    if (!bribe.vehicle || event != bribe.event)
        return 0;
    bribe.answer = Answer::Accepted;
    return 1;
}

void Civilization::onBribeRemoved(uint32_t event, void*) {
    Offer& bribe = Mod::get().civilization.offer;
    if (bribe.vehicle && event == bribe.event && bribe.answer != Answer::Accepted)
        bribe.answer = Answer::Closed;
}

void Civilization::endOffer() {
    if (!offer.vehicle)
        return;
    offer.vehicle.release();
    offer = Offer{};
}

void Civilization::cancelOffer() {
    if (!offer.vehicle)
        return;
    uint32_t event = offer.event;
    endOffer();
    game::hideEvent(event);
    offer.answer = Answer::None;
}

void Civilization::spare(Unit unit, DWORD now) {
    if (!isSpared(unit) && sparedCount < limits.maximumSpared)
        sparedIdentifiers[sparedCount++] = unit.identifier();
    for (int i = pulledCount - 1; i >= 0; --i)
        if (pulled[i].vehicle == unit)
            letGo(i, now);
    game::Combatant combatant = unit.fighter();
    Own own(ownDepth);
    if (sideShip(combatant.target()))
        combatant.setTarget(nullptr);
}

void Civilization::breach(Unit unit) {
    uint32_t identifier = unit.identifier();
    for (int i = sparedCount - 1; i >= 0; --i)
        if (sparedIdentifiers[i] == identifier)
            sparedIdentifiers[i] = sparedIdentifiers[--sparedCount];
    for (int i = sparedHereCount - 1; i >= 0; --i)
        if (sparedHere[i] == unit)
            sparedHere[i] = sparedHere[--sparedHereCount];
    ++breaches;
}

void Civilization::betrayed() {
    refreshEscorts();
    uint32_t me = game::playerPoliticalId();
    std::array<uint32_t, limits.maximumEscorts> seen;
    int seenCount = 0;
    for (game::Ship escort : used(escorts, escortCount)) {
        uint32_t politicalId = escort.politicalId();
        bool repeat = contains(used(seen, seenCount), politicalId);
        if (repeat || politicalId == me || politicalId == uint32_t(-1))
            continue;
        seen[seenCount++] = politicalId;
        Own own(ownDepth);
        game::apply(politicalId, me, map::identifiers::Relation::BeNasty, settings.mood.breachScale * float(breaches));
    }
    breaches = 0;
}

void Civilization::accept(DWORD now) {
    Unit unit = offer.vehicle;
    game::addPlayerMoney(offer.amount);
    uint32_t event = offer.event;
    if (unit.usable()) {
        spare(unit, now);
        Mod::get().radio.agreed(unit.raw());
    }
    endOffer();
    game::hideEvent(event);
    offer.answer = Answer::None;
}

void Civilization::awaitAnswer(DWORD now) {
    if (!offer.vehicle.usable() || LONG(now - offer.shown) > LONG(settings.bribe.offerMilliseconds))
        cancelOffer();
}

void Civilization::ignored(DWORD) {
    endOffer();
}

int Civilization::allies(Unit unit, uint32_t politicalId) {
    game::Array<Unit*> all = game::each<Unit>(map::civilization::vehicles);
    if (!all.data())
        return -1;
    game::Vector at = unit.where();
    auto ally = [&](Unit other) {
        return other != unit && other.usable() && other.politicalId() == politicalId && other.fights() &&
               game::surfaceDistance(other.where(), at) <= settings.bribe.aloneRadius;
    };
    return int(std::ranges::count_if(all, ally));
}

void Civilization::propose(Pulled& pulledUnit, DWORD now) {
    const civilization::Settings::Bribe& bribe = settings.bribe;
    Unit unit = pulledUnit.vehicle;
    int steps = (bribe.most - bribe.least) / bribe.step + 1;
    int amount = bribe.least + (rand() % steps) * bribe.step;
    std::array<wchar_t, 320> message;
    civilization::Text::bribeOffer(amount, message);
    uint32_t event = game::showEvent(map::identifiers::Popup::AlliesArrive, message.data(), &Civilization::onBribeClick,
                                     &Civilization::onBribeRemoved, bribe.textSeconds, bribe.seconds);
    if (!event)
        return;
    unit.addReference();
    offer = Offer{.vehicle = unit,
                  .event = event,
                  .amount = amount,
                  .shown = now,
                  .answer = Answer::None};
    game::playAudio(bribe.sound);
    Mod::get().radio.bribed(unit.raw());
    nextOffer = now + bribe.offerGapMilliseconds;
}

void Civilization::bribes(DWORD now) {
    if (offer.vehicle) {
        using Answered = void (Civilization::*)(DWORD);
        static constexpr std::array<Answered, 3> answered{&Civilization::awaitAnswer, &Civilization::accept,
                                                          &Civilization::ignored};
        (this->*answered[int(offer.answer)])(now);
        return;
    }
    if (LONG(now - nextOffer) < 0)
        return;
    for (Pulled& pulledUnit : used(pulled, pulledCount)) {
        Unit unit = pulledUnit.vehicle;
        if (!unit.usable() || identifierIn(used(asked, askedCount), unit) ||
            !identifierIn(used(struck, struckCount), unit))
            continue;
        if (allies(unit, unit.politicalId()) != 0) {
            pulledUnit.aloneSince = 0;
            continue;
        }
        if (!pulledUnit.aloneSince)
            pulledUnit.aloneSince = now;
        if (LONG(now - pulledUnit.aloneSince) < LONG(settings.bribe.aloneMilliseconds))
            continue;
        if (askedCount < limits.maximumAsked)
            asked[askedCount++] = unit.identifier();
        if (rand() % 100 < settings.bribe.chance) {
            propose(pulledUnit, now);
            return;
        }
    }
}

void Civilization::guard() {
    sparedHereCount = 0;
    if (!sparedCount)
        return;
    for (Unit unit : game::each<Unit>(map::civilization::vehicles)) {
        if (sparedHereCount >= limits.maximumSpared)
            break;
        if (unit.usable() && isSpared(unit))
            sparedHere[sparedHereCount++] = unit;
    }
    std::array<game::Ship, limits.maximumEscorts + 1> ships;
    int count = 0;
    if (game::Ship playerShip = game::playerShip())
        ships[count++] = playerShip;
    for (game::Ship escort : used(escorts, escortCount))
        ships[count++] = escort;
    Own own(ownDepth);
    for (game::Ship ship : used(ships, count)) {
        game::Combatant combatant = ship.fighter();
        void* target = combatant.target();
        game::GameData data = game::Combatant(target).owner();
        Unit unit(data.cast(Type::Vehicle));
        if (unit && isSpared(unit))
            combatant.setTarget(nullptr);
    }
}

void Civilization::picks(DWORD now) {
    if (std::optional<civilization::Designs::Job> job = designs.due(now); job && hosts(job->nation))
        designs.pick(*job, now);
}

void Civilization::rings() {
    if (!sparedCount && !offer.vehicle)
        return;
    game::Array<Unit*> all = game::each<Unit>(map::civilization::vehicles);
    if (!all.data())
        return;
    for (Unit unit : all)
        if (unit.usable() && (unit == offer.vehicle || isSpared(unit)))
            marker.add(unit);
    marker.end();
}

bool Civilization::wounded(void* combatant, float damage) {
    game::Combatant fighter(combatant);
    float maximum = fighter.maximumHealth();
    return maximum > 0.0f && fighter.health() - damage < maximum * settings.super.woundShare;
}

void Civilization::harmed(uint32_t politicalId, const game::Vector& at, uint32_t by, bool wounded) {
    DWORD now = GetTickCount();
    std::span<Harm> inUse = used(harms, harmCount);
    if (auto harm = std::ranges::find(inUse, politicalId, &Harm::politicalId); harm != inUse.end()) {
        bool keep = harm->wounded && LONG(now - harm->tick) < LONG(settings.super.harmMilliseconds);
        *harm = Harm{.politicalId = politicalId,
                     .by = by,
                     .tick = now,
                     .at = wounded ? at : harm->at,
                     .wounded = wounded || keep};
        return;
    }
    if (harmCount < limits.maximumCivilizations)
        harms[harmCount++] = Harm{.politicalId = politicalId, .by = by, .tick = now, .at = at, .wounded = wounded};
    const DWORD soon = now + settings.super.woundReactionMilliseconds;
    if (wounded && LONG(nextSuperThink - soon) > 0)
        nextSuperThink = soon;
}

void Civilization::shuffle(std::span<Shot> shots) {
    for (int i = int(shots.size()) - 1; i > 0; --i) {
        int other = rand() % (i + 1);
        std::swap(shots[i], shots[other]);
    }
}

bool Civilization::angry(uint32_t politicalId, uint32_t me) const {
    return isHunting(politicalId) || game::atWar(politicalId, me) ||
           game::standing(politicalId, me) <= settings.mood.hostileStanding;
}

std::optional<game::Vector> Civilization::swarm(uint32_t politicalId, const game::Vector& around, float radius) {
    int count = 0;
    game::Vector sum{0, 0, 0};
    for (Unit unit : game::each<Unit>(map::civilization::vehicles)) {
        if (!unit.usable() || unit.politicalId() != politicalId)
            continue;
        game::Vector at = unit.where();
        if (game::surfaceDistance(at, around) > radius)
            continue;
        sum = sum + at;
        ++count;
    }
    if (count < 2)
        return std::nullopt;
    return game::Vector{sum.x / count, sum.y / count, sum.z / count};
}

std::optional<game::Vector> Civilization::woundedAround(uint32_t politicalId, const game::Vector& around,
                                                        float radius) {
    int count = 0;
    game::Vector sum{0, 0, 0};
    for (Unit unit : game::each<Unit>(map::civilization::vehicles)) {
        if (!unit.usable() || unit.politicalId() != politicalId)
            continue;
        const game::Combatant fighter = unit.fighter();
        const float maximum = fighter.maximumHealth();
        const game::Vector at = unit.where();
        if (maximum <= 0.0f || fighter.health() >= maximum * settings.super.woundShare ||
            game::surfaceDistance(at, around) > radius)
            continue;
        sum = sum + at;
        ++count;
    }
    if (!count)
        return std::nullopt;
    return game::Vector{sum.x / count, sum.y / count, sum.z / count};
}

uint32_t Civilization::cardsOf(uint32_t politicalId, game::Civilization nation) const {
    uint32_t cards = 0;
    for (int ability = 0; ability < abilities; ++ability)
        if (nation.cards()[ability])
            cards |= 1u << ability;
    for (const Town& town : used(towns, townCount)) {
        if (town.politicalId != politicalId)
            continue;
        int specialty = town.city.specialty();
        if (specialty >= 0 && specialty < int(specialtyCards.size()))
            cards |= specialtyCards[specialty];
    }
    return cards;
}

void Civilization::consider(std::span<Shot> shots, int& count, uint32_t cards, int ability, const game::Vector& at,
                            bool ship) {
    if (count < limits.maximumShots && (cards >> ability & 1))
        shots[count++] = Shot{ability, at, ship};
}

int Civilization::plan(uint32_t politicalId, game::Civilization nation, const game::Vector& ship,
                       std::span<Shot> shots) {
    uint32_t me = game::playerPoliticalId();
    uint32_t cards = cardsOf(politicalId, nation);
    DWORD now = GetTickCount();
    int count = 0, fighters = 0;
    game::Vector sum{0, 0, 0};
    for (const Pulled& pulledUnit : used(pulled, pulledCount))
        if (pulledUnit.vehicle.usable() && pulledUnit.vehicle.politicalId() == politicalId) {
            sum = sum + pulledUnit.vehicle.where();
            ++fighters;
        }
    if (fighters >= 2)
        consider(shots, count, cards, Invulnerable, {sum.x / fighters, sum.y / fighters, sum.z / fighters}, false);
    for (const Harm& harm : used(harms, harmCount)) {
        if (harm.politicalId != politicalId || LONG(now - harm.tick) >= LONG(settings.super.harmMilliseconds))
            continue;
        bool player = harm.by == me;
        std::optional<game::Vector> army = player ? std::nullopt : swarm(politicalId, harm.at, settings.super.swarm);
        if (army)
            consider(shots, count, cards, Invulnerable, *army, false);
        const std::optional<game::Vector> wounded =
            harm.wounded ? woundedAround(politicalId, harm.at, settings.super.swarm) : std::nullopt;
        if (wounded)
            consider(shots, count, cards, Repair, *wounded, false);
    }
    Town* closest = nearestCity(politicalId, ship);
    if (closest && angry(politicalId, me) && game::surfaceDistance(closest->center, ship) <= settings.super.reach) {
        int first = count;
        for (int ability : std::array{Static, Megabomb, Technobomb})
            consider(shots, count, cards, ability, ship, true);
        shuffle(shots.subspan(size_t(first), size_t(count - first)));
    }
    int first = count;
    for (int i = 0; i < townCount; ++i) {
        uint32_t other = towns[i].politicalId;
        bool repeat = other == politicalId || contains(used(towns, i), other, &Town::politicalId);
        Town* theirs = repeat ? nullptr : nearestCity(other, ship);
        Town* front = theirs ? nearestCity(politicalId, theirs->center) : nullptr;
        theirs = front ? nearestCity(other, front->center) : nullptr;
        if (!theirs)
            continue;
        if (game::atWar(politicalId, other)) {
            if (std::optional<game::Vector> crowd = swarm(other, front->center, settings.super.swarm))
                for (int ability : std::array{Megabomb, Static, Madness})
                    consider(shots, count, cards, ability, *crowd, false);
            consider(shots, count, cards, Technobomb, theirs->center, false);
            consider(shots, count, cards, Megabomb, theirs->center, false);
            consider(shots, count, cards, BlackCloud, theirs->center, false);
        } else {
            if (game::standing(other, politicalId) < -1.0f)
                consider(shots, count, cards, Ambassador, theirs->center, false);
            if (game::standing(politicalId, other) <= 0.0f)
                consider(shots, count, cards, Propaganda, theirs->center, false);
        }
    }
    shuffle(shots.subspan(size_t(first), size_t(count - first)));
    return count;
}

bool Civilization::safe(uint32_t politicalId, const Shot& shot) {
    float blast = blastOf(shot.ability);
    if (blast <= 0.0f)
        return true;
    for (const Town& town : used(towns, townCount)) {
        uint32_t owner = town.politicalId;
        if (owner != politicalId && !game::allied(owner, politicalId))
            continue;
        game::Walls walls = town.city.walls();
        float outer = walls ? walls.outerRadius() : 0.0f;
        float reach = (outer > 1.0f ? outer : settings.super.cityExtent) + blast + settings.super.blastMargin;
        float distance = game::surfaceDistance(town.center, shot.at);
        if (distance > reach)
            continue;
        return false;
    }
    return true;
}

Civilization::Arsenal* Civilization::arsenalOf(game::Civilization nation) {
    std::span<Arsenal> inUse = used(arsenals, arsenalCount);
    if (auto arsenal = std::ranges::find(inUse, nation, &Arsenal::nation); arsenal != inUse.end())
        return &*arsenal;
    if (arsenalCount >= limits.maximumCivilizations)
        return nullptr;
    arsenals[arsenalCount] = Arsenal{.nation = nation, .next = 0, .broken = false, .last = -1};
    return &arsenals[arsenalCount++];
}

bool Civilization::ready(game::Civilization nation, Arsenal& arsenal) {
    if (int(nation.tools().size()) >= abilities)
        return true;
    bool ok = int(nation.cooldowns().size()) >= abilities;
    if (ok) {
        uint8_t& player = nation.playerOwned();
        uint8_t was = player;
        player = 1;
        game::loadSuperweaponTools(nation);
        player = was;
        game::Array<game::View*> tools = nation.tools();
        ok = int(tools.size()) >= abilities &&
             std::ranges::all_of(tools | std::views::take(abilities), [](game::View tool) { return bool(tool); });
    }
    arsenal.broken = !ok;
    return ok;
}

Civilization::Launcher Civilization::launcherFor(game::Civilization nation, const game::Vector& at) const {
    Launcher launcher{game::City(), 1e30f};
    for (game::City candidate : nation.cities()) {
        const game::Vector* place = candidate.alive() ? candidate.place().position() : nullptr;
        const float distance = place ? game::surfaceDistance(*place, at) : 1e30f;
        if (distance < launcher.distance && candidate.hall())
            launcher = Launcher{candidate, distance};
    }
    return launcher;
}

bool Civilization::launch(game::Civilization nation, const Shot& shot, DWORD now) {
    const auto [city, best] = launcherFor(nation, shot.at);
    if (!city)
        return false;
    {
        Own own(ownDepth);
        game::citySuperweapon(city, shot.ability, shot.at);
    }
    game::Timer cooldown = nation.cooldowns()[size_t(shot.ability)];
    if (cooldown.clock())
        game::startTimer(cooldown);
    if (shot.ship || blastOf(shot.ability) > 0.0f) {
        DWORD until = now + DWORD(best / settings.super.speed * settings.super.flightSlack * 1000.0f) +
                      settings.super.blastMilliseconds;
        if (!superAtShip || LONG(until - superAtShip) > 0)
            superAtShip = until;
        superDrained = false;
    }
    return true;
}

void Civilization::watchShip(DWORD now) {
    const game::Vector at = game::shipPosition();
    const float elapsed = shipWatched ? float(now - shipSeenAt) / 1000.0f : 0.0f;
    if (elapsed > 0.0f && elapsed < 1.0f) {
        const game::Vector moving = (at - shipSeen) * (1.0f / elapsed);
        const float blend = std::min(1.0f, settings.super.velocitySmoothing * elapsed);
        shipVelocity = shipVelocity + (moving - shipVelocity) * blend;
    }
    if (elapsed >= 1.0f)
        shipVelocity = game::Vector{0, 0, 0};
    shipSeen = at;
    shipSeenAt = now;
    shipWatched = true;
}

civilization::Scene Civilization::sceneAt(uint32_t politicalId, const game::Vector& at, float radius,
                                          bool atShip) const {
    civilization::Scene scene;
    const uint32_t me = game::playerPoliticalId();
    const auto enemy = [politicalId](uint32_t other) {
        return other != politicalId && game::atWar(other, politicalId);
    };
    const auto friendly = [politicalId](uint32_t other) {
        return other == politicalId || game::allied(other, politicalId);
    };
    std::array<game::Vector, 64> enemyPlaces{};
    size_t enemyCount = 0;
    for (Unit unit : game::each<Unit>(map::civilization::vehicles)) {
        if (!unit.usable() || enemyCount == enemyPlaces.size())
            continue;
        const uint32_t owner = unit.politicalId();
        if (enemy(owner))
            enemyPlaces[enemyCount++] = unit.where();
    }
    const bool hostileToPlayer = angry(politicalId, me);
    const game::Vector ship = game::shipPosition();
    const auto engaged = [&](const game::Vector& place) {
        const auto within = [&](const game::Vector& other) {
            return game::surfaceDistance(other, place) <= settings.super.engagedReach;
        };
        return std::ranges::any_of(std::span(enemyPlaces).first(enemyCount), within) ||
               (hostileToPlayer && within(ship));
    };
    for (Unit unit : game::each<Unit>(map::civilization::vehicles)) {
        if (!unit.usable())
            continue;
        const game::Vector place = unit.where();
        if (game::surfaceDistance(place, at) > radius)
            continue;
        const uint32_t owner = unit.politicalId();
        const game::Combatant fighter = unit.fighter();
        const float maximum = fighter.maximumHealth();
        scene.enemies += enemy(owner);
        scene.enemyVehicles += enemy(owner);
        scene.friends += friendly(owner);
        scene.wounded += owner == politicalId && maximum > 0.0f &&
                         fighter.health() < maximum * settings.super.woundShare;
        scene.fighting += owner == politicalId && engaged(place);
    }
    for (const Town& town : used(towns, townCount)) {
        if (!enemy(town.politicalId))
            continue;
        for (const game::Turret turret : town.city.turrets())
            scene.enemyTurrets += turret && turret.alive() && game::surfaceDistance(turret.position(), at) <= radius;
    }
    scene.hostileShip = hostileToPlayer && atShip && game::surfaceDistance(ship, at) <= radius;
    return scene;
}

float Civilization::worthOf(uint32_t politicalId, game::Civilization nation, Shot& shot) const {
    const float blast = blastOf(shot.ability);
    const float radius = blast > 0.0f ? blast : settings.super.auraRadius;
    civilization::Scene scene;
    if (shot.ship) {
        const float seconds = launcherFor(nation, shot.at).distance / settings.super.speed * settings.super.flightSlack;
        const game::Vector way = shipVelocity * seconds;
        shot.at = game::shipPosition() + way;
        scene = sceneAt(politicalId, shot.at, radius, game::length(way) * settings.super.leadMiss <= radius);
    } else {
        scene = sceneAt(politicalId, shot.at, radius, false);
    }
    return valuations[size_t(shot.ability)](scene);
}

void Civilization::superweapons(DWORD now) {
    if (LONG(now - nextSuperThink) < 0)
        return;
    nextSuperThink = now + settings.super.thinkMilliseconds;
    if (LONG(now - nextSuper) < 0)
        return;
    game::Vector ship = game::shipPosition();
    std::array<game::Civilization, limits.maximumCivilizations> seen;
    int seenCount = 0;
    superTurn = townCount ? (superTurn + 1) % townCount : 0;
    for (int offset = 0; offset < townCount && seenCount < limits.maximumCivilizations; ++offset) {
        int index = (superTurn + offset) % townCount;
        game::Civilization nation = towns[index].city.civilization();
        bool repeat = contains(used(seen, seenCount), nation);
        if (!nation || repeat || !nation.readable(game::Civilization::byteSize))
            continue;
        seen[seenCount++] = nation;
        Arsenal* arsenal = arsenalOf(nation);
        if (!arsenal || arsenal->broken || LONG(now - arsenal->next) < 0)
            continue;
        std::array<Shot, limits.maximumShots> shots;
        int count = plan(towns[index].politicalId, nation, ship, shots);
        if (!count || !ready(nation, *arsenal))
            continue;
        int pick = -1;
        float best = settings.super.worthIt;
        Shot aimed{};
        for (int candidate = 0; candidate < count; ++candidate) {
            Shot shot = shots[candidate];
            if (!safe(towns[index].politicalId, shot) || game::superweaponCooling(nation, shot.ability))
                continue;
            const float worth = worthOf(towns[index].politicalId, nation, shot);
            if (worth > best) {
                best = worth;
                pick = candidate;
                aimed = shot;
            }
        }
        if (pick < 0)
            continue;
        arsenal->next = now + settings.super.civilizationGapMilliseconds;
        if (!launch(nation, aimed, now))
            continue;
        arsenal->last = aimed.ability;
        nextSuper = now + settings.super.planetGapMilliseconds;
        return;
    }
}

void Civilization::think(float elapsed) {
    refreshCities();
    uint32_t me = game::playerPoliticalId();
    std::array<uint32_t, limits.maximumCivilizations> seen;
    int seenCount = 0;
    for (int i = 0; i < townCount && seenCount < limits.maximumCivilizations; ++i) {
        uint32_t politicalId = towns[i].politicalId;
        if (contains(used(seen, seenCount), politicalId))
            continue;
        seen[seenCount++] = politicalId;
        if (game::atWar(politicalId, me)) {
            float standing = game::standing(politicalId, me);
            if (standing >= settings.mood.peaceStanding)
                makePeace(politicalId, standing);
            else
                hunt(politicalId);
        } else {
            loiter(politicalId, elapsed);
        }
    }
    for (int i = 0; i < huntCount; ++i) {
        uint32_t enemy = hunting[i];
        auto nearShip = [enemy](const Town& town) { return town.politicalId == enemy && town.close; };
        if (!std::ranges::any_of(used(towns, townCount), nearShip))
            continue;
        for (uint32_t politicalId : used(seen, seenCount))
            if (!isHunting(politicalId) && game::atWar(politicalId, enemy) && attacking(politicalId, enemy))
                hunt(politicalId);
    }
    for (Town& town : used(towns, townCount))
        if (town.close && !isReacting(town.city) && remembered(town))
            react(town);
}

bool Civilization::ownCall() {
    return ownDepth > 0;
}

void Civilization::onBeliefLaunch(char* shell) {
    game::Belief launched(shell);
    if (auto beam = std::ranges::find(beams, launched, &Beam::shell); beam != beams.end()) {
        beam->launched = GetTickCount();
        return;
    }
    beams[beamNext] = Beam{launched, GetTickCount()};
    beamNext = (beamNext + 1) % limits.maximumBeams;
}

DWORD Civilization::launched(game::Belief shell) {
    if (auto beam = std::ranges::find(beams, shell, &Beam::shell); beam != beams.end())
        return beam->launched;
    onBeliefLaunch(shell.raw());
    return GetTickCount();
}

void Civilization::beginBelief(char* shell) {
    belief = game::Belief(shell);
    if (!civilizationPlanet || belief.timerRunning())
        return;
    game::Ship ship = sideShip(belief.target());
    const game::Vector* at = ship.body().position();
    const game::Vector* from = at ? game::GameData(shell).body().position() : nullptr;
    if (!from)
        return;
    DWORD now = GetTickCount();
    bool chasing = LONG(now - launched(belief)) <= LONG(settings.belief.chaseMilliseconds);
    if (chasing)
        game::retargetBelief(belief, *from, *at);
}

void Civilization::endBelief() {
    belief = game::Belief();
}

const game::Vector* Civilization::push(void* victim, void* attacker, const game::Vector* direction, game::Vector& out) {
    game::Ship ship = civilizationPlanet && direction ? sideShip(victim) : game::Ship();
    game::GameData source = ship ? game::Combatant(attacker).owner() : game::GameData();
    Unit unit(source.cast(Type::Vehicle));
    if (!unit || unit.politicalId() == game::playerPoliticalId())
        return direction;
    out = {0.0f, 0.0f, 0.0f};
    std::span<Shove> inUse = used(shoves, shoveCount);
    auto found = std::ranges::find(inUse, ship, &Shove::ship);
    Shove* entry = found != inUse.end() ? &*found : nullptr;
    if (!entry && shoveCount <= limits.maximumEscorts) {
        entry = &shoves[shoveCount++];
        *entry = Shove{ship, GetTickCount() - settings.push.gapMilliseconds};
    }
    DWORD now = GetTickCount();
    const game::Vector* at = ship.body().position();
    if (!entry || !at || LONG(now - entry->tick) < LONG(settings.push.gapMilliseconds))
        return &out;
    entry->tick = now;
    game::Vector away = *at - unit.where();
    float length = game::length(away);
    if (length < 1.0f)
        return &out;
    float scale = settings.push.scale / length;
    out = away * scale;
    return &out;
}

int Civilization::damageType(void* victim, void* attacker, int type) {
    if (!contains(beliefDamage, type))
        return type;
    game::Ship ship = civilizationPlanet ? sideShip(victim) : game::Ship();
    game::GameData source = ship ? game::Combatant(attacker).owner() : game::GameData();
    Unit unit(source.cast(Type::Vehicle));
    if (!unit || unit.purpose() != map::identifiers::Purpose::Cultural ||
        unit.politicalId() == game::playerPoliticalId())
        return type;
    const game::Vector* shell = belief ? game::GameData(belief.raw()).body().position() : nullptr;
    const game::Vector* at = ship.body().position();
    if (shell && at && game::distance(*shell, *at) > settings.belief.contact)
        return type;
    return int(map::identifiers::Damage::Artillery);
}

void Civilization::drain(game::Ship ship, int&) {
    if (superDrained)
        return;
    superDrained = true;
    float drain = ship.maximumEnergy() * settings.super.staticDrain;
    game::changeEnergy(ship, -drain);
}

void Civilization::asArtillery(game::Ship, int& type) {
    type = int(map::identifiers::Damage::Artillery);
}

float Civilization::superHit(void* victim, void* attacker, float damage, int& type) {
    if (!civilizationPlanet || !superAtShip || LONG(GetTickCount() - superAtShip) > 0)
        return damage;
    game::Ship ship = sideShip(victim);
    game::GameData source = ship && attacker ? game::Combatant(attacker).owner() : game::GameData();
    if (!ship || (source && castsTo(source, armed)))
        return damage;
    struct Effect {
        int type;
        void (Civilization::*apply)(game::Ship, int&);
    };
    static constexpr auto effects = std::to_array<Effect>(
        {{0xe, &Civilization::drain}, {0xb, &Civilization::asArtillery}, {0xc, &Civilization::asArtillery}});
    if (auto effect = std::ranges::find(effects, type, &Effect::type); effect != effects.end())
        (this->*effect->apply)(ship, type);
    return damage / settings.super.shipDivisor;
}

uint32_t Civilization::healerFor(void* vehicleCombatant, uint32_t healer, int type) {
    if (!civilizationPlanet || type != int(map::identifiers::Damage::Heal))
        return healer;
    uint32_t politicalId = game::Vehicle::ofFighter(vehicleCombatant).politicalId();
    return politicalId != healer && game::allied(politicalId, healer) ? politicalId : healer;
}

bool Civilization::blockHeal(void* victim, uint32_t healer, int type) {
    if (!civilizationPlanet || type != int(map::identifiers::Damage::Heal))
        return false;
    game::GameData data = game::Combatant(victim).owner();
    uint32_t politicalId = data ? data.politicalId() : uint32_t(-1);
    return politicalId == uint32_t(-1) || !(politicalId == healer || game::allied(politicalId, healer));
}

void Civilization::vehicleDesign(void* city, int purpose, int locomotion, game::Key& key) {
    if (!civilizationPlanet)
        return;
    designs.choose(game::City(city), purpose, locomotion, key);
}

bool Civilization::locked(void* vehicle) {
    return pulledCount && find(Unit(vehicle)) && Unit(vehicle).usable();
}

bool Civilization::playerSide(void* combatant) {
    return civilizationPlanet && sideShip(combatant);
}

void* Civilization::lockedTarget(void* vehicle) {
    const Pulled* pulledUnit = pulledCount ? find(Unit(vehicle)) : nullptr;
    return pulledUnit && pulledUnit->prey ? pulledUnit->prey.fighter().raw() : nullptr;
}

int Civilization::inReach(void* combatant, void* other) {
    game::Ship ship = civilizationPlanet ? sideShip(other) : game::Ship();
    if (!ship)
        return -1;
    Unit unit(game::Vehicle::ofFighter(combatant).raw());
    return within(measure(unit, ship)) ? 1 : 0;
}

void Civilization::onDamage(void* victim, uint32_t attackerPoliticalId, void* attacker, float damage, int type) {
    if ((!civilizationPlanet && !planet) || type == int(map::identifiers::Damage::Heal))
        return;
    uint32_t me = game::playerPoliticalId();
    if (victim == game::playerShip().fighter().raw())
        return;
    if (civilizationPlanet && attackerPoliticalId != me && attackerPoliticalId != uint32_t(-1) && !sideShip(attacker)) {
        game::GameData hurt = game::Combatant(victim).owner();
        uint32_t politicalId = hurt ? hurt.politicalId() : uint32_t(-1);
        if (politicalId != attackerPoliticalId && politicalId != me && politicalId != uint32_t(-1) &&
            civilizationHere(politicalId) && civilizationHere(attackerPoliticalId)) {
            game::Vector at = hurt.position();
            harmed(politicalId, at, attackerPoliticalId, wounded(victim, damage));
            threat(politicalId, attackerPoliticalId, at);
        }
        return;
    }
    if (!civilizationPlanet || (attackerPoliticalId != me && !sideShip(attacker)))
        return;
    game::GameData data = game::Combatant(victim).owner();
    if (!data)
        return;
    uint32_t politicalId = data.politicalId();
    if (politicalId == me || politicalId == uint32_t(-1))
        return;
    Unit vehicle(data.cast(Type::Vehicle));
    harmed(politicalId, data.position(), me, wounded(victim, damage));
    bool byShip = attacker ? sideShip(attacker) == game::playerShip() : attackerPoliticalId == me;
    if (vehicle && byShip && isSpared(vehicle) &&
        LONG(GetTickCount() - beamTick) < LONG(settings.bribe.beamMilliseconds))
        breach(vehicle);
    if (vehicle) {
        if (!identifierIn(used(struck, struckCount), vehicle)) {
            struck[struckNext] = vehicle.identifier();
            struckNext = (struckNext + 1) % limits.maximumStruck;
            struckCount = struckCount < limits.maximumStruck ? struckCount + 1 : limits.maximumStruck;
        }
        uint32_t defended = victimOf(vehicle);
        if (defended != uint32_t(-1) && defended != politicalId && defended != me && helpCount < limits.maximumHits)
            helps[helpCount++] = Help{defended, politicalId, data.position()};
        bool queued = contains(used(hits, hitCount), vehicle);
        if (!queued && hitCount < limits.maximumHits) {
            vehicle.addReference();
            hits[hitCount++] = vehicle;
        }
        return;
    }
    bool hall = data.cast(Type::CityHall) != nullptr;
    if (!hall && !castsTo(data, cityParts))
        return;
    unobserve(politicalId);
    attackPending = true;
    attackedHall |= hall;
    attackedPoliticalId = politicalId;
    attackedAt = data.position();
}

bool Civilization::onOrder(void* vehicle) {
    return ownDepth || !locked(vehicle);
}

bool Civilization::automatic(void* tool, game::Ship ship) const {
    bool automatic = game::Tool(tool).strategy() == settings.bribe.toggleStrategy;
    automatic |= ship && ship.fighter().automaticWeapon() == tool;
    auto fires = [tool](game::Ship escort) { return escort.fighter().automaticWeapon() == tool; };
    return automatic || std::ranges::any_of(used(escorts, escortCount), fires);
}

bool Civilization::blockShot(void* tool, void* source, void* target) {
    game::Ship ship = game::playerShip();
    bool fromShip = ship && source == ship.body().raw();
    if (civilizationPlanet && tool && fromShip && !target && !automatic(tool, ship))
        beamTick = GetTickCount();
    if (!civilizationPlanet || !sparedHereCount || !tool || !target)
        return false;
    std::span<Unit> inUse = used(sparedHere, sparedHereCount);
    auto marked = std::ranges::find_if(inUse, [target](Unit each) {
        return each.body().raw() == target || each.raw() == target;
    });
    Unit unit = marked != inUse.end() ? *marked : Unit();
    if (!unit)
        return false;
    if (!automatic(tool, ship)) {
        if (fromShip)
            breach(unit);
        return false;
    }
    return true;
}

bool Civilization::spared(void* combatant) {
    if (!sparedCount || !combatant)
        return false;
    Unit unit(game::Combatant(combatant).owner().cast(Type::Vehicle));
    return unit && isSpared(unit);
}

void Civilization::onLeavePlanet(bool destroyed) {
    releaseAll(true);
    if (destroyed)
        return;
    planet = game::Planet();
    civilizationPlanet = false;
}

void Civilization::tick() {
    marker.begin();
    if (!game::onPlanet() || !game::playerShip()) {
        if (planet)
            releaseAll(false);
        planet = game::Planet();
        civilizationPlanet = false;
        return;
    }
    if (game::Planet active = game::activePlanet(); active != planet) {
        releaseAll(false);
        newVisit(active);
    }
    if (!civilizationPlanet)
        return;
    game::Ship ship = game::playerShip();
    if (ship.fighter().health() <= 0.0f) {
        releaseAll(true);
        return;
    }
    DWORD now = GetTickCount();
    watchShip(now);
    if (breaches)
        betrayed();
    if (hitCount || helpCount) {
        refreshCities();
        thank(now);
        fightBack(ship, now);
    }
    if (attackPending) {
        attackPending = false;
        refreshCities();
        Town* town = nearestCity(attackedPoliticalId, attackedAt);
        if (town) {
            if (attackedHall)
                remember(*town);
            if (!isHunting(town->politicalId))
                react(*town);
        }
        attackedHall = false;
    }
    if (LONG(now - nextThink) >= 0) {
        float elapsed = lastThink ? (now - lastThink) / 1000.0f : 0.0f;
        lastThink = now;
        nextThink = now + settings.pace.thinkMilliseconds;
        think(elapsed > 1.0f ? 1.0f : elapsed);
    }
    if (LONG(now - nextAssert) >= 0) {
        nextAssert = now + settings.pace.assertMilliseconds;
        maintain(ship, now);
        bribes(now);
        superweapons(now);
    }
    picks(now);
    rings();
}
}

