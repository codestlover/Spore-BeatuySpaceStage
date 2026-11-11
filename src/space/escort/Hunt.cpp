
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <Windows.h>
#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <ranges>
#include <span>

module beatuy.feature.escort;

import beatuy.game;

namespace feature {
namespace {
using map::identifiers::Archetype;
using map::identifiers::ShipKind;
using map::identifiers::Technology;

Hunt* activeHunt = nullptr;
map::space::DecisionFunction reinforcementLeaves = nullptr;

float __cdecl keepCalledShips(void* ship, uint32_t lowTime, uint32_t highTime, uint32_t flags, uint32_t fifthArgument,
                              uint32_t sixthArgument, void* decider, void* state) {
    if (activeHunt && activeHunt->isCalled(ship))
        return 0.0f;
    return reinforcementLeaves(ship, lowTime, highTime, flags, fifthArgument, sixthArgument, decider, state);
}

[[nodiscard]] bool groxEmpire(uint32_t politicalId) {
    game::Empire empire = game::empireOf(politicalId);
    return empire && empire.archetype() == Archetype::Grob;
}

[[nodiscard]] bool unfriendly(uint32_t politicalId) {
    return game::empireOf(politicalId) && !groxEmpire(politicalId) &&
           game::relationLevel(politicalId, game::playerPoliticalId()) < game::neutralRelation;
}
}

bool Hunt::prepare() {
    activeHunt = this;
    if (!map::space::reinforcementLeavesDecision.matches())
        return false;
    reinforcementLeaves = std::bit_cast<map::space::DecisionFunction>(*map::space::reinforcementLeavesDecision);
    *map::space::reinforcementLeavesDecision = std::bit_cast<void*>(&keepCalledShips);
    decisionReady = true;
    return true;
}

bool Hunt::isCalled(void* ship) const {
    if (!ship)
        return false;
    game::Ship called(ship);
    auto summoned = std::span(squads).first(size_t(squadCount));
    return called.kind() == ShipKind::Reinforcement &&
           std::ranges::find(summoned, called.politicalId()) != summoned.end();
}

bool Hunt::isHunter(void* ship) const {
    if (!hunting || !ship)
        return false;
    game::Ship hunter(ship);
    ShipKind kind = hunter.kind();
    return kind != ShipKind::Player && kind != ShipKind::Escort &&
           (hunter.politicalId() == colonyOwner || isCalled(ship));
}

bool Hunt::attacks(void* ship, void* target) const {
    if (!target || !isHunter(ship))
        return false;
    game::Ship prey(target);
    ShipKind kind = prey.kind();
    return (kind == ShipKind::Player || kind == ShipKind::Escort) && prey.canFight() &&
           !game::Ship(ship).underOrder(game::Ship::standDownOrder);
}

void* Hunt::targetFor(void* ship, bool distant, void* found) const {
    if (!found || !isHunter(ship))
        return found;
    game::Vector at = game::Ship(ship).position();
    float reach = settings.searchReaches[size_t(distant)];
    auto reachable = [&](game::Ship prey) {
        return attacks(ship, prey.raw()) && game::distance(at, prey.position()) <= reach;
    };
    std::array preyInOrder{grox, game::playerShip()};
    auto chosen = std::ranges::find_if(preyInOrder, reachable);
    return chosen != preyInOrder.end() ? chosen->fighter().raw() : found;
}

void Hunt::spot(std::span<char* const> escort) {
    auto isGrox = [](char* member) {
        game::Ship ship(member);
        return ship.canFight() && groxEmpire(ship.politicalId());
    };
    auto found = std::ranges::find_if(escort, isGrox);
    game::Ship seen(found != escort.end() ? *found : nullptr);
    if (seen == grox)
        return;
    seen.addReference();
    grox.release();
    grox = seen;
}

bool Hunt::begin() {
    uint32_t owner = game::planetOwnerPoliticalId();
    if (game::planetTechnology() != Technology::Empire || owner == uint32_t(-1) || owner == game::playerPoliticalId() ||
        !unfriendly(owner))
        return false;
    hunting = true;
    colonyOwner = owner;
    return true;
}

void Hunt::aimTurrets() const {
    std::array targets{grox, game::playerShip()};
    auto active = [](game::Turret turret) { return turret.active(); };
    for (game::City city : game::each<game::City>(map::civilization::cities)) {
        bool owned = city && city.alive() && city.politicalId() == colonyOwner;
        const game::Vector* middle = owned ? city.place().position() : nullptr;
        if (!middle)
            continue;
        auto inReach = [middle](game::Ship ship) {
            return ship.canFight() && game::surfaceDistance(*middle, ship.position()) <= settings.turretReach;
        };
        auto target = std::ranges::find_if(targets, inReach);
        if (target == targets.end())
            continue;
        void* aim = target->fighter().raw();
        for (game::Turret turret : city.turrets() | std::views::filter(active)) {
            void* held = turret.fighter().target();
            if (held != aim && (!held || game::playerSide(held)))
                turret.fighter().setTarget(aim);
        }
    }
}

void Hunt::leadReinforcements() const {
    game::Ship player = game::playerShip();
    for (game::Ship ship : game::each<game::Ship>(map::space::ships)) {
        if (!ship || !ship.alive() || ship.kind() != ShipKind::Reinforcement || !isHunter(ship.raw()))
            continue;
        game::Ship leader(ship.leader());
        if (leader == player && grox.canFight())
            ship.follow(grox);
        if (leader && leader != player && !leader.canFight() && player.canFight())
            ship.follow(player);
    }
}

void Hunt::callSquad(uint32_t politicalId) {
    game::StarRecord home = game::activeStarRecord();
    float* reach = game::universeValue(settings.interceptReachProperty);
    float gameReach = reach ? *reach : 0.0f;
    uint32_t owner = home.politicalId();
    if (reach)
        *reach = settings.neighbourReach;
    home.politicalId() = politicalId;
    map::space::callReinforcements();
    home.politicalId() = owner;
    if (reach)
        *reach = gameReach;
}

void Hunt::callSquads() {
    game::StarManager stars(map::space::starManager());
    game::StarRecord home = game::activeStarRecord();
    if (!decisionReady || !stars || !home)
        return;
    uint32_t player = game::playerPoliticalId();
    for (game::StarCell cell : stars.cells())
        for (game::StarRecord star : cell.stars()) {
            uint32_t politicalId = star ? star.politicalId() : uint32_t(-1);
            auto summoned = std::span(squads).first(size_t(squadCount));
            if (politicalId == uint32_t(-1) || politicalId == colonyOwner || politicalId == player ||
                std::ranges::find(summoned, politicalId) != summoned.end() || size_t(squadCount) == squads.size() ||
                game::distance(home.position(), star.position()) > settings.neighbourReach || !unfriendly(politicalId))
                continue;
            squads[size_t(squadCount++)] = politicalId;
            callSquad(politicalId);
        }
}

void Hunt::stop() {
    grox.release();
    grox = game::Ship();
    squads = {};
    squadCount = 0;
    hunting = false;
    colonyOwner = uint32_t(-1);
}

void Hunt::tick(std::span<char* const> escort) {
    if (!game::onPlanet()) {
        stop();
        return;
    }
    DWORD now = GetTickCount();
    if (LONG(now - nextThink) < 0 || game::paused())
        return;
    nextThink = now + settings.thinkMilliseconds;
    spot(escort);
    if (!hunting && grox && begin())
        callSquads();
    if (!hunting)
        return;
    aimTurrets();
    leadReinforcements();
}
}

