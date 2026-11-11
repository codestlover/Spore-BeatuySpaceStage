
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <cstdint>

export module beatuy.game.objects:ship;

import :combat;
import :gamedata;
import :view;
import beatuy.game.map;
import beatuy.game.math;
import beatuy.game.memory;

export namespace game {
class Ship : public GameData {
public:
    using GameData::GameData;
    static constexpr uint32_t fighterOffset = 0x508;
    static constexpr uint32_t warOnPlanetFlag = 0x1000, standDownOrder = 0x10;
    static Ship ofFighter(const void* combatant) {
        return Ship(combatant ? static_cast<const char*>(combatant) - fighterOffset : nullptr);
    }
    Combatant fighter() const {
        return part<Combatant>(fighterOffset);
    }
    uint32_t decisionFlags() const {
        char* tree = at<char*>(0x6c8);
        return tree ? field<uint32_t>(tree, 0x5fc) : 0;
    }
    bool underOrder(uint32_t order) const {
        char* tree = at<char*>(0x6c8);
        return tree && map::space::activeOrder(tree + 8, order, 0);
    }
    char*& leader() const {
        return at<char*>(0x68c);
    }
    void follow(Ship guide) const {
        char*& led = leader();
        if (led == guide.raw())
            return;
        GameData before(led);
        guide.addReference();
        led = guide.raw();
        before.release();
    }
    char* aiTarget() const {
        return at<char*>(0x7ac);
    }
    char* mission() const {
        return at<char*>(0x800);
    }
    char*& homeStar() const {
        return at<char*>(0x7b0);
    }
    float& flightRadius() const {
        return at<float>(0x768);
    }
    float radius() const {
        return map::space::shipRadius(object);
    }
    Vector& location() const {
        return at<Vector>(0x718);
    }
    Vector& destination() const {
        return at<Vector>(0x750);
    }
    bool& kinematic() const {
        return at<bool>(0x691);
    }
    float& speedFactor() const {
        return at<float>(0x6b4);
    }
    float& accelerationFactor() const {
        return at<float>(0x6b8);
    }
    Vector& velocity() const {
        return at<Vector>(0x724);
    }
    char*& lastShipAttacker() const {
        return at<char*>(0x710);
    }
    map::identifiers::ShipKind& kind() const {
        return at<map::identifiers::ShipKind>(0x714);
    }
    bool& atDestination() const {
        return at<bool>(0x74c);
    }
    int& specialState() const {
        return at<int>(0x76c);
    }
    bool& desiredVisible() const {
        return at<bool>(0x77c);
    }
    char** nonPlayerWeapons() const {
        return &at<char*>(0x790);
    }
    char*& currentWeapon() const {
        return at<char*>(0x790);
    }
    char* shieldTimer() const {
        return object ? object + 0x6d0 : nullptr;
    }
    char*& groundWeapon() const {
        return at<char*>(0x794);
    }
    char*& airWeapon() const {
        return at<char*>(0x798);
    }
    bool withdrawn() const {
        return (at<uint32_t>(0x7dc) & 2u) != 0;
    }
    bool canFight() const {
        return object && alive() && !withdrawn() && fighter().health() > 0.0f;
    }
    float& energy() const {
        return at<float>(0x7f4);
    }
    float& maximumEnergy() const {
        return at<float>(0x7f8);
    }
};

class Posse : public View {
public:
    using View::View;
    bool has(uint32_t politicalId) const {
        for (char* node = object ? at<char*>(0) : nullptr; node && node != object; node = field<char*>(node, 0))
            if (field<uint32_t>(node, 8) == politicalId)
                return true;
        return false;
    }
};

class PlayerShipManager : public View {
public:
    using View::View;
    Ship ship() const {
        return reference<Ship>(0x40);
    }
    Posse escort() const {
        return part<Posse>(0x70);
    }
    char* aimed() const {
        return at<char*>(0xd4);
    }
    char* attackedCity() const {
        return at<char*>(0xdc);
    }
};
}

