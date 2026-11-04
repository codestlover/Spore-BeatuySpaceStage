
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <cstdint>

export module beatuy.game.objects:vehicle;

import :combat;
import :gamedata;
import :view;
import beatuy.game.map;
import beatuy.game.math;
import beatuy.game.memory;

export namespace game {
class Order : public View {
public:
    using View::View;
    static constexpr uint32_t byteSize = 0x18;
    char*& target() const {
        return at<char*>(0x0);
    }
    Vector& point() const {
        return at<Vector>(0x4);
    }
    map::identifiers::Order& kind() const {
        return at<map::identifiers::Order>(0x10);
    }
};

class Vehicle : public GameData {
public:
    using GameData::GameData;
    static constexpr uint32_t fighterOffset = 0x508;
    static Vehicle ofFighter(const void* combatant) {
        return Vehicle(combatant ? static_cast<const char*>(combatant) - fighterOffset : nullptr);
    }
    Combatant fighter() const {
        return part<Combatant>(fighterOffset);
    }
    map::identifiers::Locomotion& locomotion() const {
        return at<map::identifiers::Locomotion>(0xb1c);
    }
    map::identifiers::Purpose& purpose() const {
        return at<map::identifiers::Purpose>(0xb20);
    }
    char*& weapon() const {
        return at<char*>(0xb60);
    }
    char*& airWeapon() const {
        return at<char*>(0xb64);
    }
    Array<Order> orders() const {
        return arrayAt<Order>(0xb68);
    }
    int& orderFlag() const {
        return at<int>(0xbc8);
    }
    bool& dead() const {
        return at<bool>(0xbcd);
    }
};
}

