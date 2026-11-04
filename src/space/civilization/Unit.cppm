
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <algorithm>
#include <array>

export module beatuy.feature.civilization:unit;

import beatuy.game;

namespace civilization {
class Unit : public game::Vehicle {
public:
    using Vehicle::Vehicle;
    bool usable() const {
        return alive() && !dead();
    }
    bool flies() const {
        return locomotion() == map::identifiers::Locomotion::Air;
    }
    bool fights() const {
        map::identifiers::Purpose role = purpose();
        return role == map::identifiers::Purpose::Military || role == map::identifiers::Purpose::Cultural;
    }
    game::Vector where() const {
        const game::Vector* point = body().position();
        return point ? *point : game::Vector{0, 0, 0};
    }
    game::Order current() const {
        game::Array<game::Order> queue = orders();
        return queue.data() && queue.data() < queue.dataEnd() ? game::Order(queue.data()) : game::Order();
    }
    bool idle() const {
        return !current();
    }
    void* currentTarget() const {
        game::Order order = current();
        return order ? order.target() : nullptr;
    }
    int orderFlags() const {
        return orderFlag();
    }
};

template <int count> struct Nearest {
    explicit Nearest(float within) {
        distances.fill(within);
    }

    void offer(Unit unit, float distance) {
        auto place = std::ranges::find_if(distances, [distance](float other) { return distance < other; });
        if (place == distances.end())
            return;
        auto index = place - distances.begin();
        std::shift_right(units.begin() + index, units.end(), 1);
        std::shift_right(place, distances.end(), 1);
        units[index] = unit;
        *place = distance;
    }

    std::array<Unit, count> units{};
    std::array<float, count> distances;
};
}

