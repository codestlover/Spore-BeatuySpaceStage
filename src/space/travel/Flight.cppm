
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <cstdint>
#include <span>

export module beatuy.feature.travel:flight;

import beatuy.game;

namespace feature {
struct FlightSettings {
    float margin = 5.0f, climbSlope = 0.5f, alignedCourses = 0.94f;
    float leastReach = 160.0f, mostReach = 500.0f, mostArc = 0.8f, reachMilliseconds = 1500.0f, leastSpeed = 0.001f;
    int samples = 5;
    uint32_t pushingObstacles = (1u << 3) | (1u << 4);
};

export class Flight {
public:
    void onLift(char* ship, const game::Vector* point, float clearance) const;
    void onFormation(char* ship) const;

private:
    struct Course {
        game::Vector heading;
        float reach;
    };

    static constexpr FlightSettings settings{};

    [[nodiscard]] static bool flownByGame(game::Ship ship);
    [[nodiscard]] static Course courseOf(const game::Vector& motion, float speed);
    [[nodiscard]] static float clearRadius(game::Ship ship, std::span<const Course> courses, float clearance);
};
}

