
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <Windows.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <numbers>
#include <optional>
#include <span>

module beatuy.feature.generator;

import beatuy.core.language;
import beatuy.core.mod;
import beatuy.game;

namespace generator {
namespace {
constexpr uint32_t pickedFlag = 0x80000000, alienFlag = 0x01000000, colorMask = 0x00ffffff;

game::RecordObject element(game::PlanetRecord record) {
    if (!record || !record.objectsReadable())
        return game::RecordObject();
    const game::Array<game::RecordObject> objects = record.objects();
    char* begin = objects.data();
    char* end = objects.dataEnd();
    size_t bytes = size_t(end - begin);
    if (!begin || end <= begin || bytes % game::RecordObject::byteSize || bytes > game::RecordObject::byteSize * 4096 ||
        !game::readable(begin, bytes))
        return game::RecordObject();
    const auto found = std::ranges::find_if(
        objects, [](game::RecordObject object) { return object.type() == settings.tool && object.placedByTool(); });
    return found != objects.end() ? *found : game::RecordObject();
}

Color unpackColor(uint32_t color) {
    return {((color >> 16) & 255) / 255.0f, ((color >> 8) & 255) / 255.0f, (color & 255) / 255.0f};
}

std::optional<Color> colourOf(game::RecordObject element) {
    uint32_t code = element.spare();
    if (code & pickedFlag)
        return unpackColor(code & colorMask);
    const game::Empire empire = code & alienFlag ? game::planetOwner() : game::playerEmpire();
    if (Color color; empire.colour(color))
        return color;
    return std::nullopt;
}

bool claimedByOthers() {
    using enum map::identifiers::Technology;
    const uint32_t systemOwner = game::activeStarRecord() ? game::activeStarRecord().politicalId() : uint32_t(-1);
    if (systemOwner != uint32_t(-1) && systemOwner != game::playerPoliticalId())
        return true;
    constexpr std::array settled{Tribe, City, Civilization};
    const map::identifiers::Technology technology = game::planetTechnology();
    if (std::ranges::find(settled, technology) != settled.end())
        return true;
    return technology == Empire && game::planetOwner() != game::playerEmpire();
}

const char16_t* claimedText() {
    alignas(4) static std::array<char, 0x14> text{};
    static bool made = false;
    if (!made) {
        made = true;
        map::application::localizedStringConstructor(text.data(), settings.claimedTable, settings.claimedText, nullptr);
    }
    return map::application::localizedStringText(text.data());
}

constexpr uint32_t mix(uint32_t value) {
    value ^= value >> 16;
    value *= 0x7feb352d;
    value ^= value >> 15;
    value *= 0x846ca68b;
    value ^= value >> 16;
    return value;
}

struct CityReach {
    game::Vector centre;
    float reach;
};

std::optional<CityReach> ownerCity(uint32_t politicalId) {
    for (game::City city : game::each<game::City>(map::civilization::cities)) {
        if (!city || !city.alive() || city.politicalId() != politicalId)
            continue;
        const game::Vector* at = city.place().position();
        if (!at)
            continue;
        game::Walls walls = city.walls();
        float outer = walls ? walls.outerRadius() : 0.0f;
        return CityReach{.centre = *at,
                         .reach = std::isfinite(outer) && outer > 1.0f && outer < 400.0f
                                      ? outer + settings.alienFromWalls
                                      : settings.alienNoWalls};
    }
    return std::nullopt;
}

bool keepsGenerators(map::identifiers::Archetype ideology) {
    return std::ranges::find(settings.keepers, ideology) != settings.keepers.end();
}

bool nearby(const POINT& first, const POINT& second) {
    return labs(first.x - second.x) <= settings.slopPixels && labs(first.y - second.y) <= settings.slopPixels;
}

struct LocaleText {
    uint32_t table, identifier;
    bool description;
};
constexpr std::array<LocaleText, 3> localeTexts{{
    {.table = settings.spaceGameTable, .identifier = settings.tool, .description = false},
    {.table = settings.toolTable, .identifier = settings.toolNameText, .description = false},
    {.table = settings.toolTable, .identifier = settings.toolDescriptionText, .description = true},
}};
}
}

namespace feature {
std::span<const Generator::Tracked> Generator::followed() const {
    return {generators.data(), size_t(generatorCount)};
}

void Generator::forget() {
    for (const Tracked& tracked : followed())
        tracked.object.release();
    generatorCount = 0;
}

bool Generator::known(game::ToolObject object) const {
    return std::ranges::any_of(followed(), [&](const Tracked& tracked) { return tracked.object == object; });
}

void Generator::scan() {
    const generator::Settings& settings = generator::settings;
    for (game::ToolObject object : game::each<game::ToolObject>(map::planet::toolObjects)) {
        if (!object || !object.alive() || object.tool() != settings.tool || known(object) ||
            generatorCount >= settings.maximumGenerators)
            continue;
        game::Combatant fighter = object.fighter();
        float was = fighter.health();
        fighter.maximumHealth() = settings.hitPoints;
        if (!(was > 0.0f) || was > settings.hitPoints)
            fighter.health() = settings.hitPoints;
        object.addReference();
        generators[generatorCount++] = Tracked{object};
    }
}

void Generator::placeForAlien(game::PlanetRecord record) {
    const generator::Settings& settings = generator::settings;
    if (alienChecked || game::planetTechnology() != map::identifiers::Technology::Empire)
        return;
    game::Empire owner = game::planetOwner();
    if (!owner)
        return;
    uint32_t politicalId = owner.politicalId();
    if (politicalId == game::playerPoliticalId() || politicalId == uint32_t(-1)) {
        alienChecked = true;
        return;
    }
    const std::optional<generator::CityReach> city = generator::ownerCity(politicalId);
    if (!city)
        return;
    const auto [centre, reach] = *city;
    alienChecked = true;
    map::identifiers::Archetype ideology = owner.archetype();
    if (!generator::keepsGenerators(ideology))
        return;
    uint32_t identifier = record.identifier(), roll = generator::mix(identifier ^ 0x9e3779b9u);
    if (roll % 100 >= settings.alienShare || game::hasPlanetObject(record, settings.tool) ||
        generator::element(record))
        return;
    game::Vector up = game::unit(centre);
    game::Vector side = game::unit(
        game::cross(up, std::fabs(up.z) < 0.9f ? game::Vector{0.0f, 0.0f, 1.0f} : game::Vector{1.0f, 0.0f, 0.0f}));
    game::Vector ahead = game::cross(up, side);
    constexpr float pi = std::numbers::pi_v<float>;
    float first = (generator::mix(identifier) % 360) * pi / 180.0f;
    for (int i = 0; i < 8; ++i) {
        float angle = first + i * pi / 4.0f;
        game::Vector spot = game::ground(centre + (side * cosf(angle) + ahead * sinf(angle)) * reach);
        if (game::inWater(spot))
            continue;
        placingAlien = true;
        game::placeToolObject(settings.tool, spot, settings.hitPoints);
        placingAlien = false;
        game::RecordObject element = generator::element(record);
        uint32_t picks = generator::mix(identifier ^ 0x5bd1e995u);
        uint32_t code = generator::alienFlag;
        if (picks % 100 < settings.alienPicks)
            code |= generator::pickedFlag | settings.palette[(picks / 100) % settings.palette.size()];
        if (element) {
            element.spare() = code;
            element.health() = settings.hitPoints;
        }
        return;
    }
}

void Generator::fade(float elapsed) {
    const generator::Settings& settings = generator::settings;
    float step = elapsed > 0.1f ? 0.1f / settings.fadeSeconds : elapsed / settings.fadeSeconds;
    tinting.level = tinting.want ? fminf(1.0f, tinting.level + step) : fmaxf(0.0f, tinting.level - step);
    float shift = elapsed > 0.1f ? 0.1f / settings.shiftSeconds : elapsed / settings.shiftSeconds;
    for (size_t i = 0; i < tinting.color.size(); ++i)
        tinting.color[i] += (tinting.target[i] - tinting.color[i]) * (shift > 1.0f ? 1.0f : shift);
}

void Generator::picked(uint32_t packedColor, void*) {
    Mod::get().generator.applyPick(packedColor);
}

void Generator::applyPick(uint32_t packedColor) {
    game::RecordObject element = generator::element(game::activeRecord());
    if (!element)
        return;
    uint32_t old = element.spare();
    uint32_t code = (old & generator::alienFlag) | generator::pickedFlag | (packedColor & generator::colorMask);
    element.spare() = code;
    tinting.target = generator::unpackColor(packedColor & generator::colorMask);
}

std::optional<generator::Color> Generator::alienColor(game::GameData object) const {
    const game::ToolObject tool(object ? object.cast(map::identifiers::Type::ToolObject) : nullptr);
    if (!tool || tool.tool() != generator::settings.tool)
        return std::nullopt;
    const game::RecordObject element = generator::element(game::activeRecord());
    if (!placingAlien && !(element && (element.spare() & generator::alienFlag)))
        return std::nullopt;
    if (generator::Color color; game::planetOwner().colour(color))
        return color;
    return std::nullopt;
}

game::ToolObject Generator::alienTarget(void* combatant, uint32_t politicalId) const {
    if (!combatant || politicalId != game::playerPoliticalId())
        return game::ToolObject();
    const auto found = std::ranges::find_if(
        followed(), [&](const Tracked& tracked) { return tracked.object.fighter().raw() == combatant; });
    if (found == followed().end() || !alienColor(found->object))
        return game::ToolObject();
    return found->object;
}

void Generator::answerStrike(game::ToolObject target, bool wasStanding) {
    const generator::Settings& settings = generator::settings;
    const uint32_t owner = game::planetOwnerPoliticalId(), player = game::playerPoliticalId();
    if (!target || owner == uint32_t(-1) || owner == player)
        return;
    const bool destroyed = wasStanding && !(target.fighter().health() > 0.0f);
    const DWORD now = GetTickCount();
    if (!destroyed && LONG(now - nextAnswer) < 0)
        return;
    nextAnswer = now + settings.answerMilliseconds;
    using enum map::identifiers::Relation;
    const float change = game::apply(owner, player, destroyed ? DestroyedBuilding : Attacked,
                                     destroyed ? settings.destroyedWeight : settings.strikeWeight);
    const game::Vector at = target.position();
    game::City nearest;
    float best = 0.0f;
    for (game::City city : game::each<game::City>(map::civilization::cities)) {
        const game::Vector* place = city && city.alive() && city.politicalId() == owner ? city.place().position()
                                                                                       : nullptr;
        if (!place)
            continue;
        const game::Vector offset = *place - at;
        const float distance = game::dot(offset, offset);
        if (!nearest || distance < best) {
            nearest = city;
            best = distance;
        }
    }
    game::showColonyMood(nearest, change, at);
}

game::ToolObject Generator::hoveredGenerator() const {
    game::GameData hovered = game::hoveredObject();
    if (!hovered)
        return game::ToolObject();
    void* object = hovered.cast(map::identifiers::Type::ToolObject);
    const std::span<const Tracked> tracked = followed();
    const auto found = std::ranges::find_if(tracked, [&](const Tracked& candidate) {
        return candidate.object.raw() == object || candidate.object == hovered;
    });
    return found != tracked.end() ? found->object : game::ToolObject();
}

void Generator::openPalette(game::ToolObject object) {
    game::RecordObject element = generator::element(game::activeRecord());
    if (!element)
        return;
    uint32_t code = element.spare();
    const uint32_t owner = game::planetOwnerPoliticalId();
    if ((code & generator::alienFlag) || (owner != uint32_t(-1) && owner != game::playerPoliticalId()))
        return;
    if (!Mod::get().picker.reaches(object.position()))
        return;
    const generator::Color now = generator::colourOf(element).value_or(generator::Color{1.0f, 1.0f, 1.0f});
    Mod::get().picker.open(now.data(), object.position(), &Generator::picked, nullptr);
}

bool Generator::tint(char*, float color[3], float& level) {
    if (tinting.level <= 0.0f || !planet)
        return false;
    game::PropertyList properties = game::terrainProperties();
    if (!properties || properties.instance() != tinting.key || properties.group() != tinting.group)
        return false;
    std::ranges::copy(tinting.color, color);
    level = tinting.level;
    return true;
}

const char16_t* Generator::localeText(uint32_t table, uint32_t identifier) {
    const auto asked = std::ranges::find_if(generator::localeTexts, [&](const generator::LocaleText& text) {
        return text.identifier == identifier && text.table == table;
    });
    if (asked == generator::localeTexts.end())
        return nullptr;
    const Language language = activeLanguage();
    const generator::Translation& translation = inLanguage(generator::translations, language);
    return asked->description ? translation.description : translation.name;
}

std::optional<bool> Generator::placement(void* strategy, char* tool, bool show, const char16_t** failText) {
    if (!tool || game::Tool(tool).identifier() != generator::settings.tool)
        return std::nullopt;
    if (!map::space::toolStrategyUpdate(strategy, tool, show, failText))
        return false;
    if (!generator::claimedByOthers())
        return std::nullopt;
    map::space::toolShowEvent(tool, generator::settings.claimedEvent, show);
    if (failText)
        *failText = generator::claimedText();
    return false;
}

void Generator::onMouseDown(int button) {
    if (button != int(map::identifiers::Button::Left) || !game::onPlanet())
        return;
    if (Mod::get().picker.isOpen()) {
        Mod::get().picker.cancel();
        return;
    }
    game::ToolObject target = hoveredGenerator();
    POINT at{};
    GetCursorPos(&at);
    DWORD now = GetTickCount(), gap = now - lastClick.tick;
    bool same = target && target == lastClick.target && generator::nearby(at, lastClick.at);
    if (same && gap < generator::settings.echoMilliseconds)
        return;
    if (same && gap <= GetDoubleClickTime()) {
        lastClick = Click{};
        openPalette(target);
        return;
    }
    lastClick = Click{.target = target, .tick = now, .at = at};
}

void Generator::onLeavePlanet(bool) {
    Mod::get().picker.close();
}

void Generator::tick() {
    DWORD now = GetTickCount();
    float elapsed = lastTick ? (now - lastTick) / 1000.0f : 0.0f;
    lastTick = now;
    if (game::Planet here = game::onPlanet() ? game::activePlanet() : game::Planet(); here != planet) {
        forget();
        planet = here;
        tinting = Tinting{};
        alienChecked = false;
        lastClick = Click{};
        if (game::PlanetRecord record = planet ? game::activeRecord() : game::PlanetRecord()) {
            tinting.key = record.terrainInstance();
            tinting.group = record.terrainGroup();
        }
    }
    if (!planet)
        return;
    fade(elapsed);
    if (LONG(now - nextScan) < 0)
        return;
    nextScan = now + generator::settings.scanMilliseconds;
    game::PlanetRecord record = game::activeRecord();
    placeForAlien(record);
    game::RecordObject element = generator::element(record);
    for (int i = generatorCount - 1; i >= 0; --i) {
        Tracked& tracked = generators[i];
        float health = tracked.object.alive() ? tracked.object.fighter().health() : 0.0f;
        if (health <= 0.0f) {
            tracked.object.release();
            generators[i] = generators[--generatorCount];
            continue;
        }
        if (element)
            element.health() = health;
    }
    scan();
    std::optional<generator::Color> colour;
    if (generatorCount > 0 && element)
        colour = generator::colourOf(element);
    const bool want = colour.has_value();
    if (want) {
        bool first = !tinting.want && tinting.level <= 0.0f;
        tinting.target = *colour;
        if (first)
            tinting.color = *colour;
    }
    tinting.want = want;
}
}

