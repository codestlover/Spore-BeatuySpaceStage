
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <cstddef>
#include <cstdint>

export module beatuy.game.map:userinterface;

import :address;
import beatuy.game.math;

export namespace map::userInterface {
const Global<char*> eventLog{0x167eb24};
const Function<uint32_t(__thiscall*)(char*, uint32_t, uint32_t, int, void*, bool, int)> showEvent{
    0xdd85e0, "81eccc0000005356578bd933ff895c24"};
const Function<void(__thiscall*)(char*, uint32_t, const wchar_t*)> setEventText{
    0xdd6d80, "538b5c240c56578bf985db0f84b40000"};
const Function<void(__thiscall*)(char*, uint32_t, bool)> hideEvent{0xdd6ca0, "807c240800568bf1750e8b44240850e8"};
const Function<char*(__thiscall*)(char*, uint32_t)> findEvent{0xdd66a0, "51568bf18d44240c508d4c2408518d4e"};

const Function<char* (*)()> layoutManager{0x80ff80, "a15ccf6401c3"};
const Function<char*(__thiscall*)(char*, uint32_t)> layerWindow{0x8106c0, "518b442408568bf18d4c240c518d5424"};
const Function<void* (*)()> windowAllocator{0x951220, "a1b4aa6601c3"};
const Function<char*(__cdecl*)(size_t, size_t, const char*, void*)> windowNew{
    0x951230, "8b4424084883c807568b7424144085f6"};
const Function<char*(__thiscall*)(char*)> windowConstructor{0x962950, "5333db558be9895d0c895d08895d1489"};
const Function<char*(__thiscall*)(char*)> inflateEffectConstructor{0x97e600, "568bf1e8d856fefff30f100540374501"};
const Function<char*(__thiscall*)(char*)> fadeEffectConstructor{0x96efc0, "568bf1e8184dffffc70678244401c746"};
const Function<char*(__thiscall*)(char*)> glideEffectConstructor{0x970010, "568bf1e8c83cffff0f57c0c7466060de"};
const Function<char*(__thiscall*)(char*)> buttonConstructor{0x966eb0, "5356578bf1e896bafffff30f10054037"};
const Function<char*(__thiscall*)(char*, const char16_t*, uint32_t, const char16_t*, const float*, int,
                                 const char16_t*, uint32_t)>
    tooltipConstructor{0x835f10, "8b44240453568bf1c74604404c440133"};
const Function<bool(__cdecl*)(char*, const game::Key*, int)> setBackgroundByKey{
    0x807ca0, "5153568b74241033db3bf375065e32c0"};
const Function<char*(__thiscall*)(char*, uint32_t, bool)> findWindow{0x810650, "8b491485c97405e944fcffff33c0c208"};
const Function<char*(__thiscall*)(char*)> layoutConstructor{0x8100a0, "8bc133c9894804c70060834101894808"};
const Function<bool(__thiscall*)(char*, const char16_t*, uint32_t, bool, uint32_t)> layoutLoadByName{
    0x812250, "83ec0c33c05689442404894424088944"};
const Function<char*(__thiscall*)(char*)> standardDrawableConstructor{0x988350, "f30f1005cc263f01568bf1c74604404c"};

const Function<char* (*)()> spaceGameInterface{0x10666b0, "e8bbc3f9ff85c074048b4014c333c0c3"};

const Function<char*(__thiscall*)(char*)> pickerConstructor{0x5a5a40, "0f57c08bc133c9894804c74008a01a3f"};
const Function<bool(__thiscall*)(char*, char*, uint32_t, uint32_t, void*)> pickerLoad{
    0x5a5eb0, "8b44240c83ec1c5355568bf18b4c242c"};
const Function<void(__thiscall*)(char*, bool)> pickerSetVisible{0x5a41f0, "568bf1837e100074438b4e108b018b50"};
const Function<void(__thiscall*)(char*, const float*)> pickerSetColor{
    0x5a4e10, "83ec0c53568bf1578b7c241c8b078946"};
const Function<void(__thiscall*)(char*, int)> pickerUpdate{0x5a6da0, "5356578bf98b5f342b5f3033f6c1fb02"};
const Function<void(__thiscall*)(char*)> pickerDestroy{0x5a6380, "53568b71342b71308d593057c1fe0233"};
const Function<void(__thiscall*)(char*, float, float, float, float, bool)> swatchSetArea{
    0x5a44f0, "83ec10568bf18b4e5c85c90f84ae0000"};
}

