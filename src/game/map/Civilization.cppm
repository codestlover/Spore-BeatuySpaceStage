
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <cstdint>

export module beatuy.game.map:civilization;

import beatuy.game.math;
import :address;

export namespace map::civilization {
using TakeDamageFunction = int(__thiscall*)(void*, float, uint32_t, int, const void*, void*);

const Function<void* (*)()> relations{0xb3d3c0, "e84be501003d054c65017f1374222d02"};
const Function<void*(__thiscall*)(void*, uint32_t, uint32_t)> relationshipMap{
    0xd01af0, "56578d4424108bf1508d4c241051e85d"};
const Function<float*(__thiscall*)(void*, void*, uint32_t, uint32_t)> relationshipData{
    0xd01570, "83ec0856578d44241c508d4c241c5133"};
const Function<bool(__thiscall*)(void*, uint32_t, uint32_t)> isAtWar{
    0xd01f60, "8b442408568bf18b4c2408505150518b"};
const Function<bool(__thiscall*)(void*, uint32_t, uint32_t)> isAllied{
    0xd01ff0, "8b442408568bf18b4c2408505150518b"};
const Location isAlliedBit{0xd01ff0 + 0x26, "d1e8"};
Hooked<float(__thiscall*)(void*, uint32_t, uint32_t, uint32_t, float)> applyRelationship{
    0xd06280, "d94424108b44240c5651d91c24508b44"};

const Function<const float*(__thiscall*)(char*, float*)> empireColor{
    0xc32e30, "81ec8400000053558be98b45108d880d"};

const Noun cities{0x18c43e8, 0xacdf30};
const Noun vehicles{0x18c6de8, 0xae7250};
const Function<void (*)(void*, float, const game::Vector*)> moodFloater{
    0xff50a0, "8b4c240485c97473f30f10442408d905"};

Hooked<void(__thiscall*)(void*, void*, int, int)> addOrder{0xcac1a0, "83ec1855578bf9e8b4b1ffff8b6c2424"};
Hooked<void(__thiscall*)(void*, float, float, float, int)> addOrderAt{
    0xcac280, "83ec1856578bf1e814e8ffff8b866c0b"};
const Function<void(__thiscall*)(void*)> clearOrders{0xcaaaa0, "56578bf98b8ff00a00006a0068f64f00"};
const Function<void(__thiscall*)(void*, int)> startOrders{0xc9efc0, "8b4424048981280d00008981240d0000"};
Slot<void*(__thiscall*)(void*)> vehicleGetWeapon{0x1474ce8, 0x14, 0xc9f370};
Slot<TakeDamageFunction> vehicleTakeDamage{0x1474ce8, 0x18, 0xcab6f0};
Slot<bool(__thiscall*)(void*, void*)> vehicleInRange{0x1474ce8, 0x40, 0xbfc8e0};
Slot<void(__thiscall*)(void*, void*)> vehicleSetTarget{0x1474ce8, 0x50, 0xc9e8f0};
Hooked<void*(__thiscall*)(void*, int, int, game::Key, int)> spawnVehicle{
    0xbddef0, "83ec2853568bf183be9005000000570f"};
const Global<const uint32_t> vehicleSlots{0x147496c, "ad3f437dcb3d968fe6d31c44"};
const Function<const game::Key*(__thiscall*)(char*, uint32_t)> cultureKey{
    0xbf9840, "51b801000000568bf1840560cb680175"};
const Function<void(__thiscall*)(char*, uint32_t)> culturePick{0xbf9440, "83ec745333db55894c240c895c244489"};
const Function<void(__thiscall*)(char*, char**, const uint32_t*)> cultureFind{
    0xe39f60, "8b410c8d5104578b7c240c8bca85c074"};

const Function<bool(__thiscall*)(char*, int)> superweaponCooling{0xbf18d0, "53558b6c240c578bf98b8f90040000"};
const Function<void(__thiscall*)(char*, int, const game::Vector*)> citySuperweapon{
    0xbe5d30, "83ec28538b5c2430578bf983fb0b771b"};
const Function<void(__thiscall*)(char*)> loadSuperweaponTools{0xbf7020, "83ec105333db3899890000000f849b00"};
const Function<void(__thiscall*)(char*)> timerStart{0xbc32a0, "568bf18b461cffd0894610"};
const Global<uint8_t> freeSuperweapons{0x168bd6c};

Slot<bool(__thiscall*)(void*)> beliefUpdate{0x1475ac0, 0x14, 0xcbb420};
Hooked<void(__thiscall*)(void*, void*, void*, void*, void*, const void*, float, int, int)> beliefLaunch{
    0xcbdb90, "81ec880000005356578bf98b8c249800"};

Hooked<void(__thiscall*)(void*, uint32_t)> voiceQueue{0xdcc290, "8b51288b412483c124568b7424083bc2"};
const Global<const uint32_t> reportVoices{0x14749c4}, movingVoices{0x14749d0}, engageVoices{0x14749dc},
    holdVoices{0x14749f0}, underFireVoices{0x1474a7c}, vehicleFireVoices{0x1474a88}, bribedVoices{0x1474e6c},
    okayVoices{0x147c664};
}

