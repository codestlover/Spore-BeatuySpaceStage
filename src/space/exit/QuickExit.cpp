
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <Windows.h>
#include <algorithm>
#include <array>
#include <cmath>

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

[[nodiscard]] double preciseMilliseconds() {
    LARGE_INTEGER count{}, rate{};
    QueryPerformanceCounter(&count);
    QueryPerformanceFrequency(&rate);
    return double(count.QuadPart) * 1000.0 / double(rate.QuadPart);
}

[[nodiscard]] DWORD later(DWORD first, DWORD second) {
    return LONG(second - first) > 0 ? second : first;
}
}

bool QuickExit::pay() {
    game::Ship ship = game::playerShip();
    if (!ship)
        return false;
    float energy = ship.energy();
    float cost = ship.maximumEnergy() * settings.energyShare;
    if (energy < cost)
        return false;
    game::changeEnergy(ship, -cost);
    return true;
}

float QuickExit::speedOf(void* ship) const {
    if (!climbing || game::playerShip().raw() != ship)
        return 1.0f;
    DWORD flown = GetTickCount() - (climbUntil - settings.climbMilliseconds);
    float share = std::min(float(flown) / float(settings.climbEaseMilliseconds), 1.0f);
    return 1.0f + (settings.climbSpeed - 1.0f) * share * share * (3.0f - 2.0f * share);
}

bool QuickExit::zoomOut(DWORD) {
    game::Camera camera = game::spaceCamera();
    if (!camera || !pay())
        return false;
    float maximum = game::maximumZoom(Context::System);
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
    DWORD now = GetTickCount();
    if (!game::inSpace())
        return false;
    auto way = std::ranges::find(ways, game::context(), &Way::context);
    if (way == ways.end())
        return false;
    holdRush(now);
    if (LONG(now - lastRequest) < LONG(settings.cooldownMilliseconds) || climbing)
        return false;
    lastRequest = now;
    if (!(this->*way->leave)(now))
        return false;
    startRush(now, std::array{settings.zoomRushMilliseconds, settings.climbMilliseconds}[size_t(climbing)]);
    return true;
}

void QuickExit::startRush(DWORD now, DWORD milliseconds) {
    DWORD until = now + milliseconds + settings.rushHoldMilliseconds;
    if (!rush.root) {
        game::AlliesPanel panel = game::SpaceInterface::get().alliesPanel();
        game::Window root = panel ? panel.findWindow(game::AlliesPanel::containerControl) : game::Window();
        for (int depth = 0; root && root.parent() && depth < settings.rootDepth; ++depth)
            root = root.parent();
        game::Window layer = root ? game::newOverlay(game::Rectangle{}) : game::Window();
        if (!layer)
            return;
        layer.setShade(0x00ffffff);
        root.add(layer);
        root.addReference();
        rush.root = root;
        rush.layer = layer;
        rush.drawn = preciseMilliseconds();
        rush.until = until;
    }
    rush.until = later(rush.until, until);
}

void QuickExit::sound(bool flying) {
    if (flying == warp.heard)
        return;
    if (!warp.track)
        warp.track = game::newAudioTrack();
    if (flying && warp.track)
        game::playAudio(settings.rushSound, warp.track);
    if (!flying)
        game::stopAudio(warp.track);
    warp.heard = flying;
}

void QuickExit::close() {
    endRush();
    sound(false);
    climbing = false;
}

void QuickExit::holdRush(DWORD now) {
    if (rush.root)
        rush.until = later(rush.until, now + settings.rushHoldMilliseconds);
}

game::StandardDrawable QuickExit::rushLook(size_t frame) {
    game::StandardDrawable& look = rush.looks[frame];
    if (!look && !rush.pictures[frame]) {
        rush.pictures[frame] = game::loadPicture({.instance = settings.rushPictures[frame],
                                                  .type = settings.rushPictureType,
                                                  .group = settings.rushPictureGroup});
        look = game::newPictureLook(rush.pictures[frame]);
    }
    return look;
}

void QuickExit::drawRush(DWORD now) {
    if (!rush.root)
        return;
    double moment = preciseMilliseconds();
    float passed = float(moment - rush.drawn);
    rush.drawn = moment;
    bool wanted = LONG(now - rush.until) < 0;
    float change = wanted ? passed / float(settings.rushInMilliseconds) : -passed / float(settings.rushOutMilliseconds);
    rush.light = std::clamp(rush.light + change, 0.0f, 1.0f);
    double turned = moment / settings.rushFrameMilliseconds;
    double whole = std::floor(turned);
    size_t frame = size_t(std::fmod(whole, double(settings.rushPictures.size())));
    game::StandardDrawable look = rushLook(frame);
    if (!game::inSpace() || (!wanted && rush.light <= 0.0f) || !look) {
        endRush();
        return;
    }
    if (frame != rush.shown)
        rush.layer.setDrawable(look);
    rush.shown = frame;
    game::Rectangle screen = *rush.root.area();
    float width = screen.right - screen.left, height = screen.bottom - screen.top;
    float grown = std::pow(settings.rushStep, float(turned - whole));
    float spread = width * (grown - 1.0f) / 2, rise = height * (grown - 1.0f) / 2;
    rush.layer.setArea({-spread, -rise, width + spread, height + rise});
    float light = rush.light * std::pow(grown / settings.rushStep, settings.rushPower);
    rush.layer.setShade(uint32_t(light * 255.0f) << 24 | 0x00ffffff);
}

void QuickExit::endRush() {
    if (rush.layer) {
        rush.root.dispose(rush.layer);
        rush.layer.release();
    }
    for (game::StandardDrawable look : rush.looks)
        if (look)
            look.release();
    for (void* picture : rush.pictures)
        game::releasePicture(picture);
    if (rush.root)
        rush.root.release();
    rush = Rush{};
}

bool QuickExit::onWheel(int delta) {
    return delta < 0 && held(VK_SHIFT) && request();
}

void QuickExit::tick() {
    bool down = gameFocused() && held(VK_SHIFT) && held(VK_SPACE);
    if (down && !keyDown)
        request();
    keyDown = down;
    DWORD now = GetTickCount();
    drawRush(now);
    sound(rush.root && LONG(now - rush.until) < 0);
    if (!climbing)
        return;
    game::Camera camera = game::planetCamera();
    if (!game::inSpace() || game::context() != Context::Planet || !camera || LONG(now - climbUntil) > 0) {
        climbing = false;
        if (rush.root && LONG(rush.until - now) > LONG(settings.rushHoldMilliseconds))
            rush.until = now + settings.rushHoldMilliseconds;
        return;
    }
    camera.climb() = settings.climb;
}
}

