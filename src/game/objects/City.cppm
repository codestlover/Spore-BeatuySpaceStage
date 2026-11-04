
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <cstdint>

export module beatuy.game.objects:city;

import :combat;
import :gamedata;
import :view;
import beatuy.game.math;
import beatuy.game.memory;

export namespace game {
class Timer : public View {
public:
    using View::View;
    static constexpr uint32_t byteSize = 0x20;
    void*& clock() const {
        return at<void*>(0x1c);
    }
};

class City;

class Civilization : public View {
public:
    using View::View;
    char* culture() const {
        return object ? object + 0x6c : nullptr;
    }
    uint8_t& playerOwned() const {
        return at<uint8_t>(0x89);
    }
    float& wealth() const {
        return at<float>(0x98);
    }
    Array<City*> cities() const;
    uint32_t* species() const {
        return &at<uint32_t>(0x268);
    }
    void*& floater() const {
        return at<void*>(0x42c);
    }
    Array<View*> tools() const {
        return arrayAt<View*>(0x470);
    }
    uint8_t* cards() const {
        return &at<uint8_t>(0x484);
    }
    Array<Timer> cooldowns() const {
        return arrayAt<Timer>(0x490);
    }
    uint8_t& winFlag() const {
        return at<uint8_t>(0x4cc);
    }
    static constexpr uint32_t byteSize = 0x4d8;
};

class Walls : public View {
public:
    using View::View;
    float& outerRadius() const {
        return at<float>(0x25c);
    }
};

class Turret : public GameData {
public:
    using GameData::GameData;
    Combatant fighter() const {
        return part<Combatant>(0x588);
    }
};

class City : public GameData {
public:
    using GameData::GameData;
    Spatial place() const {
        return part<Spatial>(0x120);
    }
    char*& hall() const {
        return at<char*>(0x320);
    }
    Walls walls() const {
        return reference<Walls>(0x324);
    }
    Array<GameData*> buildings() const {
        return arrayAt<GameData*>(0x340);
    }
    Array<Turret*> turrets() const {
        return arrayAt<Turret*>(0x354);
    }
    int& specialty() const {
        return at<int>(0x540);
    }
    Civilization civilization() const {
        return reference<Civilization>(0x590);
    }
    float& capturePercent() const {
        return at<float>(0x748);
    }
    uint32_t& captureIdentifier() const {
        return at<uint32_t>(0x74c);
    }
};

inline Array<City*> Civilization::cities() const {
    return arrayAt<City*>(0x9c);
}
}

