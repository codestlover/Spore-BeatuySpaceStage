
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <cstddef>
#include <cstdint>

export module beatuy.game.map:application;

import beatuy.game.math;
import :address;

export namespace map::application {
const Function<void* (*)()> gameModeManager{0x67dcd0, "a1b0d85f01c3"};
const Function<void* (*)()> gameViewManager{0xb3d340, "a1c4ea6701c3"};
const Function<char* (*)()> planetModel{0xb3d450, "a1f8ea6701c3"};
const Function<void* (*)()> nounManager{0xb3d400, "a1e0ea6701c3"};
const Function<char* (*)()> timeManager{0xb3d480, "a104eb6701c3"};
const Function<void* (*)()> messageManager{0x67dc80, "a190d85f01c3"};
const Function<void* (*)()> localeManager{0x67de00, "a1f8d85f01c3"};
Slot<const char16_t*(__thiscall*)(void*, uint32_t)> localeTableText{0x1409c48, 0x14, 0x6b72d0};
const Function<char*(__thiscall*)(char*, uint32_t, uint32_t, const char16_t*)> localizedStringConstructor{
    0x6b5700, "6aff6828e7200164a100000000506489"};
const Function<const char16_t*(__thiscall*)(char*)> localizedStringText{0x6b5550, "64a1000000006aff6848532101506489"};
const Function<void* (*)()> renderer{0x67dd10, "a1c0d85f01c3"};

const Function<char*(__cdecl*)(size_t, const char*, int, unsigned, const char*, int)> allocate{
    0xf47240, "8b4424188b4c24148b542408508b4424"};

const Function<char*(__thiscall*)(void*, void*, void*, void*, void*, uint32_t)> nounList{
    0xb212d0, "83ec105356578bf18d442430508d4c24"};
const Location nounThunkFirst{0xcd7e90}, nounThunkSecond{0xd3d3d0}, nounThunkFourth{0xb1e440};

const Function<void (*)(uint32_t, int)> playAudio{0x436390, "558bec51e8d7a25e008945fc837dfc00"};
const Function<int (*)()> newAudioTrack{0x436350, "558bec83ec08e815a35e008945fc837d"};

const Global<void*> device{0x16f89d0};
const Function<char* (*)()> effectsManager{0x67dd90, "a1e8d85f01c3"};
const Function<void(__cdecl*)(void*, void**)> placeEffectOver{0x1043780, "83ec5c568b7424648b068b5034578bce"};
const Location cameraManagerViewer{0xa21090, "8b81b0000000c3"};
}

