
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <cstdint>
#include <span>

export module beatuy.game.objects:planet;

import :combat;
import :gamedata;
import :view;
import beatuy.game.map;
import beatuy.game.math;
import beatuy.game.memory;

export namespace game {
class RecordObject : public View {
public:
    using View::View;
    static constexpr uint32_t byteSize = 0x2c;
    uint32_t& type() const {
        return at<uint32_t>(0x0);
    }
    Vector& position() const {
        return at<Vector>(0x4);
    }
    float& health() const {
        return at<float>(0x14);
    }
    uint32_t& spare() const {
        return at<uint32_t>(0x18);
    }
    uint8_t& flags() const {
        return at<uint8_t>(0x28);
    }
    bool placedByTool() const {
        return flags() & 1;
    }
};

class PlanetRecord : public View {
public:
    using View::View;
    Array<RecordObject> objects() const {
        return arrayAt<RecordObject>(0x134);
    }
    bool objectsReadable() const {
        return object && game::readable(object + 0x134, 8);
    }
    std::span<Key> animalSpecies() const {
        return valuesAt<Key>(0xd0);
    }
    uint32_t& identifier() const {
        return at<uint32_t>(0x184);
    }
    map::identifiers::Technology& technology() const {
        return at<map::identifiers::Technology>(0x194);
    }
    uint32_t& terrainInstance() const {
        return at<uint32_t>(0x1a4);
    }
    uint32_t& terrainGroup() const {
        return at<uint32_t>(0x1ac);
    }
};

class ToolObject : public GameData {
public:
    using GameData::GameData;
    uint32_t& tool() const {
        return at<uint32_t>(0x64c);
    }
    Combatant fighter() const {
        return part<Combatant>(0x508);
    }
};

class StarRecord : public View {
public:
    using View::View;
    uint32_t& politicalId() const {
        return at<uint32_t>(0x54);
    }
};

class Planet : public View {
public:
    using View::View;
    PlanetRecord record() const {
        return reference<PlanetRecord>(0x13c);
    }
};

class Image : public View {
public:
    using View::View;
    static constexpr uint32_t byteSize = 0x2c;
    int& width() const {
        return at<int>(0x1c);
    }
    int& height() const {
        return at<int>(0x20);
    }
    uint8_t*& pixels() const {
        return at<uint8_t*>(0x28);
    }
};

class TerrainState : public View {
public:
    using View::View;
    float& column() const {
        return at<float>(0x30);
    }
    float* atmosphere() const {
        return &at<float>(0x3cc);
    }
    int& atmosphereType() const {
        return at<int>(0x5c0);
    }
};

class PropertyList : public View {
public:
    using View::View;
    uint32_t& instance() const {
        return at<uint32_t>(0x8);
    }
    uint32_t& group() const {
        return at<uint32_t>(0x10);
    }
};

class Terrain : public View {
public:
    using View::View;
    TerrainState state() const {
        return TerrainState(call<char*>(0x10));
    }
    PropertyList properties() const {
        return reference<PropertyList>(0x28);
    }
};

class PlanetModel : public View {
public:
    using View::View;
    Terrain ground() const {
        return reference<Terrain>(0x24);
    }
};
}

