
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <cstddef>
#include <cstdint>

export module beatuy.game.map:saving;

import :address;

export namespace map::saving {
Slot<bool(__thiscall*)(void*, void*)> playerShipWrite{0x14951a8, 0x10, 0xffbd10};
Slot<bool(__thiscall*)(void*, void*)> playerShipRead{0x14951a8, 0x14, 0xffbd80};

const Global<const std::byte> playerShipAttributes{0x15b6378};

const Function<void*(__thiscall*)(void*, void*, void*, uint32_t)> classSerializerConstructor{
    0x692f50, "8b44240c8b542408508b4424085250e8"};
const Function<bool(__thiscall*)(void*, void*)> classSerializerWrite{0x6928c0, "83ec0c5355568b74241c8b068b502057"};
const Function<bool(__thiscall*)(void*, void*)> classSerializerRead{0x693de0, "81ec3c0a00005355568bb4244c0a0000"};

const Function<bool(__cdecl*)(void*, const uint32_t*, uint32_t, int)> writeWords{0x93a9e0,
                                                                                 "568b742408578b7c241883ff0174098b"};
const Function<bool(__cdecl*)(void*, uint32_t*, uint32_t, int)> readWords{0x93a6f0, "837c241001538b5c240874098b038b50"};
}

