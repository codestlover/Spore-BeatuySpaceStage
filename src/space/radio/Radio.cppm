
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <Windows.h>
#include <array>
#include <cstddef>
#include <cstdint>

export module beatuy.feature.radio;

import :recordings;
import beatuy.game;

namespace feature {
struct RadioSettings {
    float hearing = 250.0f;
    int tracks = 16;
    float hurt = 0.01f;
    DWORD scanMilliseconds = 200, gapMilliseconds = 500, unitCooldownMilliseconds = 3000, announceMilliseconds = 2500;
    int jamChance = 40, jamFewest = 2, jamMost = 3, stickChance = 40, stickFewest = 2, stickMost = 3;
    DWORD jamShortest = 110, jamLongest = 170, endMargin = 60, longestTransmission = 10000;
    DWORD pieceShortest = 90, pieceLongest = 120, pieceAfterFirstWord = 40, pieceBeforeLastWord = 100,
          pieceLatest = 1500;
};

struct ReceptionSettings {
    DWORD stepMilliseconds = 40, dropShortest = 40, dropLongest = 160, fadeShortest = 700, fadeLongest = 2500;
    int stepFrames = 2, dropGapSteps = 2;
    float strongest = 0.05f, weakest = 0.95f, jitter = 0.1f, weaknessPerSecond = 1.0f;
    float fadeDepth = 18.0f, fadeCurve = 1.5f, fadeSpread = 3.6f, dropChance = 0.07f, dropCurve = 1.5f,
          dropDecibels = -40.0f;
    float clearCutoff = 3600.0f, cutoffLoss = 1400.0f, cutoffFading = 0.35f, cutoffStep = 0.03f, gainStep = 0.05f;
    float flutterFrom = 0.3f, flutterMix = 0.45f, flutterSlowest = 9.0f, flutterFastest = 16.0f, flutterStep = 0.02f;
    float hissQuiet = -34.0f, hissRise = 16.0f, hissFollow = 0.4f, hissDrop = 3.0f, hissCeiling = -8.0f;
    float crashChance = 0.06f, crashLeast = 4.0f, crashMost = 10.0f;
    uint32_t hissSound = 0x945b4d7d;
    float hissUnity = 4.9f, hissFloor = 1e-4f, hissCutoff = 4500.0f, hissFadeOut = 0.06f;
    DWORD hissTail = 700;
};

export class Radio {
public:
    void tick();
    void onLeavePlanet(bool destroyed);
    void onQueued(uint32_t voice);
    void bribed(void* vehicle);
    void agreed(void* vehicle);
    void heard(map::identifiers::AudioEvent event, int track, uint32_t value);
    [[nodiscard]] static const void* seekTable(void* player, uint32_t request, const void* table, int sample);
    [[nodiscard]] static bool decodesThrough(char* decoder);

    void* speaker = nullptr;

private:
    enum class Line { None, Hold, Report, Engage, UnderFire };

    struct Track {
        game::Vehicle vehicle;
        map::identifiers::Order kind{};
        float health = 0.0f;
        DWORD quietUntil = 0;
    };

    struct Transmission {
        uint32_t line = 0;
        int track = 0;
        Recording recording;
        DWORD sentAt = 0, segmentAt = 0, nextRestart = 0, piece = 0, pieceLength = 0;
        int restarts = 0, repeats = 0;
        bool started = false, stopping = false;
        game::Vehicle vehicle;
        float weakness = 0.0f, offset = 0.0f, gain = 1.0f, cutoff = 0.0f, flutter = 0.0f;
        std::array<float, 3> fadePeriods{}, fadePhases{};
        DWORD dropUntil = 0;
    };

    struct Hiss {
        int track = 0;
        float gain = 0.0f, heldGain = 0.0f;
        DWORD heardAt = 0;
        bool spent = false;
    };

    using Voices = map::Global<const uint32_t>;
    static constexpr RadioSettings settings{};
    static constexpr ReceptionSettings reception{};
    using Tracks = std::array<Track, settings.tracks>;
    static constexpr size_t mostTransmissions = 4;

    [[nodiscard]] static Line classify(map::identifiers::Order was, map::identifiers::Order now);
    [[nodiscard]] static bool audible(game::Vehicle vehicle);
    [[nodiscard]] static bool onAir(const Transmission& transmission, DWORD now);
    void say(game::Vehicle vehicle, Line line, bool byVehicle);
    void announce(game::Vehicle vehicle, const Voices& voices);
    void forget();
    void voice(uint32_t line, game::Vehicle vehicle);
    static void placePiece(Transmission& transmission);
    void transmit(Transmission& transmission, DWORD now);
    void listen();
    void receive(DWORD now);
    [[nodiscard]] float channelGain(DWORD now);
    [[nodiscard]] static float tune(Transmission& transmission, DWORD now);
    [[nodiscard]] static float weaknessOf(const Transmission& transmission);
    static void follow(Transmission& transmission);
    static void finish(Transmission& transmission);
    void startHiss();
    void silence();

    Tracks tracks;
    std::array<Transmission, mostTransmissions> transmissions;
    Hiss hiss;
    DWORD nextScan = 0, nextLine = 0, nextStep = 0;
    int framesSinceStep = 0;
    bool alternate = false, hearsAudio = false;
};
}

