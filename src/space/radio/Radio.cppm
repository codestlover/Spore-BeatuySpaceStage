
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <Windows.h>
#include <array>
#include <cstdint>

export module beatuy.feature.radio;

import beatuy.game;

namespace feature {
struct RadioSettings {
    float hearing = 250.0f;
    int tracks = 16;
    float hurt = 0.01f;
    DWORD scanMilliseconds = 200, gapMilliseconds = 500, unitCooldownMilliseconds = 3000, announceMilliseconds = 2500;
};

export class Radio {
public:
    void tick();
    void onLeavePlanet(bool destroyed);
    void onQueued(uint32_t voice);
    void bribed(void* vehicle);
    void agreed(void* vehicle);

    void* speaker = nullptr;

private:
    enum class Line { None, Hold, Report, Engage, UnderFire };
    struct Track {
        game::Vehicle vehicle;
        map::identifiers::Order kind{};
        float health = 0.0f;
        DWORD quietUntil = 0;
    };
    using Voices = map::Global<const uint32_t>;
    static constexpr RadioSettings settings{};
    using Tracks = std::array<Track, settings.tracks>;

    [[nodiscard]] static Line classify(map::identifiers::Order was, map::identifiers::Order now);
    [[nodiscard]] static bool audible(game::Vehicle vehicle);
    void say(game::Vehicle vehicle, Line line, bool byVehicle);
    void announce(game::Vehicle vehicle, const Voices& voices);
    void forget();

    Tracks tracks;
    DWORD nextScan = 0, nextLine = 0;
    bool alternate = false;
};
}

