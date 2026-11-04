
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <cstdint>

export module beatuy.feature.tribe:spear;

import beatuy.game;

namespace feature::tribe {
struct Quaternion {
    float x, y, z, w;

    static Quaternion of(game::Spatial spatial) {
        const float* components = spatial.orientation();
        return {components[0], components[1], components[2], components[3]};
    }
    [[nodiscard]] constexpr Quaternion conjugate() const {
        return {-x, -y, -z, w};
    }
    [[nodiscard]] constexpr game::Vector rotate(const game::Vector& vector) const {
        game::Vector axis{x, y, z};
        game::Vector twiceCross = game::cross(axis, vector) * 2.0f;
        return vector + twiceCross * w + game::cross(axis, twiceCross);
    }
    const float* data() const {
        return &x;
    }

    friend constexpr Quaternion operator*(const Quaternion& left, const Quaternion& right) {
        return {left.w * right.x + left.x * right.w + left.y * right.z - left.z * right.y,
                left.w * right.y - left.x * right.z + left.y * right.w + left.z * right.x,
                left.w * right.z + left.x * right.y - left.y * right.x + left.z * right.w,
                left.w * right.w - left.x * right.x - left.y * right.y - left.z * right.z};
    }
};

struct ModelFilter {
    uint64_t required, excluded;
    void* filter;
    uint8_t mode, flags;
};

class Model : public game::View {
public:
    using game::View::View;
    const float* bounds() const {
        return &at<float>(0x70);
    }
};

class ModelWorld : public game::View {
public:
    using game::View::View;
    static constexpr uint32_t pickSlot = 0x44;
    void* pickFunction() const {
        return game::virtualTable(object)[pickSlot / 4];
    }
    bool pick(const game::Vector& from, const game::Vector& to, void* model, float& share, ModelFilter& filter) const {
        return call<bool>(pickSlot, &from, &to, model, &share, &filter);
    }
};

struct Mesh {
    void* model;
    ModelWorld world;
};

class Spear : public game::GameData {
public:
    using game::GameData::GameData;
    static constexpr uint32_t projectileOffset = 0x504, targetOffset = 0x53c;
    static Spear ofProjectile(const void* projectile) {
        return Spear(projectile ? static_cast<const char*>(projectile) - projectileOffset : nullptr);
    }
    static Spear create(void* nouns) {
        return Spear(map::tribe::spearCreate(nouns));
    }
    game::Locomotive motion() const {
        return game::Locomotive(body().raw());
    }
    game::Vector& lastPosition() const {
        return at<game::Vector>(0x518);
    }
    float& damage() const {
        return at<float>(0x538);
    }
    void* target() const {
        return at<void*>(targetOffset);
    }
    void setTarget(game::Combatant target) const {
        map::tribe::combatantPointerAssign(object + targetOffset, target.raw());
    }
    void initialize(int damageType, void* owner) const {
        call<void>(0x58, 0, 0, 0, damageType, owner);
    }
    void launch(const game::Vector& velocity) const {
        map::tribe::spearLaunch(object, &velocity);
    }
};
}

