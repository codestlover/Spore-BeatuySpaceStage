
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <Windows.h>
#include <detours.h>
#include <array>
#include <bit>
#include <cstdint>
#include <cstring>
#include <span>
#include <type_traits>

extern "C" LONG WINAPI DetourTransactionAbort();

export module beatuy.core.hooks;

import beatuy.game;

namespace hooks {
constexpr DWORD executableProtections =
    PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY;

bool executable(const void* code) {
    MEMORY_BASIC_INFORMATION information{};
    return code && VirtualQuery(code, &information, sizeof(information)) && information.State == MEM_COMMIT &&
           (information.Protect & executableProtections);
}

uintptr_t foreignTarget(const void* code) {
    auto* instruction = static_cast<const uint8_t*>(code);
    if (instruction[0] != 0xe9)
        return 0;
    int32_t offset;
    memcpy(&offset, instruction + 1, sizeof(offset));
    uintptr_t target = reinterpret_cast<uintptr_t>(instruction) + 5 + uintptr_t(offset);
    if (target >= game::base && target - game::base < map::imageSize)
        return 0;
    return executable(reinterpret_cast<const void*>(target)) ? target : 0;
}

bool jumpTo(void* code, const void* destination) {
    auto* instruction = static_cast<uint8_t*>(code);
    DWORD old = 0;
    if (!VirtualProtect(instruction, 5, PAGE_EXECUTE_READWRITE, &old))
        return false;
    std::array<uint8_t, 5> jump{0xe9};
    int32_t offset = int32_t(reinterpret_cast<uintptr_t>(destination) - (reinterpret_cast<uintptr_t>(instruction) + 5));
    memcpy(jump.data() + 1, &offset, sizeof(offset));
    memcpy(instruction, jump.data(), jump.size());
    VirtualProtect(instruction, 5, old, &old);
    FlushInstructionCache(GetCurrentProcess(), instruction, 5);
    return true;
}

bool attach(const map::Entry& entry, void** original, void* detour) {
    void* code = entry.at();
    if (game::matches(entry.address, entry.signature)) {
        *original = code;
        LONG error = DetourTransactionBegin();
        if (error == NO_ERROR)
            error = DetourUpdateThread(GetCurrentThread());
        if (error == NO_ERROR)
            error = DetourAttach(original, detour);
        if (error != NO_ERROR) {
            DetourTransactionAbort();
            return false;
        }
        return DetourTransactionCommit() == NO_ERROR;
    }
    uintptr_t next = foreignTarget(code);
    if (!next)
        return false;
    *original = reinterpret_cast<void*>(next);
    return jumpTo(code, detour);
}
}

export namespace hooks {
template <class Type>
concept Detour = std::is_function_v<Type>;

template <map::GameFunction FunctionPointer, Detour Replacement>
bool install(map::Hooked<FunctionPointer>& entry, Replacement* detour, void** copy = nullptr) {
    bool installed = attach(entry, reinterpret_cast<void**>(&entry.original), std::bit_cast<void*>(detour));
    if (copy)
        *copy = std::bit_cast<void*>(entry.original);
    return installed;
}

template <map::GameFunction FunctionPointer, Detour Replacement>
bool install(map::Slot<FunctionPointer>& entry, Replacement* detour) {
    auto* slot = entry.template at<void**>();
    void* current = *slot;
    bool installed = current == game::address(entry.function) || executable(current);
    DWORD old = 0;
    if (installed && VirtualProtect(slot, 4, PAGE_READWRITE, &old)) {
        entry.original = std::bit_cast<FunctionPointer>(current);
        *slot = std::bit_cast<void*>(detour);
        VirtualProtect(slot, 4, old, &old);
    } else {
        installed = false;
    }
    return installed;
}

bool patch(const map::Entry& entry, uint32_t offset, std::span<const uint8_t> replacement) {
    auto* code = static_cast<uint8_t*>(entry.at()) + offset;
    DWORD old = 0;
    const bool patched = entry.matches() && VirtualProtect(code, replacement.size(), PAGE_EXECUTE_READWRITE, &old);
    if (patched) {
        memcpy(code, replacement.data(), replacement.size());
        VirtualProtect(code, replacement.size(), old, &old);
        FlushInstructionCache(GetCurrentProcess(), code, replacement.size());
    }
    return patched;
}
}

