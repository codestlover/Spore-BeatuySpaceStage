
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <Windows.h>
#include <d3d9.h>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <numbers>
#include <span>

module beatuy.feature.civilization;

import beatuy.game;

namespace civilization {
namespace {
using game::Vector;

constexpr float pi = std::numbers::pi_v<float>;

Vector normalized(const Vector& direction, const Vector& fallback = {0, 0, 1}) {
    float length = game::length(direction);
    return length > 1e-6f ? direction * (1.0f / length) : fallback;
}

Vector interpolate(const Vector& from, const Vector& to, float fraction) {
    return from + (to - from) * fraction;
}

Vector surface(char* model, const Vector& point) {
    Vector out = point;
    map::planet::toSurface(model, &out, &point);
    return game::finite(out) ? out : point;
}

DWORD withAlpha(uint32_t packedColor, float alpha) {
    float value = float(packedColor >> 24) / 255.0f * alpha;
    value = value < 0 ? 0 : value > 1 ? 1 : value;
    return DWORD(value * 255.0f + 0.5f) << 24 | (packedColor & 0x00FFFFFF);
}

void states(IDirect3DDevice9* device) {
    device->SetVertexShader(nullptr);
    device->SetPixelShader(nullptr);
    device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
    for (DWORD stage = 0; stage < 4; ++stage)
        device->SetTexture(stage, nullptr);
    device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
    device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
    device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
    device->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
    device->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    device->SetRenderState(D3DRS_ZENABLE, FALSE);
    device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
    device->SetRenderState(D3DRS_STENCILENABLE, FALSE);
    device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
    device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
    device->SetRenderState(D3DRS_LIGHTING, FALSE);
    device->SetRenderState(D3DRS_FOGENABLE, FALSE);
    device->SetRenderState(D3DRS_CLIPPLANEENABLE, 0);
    device->SetRenderState(D3DRS_COLORWRITEENABLE, 0xF);
    device->SetRenderState(D3DRS_SRGBWRITEENABLE, FALSE);
    device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    device->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
}

bool visible(const game::Projection& projection, const Vector& point) {
    return game::dot(projection.eye() - point, normalized(point)) > -settings.rings.horizon;
}
}

void Marker::Layer::drawLayer(int, int, void**, void*) {
    owner.draw();
}

void Marker::push(const Vertex& vertex) {
    if (vertexCount < maximumVertices)
        vertices[vertexCount++] = vertex;
}

void Marker::segment(float fromX, float fromY, float toX, float toY, float halfWidth, DWORD core, DWORD edge) {
    float deltaX = toX - fromX, deltaY = toY - fromY;
    float length = std::sqrt(deltaX * deltaX + deltaY * deltaY);
    if (length < 0.01f)
        return;
    float normalX = -deltaY / length, normalY = deltaX / length;
    const float feather = settings.rings.feather;
    const std::array offsets{-(halfWidth + feather), -halfWidth, halfWidth, halfWidth + feather};
    const std::array colors{edge, core, core, edge};
    for (int strip = 0; strip < 3; ++strip) {
        Vertex fromNear{fromX + normalX * offsets[strip], fromY + normalY * offsets[strip], 0, 1, colors[strip]};
        Vertex fromFar{fromX + normalX * offsets[strip + 1], fromY + normalY * offsets[strip + 1], 0, 1,
                       colors[strip + 1]};
        Vertex toNear{toX + normalX * offsets[strip], toY + normalY * offsets[strip], 0, 1, colors[strip]};
        Vertex toFar{toX + normalX * offsets[strip + 1], toY + normalY * offsets[strip + 1], 0, 1, colors[strip + 1]};
        for (const Vertex& vertex : std::array{fromNear, fromFar, toNear, fromFar, toFar, toNear})
            push(vertex);
    }
}

void Marker::drawRing(const game::Projection& projection, const Ring& ring, float pulse) {
    const int points = settings.rings.points;
    float nearY = projection.nearDepth() * 1.05f;
    for (int i = 0; i < points; ++i) {
        if (!visible(projection, ring.points[i]) && !visible(projection, ring.points[i + 1]))
            continue;
        Vector from = projection.toCamera(ring.points[i]), to = projection.toCamera(ring.points[i + 1]);
        if (from.y < nearY && to.y < nearY)
            continue;
        if (from.y < nearY)
            from = interpolate(from, to, (nearY - from.y) / (to.y - from.y));
        else if (to.y < nearY)
            to = interpolate(to, from, (nearY - to.y) / (from.y - to.y));
        auto [fromX, fromY] = projection.toScreen(from);
        auto [toX, toY] = projection.toScreen(to);
        if (!std::isfinite(fromX) || !std::isfinite(fromY) || !std::isfinite(toX) || !std::isfinite(toY))
            continue;
        float angle = 2 * pi * float(i) / float(points);
        float sweep = std::pow(std::fmax(0.0f, std::cos(angle - pulse)), 12.0f);
        uint32_t color = settings.rings.color;
        segment(fromX, fromY, toX, toY, settings.rings.width * 0.5f, withAlpha(color, 0.80f + 0.20f * sweep),
                withAlpha(color, 0.0f));
    }
}

game::Viewer Marker::findViewer() {
    game::CameraManager cameras = game::GameModes::get().cameras();
    if (!cameras || !cameras.knowsViewer())
        return game::Viewer();
    return game::Viewer(cameras.viewer());
}

void Marker::draw() {
    if (!ringCount || LONG(GetTickCount() - updated) > LONG(settings.rings.staleMilliseconds))
        return;
    auto* device = static_cast<IDirect3DDevice9*>(*map::application::device);
    if (!device || device->TestCooperativeLevel() != D3D_OK)
        return;
    const game::Projection projection(findViewer());
    if (!projection.usable())
        return;
    vertexCount = 0;
    float pulse = float(GetTickCount() % 4000) / 4000.0f * 2 * pi;
    for (const Ring& ring : std::span(rings).first(size_t(ringCount)))
        drawRing(projection, ring, pulse);
    if (!vertexCount)
        return;
    IDirect3DStateBlock9* saved = nullptr;
    device->CreateStateBlock(D3DSBT_ALL, &saved);
    states(device);
    device->DrawPrimitiveUP(D3DPT_TRIANGLELIST, vertexCount / 3, vertices.data(), sizeof(Vertex));
    if (saved) {
        saved->Apply();
        saved->Release();
    }
}

void Marker::ensure() {
    Renderer renderer(map::application::renderer());
    if (!renderer)
        return;
    if (layerIndex >= 0 && renderer.layer(layerIndex) == &layer)
        return;
    layerIndex = -1;
    for (int index : std::array{29, 28, 27, 26, 25, 24, 23}) {
        void* existing = renderer.layer(index);
        if (existing && existing != &layer)
            continue;
        if (!existing)
            renderer.registerLayer(&layer, index);
        layerIndex = index;
        return;
    }
}

void Marker::begin() {
    ringCount = 0;
    updated = GetTickCount();
    eyeChecked = eyeKnown = false;
}

void Marker::add(game::GameData unit) {
    char* model = map::application::planetModel();
    game::Spatial spatial = unit.body();
    const Vector* at = spatial.position();
    if (ringCount >= settings.limits.maximumRings || !model || !at || !game::finite(*at))
        return;
    if (!eyeChecked) {
        eyeChecked = true;
        game::Viewer viewer = findViewer();
        eye = viewer ? viewer.row(3) : Vector{};
        eyeKnown = viewer && game::finite(eye);
    }
    float radius = spatial.boundingRadius() * 1.15f;
    radius = radius > 2.5f ? radius < 45.0f ? radius : 45.0f : 2.5f;
    Vector center = surface(model, *at);
    Vector up = normalized(center);
    if (eyeKnown && game::dot(eye - center, up) < -(radius + settings.rings.horizonRing))
        return;
    Vector tangent = normalized(game::cross(up, std::fabs(up.z) < 0.9f ? Vector{0, 0, 1} : Vector{1, 0, 0}), {1, 0, 0});
    Vector bitangent = normalized(game::cross(up, tangent));
    const int points = settings.rings.points;
    Ring& ring = rings[ringCount++];
    for (int i = 0; i <= points; ++i) {
        float angle = 2 * pi * float(i) / float(points);
        Vector point = center + (tangent * std::cos(angle) + bitangent * std::sin(angle)) * radius;
        ring.points[i] = normalized(point) * (game::length(surface(model, point)) + settings.rings.lift);
    }
}

void Marker::end() {
    if (ringCount)
        ensure();
}
}

