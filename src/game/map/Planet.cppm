
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <cstdint>

export module beatuy.game.map:planet;

import beatuy.game.math;
import :address;

export namespace map::planet {
const Function<char*(__thiscall*)(char*)> owner{0xc71fd0, "568bf18b8e3c01000085c9742ae81ebc"};
const Function<float (*)(char*, uint32_t*)> capture{0xc8c980, "83ec3033c0538b5c24388944241c8944"};
const Function<game::Vector*(__thiscall*)(char*, game::Vector*, const game::Vector*)> toSurface{
    0xb81780, "568b74240c57568bf9"};
const Function<bool(__thiscall*)(char*, const game::Vector*)> inWater{0xb7e530, "568bf1837e2400743f8b4e248b442408"};
const Function<float(__thiscall*)(void*)> gravity{0xb7e5e0, "e82bd3fdff3d104c65017507d9057082"};

const Location terrainStateManager{0xfa0d20};
const Location terrainProperties{0x641760, "8b4128c3"};

const Global<char*> atmosphereImages{0x16d6d30};
Hooked<void(__thiscall*)(char*, float, int)> atmosphereColor{0xfbc740, "83ec38a1cc155b014889442404db4424"};
Hooked<void(__thiscall*)(char*, float*, int)> atmosphereModel{0xfbc260, "83ec68a1c8155b0148890424db042453"};
Slot<void(__thiscall*)(void*, uint32_t, uint32_t, uint32_t, uint32_t)> bakeLayer{0x14906c8, 0x10, 0xf9e490};

const Function<void(__cdecl*)(uint32_t, const game::Vector*, float)> placeToolObject{0xff8410, "83ec5453555657e8"};
const Function<bool(__thiscall*)(char*, uint32_t)> hasPlanetObject{0xb8e190, "8b91380100002b9134010000b8e9a28b"};
const Location toolObjectThunk{0xdc31e0, "8b4c24085685c974108b018b500c68af"};
const Noun toolObjects{0x4e3fab5, 0xdc31e0};
}

