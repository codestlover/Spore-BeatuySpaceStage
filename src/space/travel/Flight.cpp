
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <optional>
#include <span>

module beatuy.feature.travel;

import beatuy.game;

namespace feature {
namespace {
using game::Vector;

Vector flat(const Vector& direction, const Vector& up) {
    return direction - up * game::dot(direction, up);
}

Vector along(const Vector& from, const Vector& heading, float distance) {
    float radius = game::length(from);
    float angle = distance / radius;
    return from * std::cos(angle) + heading * (radius * std::sin(angle));
}
}

void Flight::onLift(char* raw, const Vector* point, float clearance) const {
    game::Ship ship(raw);
    if (!ship || point != &ship.location() || !flownByGame(ship))
        return;
    Vector from = ship.location();
    Vector up = game::unit(from);
    std::array<Course, 2> courses{};
    size_t count = 0;
    Vector motion = flat(ship.velocity(), up);
    float speed = game::length(motion);
    if (speed > settings.leastSpeed)
        courses[count++] = courseOf(motion, speed);
    Vector toward = flat(ship.destination() - from, up);
    float away = game::length(toward);
    Vector heading = game::unit(toward);
    float reach = count ? courses[0].reach : settings.leastReach;
    if (away > settings.margin && (count == 0 || game::dot(heading, courses[0].heading) < settings.alignedCourses))
        courses[count++] = {heading, std::min(away, reach)};
    float need = clearRadius(ship, std::span(courses).first(count), clearance);
    float& flight = ship.flightRadius();
    if (need <= flight)
        return;
    flight = need;
    ship.destination() = game::unit(ship.destination()) * need;
}

void Flight::onFormation(char* raw) const {
    game::Ship ship(raw);
    if (!ship || !ship.kinematic() || !flownByGame(ship))
        return;
    Vector from = ship.location();
    Vector up = game::unit(from);
    std::array<Course, 1> courses{};
    size_t count = 0;
    game::Ship leader(ship.leader());
    Vector motion = leader ? flat(leader.velocity(), up) : Vector{};
    float speed = game::length(motion);
    if (speed > settings.leastSpeed)
        courses[count++] = courseOf(motion, speed);
    float need = clearRadius(ship, std::span(courses).first(count), game::groundClearance());
    if (need <= game::length(from))
        return;
    Vector lifted = up * need;
    map::space::shipSnapTo(raw, &lifted);
}

bool Flight::flownByGame(game::Ship ship) {
    return ship && ship.kind() != map::identifiers::ShipKind::Player && ship != game::playerShip() && game::onPlanet();
}

Flight::Course Flight::courseOf(const Vector& motion, float speed) {
    return {game::dividedBy(motion, speed),
            std::clamp(speed * settings.reachMilliseconds, settings.leastReach, settings.mostReach)};
}

float Flight::clearRadius(game::Ship ship, std::span<const Course> courses, float clearance) {
    Vector from = ship.location();
    Vector up = game::unit(from);
    float body = ship.radius();
    float floor = std::max(clearance, body + settings.margin);
    float need = 0.0f;
    auto rocks = [&](const Vector& ground, float spread, const Vector* heading) {
        game::ObstacleList list;
        if (!game::findObstacles(ground, spread, settings.pushingObstacles, list))
            return;
        for (game::Obstacle rock : list.found()) {
            if (rock.ignored())
                continue;
            Vector offset = flat(rock.base() - from, up);
            float ahead = heading ? game::dot(offset, *heading) : 0.0f;
            Vector aside = heading ? offset - *heading * ahead : offset;
            float width = std::max(rock.baseRadius(), rock.canopyRadius()) + body + settings.margin;
            if (game::length(aside) >= width || ahead < -width)
                continue;
            float top = game::length(rock.base()) + rock.height() + rock.canopyRadius() + body + settings.margin;
            need = std::max(need, top - settings.climbSlope * std::max(0.0f, ahead - width));
        }
    };
    auto sample = [&](const Vector& point, float ahead, float spread, const Vector* heading) {
        std::optional<Vector> ground = game::surfaceUnder(point);
        if (!ground)
            return;
        need = std::max(need, game::length(*ground) + floor - settings.climbSlope * ahead);
        rocks(*ground, spread, heading);
    };
    sample(from, 0.0f, body + settings.margin, nullptr);
    for (const Course& course : courses) {
        float spacing = std::min(course.reach, game::length(from) * settings.mostArc) / float(settings.samples);
        for (int step = 1; step <= settings.samples; ++step) {
            float ahead = spacing * float(step);
            sample(along(from, course.heading, ahead), ahead, spacing * 0.5f + body + settings.margin, &course.heading);
        }
    }
    return need;
}
}

