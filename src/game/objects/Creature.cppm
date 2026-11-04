
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <cstdint>

export module beatuy.game.objects:creature;

import :combat;
import :gamedata;
import :view;
import beatuy.game.map;
import beatuy.game.math;
import beatuy.game.memory;

export namespace game {
class Locomotive : public Spatial {
public:
    using Spatial::Spatial;
    void* locomotion() const {
        return at<void*>(0x4cc);
    }
    void setLocomotion(void* locomotion) const {
        map::tribe::setLocomotion(object, locomotion);
    }
    void stop() const {
        call<void>(0xec);
    }
};

class ItemSlot : public View {
public:
    using View::View;
    static constexpr uint32_t byteSize = 0x24;
    int& appendage() const {
        return at<int>(0x0);
    }
};

class Creature : public GameData {
public:
    using GameData::GameData;
    Spatial place() const {
        return part<Spatial>(0xc0);
    }
    Locomotive legs() const {
        return part<Locomotive>(0xc0);
    }
    Combatant fighter() const {
        return part<Combatant>(0x5a8);
    }
    int& ageStage() const {
        return at<int>(0xb34);
    }
    void*& animated() const {
        return at<void*>(0xb54);
    }
    Array<ItemSlot> items() const {
        return arrayAt<ItemSlot>(0xe40);
    }
    int& grasping() const {
        return at<int>(0xfc4);
    }
    int& heldItem() const {
        return at<int>(0xffc);
    }
    int& toolKind() const {
        return at<int>(0x102c);
    }
    void activate() const {
        call<void>(0x54);
    }
};

class Walker : public Creature {
public:
    using Creature::Creature;
    const Vector* where() const {
        return place().position();
    }
    bool living() const {
        return alive() && fighter().health() > 0.0f;
    }
    void play(uint32_t animation, int limb = -1) const {
        map::tribe::creaturePlayAnimation(object, animation, limb, false);
    }
    void walkTo(int speed, const Vector& to, const Vector& facing) const {
        map::tribe::creatureWalkTo(object, speed, &to, &facing, 1.0f, 2.0f);
    }
    void face(const Vector& target, int speed) const {
        const Vector* standing = where();
        if (!standing)
            return;
        Vector offset = target - *standing;
        float magnitude = length(offset);
        if (magnitude < 1e-3f)
            return;
        Vector direction = dividedBy(offset, magnitude);
        walkTo(speed, *standing + direction * 0.5f, direction);
    }
};
}

