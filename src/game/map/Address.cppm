
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <concepts>
#include <cstdint>
#include <type_traits>
#include <utility>

export module beatuy.game.map:address;

import beatuy.game.memory;

export namespace map {
enum class Kind : uint8_t { Checked, Hooked, Slot };

inline constexpr uint32_t imageSize = 26271744, timestamp = 1727737552;

template <class FunctionPointer>
concept GameFunction = std::is_pointer_v<FunctionPointer> && std::is_function_v<std::remove_pointer_t<FunctionPointer>>;

class Entry {
public:
    Entry(uint32_t address, game::Signature signature, Kind kind);

    template <class T = void*> T at() const {
        return game::address<T>(address);
    }
    bool matches() const;

    const uint32_t address;
    const game::Signature signature;
    const Kind kind;
};

bool verify();
bool hostMatches();

template <GameFunction FunctionPointer> class Function : public Entry {
public:
    Function(uint32_t address, game::Signature signature) : Entry(address, signature, Kind::Checked) {}

    template <class... Arguments>
        requires std::invocable<FunctionPointer, Arguments...>
    decltype(auto) operator()(Arguments&&... arguments) const {
        return at<FunctionPointer>()(std::forward<Arguments>(arguments)...);
    }
};

template <GameFunction FunctionPointer> class Hooked : public Entry {
public:
    Hooked(uint32_t address, game::Signature signature) : Entry(address, signature, Kind::Hooked) {}

    template <class... Arguments>
        requires std::invocable<FunctionPointer, Arguments...>
    decltype(auto) callOriginal(Arguments&&... arguments) const {
        return original(std::forward<Arguments>(arguments)...);
    }
    template <class... Arguments>
        requires std::invocable<FunctionPointer, Arguments...>
    decltype(auto) operator()(Arguments&&... arguments) const {
        return at<FunctionPointer>()(std::forward<Arguments>(arguments)...);
    }

    FunctionPointer original = nullptr;
};

template <GameFunction FunctionPointer> class Slot : public Entry {
public:
    Slot(uint32_t virtualTable, uint32_t offset, uint32_t function)
        : Entry(virtualTable + offset, {}, Kind::Slot), virtualTable(virtualTable), offset(offset),
          function(function) {}

    template <class... Arguments>
        requires std::invocable<FunctionPointer, Arguments...>
    decltype(auto) callOriginal(Arguments&&... arguments) const {
        return original(std::forward<Arguments>(arguments)...);
    }

    const uint32_t virtualTable, offset, function;
    FunctionPointer original = nullptr;
};

template <class T> class Global : public Entry {
public:
    Global(uint32_t address, game::Signature signature = {}) : Entry(address, signature, Kind::Checked) {}

    T& operator*() const {
        return *at<T*>();
    }
    T* operator->() const {
        return at<T*>();
    }
    T& operator[](size_t index) const {
        return at<T*>()[index];
    }
};

class Location : public Entry {
public:
    Location(uint32_t address, game::Signature signature = {}) : Entry(address, signature, Kind::Checked) {}
};

struct Noun {
    uint32_t noun, thunk;
};
}

