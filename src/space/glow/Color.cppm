
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <array>
#include <cmath>

export module beatuy.feature.glow:color;

namespace glow {
struct Color {
    std::array<float, 3> channels;

    constexpr float& operator[](int channel) {
        return channels[channel];
    }
    constexpr float operator[](int channel) const {
        return channels[channel];
    }
};

struct HueSaturationValue {
    float hue, saturation, value;
};

constexpr Color interpolate(const Color& from, const Color& to, float fraction) {
    return {from[0] + (to[0] - from[0]) * fraction, from[1] + (to[1] - from[1]) * fraction,
            from[2] + (to[2] - from[2]) * fraction};
}

inline HueSaturationValue toHueSaturationValue(const Color& color) {
    enum Channel { Red, Green, Blue };
    struct Sextant {
        float start;
        Channel up, down;
    };
    static constexpr auto brightest =
        std::to_array<Sextant>({{0.0f, Green, Blue}, {2.0f, Blue, Red}, {4.0f, Red, Green}});
    float highest = fmaxf(color[Red], fmaxf(color[Green], color[Blue])),
          lowest = fminf(color[Red], fminf(color[Green], color[Blue])), difference = highest - lowest;
    HueSaturationValue hueSaturationValue{
        .hue = 0.0f, .saturation = highest > 0.0f ? difference / highest : 0.0f, .value = highest};
    if (difference <= 0.0f)
        return hueSaturationValue;
    int channel = Red;
    while (channel < Blue && highest != color[channel])
        ++channel;
    const Sextant& from = brightest[channel];
    float& hue = hueSaturationValue.hue;
    hue = from.start + (color[from.up] - color[from.down]) / difference;
    hue /= 6.0f;
    if (hue < 0.0f)
        hue += 1.0f;
    return hueSaturationValue;
}

inline Color fromHueSaturationValue(float hue, float saturation, float value) {
    enum Part { Full, Lowest, Falling, Rising };
    static constexpr auto sextants = std::to_array<std::array<Part, 3>>({{Full, Rising, Lowest},
                                                                          {Falling, Full, Lowest},
                                                                          {Lowest, Full, Rising},
                                                                          {Lowest, Falling, Full},
                                                                          {Rising, Lowest, Full},
                                                                          {Full, Lowest, Falling}});
    float position = hue * 6.0f;
    int sextantIndex = int(floorf(position)) % 6;
    position -= floorf(position);
    const std::array parts{value, value * (1 - saturation), value * (1 - position * saturation),
                           value * (1 - (1 - position) * saturation)};
    const std::array<Part, 3>& sextant = sextants[sextantIndex < 0 ? sextantIndex + 6 : sextantIndex];
    return {parts[sextant[0]], parts[sextant[1]], parts[sextant[2]]};
}
}

