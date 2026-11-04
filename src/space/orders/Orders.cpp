
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <cstdint>
#include <optional>

module beatuy.feature.orders;

import beatuy.game;

namespace feature {
namespace {
void forgetDeparted(OrderBook& book) {
    if (book.standings().empty() || !game::inSpace() || !game::playerShip())
        return;
    const game::PlayerShipManager manager = game::shipManager();
    if (!manager)
        return;
    const game::Posse escort = manager.escort();
    book.keepOnly([&escort](uint32_t politicalId) { return escort.has(politicalId); });
}
}

void Orders::tick() {
    saving.tick(book);
    forgetDeparted(book);
    panel.tick(book, replies);
    replies.tick(book);
}

void Orders::onMouseDown(int button) {
    panel.onMouseDown(button);
}

void Orders::onLeavePlanet(bool) {
    behaviour.onLeavePlanet();
}

void Orders::close() {
    panel.close();
    replies.reset();
    behaviour.reset();
}

void Orders::prepare() {
    saving.prepare(book);
    behaviour.prepareRose();
}

void Orders::onGameUpdate(uint32_t elapsedLowerWord) {
    behaviour.update(book, elapsedLowerWord);
}

void* Orders::target(void* combatant, void* target) {
    return behaviour.orderedTarget(book, combatant).value_or(target);
}

std::optional<void*> Orders::orderedTarget(void* combatant) {
    return behaviour.orderedTarget(book, combatant);
}

void Orders::onHostile(void* combatant, void* attacker) {
    behaviour.onHostile(combatant, attacker);
}

void Orders::onRallyCall() {
    const game::PlayerShipManager manager = game::inSpace() ? game::shipManager() : game::PlayerShipManager();
    if (!manager)
        return;
    const game::Posse escort = manager.escort();
    const uint32_t player = game::playerPoliticalId();
    for (const game::Ship ship : game::each<game::Ship>(map::space::ships)) {
        if (!ship || ship.kind() != map::identifiers::ShipKind::Escort || !ship.alive())
            continue;
        const uint32_t politicalId = ship.safePoliticalId();
        if (politicalId == player || !escort.has(politicalId) || book.orderOf(politicalId) == Order::AttackNearby)
            continue;
        book.setOrder(politicalId, Order::AttackNearby);
    }
}

std::optional<bool> Orders::write(void* serializable, void* stream) {
    return saving.write(serializable, stream);
}

std::optional<bool> Orders::read(void* serializable, void* stream) {
    return saving.read(serializable, stream);
}
}

