
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

class Tool : public View {
public:
    using View::View;
    uint32_t& identifier() const {
        return at<uint32_t>(0x5c);
    }
    bool& inUse() const {
        return at<bool>(0x80);
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
    float& maximumRange() const {
        return at<float>(0x1cc);
    }
    float range() const {
        return object ? maximumRange() * rangeMultiplier() : 0.0f;
    }
};
}

