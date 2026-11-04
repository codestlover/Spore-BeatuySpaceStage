
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <Windows.h>
#include <array>
#include <optional>
#include <cstdint>
#include <span>

export module beatuy.feature.generator;

export import :picker;
export import :texts;
export import :windowprocedure;
import beatuy.game;

namespace generator {
using Color = std::array<float, 3>;

struct Settings {
    uint32_t tool = 0xc94b0300;
    uint32_t spaceGameTable = 0x2db6dad3, toolTable = 0xab14d8fc;
    uint32_t toolNameText = 1, toolDescriptionText = 2;
    uint32_t claimedEvent = 0x147e3c98, claimedTable = 0x63bfe3fe, claimedText = 0x31;
    float hitPoints = 1000.0f;
    float fadeSeconds = 2.0f, shiftSeconds = 0.6f;
    DWORD scanMilliseconds = 500, echoMilliseconds = 40, answerMilliseconds = 1000;
    float strikeWeight = 1.0f, destroyedWeight = 1.0f;
    LONG slopPixels = 6;
    int maximumGenerators = 16;
    std::array<map::identifiers::Archetype, 3> keepers = {map::identifiers::Archetype::Scientist,
                                                          map::identifiers::Archetype::Shaman,
                                                          map::identifiers::Archetype::Bard};
    uint32_t alienShare = 60, alienPicks = 30;
    float alienFromWalls = 24.0f, alienNoWalls = 60.0f;
    std::array<uint32_t, 14> palette = {0xcb425b, 0xed4d41, 0xeb7040, 0xf8bf43, 0xf2e76d, 0xacd96d, 0x6ecc66,
                                        0x60bfbf, 0x75b1e9, 0x6c76f0, 0x8656e8, 0xad56da, 0xee71e1, 0xee6096};
};
inline constexpr Settings settings{};
}

namespace feature {
export class Generator {
public:
    void tick();
    void onLeavePlanet(bool destroyed);
    void onMouseDown(int button);
    bool tint(char* state, float color[3], float& level);
    [[nodiscard]] static const char16_t* localeText(uint32_t table, uint32_t identifier);
    [[nodiscard]] static std::optional<bool> placement(void* strategy, char* tool, bool show, const char16_t** failText);
    [[nodiscard]] std::optional<generator::Color> alienColor(game::GameData object) const;
    [[nodiscard]] game::ToolObject alienTarget(void* combatant, uint32_t politicalId) const;
    void answerStrike(game::ToolObject target, bool wasStanding);

private:
    struct Tracked {
        game::ToolObject object;
    };
    struct Tinting {
        uint32_t key = 0, group = 0;
        generator::Color color{1.0f, 1.0f, 1.0f}, target{1.0f, 1.0f, 1.0f};
        float level = 0.0f;
        bool want = false;
    };
    struct Click {
        game::ToolObject target;
        DWORD tick = 0;
        POINT at{};
    };

    static void picked(uint32_t packedColor, void* context);
    void applyPick(uint32_t packedColor);
    void forget();
    [[nodiscard]] std::span<const Tracked> followed() const;
    [[nodiscard]] bool known(game::ToolObject object) const;
    void scan();
    void placeForAlien(game::PlanetRecord record);
    void fade(float elapsed);
    [[nodiscard]] game::ToolObject hoveredGenerator() const;
    void openPalette(game::ToolObject object);

    game::Planet planet;
    std::array<Tracked, generator::settings.maximumGenerators> generators;
    int generatorCount = 0;
    DWORD nextScan = 0, lastTick = 0, nextAnswer = 0;
    Tinting tinting;
    bool alienChecked = false, placingAlien = false;
    Click lastClick;
};
}

