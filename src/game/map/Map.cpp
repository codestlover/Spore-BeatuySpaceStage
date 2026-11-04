
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <Windows.h>
#include <array>
#include <cstdint>
#include <span>

module beatuy.game.map;

import beatuy.game.memory;

namespace map {
namespace {
struct Registry {
    std::array<const Entry*, 512> entries{};
    size_t count = 0;

    std::span<const Entry* const> all() const {
        return std::span(entries).first(count);
    }
};
constinit Registry registry;
}

Entry::Entry(uint32_t address, game::Signature signature, Kind kind)
    : address(address), signature(signature), kind(kind) {
    if (registry.count < registry.entries.size())
        registry.entries[registry.count++] = this;
}

bool Entry::matches() const {
    return signature.empty() || game::matches(address, signature);
}

bool verify() {
    for (const Entry* entry : registry.all())
        if (entry->kind == Kind::Checked && !entry->signature.empty() && !entry->matches())
            return false;
    return registry.count < registry.entries.size();
}

bool hostMatches() {
    auto* legacyHeader = reinterpret_cast<const IMAGE_DOS_HEADER*>(game::base);
    auto* headers = reinterpret_cast<const IMAGE_NT_HEADERS*>(game::base + legacyHeader->e_lfanew);
    return headers->FileHeader.Machine == IMAGE_FILE_MACHINE_I386 && headers->OptionalHeader.SizeOfImage == imageSize &&
           headers->FileHeader.TimeDateStamp == timestamp;
}
}

