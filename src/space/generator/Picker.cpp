
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <Windows.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <concepts>
#include <cstdint>
#include <optional>

module beatuy.feature.generator;

import beatuy.core.mod;
import beatuy.game;

namespace picker {
namespace {
struct Settings {
    uint32_t layer = 0x05b598fa, configuration = 0xab14d8fc;
    int swatches = 15;
    uint32_t colourPicked = 0x90e08f60;
    float swatch = 46.0f, padding = 12.0f;
    float lift = 20.0f, gap = 10.0f, margin = 8.0f;
    float fullSizeWithin = 70.0f, reach = 200.0f, smallest = 0.3f;
    uint32_t hostFill = 0xd0141c26, clear = 0x00000000;
    float effectSeconds = 0.2f, springDamping = 10.0f, fadeDamping = 0.1f, growFrom = 0.5f;
    uint32_t openSound = 0x38eb66f8, pickSound = 0x2570ae6d;
    DWORD settleMilliseconds = 400, hideMilliseconds = 250;
};
constexpr Settings settings{};

game::Window newWindow(const game::Rectangle& area, uint32_t fill) {
    void* allocator = map::userInterface::windowAllocator();
    char* object = map::userInterface::windowNew(game::Window::objectByteSize, 4, "UI/Window", allocator);
    if (!object)
        return game::Window();
    map::userInterface::windowConstructor(object);
    game::Window window(object + game::Window::faceOffset);
    window.addReference();
    window.setArea(area);
    window.setFill(fill);
    return window;
}

template <std::derived_from<game::WindowEffect> Effect> Effect newEffect(const char* name, const auto& constructor) {
    void* allocator = map::userInterface::windowAllocator();
    char* memory = map::userInterface::windowNew(Effect::byteSize, game::WindowEffect::alignment, name, allocator);
    if (!memory)
        return Effect();
    Effect effect(constructor(memory));
    effect.addReference();
    return effect;
}

void setUp(game::WindowEffect effect, game::TwoStateEffect::Interpolation interpolation, float damping) {
    const game::TwoStateEffect face = effect.twoState();
    face.setTime(settings.effectSeconds);
    face.setEase(0.0f, 0.0f);
    face.setInterpolation(interpolation);
    face.setDamping(damping);
    face.setTrigger(game::TwoStateEffect::Trigger::Invisible);
}

void addEffects(game::Window window) {
    using enum game::TwoStateEffect::Interpolation;
    const game::InflateEffect inflate =
        newEffect<game::InflateEffect>("UI/InflateEffect", map::userInterface::inflateEffectConstructor);
    if (inflate) {
        setUp(inflate, DampedSpring, settings.springDamping);
        inflate.setScale(settings.growFrom);
    }
    const game::FadeEffect fade =
        newEffect<game::FadeEffect>("UI/FadeEffect", map::userInterface::fadeEffectConstructor);
    if (fade)
        setUp(fade, Linear, settings.fadeDamping);
    for (const game::WindowEffect effect : std::array<game::WindowEffect, 2>{inflate, fade}) {
        if (!effect)
            continue;
        window.addProcedure(effect.raw());
        effect.release();
    }
}

float scaleAt(float distance) {
    if (distance <= settings.fullSizeWithin)
        return 1.0f;
    const float share = (distance - settings.fullSizeWithin) / (settings.reach - settings.fullSizeWithin);
    return 1.0f - (1.0f - settings.smallest) * std::min(share, 1.0f);
}

bool gameFocused() {
    DWORD process = 0;
    HWND window = GetForegroundWindow();
    return window && GetWindowThreadProcessId(window, &process) && process == GetCurrentProcessId();
}

Settling settlingOf(game::ColorPicker palette) {
    const auto flies = [](game::ColorSwatch shade) { return shade && shade.flying(); };
    Settling settling;
    for (const game::ColorSwatch swatch : palette.swatches()) {
        if (!swatch)
            continue;
        settling.pressed = settling.pressed || swatch.pressed();
        settling.shadesOpen = settling.shadesOpen || swatch.shadesOpen();
        settling.flying = settling.flying || std::ranges::any_of(swatch.shades(), flies);
    }
    return settling;
}
}
}

namespace feature {
void __cdecl Picker::onMessage(uint32_t identifier, void* message, void*) {
    Mod::get().picker.receive(identifier, message);
}

void Picker::receive(uint32_t identifier, void* message) {
    Session& current = session;
    if (identifier != picker::settings.colourPicked || !message || !current.picker || current.hiding)
        return;
    game::PickedMessage pickedMessage(message);
    if (pickedMessage.picker() != current.picker.objectFace())
        return;
    current.packedColor = pickedMessage.colour();
    current.picked = true;
    current.newPick = true;
}

void Picker::deliverPick(DWORD now) {
    Session& current = session;
    current.newPick = false;
    current.settleFrom = now;
    game::playAudio(picker::settings.pickSound);
    if (current.chosen)
        current.chosen(current.packedColor, current.context);
}

void Picker::settle(DWORD now) {
    Session& current = session;
    const picker::Settling settling = picker::settlingOf(current.picker);
    if (!current.picked)
        return;
    if (settling.holds())
        current.settleFrom = now;
    else if (now - current.settleFrom >= picker::settings.settleMilliseconds)
        hide(now);
}

void Picker::hide(DWORD now) {
    Session& current = session;
    current.hiding = true;
    current.closeAt = now + picker::settings.hideMilliseconds;
    if (current.host)
        current.host.setFlag(game::Window::visibleFlag, false);
}

void Picker::teardown() {
    Session& current = session;
    if (current.picker) {
        map::userInterface::pickerDestroy(current.picker.raw());
        current.picker.release();
    }
    if (current.handler)
        if (game::MessageManager manager = game::MessageManager::get())
            manager.removeHandler(&onMessage, picker::settings.colourPicked);
    if (current.layer && current.shown)
        current.layer.dispose(current.host);
    for (game::Window window : std::array{current.container, current.host})
        if (window)
            window.release();
    current = Session{};
}

bool Picker::build() {
    const picker::Settings& settings = picker::settings;
    Session& current = session;
    char* layouts = map::userInterface::layoutManager();
    current.layer = game::Window(layouts ? map::userInterface::layerWindow(layouts, settings.layer) : nullptr);
    if (!current.layer)
        return false;
    const game::Rectangle* area = current.layer.realArea();
    if (!area || !(area->right > area->left) || !(area->bottom > area->top))
        return false;
    current.width = 2 * settings.padding + settings.swatches * settings.swatch;
    current.height = 2 * settings.padding + settings.swatch;
    current.host = picker::newWindow(game::Rectangle{0, 0, current.width, current.height}, settings.hostFill);
    current.container = picker::newWindow(game::Rectangle{settings.padding, settings.padding,
                                                          settings.padding + settings.swatches * settings.swatch,
                                                          settings.padding + settings.swatch},
                                          settings.clear);
    if (!current.host || !current.container)
        return false;
    if (!follow())
        return false;
    current.host.addProcedure(&hostProcedure);
    current.layer.add(current.host);
    current.host.add(current.container);
    current.shown = true;
    current.layer.toFront(current.host);

    current.picker =
        game::ColorPicker(map::application::allocate(game::ColorPicker::byteSize, "UI/ColorPicker", 0, 0, nullptr, 0));
    if (!current.picker)
        return false;
    map::userInterface::pickerConstructor(current.picker.raw());
    current.picker.addReference();
    map::userInterface::pickerLoad(current.picker.raw(), current.container.raw(), settings.configuration, 0, nullptr);
    if (!current.picker.configuration() || current.picker.swatches().data() == current.picker.swatches().dataEnd())
        return false;
    map::userInterface::pickerSetVisible(current.picker.raw(), true);
    map::userInterface::pickerSetColor(current.picker.raw(), current.color.data());
    for (const game::ColorSwatch swatch : current.picker.swatches())
        if (swatch && current.swatchCount < current.swatchAreas.size())
            current.swatchAreas[current.swatchCount++] = swatch.area();
    resize(picker::scaleAt(game::distance(game::shipPosition(), current.place)));
    follow();
    current.host.setFlag(game::Window::visibleFlag, false);
    picker::addEffects(current.host);
    current.host.setFlag(game::Window::visibleFlag, true);
    if (game::MessageManager manager = game::MessageManager::get()) {
        manager.addHandler(&onMessage, settings.colourPicked);
        current.handler = true;
    }
    current.lastTick = GetTickCount();
    game::playAudio(settings.openSound);
    return true;
}

bool Picker::follow() {
    const picker::Settings& settings = picker::settings;
    Session& current = session;
    const game::Rectangle* area = current.layer ? current.layer.realArea() : nullptr;
    const game::Projection projection(game::activeViewer());
    if (!area || !projection.usable())
        return false;
    const game::Vector above = current.place + game::unit(current.place) * settings.lift;
    const std::optional<game::Projection::ScreenPoint> point = projection.project(above);
    if (!point)
        return false;
    const float screenWidth = area->right - area->left, screenHeight = area->bottom - area->top;
    const float x = area->left + point->x * screenWidth / projection.viewportWidth();
    const float y = area->top + point->y * screenHeight / projection.viewportHeight();
    if (x < area->left || x > area->right || y < area->top || y > area->bottom)
        return false;
    const float left = std::clamp(x - current.width / 2, area->left + settings.margin,
                                  area->right - settings.margin - current.width);
    const float top = std::clamp(y - settings.gap - current.height, area->top + settings.margin,
                                 area->bottom - settings.margin - current.height);
    current.host.setArea(game::Rectangle{left, top, left + current.width, top + current.height});
    return true;
}

void Picker::resize(float scale) {
    const picker::Settings& settings = picker::settings;
    Session& current = session;
    current.scale = scale;
    current.width = (2 * settings.padding + settings.swatches * settings.swatch) * scale;
    current.height = (2 * settings.padding + settings.swatch) * scale;
    current.container.setArea(game::Rectangle{settings.padding * scale, settings.padding * scale,
                                              (settings.padding + settings.swatches * settings.swatch) * scale,
                                              (settings.padding + settings.swatch) * scale});
    size_t index = 0;
    for (const game::ColorSwatch swatch : current.picker.swatches()) {
        if (index >= current.swatchCount)
            break;
        const game::Rectangle& area = current.swatchAreas[index++];
        if (swatch)
            map::userInterface::swatchSetArea(swatch.raw(), area.left * scale, area.top * scale, area.right * scale,
                                              area.bottom * scale, true);
    }
}

bool Picker::reaches(const game::Vector& place) const {
    return game::distance(game::shipPosition(), place) <= picker::settings.reach;
}

void Picker::cancel() {
    Session& current = session;
    if (current.pending && !current.picker)
        close();
    else if (current.picker && !current.hiding)
        hide(GetTickCount());
}

bool Picker::open(const float color[3], const game::Vector& place, Chosen chosen, void* context) {
    Session& current = session;
    if (current.pending || current.picker)
        return false;
    current = Session{};
    current.pending = true;
    std::ranges::copy_n(color, 3, current.color.begin());
    current.place = place;
    current.chosen = chosen;
    current.context = context;
    return true;
}

bool Picker::isOpen() const {
    return session.pending || session.picker;
}

void Picker::close() {
    const Session& current = session;
    if (!current.pending && !current.picker && !current.host)
        return;
    teardown();
}

void Picker::tick() {
    Session& current = session;
    if (!current.pending && !current.picker)
        return;
    if (!game::onPlanet()) {
        close();
        return;
    }
    if (current.pending) {
        if (GetAsyncKeyState(VK_LBUTTON) & 0x8000)
            return;
        current.pending = false;
        if (!build())
            close();
        return;
    }
    DWORD now = GetTickCount();
    map::userInterface::pickerUpdate(current.picker.raw(), int(now - current.lastTick));
    current.lastTick = now;
    if (current.newPick) {
        deliverPick(now);
        if (!current.picker)
            return;
    }
    const bool escape = picker::gameFocused() && (GetAsyncKeyState(VK_ESCAPE) & 0x8000);
    const bool escapePressed = escape && !current.escapeDown;
    current.escapeDown = escape;
    if (current.hiding) {
        if (LONG(now - current.closeAt) >= 0)
            close();
    } else if (current.cancelled || escapePressed) {
        hide(now);
    } else if (!reaches(current.place)) {
        hide(now);
    } else {
        if (const float scale = picker::scaleAt(game::distance(game::shipPosition(), current.place));
            std::abs(scale - current.scale) > 0.01f)
            resize(scale);
        if (follow())
            settle(now);
        else
            hide(now);
    }
}
}

