
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <Windows.h>
#include <algorithm>
#include <array>

module beatuy.feature.exit;

import beatuy.game;

namespace feature {
namespace {
using map::identifiers::Context;

[[nodiscard]] bool held(int key) {
    return GetAsyncKeyState(key) & 0x8000;
}

[[nodiscard]] bool gameFocused() {
    DWORD processIdentifier = 0;
    HWND window = GetForegroundWindow();
    return window && GetWindowThreadProcessId(window, &processIdentifier) && processIdentifier == GetCurrentProcessId();
}
}

bool QuickExit::pay() {
    const game::Ship ship = game::playerShip();
    if (!ship)
        return false;
    const float energy = ship.energy();
    const float cost = ship.maximumEnergy() * settings.energyShare;
    if (energy < cost)
        return false;
    game::changeEnergy(ship, -cost);
    return true;
}

bool QuickExit::zoomOut(DWORD) {
    const game::Camera camera = game::spaceCamera();
    if (!camera || !pay())
        return false;
    const float maximum = game::maximumZoom(Context::System);
    camera.zoomTarget() = maximum;
    camera.discreteZoom() = true;
    return true;
}

bool QuickExit::climbOut(DWORD now) {
    if (!game::planetCamera() || !pay())
        return false;
    climbing = true;
    climbUntil = now + settings.climbMilliseconds;
    return true;
}

bool QuickExit::request() {
    struct Way {
        Context context;
        bool (QuickExit::*leave)(DWORD now);
    };
    static constexpr auto ways = std::to_array<Way>({
        {.context = Context::Planet, .leave = &QuickExit::climbOut},
        {.context = Context::System, .leave = &QuickExit::zoomOut},
    });
    const DWORD now = GetTickCount();
    if (!game::inSpace() || LONG(now - lastRequest) < LONG(settings.cooldownMilliseconds) || climbing)
        return false;
    const auto way = std::ranges::find(ways, game::context(), &Way::context);
    if (way == ways.end())
        return false;
    lastRequest = now;
    return (this->*way->leave)(now);
}

bool QuickExit::onWheel(int delta) {
    return delta < 0 && held(VK_SHIFT) && request();
}

void QuickExit::tick() {
    const bool down = gameFocused() && held(VK_SHIFT) && held(VK_SPACE);
    if (down && !keyDown)
        request();
    keyDown = down;
    if (!climbing)
        return;
    const game::Camera camera = game::planetCamera();
    if (!game::inSpace() || game::context() != Context::Planet || !camera || LONG(GetTickCount() - climbUntil) > 0) {
        climbing = false;
        return;
    }
    camera.climb() = settings.climb;
}
}

