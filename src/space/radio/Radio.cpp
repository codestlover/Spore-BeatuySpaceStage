
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <Windows.h>
#include <algorithm>
#include <array>
#include <cstdint>
#include <ranges>

module beatuy.feature.radio;

import beatuy.game;

namespace feature {
namespace {
using map::identifiers::Locomotion;
using map::identifiers::Order;
using map::identifiers::Purpose;

[[nodiscard]] bool listening() {
    map::identifiers::Technology technology =
        game::onPlanet() ? game::planetTechnology() : map::identifiers::Technology::None;
    return technology == map::identifiers::Technology::City || technology == map::identifiers::Technology::Civilization;
}

[[nodiscard]] bool voiced(Purpose purpose) {
    return purpose >= Purpose::Military && purpose <= Purpose::Economic;
}

[[nodiscard]] Order firstOrder(game::Vehicle vehicle) {
    game::Array<game::Order> orders = vehicle.orders();
    return orders.data() && orders.data() < orders.dataEnd() ? orders[0].kind() : Order{};
}

[[nodiscard]] bool hitByVehicle(game::Vehicle vehicle) {
    game::GameData data = game::Combatant(vehicle.fighter().lastAttacker()).owner();
    return data && data.cast(map::identifiers::Type::Vehicle);
}
}

Radio::Line Radio::classify(Order was, Order now) {
    struct Turn {
        Order kind;
        Line starts, ends;
    };
    using enum Line;
    static constexpr auto turns = std::to_array<Turn>({
        {Order::Attack, Engage, Hold},
        {Order::AttackCity, Engage, Hold},
        {Order::ConvertCity, Engage, None},
        {Order::Move, Report, None},
    });
    auto turn = [](Order kind) -> const Turn* {
        auto found = std::ranges::find(turns, kind, &Turn::kind);
        return found != turns.end() ? &*found : nullptr;
    };
    const Turn* into = turn(now);
    if (into && was != now)
        return into->starts;
    const Turn* ended = now == Order{} ? turn(was) : nullptr;
    return ended ? ended->ends : None;
}

bool Radio::audible(game::Vehicle vehicle) {
    if (!vehicle.alive() || vehicle.dead() || vehicle.politicalId() == game::playerPoliticalId())
        return false;
    const game::Vector* at = vehicle.body().position();
    return at && game::surfaceDistance(game::shipPosition(), *at) <= settings.hearing;
}

void Radio::say(game::Vehicle vehicle, Line line, bool byVehicle) {
    struct Voicing {
        const Voices* said = nullptr;
        const Voices* moving = nullptr;
        const Voices* byVehicle = nullptr;
    };
    static constexpr auto voicings = std::to_array<Voicing>({
        {.said = &map::civilization::holdVoices},
        {.said = &map::civilization::holdVoices},
        {.said = &map::civilization::reportVoices, .moving = &map::civilization::movingVoices},
        {.said = &map::civilization::engageVoices},
        {.said = &map::civilization::underFireVoices, .byVehicle = &map::civilization::vehicleFireVoices},
    });
    Purpose purpose = vehicle.purpose();
    if (!voiced(purpose))
        return;
    int locomotion = int(vehicle.locomotion());
    const Voicing& voicing = voicings[int(line)];
    bool moving = voicing.moving && (alternate = !alternate) && locomotion >= int(Locomotion::Land) &&
                  locomotion <= int(Locomotion::Air);
    const Voices& voices =
        moving ? *voicing.moving : byVehicle && voicing.byVehicle ? *voicing.byVehicle : *voicing.said;
    uint32_t voice = voices[moving ? locomotion : int(purpose)];
    game::playAudio(voice);
}

void Radio::forget() {
    tracks.fill(Track{});
}

void Radio::announce(game::Vehicle vehicle, const Voices& voices) {
    Purpose purpose = vehicle.purpose();
    if (!voiced(purpose))
        return;
    uint32_t voice = voices[int(purpose)];
    nextLine = GetTickCount() + settings.announceMilliseconds;
    game::playAudio(voice);
}

void Radio::bribed(void* vehicle) {
    announce(game::Vehicle(vehicle), map::civilization::bribedVoices);
}

void Radio::agreed(void* vehicle) {
    announce(game::Vehicle(vehicle), map::civilization::okayVoices);
}

void Radio::onLeavePlanet(bool) {
    forget();
}

void Radio::onQueued(uint32_t voice) {
    game::Vehicle vehicle(speaker);
    if (!vehicle || !voice || !listening() || !audible(vehicle))
        return;
    DWORD now = GetTickCount();
    if (LONG(now - nextLine) < 0)
        return;
    nextLine = now + settings.gapMilliseconds;
    game::playAudio(voice);
}

void Radio::tick() {
    DWORD now = GetTickCount();
    if (LONG(now - nextScan) < 0)
        return;
    nextScan = now + settings.scanMilliseconds;
    if (!listening()) {
        forget();
        return;
    }
    game::Array<game::Vehicle*> vehicles = game::each<game::Vehicle>(map::civilization::vehicles);
    if (!vehicles.data())
        return;
    game::Vector me = game::shipPosition();
    uint32_t player = game::playerPoliticalId();
    auto foreign = [player](game::Vehicle vehicle) {
        return vehicle.alive() && !vehicle.dead() && vehicle.politicalId() != player;
    };
    std::array<game::Vehicle, settings.tracks> nearest;
    std::array<float, settings.tracks> distances;
    distances.fill(settings.hearing);
    for (game::Vehicle vehicle : vehicles | std::views::filter(foreign)) {
        const game::Vector* at = vehicle.body().position();
        float distance = at ? game::surfaceDistance(me, *at) : settings.hearing;
        for (int i = 0; i < settings.tracks; ++i)
            if (distance < distances[i]) {
                for (int slot = settings.tracks - 1; slot > i; --slot) {
                    nearest[slot] = nearest[slot - 1];
                    distances[slot] = distances[slot - 1];
                }
                nearest[i] = vehicle;
                distances[i] = distance;
                break;
            }
    }
    Tracks next;
    game::Vehicle talker;
    Line best = Line::None;
    bool bestByVehicle = false;
    for (int i = 0; i < settings.tracks && nearest[i]; ++i) {
        game::Vehicle vehicle = nearest[i];
        Track track{.vehicle = vehicle, .kind = firstOrder(vehicle), .health = vehicle.fighter().health()};
        if (auto old = std::ranges::find(tracks, vehicle, &Track::vehicle); old != tracks.end()) {
            track.quietUntil = old->quietUntil;
            Line line = track.health < old->health - settings.hurt ? Line::UnderFire : classify(old->kind, track.kind);
            bool byVehicle = line == Line::UnderFire && hitByVehicle(vehicle);
            if (line > best && LONG(now - track.quietUntil) >= 0) {
                best = line;
                talker = vehicle;
                bestByVehicle = byVehicle;
            }
        }
        next[i] = track;
    }
    if (talker && LONG(now - nextLine) >= 0) {
        say(talker, best, bestByVehicle);
        nextLine = now + settings.gapMilliseconds;
        for (Track& track : next)
            if (track.vehicle == talker)
                track.quietUntil = now + settings.unitCooldownMilliseconds;
    }
    tracks = next;
}
}

