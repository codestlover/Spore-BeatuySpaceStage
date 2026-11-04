
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <Windows.h>
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <utility>

module beatuy.feature.orders;

import beatuy.game;

namespace feature::reply {
namespace {
using map::identifiers::Archetype;

struct Settings {
    uint32_t repeatMilliseconds = 3000;
};
constexpr Settings settings{};

enum class Line : uint8_t { Joins, Rally, Attacks };
constexpr size_t lineCount = size_t(Line::Attacks) + 1;
constexpr size_t archetypeCount = size_t(Archetype::Grob) + 1;
template <class Value> using PerArchetype = std::array<Value, archetypeCount>;

constexpr PerOrder<Line> lineOf{Line::Joins, Line::Rally, Line::Rally, Line::Joins, Line::Attacks};
constexpr std::array<PerArchetype<uint32_t>, lineCount> voices{{
    {0x99422df0, 0xd3feb879, 0x350cfd88, 0x51d62aab, 0xf628f0e1, 0x8b050578, 0xdb4ad9f1, 0xb07352e1, 0x2b67bd78},
    {0xee00df14, 0x0f6ba52d, 0x12a33e08, 0xe3f4933b, 0x170454dd, 0x13284838, 0x70345d35, 0x8af423b1, 0x03305914},
    {0xef2fc9e8, 0x94a77229, 0xe4129110, 0x6139d767, 0x6263b501, 0x367a1940, 0x62d3be51, 0x0199e701, 0x7aa79140},
}};
int32_t since(uint32_t now, uint32_t moment) {
    return int32_t(now - moment);
}

bool sameStanding(const Standing& now, const Standing& before) {
    if (now.order != before.order)
        return false;
    if (now.order != Order::ProtectEscort)
        return true;
    if (now.guardsAll || before.guardsAll)
        return now.guardsAll == before.guardsAll;
    return now.guardedCount == before.guardedCount &&
           std::ranges::all_of(now.guardedShips(), [&before](uint32_t ship) { return before.guards(ship); });
}

Standing withinEscort(Standing standing, const game::Posse& escort) {
    const auto absent = [&escort](uint32_t ship) { return !escort.has(ship); };
    const std::span<uint32_t> listed = std::span(standing.guarded).first(standing.guardedCount);
    standing.guardedCount = uint8_t(standing.guardedCount - std::ranges::remove_if(listed, absent).size());
    if (standing.guardsAll || standing.guardedCount)
        return standing;
    if (standing.order == Order::ProtectEscort)
        standing.order = Order::ProtectPlayer;
    standing.guardsAll = true;
    return standing;
}
}
}

namespace feature {
void EscortReplies::expect(uint32_t politicalId, const Standing& before) {
    const std::span<const Pending> changed = std::span(pending).first(pendingCount);
    if (std::ranges::find(changed, politicalId, &Pending::politicalId) != changed.end() ||
        pendingCount == pending.size())
        return;
    pending[pendingCount++] = Pending{.politicalId = politicalId, .before = before};
}

void EscortReplies::tick(const OrderBook& book) {
    const uint32_t now = GetTickCount();
    const auto old = [now](const Answer& answer) {
        return reply::since(now, answer.at) >= int32_t(reply::settings.repeatMilliseconds);
    };
    answerCount -= std::ranges::remove_if(std::span(answers).first(answerCount), old).size();
    const size_t count = std::exchange(pendingCount, 0);
    const game::PlayerShipManager manager = game::inSpace() ? game::shipManager() : game::PlayerShipManager();
    if (!count || !manager)
        return;
    const game::Posse escort = manager.escort();
    for (const Pending& changed : std::span(pending).first(count)) {
        const Standing standing = book.standingOf(changed.politicalId);
        if (escort.has(changed.politicalId) &&
            !reply::sameStanding(reply::withinEscort(standing, escort), reply::withinEscort(changed.before, escort)))
            say(standing, now);
    }
}

void EscortReplies::reset() {
    pendingCount = 0;
    answerCount = 0;
}

void EscortReplies::say(const Standing& standing, uint32_t now) {
    const game::Empire empire = game::empireOf(standing.politicalId);
    if (!empire)
        return;
    const int archetype = int(empire.archetype());
    if (size_t(archetype) >= reply::archetypeCount)
        return;
    const reply::Line line = reply::lineOf[size_t(standing.order)];
    const uint32_t voice = reply::voices[size_t(line)][size_t(archetype)];
    const std::span<Answer> said = std::span(answers).first(answerCount);
    const auto last = std::ranges::find(said, standing.politicalId, &Answer::politicalId);
    if (last != said.end() && last->voice == voice)
        return;
    game::playAudio(voice);
    answerCount -= std::ranges::remove(said, standing.politicalId, &Answer::politicalId).size();
    if (answerCount < answers.size())
        answers[answerCount++] = Answer{.politicalId = standing.politicalId, .voice = voice, .at = now};
}
}

