
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module beatuy.core.mod;

import beatuy.core.feature;
import beatuy.game;

namespace {
template <class Moment, class... Types> void inOrder(Moment moment, Types&... features) {
    (moment(features), ...);
}
}

Mod& Mod::get() {
    static Mod mod;
    return mod;
}

void Mod::tick() {
    inOrder([](Feature auto& feature) { feature.tick(); }, picker, travel, quickExit, civilization, colony, generator,
            tribe, escort, orders, glow, radio);
}

void Mod::leavePlanet(bool destroyed) {
    inOrder([destroyed](LeavesPlanet auto& feature) { feature.onLeavePlanet(destroyed); }, civilization, glow,
            generator, radio, orders);
    game::markLeaving();
}

void Mod::mouseDown(int button) {
    inOrder([button](TakesClicks auto& feature) { feature.onMouseDown(button); }, travel, generator, orders);
}

