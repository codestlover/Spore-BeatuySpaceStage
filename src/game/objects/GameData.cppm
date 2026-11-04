
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <cstdint>

export module beatuy.game.objects:gamedata;

import :view;
import beatuy.game.map;
import beatuy.game.math;
import beatuy.game.memory;

export namespace game {
class Spatial : public View {
public:
    using View::View;
    const Vector* position() const {
        return object ? call<const Vector*>(0x2c) : nullptr;
    }
    const float* orientation() const {
        return call<const float*>(0x30);
    }
    void setPosition(const Vector& position) const {
        call<void>(0x38, &position);
    }
    void setOrientation(const float* quaternion) const {
        call<void>(0x3c, quaternion);
    }
    void setScale(float scale) const {
        call<void>(0x40, scale);
    }
    char* model() const {
        return call<char*>(0xac);
    }
    char* modelWorld() const {
        return call<char*>(0xb0);
    }
    float& boundingRadius() const {
        return at<float>(0x5c);
    }
    float* extents() const {
        return &at<float>(0x38);
    }
    bool& fixed() const {
        return at<bool>(0x71);
    }
    bool& enabled() const {
        return at<bool>(0x75);
    }
};

class GameData : public View {
public:
    using View::View;
    static constexpr uint32_t bodyOffset = 0x34;
    static GameData ofBody(const void* spatial) {
        return GameData(spatial ? static_cast<const char*>(spatial) - bodyOffset : nullptr);
    }
    void addReference() const {
        if (object)
            call<int>(0x0);
    }
    void release() const {
        if (object)
            call<int>(0x4);
    }
    void* cast(map::identifiers::Type type) const {
        return object ? call<void*>(0xc, uint32_t(type)) : nullptr;
    }
    bool alive() const {
        return object && !at<bool>(0x21);
    }
    uint32_t& identifier() const {
        return at<uint32_t>(0x24);
    }
    uint32_t& politicalId() const {
        return at<uint32_t>(0x30);
    }
    uint32_t safePoliticalId() const {
        return object && readable(bodyOffset) ? politicalId() : uint32_t(-2);
    }
    void setPoliticalId(uint32_t politicalId) const {
        call<void>(0x48, politicalId);
    }
    uint32_t askPoliticalId() const {
        return call<uint32_t>(0x4c);
    }
    Spatial body() const {
        return part<Spatial>(bodyOffset);
    }
    Vector position() const {
        const Vector* spatialPosition = Spatial(cast(map::identifiers::Type::Spatial)).position();
        return spatialPosition ? *spatialPosition : Vector{0.0f, 0.0f, 0.0f};
    }
};
}

