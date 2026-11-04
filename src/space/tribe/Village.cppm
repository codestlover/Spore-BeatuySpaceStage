
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <cstdint>

export module beatuy.feature.tribe:village;

import beatuy.game;

namespace feature::tribe {
class Citizen : public game::Walker {
public:
    using Walker::Walker;
    void setItem(int item) const {
        map::tribe::setHandheldItem(object, item, -1);
    }
};

class TribeTool : public game::GameData {
public:
    using game::GameData::GameData;
    game::Combatant fighter() const {
        return part<game::Combatant>(0x120);
    }
    int& kind() const {
        return at<int>(0x1fc);
    }
    bool standing() const {
        return alive() && (fighter().maximumHealth() <= 0.0f || fighter().health() > 0.0f);
    }
};

class Village : public game::GameData {
public:
    using game::GameData::GameData;
    game::Spatial place() const {
        return part<game::Spatial>(0x120);
    }
    game::Array<Citizen*> members() const {
        return handed<Citizen>(call<char*>(0x90));
    }
    game::Array<TribeTool*> tools() const {
        return handed<TribeTool>(call<char*>(0xbc));
    }
    uint32_t toolKinds() const {
        uint32_t kinds = 0;
        for (TribeTool tool : tools())
            if (tool && tool.standing())
                kinds |= 1u << (tool.kind() & 31);
        return kinds;
    }

private:
    template <game::ObjectView V> static game::Array<V*> handed(char* vector) {
        return vector ? game::Array<V*>(game::field<char*>(vector, 0), game::field<char*>(vector, 4))
                      : game::Array<V*>();
    }
};
}

