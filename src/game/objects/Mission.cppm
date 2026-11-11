
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <cstdint>

export module beatuy.game.objects:mission;

import :view;
import beatuy.game.map;
import beatuy.game.memory;

export namespace game {
class Mission : public View {
public:
    using View::View;

    char* cast(map::identifiers::Type type) const {
        return object ? call<char*>(0xc, uint32_t(type)) : nullptr;
    }
};

class RaidShip : public View {
public:
    using View::View;

    static constexpr uint32_t byteSize = 0x20;

    map::identifiers::ShipKind& kind() const {
        return at<map::identifiers::ShipKind>(0x4);
    }
};

class RaidEvent : public View {
public:
    using View::View;

    static constexpr uint32_t table = 0x146f828, shipsOffset = 0x238;

    static RaidEvent ofShips(char* ships) {
        char* raid = ships ? ships - shipsOffset : nullptr;
        return RaidEvent(raid && game::readable(raid, 4) && gameAddress(virtualTable(raid)) == table ? raid : nullptr);
    }

    static RaidEvent of(Mission mission) {
        return RaidEvent(mission.cast(map::identifiers::Type::RaidEvent));
    }

    int& bombers() const {
        return at<int>(0x1f8);
    }

    bool& logged() const {
        return at<bool>(0x21e);
    }

    uint32_t& attacker() const {
        return at<uint32_t>(0x230);
    }

    uint32_t& owner() const {
        return at<uint32_t>(0x94);
    }
};
}

