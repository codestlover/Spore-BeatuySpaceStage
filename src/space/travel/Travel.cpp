
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <Windows.h>
#include <algorithm>
#include <array>
#include <cstdlib>

module beatuy.feature.travel;

import beatuy.game;

namespace feature {
namespace {
using map::identifiers::Context;

struct Destination {
    Context context;
    void* (*hovered)();
    bool (*arrived)(void* target);
};

constexpr auto destinations = std::to_array<Destination>({
    {.context = Context::Galaxy,
     .hovered = [] { return static_cast<void*>(game::hoveredStar().raw()); },
     .arrived = [](void* star) { return game::canEnterSystem() && game::activeStarRecord() == game::View(star); }},
    {.context = Context::System,
     .hovered = [] { return game::hoveredObject().cast(map::identifiers::Type::Planet); },
     .arrived = [](void* planet) { return game::canEnterPlanet() && game::activePlanet() == game::View(planet); }},
});

[[nodiscard]] constexpr const Destination* destinationIn(Context context) {
    const auto found = std::ranges::find(destinations, context, &Destination::context);
    return found != destinations.end() ? &*found : nullptr;
}
}

void Travel::disarm() {
    armed = nullptr;
    armedContext = Context::None;
    zoomed = false;
}

bool Travel::nearby(const POINT& first, const POINT& second) {
    return labs(first.x - second.x) <= settings.slopPixels && labs(first.y - second.y) <= settings.slopPixels;
}

void Travel::onMouseDown(int button) {
    if (button != int(map::identifiers::Button::Left) || !game::inSpace())
        return;
    const Context context = game::context();
    const Destination* destination = destinationIn(context);
    if (!destination)
        return;
    void* target = destination->hovered();
    POINT at{};
    GetCursorPos(&at);
    const DWORD now = GetTickCount();
    const DWORD gap = now - last.tick;
    const bool same = target && target == last.target && context == last.context && nearby(at, last.at);
    if (same && gap < settings.echoMilliseconds)
        return;
    if (same && gap <= GetDoubleClickTime()) {
        armed = target;
        armedContext = context;
        deadline = now + settings.travelMilliseconds;
        zoomed = false;
        last = Click{};
        return;
    }
    if (armed && target != armed)
        disarm();
    last = Click{.target = target, .context = context, .tick = now, .at = at};
}

void Travel::tick() {
    if (!armed)
        return;
    const DWORD now = GetTickCount();
    if (!game::inSpace() || game::context() != armedContext) {
        disarm();
        return;
    }
    if (LONG(now - deadline) > 0) {
        disarm();
        return;
    }
    if (zoomed)
        return;
    if (const Destination* destination = destinationIn(armedContext); !destination || !destination->arrived(armed))
        return;
    const game::Camera camera = game::spaceCamera();
    if (!camera)
        return;
    const float minimum = game::minimumZoom(armedContext);
    camera.zoomTarget() = minimum;
    camera.discreteZoom() = true;
    zoomed = true;
    deadline = now + settings.enterMilliseconds;
}
}

