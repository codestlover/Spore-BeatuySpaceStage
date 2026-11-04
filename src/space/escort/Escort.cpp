
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <Windows.h>
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

module beatuy.feature.escort;

import beatuy.game;

namespace feature {
namespace {
using map::identifiers::ShipKind;

class Ship : public game::Ship {
public:
    using game::Ship::Ship;
    [[nodiscard]] bool isEscort() const {
        return kind() == ShipKind::Escort;
    }
};

[[nodiscard]] game::Array<Ship*> spaceShips() {
    return game::each<Ship>(map::space::ships);
}

[[nodiscard]] bool inPosse(uint32_t politicalId) {
    game::PlayerShipManager manager = game::shipManager();
    return manager && manager.escort().has(politicalId);
}

[[nodiscard]] bool listed(std::span<const uint32_t> identifiers, int count, uint32_t identifier) {
    std::span<const uint32_t> kept = identifiers.first(size_t(count));
    return std::ranges::find(kept, identifier) != kept.end();
}
}

void Escort::watch(game::Ship playerShip) {
    DWORD now = GetTickCount();
    if (LONG(now - nextWatch) < 0)
        return;
    game::Array<Ship*> ships = spaceShips();
    if (!ships.data())
        return;
    nextWatch = now + settings.watchMilliseconds;
    std::array<uint32_t, settings.maximumShips> current;
    std::array<uint32_t, settings.maximumFormer> former;
    int currentCount = 0, count = 0;
    for (Ship ship : ships) {
        if (!ship || ship == playerShip || !ship.alive())
            continue;
        uint32_t identifier = ship.identifier();
        ShipKind kind = ship.kind();
        if (kind == ShipKind::Escort && inPosse(ship.safePoliticalId())) {
            if (currentCount < settings.maximumShips)
                current[currentCount++] = identifier;
            continue;
        }
        bool was = listed(escortIdentifiers, escortIdentifierCount, identifier),
             known = listed(formerIdentifiers, formerCount, identifier);
        if ((was || known || kind == ShipKind::Escort) && count < settings.maximumFormer)
            former[count++] = identifier;
    }
    std::ranges::copy(std::span(current).first(size_t(currentCount)), escortIdentifiers.begin());
    escortIdentifierCount = currentCount;
    std::ranges::copy(std::span(former).first(size_t(count)), formerIdentifiers.begin());
    formerCount = count;
}

int Escort::ships(char** out, int maximum) {
    game::Ship playerShip = game::playerShip();
    int count = 0;
    if (!playerShip)
        return 0;
    for (Ship ship : spaceShips()) {
        if (count >= maximum)
            break;
        if (ship && ship != playerShip && ship.alive() && ship.isEscort() && inPosse(ship.safePoliticalId()))
            out[count++] = ship.raw();
    }
    return count;
}

void Escort::maskFormer() {
    maskedCount = 0;
    if (!formerCount)
        return;
    for (Ship ship : spaceShips()) {
        if (maskedCount >= settings.maximumShips)
            break;
        if (!ship || !listed(formerIdentifiers, formerCount, ship.identifier()))
            continue;
        masked[maskedCount++] = Masked{.ship = ship, .politicalId = ship.safePoliticalId()};
        ship.politicalId() = uint32_t(-1);
    }
}

void Escort::unmask() {
    for (const auto& [ship, politicalId] : std::span(masked).first(size_t(maskedCount)))
        ship.politicalId() = politicalId;
    maskedCount = 0;
}

bool Escort::blocks(void* target) {
    if (!target || !game::inSpace())
        return false;
    Ship ship(game::GameData(target).cast(map::identifiers::Type::Ship));
    return ship && ship.isEscort();
}

void Escort::tick() {
    game::Ship playerShip = game::inSpace() ? game::playerShip() : game::Ship();
    if (!playerShip)
        return;
    watch(playerShip);
}
}

