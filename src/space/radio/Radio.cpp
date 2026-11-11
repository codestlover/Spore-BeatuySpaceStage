
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <Windows.h>
#include <cstdlib>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <numbers>
#include <ranges>

module beatuy.feature.radio;

import beatuy.game;

namespace feature {
namespace {
using map::identifiers::AudioEvent;
using map::identifiers::AudioProperty;
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

[[nodiscard]] DWORD within(DWORD low, DWORD high) {
    return low + DWORD(rand()) % (high - low + 1);
}

[[nodiscard]] float decibels(float level) {
    return std::pow(10.0f, level / 20.0f);
}

[[nodiscard]] bool hitByVehicle(game::Vehicle vehicle) {
    game::GameData data = game::Combatant(vehicle.fighter().lastAttacker()).owner();
    return data && data.cast(map::identifiers::Type::Vehicle);
}

[[nodiscard]] Recording recordingOf(uint32_t identifier) {
    auto found = std::ranges::lower_bound(recordings, identifier, {}, &Recording::identifier);
    return found != recordings.end() && found->identifier == identifier ? *found : Recording{};
}

Radio* activeRadio = nullptr;

void __cdecl hearAudio(uint32_t message, void* fields, void*) {
    if (message == uint32_t(AudioEvent::Message) && fields && activeRadio)
        activeRadio->heard(AudioEvent(game::field<uint32_t>(fields, 0x8)), game::field<int>(fields, 0x10),
                           game::field<uint32_t>(fields, 0x18));
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
    const Voices& voices = moving                           ? *voicing.moving
                           : byVehicle && voicing.byVehicle ? *voicing.byVehicle
                                                            : *voicing.said;
    voice(voices[moving ? locomotion : int(purpose)], vehicle);
}

void Radio::voice(uint32_t line, game::Vehicle vehicle) {
    auto free = std::ranges::find(transmissions, 0, &Transmission::track);
    int track = free != transmissions.end() ? game::newAudioTrack() : 0;
    if (!track) {
        game::playAudio(line);
        return;
    }
    game::playAudio(line, track);
    bool jams = rand() % 100 < settings.jamChance, sticks = rand() % 100 < settings.stickChance;
    Transmission transmission{
        .line = line,
        .track = track,
        .sentAt = GetTickCount(),
        .restarts = jams ? settings.jamFewest + rand() % (settings.jamMost - settings.jamFewest + 1) : 0,
        .repeats = sticks ? settings.stickFewest + rand() % (settings.stickMost - settings.stickFewest + 1) : 0,
        .vehicle = vehicle,
        .weakness = reception.weakest,
        .offset = game::between(-reception.jitter, reception.jitter)};
    transmission.weakness = weaknessOf(transmission);
    transmission.cutoff = reception.clearCutoff - reception.cutoffLoss * transmission.weakness;
    for (float& period : transmission.fadePeriods)
        period = float(within(reception.fadeShortest, reception.fadeLongest)) / 1000.0f;
    for (float& phase : transmission.fadePhases)
        phase = game::between(0.0f, 2.0f * std::numbers::pi_v<float>);
    game::setAudioProperty(track, AudioProperty::LowPass, transmission.cutoff);
    game::setAudioProperty(track, AudioProperty::RingFrequency,
                           game::between(reception.flutterSlowest, reception.flutterFastest));
    vehicle.addReference();
    *free = transmission;
}

float Radio::weaknessOf(const Transmission& transmission) {
    const game::Vector* at = transmission.vehicle.alive() ? transmission.vehicle.body().position() : nullptr;
    if (!at)
        return transmission.weakness;
    float distance = game::surfaceDistance(game::shipPosition(), *at);
    return std::clamp(distance / settings.hearing + transmission.offset, reception.strongest, reception.weakest);
}

void Radio::follow(Transmission& transmission) {
    float most = reception.weaknessPerSecond * float(reception.stepMilliseconds) / 1000.0f;
    transmission.weakness += std::clamp(weaknessOf(transmission) - transmission.weakness, -most, most);
}

void Radio::finish(Transmission& transmission) {
    transmission.vehicle.release();
    transmission = Transmission{};
}

void Radio::heard(AudioEvent event, int track, uint32_t value) {
    if (event == AudioEvent::Finished && track && track == hiss.track)
        hiss.track = 0;
    auto found = track ? std::ranges::find(transmissions, track, &Transmission::track) : transmissions.end();
    if (found == transmissions.end())
        return;
    Transmission& transmission = *found;
    if (event == AudioEvent::Started && !transmission.started) {
        DWORD now = GetTickCount();
        transmission.started = true;
        transmission.recording = recordingOf(value);
        transmission.segmentAt = now;
        transmission.nextRestart = now + within(settings.jamShortest, settings.jamLongest);
        placePiece(transmission);
    }
    transmission.stopping = transmission.stopping || event == AudioEvent::Ended || event == AudioEvent::Stopping;
    if (event == AudioEvent::Finished)
        finish(transmission);
}

bool Radio::onAir(const Transmission& transmission, DWORD now) {
    return transmission.started && !transmission.stopping && transmission.recording.identifier &&
           now - transmission.segmentAt + settings.endMargin < transmission.recording.lastWord;
}

void Radio::placePiece(Transmission& transmission) {
    const Recording& recording = transmission.recording;
    DWORD length = within(settings.pieceShortest, settings.pieceLongest);
    DWORD room = length + settings.pieceBeforeLastWord;
    DWORD earliest = recording.firstWord + settings.pieceAfterFirstWord;
    DWORD latest = recording.lastWord > room ? std::min<DWORD>(settings.pieceLatest, recording.lastWord - room) : 0;
    bool fits = recording.identifier && earliest <= latest;
    transmission.repeats = fits ? transmission.repeats : 0;
    transmission.piece = fits ? within(earliest, latest) : 0;
    transmission.pieceLength = length;
}

void Radio::transmit(Transmission& transmission, DWORD now) {
    if (transmission.track && now - transmission.sentAt > settings.longestTransmission)
        finish(transmission);
    bool jamming = transmission.restarts > 0;
    DWORD due =
        jamming ? transmission.nextRestart : transmission.segmentAt + transmission.piece + transmission.pieceLength;
    if (!transmission.started || !(jamming || transmission.repeats) || LONG(now - due) < 0)
        return;
    if (game::paused() || !onAir(transmission, now)) {
        transmission.restarts = transmission.repeats = 0;
        return;
    }
    if (!jamming)
        game::setAudioProperty(transmission.track, AudioProperty::SeekTime, float(transmission.piece) / 1000.0f);
    game::playAudio(transmission.line, transmission.track);
    transmission.segmentAt = now - (jamming ? 0 : transmission.piece);
    transmission.nextRestart = now + within(settings.jamShortest, settings.jamLongest);
    int& left = jamming ? transmission.restarts : transmission.repeats;
    --left;
}

const void* Radio::seekTable(void* player, uint32_t request, const void* table, int sample) {
    static constexpr auto wholeBlock = [] {
        constexpr std::array<uint32_t, 6> words{0, 0, 0, 0, 0x7fffffff, 1};
        std::array<uint8_t, 24> bytes{};
        for (size_t index = 0; index < bytes.size(); ++index)
            bytes[index] = uint8_t(words[index / 4] >> (24 - index % 4 * 8));
        return bytes;
    }();
    bool ours = !table && sample > 0 && game::AudioPlayer(player).playsLayerThreeFromMemory(request);
    return ours ? wholeBlock.data() : table;
}

bool Radio::decodesThrough(char* decoder) {
    return game::LayerThreeDecoder(decoder).lacksGranuleSizes();
}

void Radio::receive(DWORD now) {
    bool onAir =
        std::ranges::any_of(transmissions, [](const Transmission& transmission) { return transmission.track != 0; });
    if (onAir)
        hiss.heardAt = now;
    DWORD quiet = now - hiss.heardAt;
    if (!onAir && (!hiss.track || quiet >= reception.hissTail)) {
        silence();
        return;
    }
    if (onAir && !hiss.track && !hiss.spent)
        startHiss();
    if (game::paused() || ++framesSinceStep < reception.stepFrames || LONG(now - nextStep) < 0)
        return;
    framesSinceStep = 0;
    nextStep = now + reception.stepMilliseconds;
    float fading = hiss.heldGain * (1.0f - float(quiet) / float(reception.hissTail));
    float gain = std::max(onAir ? channelGain(now) : fading, reception.hissFloor);
    hiss.heldGain = onAir ? gain : hiss.heldGain;
    if (!hiss.track || std::abs(gain - hiss.gain) <= hiss.gain * reception.gainStep)
        return;
    game::setAudioProperty(hiss.track, AudioProperty::Gain, gain);
    hiss.gain = gain;
}

float Radio::channelGain(DWORD now) {
    float loudest = reception.dropDecibels, weakest = 0.0f;
    for (Transmission& transmission : transmissions | std::views::filter(&Transmission::track)) {
        follow(transmission);
        loudest = std::max(loudest, tune(transmission, now));
        weakest = std::max(weakest, transmission.weakness);
    }
    bool crashes = game::between(0.0f, 1.0f) < reception.crashChance * weakest;
    float level = loudest + (crashes ? game::between(reception.crashLeast, reception.crashMost) : 0.0f);
    return reception.hissUnity * decibels(std::min(level, reception.hissCeiling));
}

float Radio::tune(Transmission& transmission, DWORD now) {
    float weakness = transmission.weakness, seconds = float(now - transmission.sentAt) / 1000.0f, wave = 0.0f;
    for (size_t index = 0; index < transmission.fadePeriods.size(); ++index)
        wave += std::sin(2.0f * std::numbers::pi_v<float> * seconds / transmission.fadePeriods[index] +
                         transmission.fadePhases[index]);
    float fading = std::pow(std::clamp(0.5f + wave / reception.fadeSpread, 0.0f, 1.0f), reception.fadeCurve);
    bool rested = LONG(now - transmission.dropUntil) >= LONG(reception.dropGapSteps * reception.stepMilliseconds);
    if (rested && game::between(0.0f, 1.0f) < reception.dropChance * std::pow(weakness, reception.dropCurve))
        transmission.dropUntil = now + within(reception.dropShortest, reception.dropLongest);
    bool dropped = LONG(now - transmission.dropUntil) < 0;
    float faded = -reception.fadeDepth * weakness * fading;
    float gain = decibels(dropped ? reception.dropDecibels : faded);
    float cutoff =
        (reception.clearCutoff - reception.cutoffLoss * weakness) * (1.0f - reception.cutoffFading * weakness * fading);
    if (!transmission.stopping && std::abs(gain - transmission.gain) > transmission.gain * reception.gainStep) {
        game::setAudioProperty(transmission.track, AudioProperty::Gain, gain);
        transmission.gain = gain;
    }
    if (!transmission.stopping && std::abs(cutoff - transmission.cutoff) > transmission.cutoff * reception.cutoffStep) {
        game::setAudioProperty(transmission.track, AudioProperty::LowPass, cutoff);
        transmission.cutoff = cutoff;
    }
    float flutter =
        reception.flutterMix * std::max(0.0f, weakness - reception.flutterFrom) / (1.0f - reception.flutterFrom);
    if (!transmission.stopping && std::abs(flutter - transmission.flutter) > reception.flutterStep) {
        game::setAudioProperty(transmission.track, AudioProperty::RingMix, flutter);
        transmission.flutter = flutter;
    }
    return reception.hissQuiet + reception.hissRise * weakness - reception.hissFollow * faded +
           (dropped ? reception.hissDrop : 0.0f);
}

void Radio::startHiss() {
    struct Setting {
        AudioProperty property;
        float value;
    };

    static constexpr auto setup = std::to_array<Setting>({
        {AudioProperty::Spatial, 0.0f},
        {AudioProperty::Virtual, 0.0f},
        {AudioProperty::FadeIn, 0.0f},
        {AudioProperty::FadeOut, reception.hissFadeOut},
        {AudioProperty::LowPass, reception.hissCutoff},
        {AudioProperty::Gain, reception.hissFloor},
    });
    int track = game::prepareAudio(reception.hissSound);
    if (!track)
        return;
    for (auto [property, value] : setup)
        game::setAudioProperty(track, property, value);
    game::startAudio(track);
    hiss = Hiss{.track = track, .gain = reception.hissFloor, .heardAt = hiss.heardAt, .spent = true};
}

void Radio::silence() {
    if (hiss.track)
        game::stopAudio(hiss.track);
    hiss = Hiss{};
}

void Radio::listen() {
    game::MessageManager manager = hearsAudio ? game::MessageManager() : game::MessageManager::get();
    if (!manager)
        return;
    activeRadio = this;
    manager.addHandler(&hearAudio, uint32_t(AudioEvent::Message));
    hearsAudio = true;
}

void Radio::forget() {
    tracks.fill(Track{});
    std::ranges::for_each(transmissions, &Radio::finish);
    silence();
}

void Radio::announce(game::Vehicle vehicle, const Voices& voices) {
    Purpose purpose = vehicle.purpose();
    if (!voiced(purpose))
        return;
    nextLine = GetTickCount() + settings.announceMilliseconds;
    voice(voices[int(purpose)], vehicle);
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

void Radio::onQueued(uint32_t line) {
    game::Vehicle vehicle(speaker);
    if (!vehicle || !line || !listening() || !audible(vehicle))
        return;
    DWORD now = GetTickCount();
    if (LONG(now - nextLine) < 0)
        return;
    nextLine = now + settings.gapMilliseconds;
    voice(line, vehicle);
}

void Radio::tick() {
    listen();
    DWORD now = GetTickCount();
    for (Transmission& transmission : transmissions)
        transmit(transmission, now);
    receive(now);
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

