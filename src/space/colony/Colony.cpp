
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <Windows.h>
#include <cstdint>
#include <ranges>
#include <span>

module beatuy.feature.colony;

import beatuy.game;

namespace feature {
int Colony::aimTurrets(game::Ship ship, uint32_t player) {
    game::Array<game::City*> cities = game::each<game::City>(map::civilization::cities);
    if (!cities.data())
        return 0;
    const game::Vector shipPosition = game::shipPosition();
    auto hostile = [&](game::City city) {
        if (!city.alive())
            return false;
        uint32_t politicalId = city.politicalId();
        if (politicalId == player || politicalId == uint32_t(-1) || !game::atWar(politicalId, player))
            return false;
        const game::Vector* center = city.place().position();
        return center && game::surfaceDistance(*center, shipPosition) <= settings.reach;
    };
    auto idle = [](game::Turret turret) { return turret.active() && !turret.fighter().target(); };
    void* prey = ship.fighter().raw();
    int aimed = 0;
    for (game::City city : cities | std::views::filter(hostile)) {
        for (game::Turret turret : city.turrets() | std::views::filter(idle)) {
            turret.fighter().setTarget(prey);
            ++aimed;
        }
    }
    return aimed;
}

void Colony::onContact(void* screen, uint32_t politicalId) {
    contact.onHeader(screen, politicalId);
}

void Colony::onContactUnload() {
    contact.onUnload();
}

void Colony::close() {
    crusade.stop();
}

bool Colony::pardons(uint32_t who, uint32_t cause, map::identifiers::Relation event, float scale) {
    return crusade.pardons(who, cause, event, scale);
}

std::span<game::RaidShip> Colony::lighten(const void* raid, char* list, std::span<game::RaidShip> changed) const {
    return crusade.lighten(raid, list, changed);
}

void Colony::restore(std::span<const game::RaidShip> changed) {
    Crusade::restore(changed);
}

bool Colony::keepsGrox(char* civ) const {
    game::CivData data(civ);
    std::span<game::Key> keys = data ? data.modelKeys() : std::span<game::Key>();
    if (keys.empty() || game::groxEmpire(data.politicalId()))
        return false;
    game::Empire grox = game::findGrox();
    const game::Key* hall = grox ? game::cultureKey(grox.culture(), map::space::colonyModelTypes[0]) : nullptr;
    return hall && hall->instance && hall->instance != uint32_t(-1) && keys.front() == *hall;
}

void* Colony::aim(void* combatant, void* target) const {
    return crusade.aim(combatant, target);
}

bool Colony::steer(char* ship, game::Vector& point) const {
    return crusade.steer(ship, point);
}

void Colony::tick() {
    contact.tick();
    crusade.tick();
    if (!game::onPlanet() || game::planetTechnology() != map::identifiers::Technology::Empire)
        return;
    game::Ship ship = game::playerShip();
    DWORD now = GetTickCount();
    if (!ship || LONG(now - nextAim) < 0)
        return;
    nextAim = now + settings.aimMilliseconds;
    if (ship.fighter().health() <= 0.0f)
        return;
    aimTurrets(ship, game::playerPoliticalId());
}
}

