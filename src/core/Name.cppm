
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <array>
#include <cstddef>

export module beatuy.core.name;

export namespace beatuy {
inline constexpr wchar_t wideName[] = L"Beatuy!SpaceStage";

template <size_t... sizes> consteval auto join(const wchar_t (&... parts)[sizes]) {
    std::array<wchar_t, (sizes + ...) - sizeof...(sizes) + 1> joined{};
    size_t at = 0;
    auto append = [&](const wchar_t* part, size_t size) {
        for (size_t i = 0; i + 1 < size; ++i)
            joined[at++] = part[i];
    };
    (append(parts, sizes), ...);
    return joined;
}
}

