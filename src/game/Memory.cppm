
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <Windows.h>
#include <cstddef>
#include <cstdint>
#include <string_view>

export module beatuy.game.memory;

export namespace game {
inline constinit uintptr_t base = 0;
inline constexpr uint32_t imageBase = 0x400000;

template <class T = void*> T address(uint32_t gameAddress) {
    return reinterpret_cast<T>(base + gameAddress - imageBase);
}

inline uint32_t gameAddress(const void* pointer) {
    return uint32_t(reinterpret_cast<uintptr_t>(pointer) - base + imageBase);
}

template <class T = uint32_t> T& field(const void* object, uint32_t offset) {
    return *reinterpret_cast<T*>(static_cast<char*>(const_cast<void*>(object)) + offset);
}

inline void** virtualTable(const void* object) {
    return *reinterpret_cast<void** const*>(object);
}

template <class R, class... A> R callVirtual(const void* object, uint32_t slot, A... arguments) {
    return reinterpret_cast<R(__thiscall*)(const void*, A...)>(virtualTable(object)[slot / 4])(object, arguments...);
}

bool readable(const void* pointer, size_t size);

class Signature {
public:
    constexpr Signature() = default;
    consteval Signature(const char* text) : digits(text), digitCount(checked(text)) {}

    constexpr size_t size() const {
        return digitCount / 2;
    }
    constexpr bool empty() const {
        return digitCount == 0;
    }
    constexpr uint8_t operator[](size_t index) const {
        return uint8_t(digit(digits[2 * index]) << 4 | digit(digits[2 * index + 1]));
    }

private:
    static constexpr int digit(char character) {
        if (character >= '0' && character <= '9')
            return character - '0';
        if (character >= 'a' && character <= 'f')
            return character - 'a' + 10;
        return character >= 'A' && character <= 'F' ? character - 'A' + 10 : -1;
    }
    static consteval size_t checked(std::string_view text) {
        if (text.size() % 2)
            throw "the bytes must be written as pairs of hex digits";
        for (char character : text)
            if (digit(character) < 0)
                throw "the bytes must be written in hex digits";
        return text.size();
    }

    const char* digits = "";
    size_t digitCount = 0;
};

bool matches(uint32_t gameAddress, Signature signature);
}

module :private;

namespace game {
bool readable(const void* pointer, size_t size) {
    MEMORY_BASIC_INFORMATION information{};
    if (!pointer || !VirtualQuery(pointer, &information, sizeof(information)) || information.State != MEM_COMMIT)
        return false;
    auto end = reinterpret_cast<uintptr_t>(information.BaseAddress) + information.RegionSize;
    return reinterpret_cast<uintptr_t>(pointer) + size <= end && !(information.Protect & (PAGE_NOACCESS | PAGE_GUARD));
}

bool matches(uint32_t gameAddress, Signature signature) {
    auto* code = address<const uint8_t*>(gameAddress);
    if (!readable(code, signature.size()))
        return false;
    for (size_t i = 0; i < signature.size(); ++i)
        if (code[i] != signature[i])
            return false;
    return true;
}
}

