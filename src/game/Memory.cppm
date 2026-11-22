
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <Windows.h>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <span>
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
    static constexpr size_t capacity = 64;

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
        if (text.size() > 2 * capacity)
            throw "a signature holds at most 64 bytes";
        for (char character : text)
            if (digit(character) < 0)
                throw "the bytes must be written in hex digits";
        return text.size();
    }

    const char* digits = "";
    size_t digitCount = 0;
};

bool original(uint32_t gameAddress, std::span<uint8_t> bytes);
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

namespace {
constexpr uint32_t pageSize = 0x1000;

std::span<const uint8_t> relocationTable() {
    auto* legacyHeader = reinterpret_cast<const IMAGE_DOS_HEADER*>(base);
    if (!readable(legacyHeader, sizeof(IMAGE_DOS_HEADER)))
        return {};
    auto* headers = reinterpret_cast<const IMAGE_NT_HEADERS*>(base + legacyHeader->e_lfanew);
    if (!readable(headers, sizeof(IMAGE_NT_HEADERS)))
        return {};
    const IMAGE_DATA_DIRECTORY& directory = headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];
    auto* table = reinterpret_cast<const uint8_t*>(base + directory.VirtualAddress);
    if (!directory.VirtualAddress || !readable(table, directory.Size))
        return {};
    return std::span(table, directory.Size);
}
}

bool original(uint32_t gameAddress, std::span<uint8_t> bytes) {
    auto* code = address<const uint8_t*>(gameAddress);
    if (!readable(code, bytes.size()))
        return false;
    memcpy(bytes.data(), code, bytes.size());
    const uint32_t moved = uint32_t(base - imageBase);
    if (!moved)
        return true;
    const std::span<const uint8_t> table = relocationTable();
    const uint32_t first = gameAddress - imageBase, end = first + uint32_t(bytes.size());
    for (size_t at = 0; at + sizeof(IMAGE_BASE_RELOCATION) <= table.size();) {
        auto* block = reinterpret_cast<const IMAGE_BASE_RELOCATION*>(table.data() + at);
        if (block->SizeOfBlock < sizeof(IMAGE_BASE_RELOCATION) || block->SizeOfBlock > table.size() - at)
            return false;
        const uint32_t page = block->VirtualAddress;
        const std::span entries(reinterpret_cast<const uint16_t*>(block + 1),
                                (block->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(uint16_t));
        if (page < end && page + pageSize + 3 > first)
            for (uint16_t entry : entries) {
                const uint32_t dword = page + (entry & 0xfff);
                if (entry >> 12 != IMAGE_REL_BASED_HIGHLOW || dword + 4 <= first || dword >= end)
                    continue;
                auto* relocated = reinterpret_cast<const uint8_t*>(base + dword);
                uint32_t value = 0;
                if (!readable(relocated, sizeof(value)))
                    return false;
                memcpy(&value, relocated, sizeof(value));
                value -= moved;
                for (uint32_t byte = 0; byte < sizeof(value); ++byte)
                    if (dword + byte >= first && dword + byte < end)
                        bytes[dword + byte - first] = uint8_t(value >> 8 * byte);
            }
        at += block->SizeOfBlock;
    }
    return !table.empty();
}

bool matches(uint32_t gameAddress, Signature signature) {
    std::array<uint8_t, Signature::capacity> bytes{};
    const std::span<uint8_t> read = std::span(bytes).first(signature.size());
    if (!original(gameAddress, read))
        return false;
    for (size_t i = 0; i < read.size(); ++i)
        if (read[i] != signature[i])
            return false;
    return true;
}
}

