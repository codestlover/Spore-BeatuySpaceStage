
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <cstdint>

export module beatuy.game.map:tribe;

import beatuy.game.math;
import :address;

export namespace map::tribe {
using NodeTickFunction = int(__cdecl*)(void*, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t,
                                       uint32_t, uint32_t);

const Function<void(__thiscall*)(char*, int, const game::Vector*, const game::Vector*, float, float)> creatureWalkTo{
    0xc1c720, "81ec90000000535556578bbc24ac0000"};
const Function<int(__thiscall*)(char*, uint32_t, int, bool)> creaturePlayAnimation{
    0xc12470, "535556578bf133ff39be540b00000f84"};
const Function<void(__thiscall*)(char*, int, int)> setHandheldItem{0xc250a0, "83ec4053558b6c245056578bf983fdff"};
const Function<char* (*)(int)> tribeToolDataGetter{0xc9d050, "8b4424048d48ff83f90a77088b0485149a6901"};
const Global<char*> tribeToolData{0x1699a14};

const Function<int(__cdecl*)(const game::Vector*, const game::Vector*, float, game::Vector*, const game::Vector*)>
    ballisticAim{0xc0cb60, "83ec2456e8e708f3ff8bc8e8701af7ff"};
const Function<char*(__thiscall*)(void*)> spearCreate{0xc0fe30, "68c9704202e8b60df1ff85c0740f8b10"};
const Function<void(__thiscall*)(char*, const game::Vector*)> spearLaunch{
    0xcb71c0, "568bf18b46348b502c578d7e348bcfff"};
const Function<void*(__thiscall*)(char*, void*)> combatantPointerAssign{
    0xae6610, "538b5c2408568bf1578b3e3bdf741c85"};
const Function<void(__thiscall*)(char*, void*)> setLocomotion{0xc42310, "53568b74240c8bd9578bbbcc0400003b"};
const Location pickAgainstModel{0x749c40, "81ec80000000568bb4249000000085f6"};
const Location spearUpdateCode{0xc91e30, "81ec08010000538bd98b8330fbffff8b"};
Slot<bool(__thiscall*)(void*)> spearUpdate{0x145eaa0, 0x14, 0xc91e30};

Hooked<NodeTickFunction> panicTick{0xdaf270, "83ec68578bbc2484000000f30f104704"};
Hooked<NodeTickFunction> cowerTick{0xdaae50, "578b7c241c8b0f8b018b502cffd284c0"};

const Noun tribes{0x18c6d19, 0xaccaf0};
}

