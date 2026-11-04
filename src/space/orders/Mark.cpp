
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <ranges>
#include <span>

module beatuy.feature.orders;

import beatuy.game;

namespace feature::mark {
namespace {
constexpr uint32_t firstColorIdentifier = 0x053dbcf1;
constexpr size_t colorCount = 13;
constexpr std::array<std::array<float, 3>, colorCount> colors{{
    {0.5f, 0.5f, 0.5f},
    {0.685f, 0.425f, 0.0f},
    {0.878f, 0.0f, 1.0f},
    {0.0f, 1.0f, 0.878f},
    {0.474f, 0.871f, 0.031f},
    {0.62f, 0.539f, 1.0f},
    {1.0f, 0.663f, 0.173f},
    {1.0f, 1.0f, 0.0f},
    {1.0f, 0.08f, 0.25f},
    {0.0f, 0.46f, 1.0f},
    {0.0f, 0.395f, 0.009f},
    {0.562f, 0.0f, 0.0f},
    {0.294f, 0.08f, 0.88f},
}};
constexpr std::array<uint32_t, colorCount + 1> effects{0x6615c708, 0x6615c70b, 0x6615c70a, 0x6615c70d, 0x6615c70c,
                                                       0x6615c70f, 0x6615c70e, 0x6615c701, 0x6615c700, 0x6515c596,
                                                       0x6515c597, 0x6515c594, 0x6515c595, 0x696e4bd4};
constexpr size_t white = colorCount, whitening = 2;
constexpr float firstScale = 1.3f, scaleStep = 0.3f;
}

struct Transform {
    uint16_t flags;
    uint16_t count;
    std::array<float, 3> position;
    float scale;
    std::array<float, 9> rotation;
};
static_assert(sizeof(Transform) == 0x38 && offsetof(Transform, scale) == 0x10);

struct TransformCatcher {
    Transform caught{};
    bool kept = false;

    virtual int addReference() {
        return 1;
    }
    virtual int release() {
        return 1;
    }
    virtual bool start(int) {
        return true;
    }
    virtual bool stop(int) {
        return true;
    }
    virtual bool running() {
        return true;
    }
    virtual void reserved() {}
    virtual void setSourceTransform(const Transform* transform) {
        caught = *transform;
        kept = true;
    }
};

namespace {
size_t colorOf(uint32_t politicalId) {
    const game::Empire empire = game::empireOf(politicalId);
    if (!empire)
        return 0;
    const uint32_t place = empire.colorIdentifier() - firstColorIdentifier - 1;
    if (place < colorCount)
        return place;
    std::array<float, 3> shown{};
    if (!empire.colour(shown))
        return 0;
    const auto distance = [&shown](const std::array<float, 3>& color) {
        float sum = 0.0f;
        for (size_t channel = 0; channel < color.size(); ++channel)
            sum += (color[channel] - shown[channel]) * (color[channel] - shown[channel]);
        return sum;
    };
    return size_t(std::ranges::min_element(colors, {}, distance) - colors.begin());
}
}
}

namespace feature {
void AttackMarks::begin() {
    requestCount = 0;
    for (Mark& mark : std::span(marks).first(count))
        mark.shown = false;
}

void AttackMarks::show(uint32_t politicalId, void* target) {
    void* spatial = game::Combatant(target).spatial();
    if (spatial && requestCount < requests.size())
        requests[requestCount++] = Request{politicalId, spatial};
}

void AttackMarks::end() {
    const std::span<const Request> asked = std::span(requests).first(requestCount);
    for (const Request& request : asked) {
        const auto sharing = [&request](const Request& other) { return other.spatial == request.spatial; };
        const bool crowded = size_t(std::ranges::count_if(asked, sharing)) > mark::whitening;
        if (crowded && &*std::ranges::find_if(asked, sharing) != &request)
            continue;
        const auto before = asked.first(size_t(&request - asked.data()));
        const float scale = mark::firstScale + mark::scaleStep * float(std::ranges::count_if(before, sharing));
        place(request.politicalId, std::array{mark::colorOf(request.politicalId), mark::white}[size_t(crowded)],
              request.spatial, scale);
    }
    const auto hidden = std::ranges::remove_if(std::span(marks).first(count), [](const Mark& mark) {
        if (mark.shown)
            return false;
        mark.effect.stop();
        mark.effect.release();
        return true;
    });
    count -= hidden.size();
}

void AttackMarks::clear() {
    begin();
    end();
}

void AttackMarks::place(uint32_t politicalId, size_t color, void* spatial, float scale) {
    const std::span<Mark> kept = std::span(marks).first(count);
    auto found = std::ranges::find(kept, politicalId, &Mark::politicalId);
    if (found != kept.end() && found->color != color) {
        found->effect.stop();
        found->effect.release();
        *found = marks[--count];
        found = std::span(marks).first(count).end();
    }
    if (found == std::span(marks).first(count).end()) {
        if (count == marks.size())
            return;
        const game::VisualEffect effect = game::EffectsManager::get().create(mark::effects[color]);
        if (!effect)
            return;
        effect.start();
        marks[count] = Mark{.politicalId = politicalId, .color = color, .effect = effect, .shown = false};
        found = std::span(marks).first(++count).end() - 1;
    }
    mark::TransformCatcher catcher;
    void* pointer = &catcher;
    map::application::placeEffectOver(spatial, &pointer);
    if (catcher.kept) {
        catcher.caught.scale *= scale;
        found->effect.setSourceTransform(&catcher.caught);
    }
    found->shown = true;
}
}

