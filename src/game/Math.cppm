
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <cmath>
#include <cstdint>

export module beatuy.game.math;

export namespace game {
struct Vector {
    float x, y, z;

    friend constexpr Vector operator+(const Vector& left, const Vector& right) {
        return {left.x + right.x, left.y + right.y, left.z + right.z};
    }

    friend constexpr Vector operator-(const Vector& left, const Vector& right) {
        return {left.x - right.x, left.y - right.y, left.z - right.z};
    }

    friend constexpr Vector operator*(const Vector& left, float factor) {
        return {left.x * factor, left.y * factor, left.z * factor};
    }

    friend constexpr bool operator==(const Vector&, const Vector&) = default;
};

struct Key {
    uint32_t instance, type, group;

    friend constexpr bool operator==(const Key&, const Key&) = default;
};

constexpr float dot(const Vector& left, const Vector& right) {
    return left.x * right.x + left.y * right.y + left.z * right.z;
}

constexpr Vector cross(const Vector& left, const Vector& right) {
    return {left.y * right.z - left.z * right.y, left.z * right.x - left.x * right.z,
            left.x * right.y - left.y * right.x};
}

inline float length(const Vector& vector) {
    return std::sqrt(dot(vector, vector));
}

inline float distance(const Vector& from, const Vector& to) {
    return length(from - to);
}

inline bool finite(const Vector& vector) {
    return std::isfinite(vector.x) && std::isfinite(vector.y) && std::isfinite(vector.z);
}

constexpr Vector dividedBy(const Vector& vector, float length) {
    return {vector.x / length, vector.y / length, vector.z / length};
}

inline Vector unit(const Vector& vector, const Vector& fallback = {0.0f, 0.0f, 1.0f}) {
    float magnitude = length(vector);
    return magnitude > 1e-4f ? dividedBy(vector, magnitude) : fallback;
}

inline Vector toward(const Vector& from, const Vector& to, float distance) {
    Vector offset = to - from;
    float magnitude = length(offset);
    if (magnitude < 0.001f)
        return from;
    float factor = distance / magnitude;
    return {from.x + offset.x * factor, from.y + offset.y * factor, from.z + offset.z * factor};
}

inline float surfaceDistance(const Vector& from, const Vector& to) {
    float fromHeight = length(from), toHeight = length(to);
    if (fromHeight < 1.0f || toHeight < 1.0f)
        return distance(from, to);
    return distance(from,
                    Vector{to.x * fromHeight / toHeight, to.y * fromHeight / toHeight, to.z * fromHeight / toHeight});
}
}

