
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <Windows.h>
#include <cstdint>
#include <ranges>

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
    auto idle = [](game::Turret turret) {
        return turret.alive() && turret.cast(map::identifiers::Type::Turret) == turret.raw() &&
               !turret.fighter().target();
    };
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

void Colony::tick() {
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

