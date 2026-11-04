
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <span>

export module beatuy.feature.orders:book;

export namespace feature {
enum class Order : uint8_t {
    Free,
    ProtectPlayer,
    ProtectEscort,
    ProtectSelf,
    AttackNearby,
};
inline constexpr size_t orderCount = size_t(Order::AttackNearby) + 1;

template <class Value> using PerOrder = std::array<Value, orderCount>;

inline constexpr size_t maximumEscort = 16;

struct Standing {
    uint32_t politicalId = 0;
    Order order = Order::Free;
    bool guardsAll = true;
    uint8_t guardedCount = 0;
    std::array<uint32_t, maximumEscort> guarded{};

    [[nodiscard]] std::span<const uint32_t> guardedShips() const {
        return std::span(guarded).first(guardedCount);
    }
    [[nodiscard]] bool guards(uint32_t shipPoliticalId) const {
        return guardsAll || std::ranges::find(guardedShips(), shipPoliticalId) != guardedShips().end();
    }
};

class OrderBook {
public:
    [[nodiscard]] Order orderOf(uint32_t politicalId) const {
        const Standing* standing = find(politicalId);
        return standing ? standing->order : Order::Free;
    }
    [[nodiscard]] Standing standingOf(uint32_t politicalId) const {
        const Standing* standing = find(politicalId);
        return standing ? *standing : Standing{.politicalId = politicalId};
    }
    void setOrder(uint32_t politicalId, Order order) {
        if (Standing* standing = findOrAdd(politicalId)) {
            standing->order = order;
            ++changes;
        }
    }
    void setGuarded(uint32_t politicalId, std::span<const uint32_t> ships, bool all) {
        Standing* standing = findOrAdd(politicalId);
        if (!standing)
            return;
        standing->guardsAll = all || ships.empty();
        standing->guardedCount = uint8_t(std::min(ships.size(), maximumEscort));
        std::ranges::copy(ships.first(standing->guardedCount), standing->guarded.begin());
        ++changes;
    }
    template <std::predicate<uint32_t> Present> void keepOnly(const Present& present) {
        const auto absent = [&present](uint32_t politicalId) { return !present(politicalId); };
        size_t removed = std::ranges::remove_if(kept(), absent, &Standing::politicalId).size();
        count -= removed;
        for (Standing& standing : kept()) {
            const size_t gone =
                std::ranges::remove_if(std::span(standing.guarded).first(standing.guardedCount), absent).size();
            standing.guardedCount = uint8_t(standing.guardedCount - gone);
            removed += gone;
            settle(standing);
        }
        if (removed)
            ++changes;
    }
    [[nodiscard]] std::span<const Standing> standings() const {
        return std::span(entries).first(count);
    }
    void load(std::span<const Standing> saved) {
        count = std::min(saved.size(), maximumEscort);
        std::ranges::copy(saved.first(count), entries.begin());
        std::ranges::for_each(kept(), settle);
        ++changes;
    }
    void clear() {
        count = 0;
        ++changes;
    }
    [[nodiscard]] uint32_t revision() const {
        return changes;
    }

private:
    static void settle(Standing& standing) {
        if (standing.guardsAll || standing.guardedCount)
            return;
        if (standing.order == Order::ProtectEscort)
            standing.order = Order::ProtectPlayer;
        standing.guardsAll = true;
    }
    std::span<Standing> kept() {
        return std::span(entries).first(count);
    }
    const Standing* find(uint32_t politicalId) const {
        const auto found = std::ranges::find(standings(), politicalId, &Standing::politicalId);
        return found != standings().end() ? &*found : nullptr;
    }
    Standing* findOrAdd(uint32_t politicalId) {
        const auto found = std::ranges::find(kept(), politicalId, &Standing::politicalId);
        if (found != kept().end())
            return &*found;
        if (count >= maximumEscort)
            return nullptr;
        entries[count] = Standing{.politicalId = politicalId};
        return &entries[count++];
    }

    std::array<Standing, maximumEscort> entries{};
    size_t count = 0;
    uint32_t changes = 0;
};
}

