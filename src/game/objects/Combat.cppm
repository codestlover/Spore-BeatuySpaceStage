
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <cstdint>

export module beatuy.game.objects:combat;

import :gamedata;
import :view;
import beatuy.game.math;
import beatuy.game.memory;

export namespace game {
class Combatant : public View {
public:
    using View::View;
    float& maximumHealth() const {
        return at<float>(0x2c);
    }
    float& health() const {
        return at<float>(0x38);
    }
    int& state() const {
        return at<int>(0x34);
    }
    bool destroyed() const {
        return state() == 2;
    }
    char*& target() const {
        return at<char*>(0x48);
    }
    bool& invincible() const {
        return at<bool>(0x80);
    }
    char*& lastAttacker() const {
        return at<char*>(0x28);
    }
    char*& automaticWeapon() const {
        return at<char*>(0x288);
    }
    GameData owner() const {
        return GameData(object ? call<void*>(0xc) : nullptr);
    }
    void* spatial() const {
        return object ? call<void*>(0x8) : nullptr;
    }
    char* weapon() const {
        return call<char*>(0x14);
    }
    int takeDamage(float damage, uint32_t politicalId, int kind, const void* position, void* attacker) const {
        return call<int>(0x18, damage, politicalId, kind, position, attacker);
    }
    bool inRange(void* other) const {
        return call<bool>(0x40, other);
    }
    void setTarget(void* target) const {
        if (object)
            call<void>(0x50, target);
    }
};

class Guidance : public View {
public:
    using View::View;
    float& speed() const {
        return at<float>(0x10);
    }
    float& turnRate() const {
        return at<float>(0x14);
    }
    uint64_t& flownMilliseconds() const {
        return at<uint64_t>(0x28);
    }
};

class Missile : public GameData {
public:
    using GameData::GameData;
    Guidance guidance() const {
        return reference<Guidance>(0x500);
    }
};

class Tool : public View {
public:
    using View::View;
    uint32_t& identifier() const {
        return at<uint32_t>(0x5c);
    }
    bool& inUse() const {
        return at<bool>(0x80);
    }
    void*& source() const {
        return at<void*>(0x114);
    }
    void*& target() const {
        return at<void*>(0x118);
    }
    float& rangeMultiplier() const {
        return at<float>(0x148);
    }
    bool& ready() const {
        return at<bool>(0x150);
    }
    uint32_t& strategy() const {
        return at<uint32_t>(0x178);
    }
    float& maximumDamage() const {
        return at<float>(0x1b8);
    }
    float& maximumRange() const {
        return at<float>(0x1cc);
    }
    float& projectileSpeed() const {
        return at<float>(0x1d8);
    }
    float range() const {
        return object ? maximumRange() * rangeMultiplier() : 0.0f;
    }
};

class Projectile : public GameData {
public:
    using GameData::GameData;
    static constexpr int flying = 0;
    Tool tool() const {
        return reference<Tool>(0x518);
    }
    int& lowestDamage() const {
        return at<int>(0x52c);
    }
    int& highestDamage() const {
        return at<int>(0x530);
    }
    int& damageKind() const {
        return at<int>(0x538);
    }
    int& stage() const {
        return at<int>(0x544);
    }
    uint64_t& flownMilliseconds() const {
        return at<uint64_t>(0x550);
    }
};

class Beam : public View {
public:
    using View::View;
    static constexpr uint32_t type = 0x24630ce;
    Tool tool() const {
        return reference<Tool>(0x134);
    }
    Vector*& firstPoint() const {
        return at<Vector*>(0x1e4);
    }
    Vector*& pastLastPoint() const {
        return at<Vector*>(0x1e8);
    }
};

class BeamView : public View {
public:
    using View::View;
    Beam beam() const {
        void* shown = at<void*>(0xc);
        return Beam(shown ? callVirtual<char*>(shown, 0xb8, Beam::type) : nullptr);
    }
};
}

