
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <Windows.h>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <optional>

module beatuy.feature.glow;

import beatuy.core.mod;
import beatuy.game;

namespace glow {
namespace {
struct Settings {
    DWORD probeMilliseconds = 500, squadGraceMilliseconds = 5000, secondSquadMilliseconds = 400,
          squadCooldownMilliseconds = 60000;
    float fadeSeconds = 2.0f, mostLightChange = 4.0f;
    float tintLightShare = 0.5f, tintLightFloor = 0.6f, tintLightCeiling = 2.0f;
    int maximumSide = 2048;
};
constexpr Settings settings{};

Color tinted(const Color& native, const Color& tint) {
    const auto [tintHue, tintSaturation, tintValue] = toHueSaturationValue(tint);
    const float nativeValue = toHueSaturationValue(native).value;
    return fromHueSaturationValue(tintHue, tintSaturation, nativeValue * (0.4f + 0.6f * tintValue));
}

[[nodiscard]] std::optional<Color> warRed(game::TerrainState state, float altitude) {
    game::Image image = game::atmosphereImage(map::identifiers::Atmosphere::Red);
    if (!image || !image.readable(game::Image::byteSize))
        return std::nullopt;
    int width = image.width(), height = image.height();
    const uint8_t* pixels = image.pixels();
    if (width <= 0 || height <= 0 || width > settings.maximumSide || height > settings.maximumSide || !pixels)
        return std::nullopt;
    float timeOfDay = state.column();
    if (!std::isfinite(altitude) || !std::isfinite(timeOfDay))
        return std::nullopt;
    int row = int(floorf((1.0f - altitude) * float(height - 1))), column = int(floorf(float(width - 1) * timeOfDay));
    row = std::clamp(row, 0, height - 1);
    column = std::clamp(column, 0, width - 1);
    const uint8_t* pixel = pixels + (size_t(row) * width + column) * 4;
    if (!game::readable(pixel, 4))
        return std::nullopt;
    return Color{pixel[2] / 255.0f, pixel[1] / 255.0f, pixel[0] / 255.0f};
}

float changeRatio(float after, float before) {
    if (before < 0.004f)
        return after < 0.004f ? 1.0f : settings.mostLightChange;
    return std::min(after / before, settings.mostLightChange);
}

float softened(float ratio) {
    float factor = 1.0f + (ratio - 1.0f) * settings.tintLightShare;
    return std::clamp(factor, settings.tintLightFloor, settings.tintLightCeiling);
}
}
}

namespace feature {
void Glow::probe() {
    DWORD now = GetTickCount();
    if (LONG(now - defense.nextProbe) < 0)
        return;
    defense.nextProbe = now + glow::settings.probeMilliseconds;
    const float percent = game::planetCapture().percent;
    bool visible = percent > 0.0f;
    if (visible && !defense.bar &&
        (!defense.calledOnce || LONG(now - defense.calledTick) >= LONG(glow::settings.squadCooldownMilliseconds))) {
        defense.calledOnce = true;
        defense.calledTick = now;
        map::space::callReinforcements();
        defense.secondPending = true;
        defense.secondDue = now + glow::settings.secondSquadMilliseconds;
    }
    defense.bar = visible;
}

bool Glow::wanted() {
    if (game::planetTechnology() != map::identifiers::Technology::Empire)
        return false;
    uint32_t owner = game::planetOwnerPoliticalId(), player = game::playerPoliticalId();
    if (owner == uint32_t(-1) || owner == player)
        return false;
    probe();
    bool squad = defense.squadPlanet && defense.squadPlanet == game::activePlanet() &&
                 GetTickCount() - defense.squadTick < glow::settings.squadGraceMilliseconds;
    return defense.bar || squad;
}

void Glow::onSquad() {
    defense.squadTick = GetTickCount();
    defense.squadPlanet = game::activePlanet();
}

void Glow::onLeavePlanet(bool) {
    defense.secondPending = false;
    defense.calledOnce = false;
    war.level = 0.0f;
    defense.squadPlanet = game::Planet();
    defense.bar = false;
}

void Glow::beginBake() {
    ++bakes.depth;
}

void Glow::endBake() {
    --bakes.depth;
}

void Glow::onAtmosphere(char* state, float altitude) {
    change.state = nullptr;
    game::TerrainState terrain(state);
    if (!terrain || terrain != game::activeTerrainState())
        return;
    float* colour = terrain.atmosphere();
    const glow::Color native{colour[0], colour[1], colour[2]};
    glow::Color out = native;
    float tint[3] = {0.0f, 0.0f, 0.0f}, tintLevel = 0.0f;
    if (Mod::get().generator.tint(state, tint, tintLevel) && tintLevel > 0.0f)
        out = glow::interpolate(out, glow::tinted(native, glow::Color{tint[0], tint[1], tint[2]}), tintLevel);
    const glow::Color tinted = out;
    if (war.level > 0.0f && !bakes.depth) {
        if (const std::optional<glow::Color> red = glow::warRed(terrain, altitude))
            out = glow::interpolate(out, *red, war.level);
    }
    glow::Color ratio{1.0f, 1.0f, 1.0f};
    for (int i = 0; i < 3; ++i)
        ratio[i] = glow::softened(glow::changeRatio(tinted[i], native[i])) * glow::changeRatio(out[i], tinted[i]);
    auto finite = [](const glow::Color& color) { return std::ranges::all_of(color.channels, [](float channel) {
        return std::isfinite(channel);
    }); };
    if (!finite(out) || !finite(ratio))
        return;
    std::ranges::copy(out.channels, colour);
    change.state = state;
    change.ratio = ratio;
}

void Glow::onLighting(char* state, float* model) {
    if (!model || !state || state != change.state)
        return;
    for (int i = 0; i < 3; ++i) {
        const float light = std::clamp(model[i] * change.ratio[i], 0.0f, 1.0f);
        if (std::isfinite(light))
            model[i] = light;
    }
}

void Glow::tick() {
    DWORD now = GetTickCount();
    float elapsed = war.lastTick ? (now - war.lastTick) / 1000.0f : 0.0f;
    war.lastTick = now;
    if (!game::onPlanet()) {
        war.level = 0.0f;
        return;
    }
    if (game::paused())
        return;
    if (defense.secondPending && LONG(now - defense.secondDue) >= 0) {
        defense.secondPending = false;
        map::space::callReinforcements();
    }
    bool want = wanted();
    float step = std::min(elapsed, 0.1f) / glow::settings.fadeSeconds;
    float& level = war.level;
    level = want ? std::min(level + step, 1.0f) : std::max(level - step, 0.0f);
}
}

