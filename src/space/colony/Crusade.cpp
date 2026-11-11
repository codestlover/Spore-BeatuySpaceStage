
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
#include <limits>
#include <optional>
#include <ranges>
#include <span>
#include <utility>

module beatuy.feature.colony;

import beatuy.game;

namespace feature {
namespace {
using map::identifiers::Mission;
using map::identifiers::Relation;
using map::identifiers::ShipKind;
using map::identifiers::Technology;
using map::identifiers::Type;

constexpr uint32_t nobody = uint32_t(-1);

[[nodiscard]] float reach(uint32_t politicalId, const game::Vector& to) {
    float nearest = std::numeric_limits<float>::max();
    for (game::StarRecord star : game::empireOf(politicalId).stars())
        if (star)
            nearest = std::min(nearest, game::distance(star.position(), to));
    return nearest;
}

[[nodiscard]] float reach(std::span<const uint32_t> members, const game::Vector& to) {
    return std::ranges::min(members | std::views::transform([&](uint32_t member) { return reach(member, to); }));
}

}

void Crusade::tick() {
    DWORD now = GetTickCount();
    float seconds = float(std::min(now - lastTick, settings.longestStepMilliseconds)) / 1000.0f;
    lastTick = now;
    if (!game::inSpace() || game::paused())
        return;
    if (!countdown)
        countdown = targetCount > 0 ? settings.watchSeconds
                                    : game::between(settings.leastWaitSeconds, settings.mostWaitSeconds);
    *countdown -= seconds;
    if (*countdown > 0.0f)
        return;
    countdown.reset();
    if (targetCount > 0)
        watch();
    else
        start();
}

bool Crusade::pardons(uint32_t who, uint32_t cause, Relation event, float scale) {
    if (targetCount == 0 || who != grox || cause != game::playerPoliticalId())
        return false;
    if (event == Relation::MissionCompleted && scale > 0.0f)
        helped = true;
    bool missed = std::ranges::find(settings.missedHelp, event) != settings.missedHelp.end();
    if (missed)
        owed = std::max(owed, std::abs(scale));
    return missed;
}

std::span<game::RaidShip> Crusade::lighten(const void* raid, char* list, std::span<game::RaidShip> changed) const {
    auto targeted = std::span(targets).first(targetCount);
    if (!raid || !list || std::ranges::none_of(targeted, [raid](const Target& target) { return target.raid == raid; }))
        return changed.first(0);
    size_t count = 0;
    for (game::RaidShip ship : game::Array<game::RaidShip>(game::field<char*>(list, 0), game::field<char*>(list, 4))) {
        if (ship.kind() != ShipKind::Bomber || count == changed.size())
            continue;
        ship.kind() = ShipKind::Raider;
        changed[count++] = ship;
    }
    return changed.first(count);
}

void Crusade::restore(std::span<const game::RaidShip> changed) {
    for (game::RaidShip ship : changed)
        ship.kind() = ShipKind::Bomber;
}

bool Crusade::raiding(game::Ship ship) const {
    if (targetCount == 0 || !ship || !ship.alive() || ship.kind() != ShipKind::Raider || ship.leader() ||
        !game::onPlanet())
        return false;
    game::PlanetRecord here = game::activeRecord();
    return std::ranges::any_of(std::span(targets).first(targetCount), [&](const Target& target) {
        return target.planet == here && target.attacker == ship.politicalId();
    });
}

game::City Crusade::siege(game::Ship ship) const {
    game::Vector at = ship.position();
    auto standing = [this](game::City city) { return city && city.alive() && city.politicalId() == grox; };
    auto cities = game::each<game::City>(map::civilization::cities) | std::views::filter(standing);
    auto nearest = std::ranges::min_element(cities, {}, [&](game::City city) {
        const game::Vector* middle = city.place().position();
        return middle ? game::distance(*middle, at) : std::numeric_limits<float>::max();
    });
    return nearest != cities.end() ? *nearest : game::City();
}

void* Crusade::aim(void* combatant, void* target) const {
    if (target || targetCount == 0)
        return target;
    game::Ship ship = game::Ship::ofFighter(combatant);
    game::City city = raiding(ship) ? siege(ship) : game::City();
    if (!city)
        return target;
    game::Vector at = ship.position();
    auto apart = [&](game::GameData part) { return game::distance(part.position(), at); };
    auto turrets = city.turrets() | std::views::filter([](game::Turret turret) { return turret.active(); });
    auto turret = std::ranges::min_element(turrets, {}, apart);
    if (turret != turrets.end())
        return (*turret).fighter().raw();
    auto buildings = city.buildings() | std::views::filter([](game::GameData part) { return part && part.alive(); });
    auto building = std::ranges::min_element(buildings, {}, apart);
    game::GameData part = building != buildings.end() ? *building : game::GameData(city.hall());
    return part && part.alive() ? part.cast(Type::Combatant) : target;
}

bool Crusade::steer(char* shipObject, game::Vector& point) const {
    game::Ship ship(shipObject);
    void* aimed = ship ? ship.fighter().target() : nullptr;
    if (!raiding(ship) || (aimed && game::Combatant(aimed).owner().cast(Type::Ship)))
        return false;
    game::City city = siege(ship);
    const game::Vector* middle = city ? city.place().position() : nullptr;
    if (!middle)
        return false;
    point = *middle;
    return true;
}

void Crusade::stop() {
    if (targetCount > 0)
        countdown.reset();
    grox = nobody;
    targetCount = 0;
}

Crusade::Census Crusade::census(std::span<uint32_t> callers) const {
    game::StarManager stars(map::space::starManager());
    uint32_t player = game::playerPoliticalId();
    Census result{0, nobody};
    if (!stars)
        return result;
    for (game::StarCell cell : stars.cells())
        for (game::StarRecord star : cell.stars()) {
            uint32_t owner = star ? star.politicalId() : nobody;
            auto known = callers.first(result.callers);
            if (owner == nobody || owner == player || owner == result.grox ||
                std::ranges::find(known, owner) != known.end())
                continue;
            if (result.grox == nobody && game::groxEmpire(owner)) {
                result.grox = owner;
                continue;
            }
            int level = game::empireLevel(game::empireOf(owner)) + 1;
            if (level >= settings.lowestLevel && level <= settings.highestLevel && result.callers < callers.size())
                callers[result.callers++] = owner;
        }
    return result;
}

void Crusade::start() {
    std::array<uint32_t, mostCallers> found{};
    Census seen = census(found);
    if (seen.grox == nobody)
        return;
    grox = seen.grox;
    auto known = std::span(found).first(seen.callers);
    auto free = [&](uint32_t politicalId) { return !game::allied(politicalId, grox); };
    auto leaders = known.first(size_t(std::ranges::partition(known, free).begin() - known.begin()));
    if (leaders.empty())
        return;
    size_t first = size_t(std::rand()) % leaders.size();
    Members members{};
    for (size_t step = 0; step < leaders.size(); ++step) {
        size_t count = rally(leaders[(first + step) % leaders.size()], members);
        if (count >= settings.fewestMembers) {
            besiege(std::span(members).first(count));
            return;
        }
    }
}

size_t Crusade::rally(uint32_t leader, Members& members) const {
    uint32_t player = game::playerPoliticalId();
    size_t count = 0;
    members[count++] = leader;
    auto agrees = [&](game::Empire ally) {
        uint32_t politicalId = ally.politicalId();
        auto joined = std::span(members).first(count);
        return politicalId != player && politicalId != grox && !ally.stars().empty() &&
               std::ranges::find(joined, politicalId) == joined.end() && !game::allied(politicalId, grox);
    };
    for (game::Empire ally : game::empireOf(leader).allies())
        if (ally && count < members.size() && agrees(ally))
            members[count++] = ally.politicalId();
    return count;
}

void Crusade::besiege(std::span<const uint32_t> members) {
    game::Empire enemy = game::empireOf(grox);
    auto standing = enemy.stars() | std::views::filter([](game::StarRecord star) { return bool(star); });
    auto seed =
        std::ranges::min_element(standing, {}, [&](game::StarRecord star) { return reach(members, star.position()); });
    if (seed == standing.end())
        return;
    game::Vector center = (*seed).position();
    auto fromCenter = [&](game::StarRecord star) { return game::distance(star.position(), center); };
    std::array<game::StarRecord, settings.clusterStars> cluster{};
    auto sorted = std::ranges::partial_sort_copy(standing, cluster, {}, fromCenter, fromCenter);
    targetCount = 0;
    helped = false;
    owed = 0.0f;
    std::array<size_t, mostMembers> load{};
    auto chosen = [&](game::StarRecord star) {
        auto burden = [&](size_t member) { return std::pair(load[member], reach(members[member], star.position())); };
        return std::ranges::min(std::views::iota(size_t(0), members.size()), {}, burden);
    };
    for (game::StarRecord star : std::span(cluster.begin(), sorted.out))
        for (game::PlanetRecord planet : game::planetsOf(star))
            if (planet && targetCount < targets.size() && planet.technology() == Technology::Empire &&
                !game::eventOnPlanet(planet)) {
                size_t member = chosen(star);
                ++load[member];
                targets[targetCount++] = {planet, star, members[member], nullptr};
            }
    if (targetCount == 0)
        return;
    for (uint32_t member : members)
        game::startWar(game::empireOf(member), enemy);
    bool asked = game::allied(game::playerPoliticalId(), grox);
    Mission mission = asked ? Mission::RaidAlly : Mission::RaidWar;
    size_t raided = 0, started = 0;
    for (Target& target : std::span(targets).first(targetCount)) {
        int bombers = settings.raidBombers[started++ % settings.raidBombers.size()];
        target.raid = game::startRaid(mission, target.planet, enemy, target.attacker, bombers, asked).raw();
        if (target.raid)
            targets[raided++] = target;
    }
    targetCount = raided;
}

void Crusade::watch() {
    size_t left = 0;
    for (const Target& target : std::span(targets).first(targetCount)) {
        if (game::eventOnPlanet(target.planet))
            targets[left++] = target;
        else
            conquer(target);
    }
    targetCount = left;
    if (targetCount == 0)
        settle();
}

void Crusade::settle() {
    if (!helped && owed > 0.0f)
        game::apply(grox, game::playerPoliticalId(), Relation::MissionFailed, owed);
    helped = false;
    owed = 0.0f;
}

void Crusade::conquer(const Target& target) const {
    game::Empire attacker = game::empireOf(target.attacker);
    game::Array<game::StarRecord*> held = game::empireOf(grox).stars();
    uint32_t owner = target.star.politicalId();
    if (!attacker || target.planet.technology() == Technology::Empire || (owner != grox && owner != nobody) ||
        std::ranges::find(held, target.star) != held.end())
        return;
    game::claimStar(target.star, attacker);
}
}

