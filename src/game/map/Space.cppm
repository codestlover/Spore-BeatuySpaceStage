
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <cstdint>

export module beatuy.game.map:space;

import beatuy.game.math;
import :address;

export namespace map::space {
using ModeUpdateFunction = void(__thiscall*)(void*, uint32_t, uint32_t);
using ModeExitFunction = void(__thiscall*)(void*);
using MouseDownFunction = bool(__thiscall*)(void*, int, float, float, int);
using WheelFunction = bool(__thiscall*)(void*, int, float, float, int);
using TakeDamageFunction = int(__thiscall*)(void*, float, uint32_t, int, const void*, void*);
using InRangeFunction = bool(__thiscall*)(void*, void*);

Slot<ModeUpdateFunction> modeUpdate{0x1493fc8, 0x3c, 0xfe0f30};
Slot<ModeExitFunction> modeExit{0x1493fc8, 0x1c, 0xfda380};
Slot<MouseDownFunction> modeMouseDown{0x1493fc8, 0x2c, 0xfdc510};
Slot<WheelFunction> systemCameraWheel{0x1495a10, 0x3c, 0x101b9c0};
Slot<WheelFunction> planetCameraWheel{0x1495a78, 0x3c, 0x101b9c0};

const Global<char*> spacePlayerData{0x16dda8c};
const Global<char*> playerShipManager{0x16dba14};
const Function<char* (*)()> playerShipManagerGetter{0xffbd00, "a114ba6d01c3"};
const Function<int (*)()> currentContext{0x1020f30, "a18cda6d018b4010c3"};
const Function<uint32_t (*)()> playerEmpireIdentifier{0x1020f40, "a18cda6d018b4018c3"};
const Function<char* (*)()> playerEmpire{0x10211b0, "8b0d8cda6d018b411883f8ff74588b49"};
const Function<char* (*)()> starManager{0xb3d3a0, "a1e4ea6701c3cccccccccccccccccccc"};
const Function<char*(__thiscall*)(char*, uint32_t)> empireOf{0xba94c0, "51837c2408ff568bf174298d44240c50"};
const Function<char* (*)()> activePlanet{0x1021110, "5133d2891424a18cda6d0185c074088b"};
const Function<char* (*)()> activeStarRecord{0x10210f0, "a18cda6d018b400885c074048b4048c3"};
const Function<char*(__thiscall*)(char*)> hoveredStar{0xfff780, "81ec90000000535556578be9c6442413"};
const Function<bool (*)()> canEnterPlanet{0x10422f0, "56e8dab963ff8b108bc88b4250ffd08b"};
const Function<bool (*)()> canEnterSystem{0x1042460, "e81bb0affff64048017518e89098fbff"};
const Function<float (*)(int)> minimumZoom{0x1016ed0, "518b4424080f57c0f30f11042483f801"};
const Function<float (*)(int)> maximumZoom{0x1016f10, "518b4424080f57c0f30f11042483f801"};
const Function<float(__thiscall*)(char*, float)> changeEnergy{0xc38b50, "0f57c083ec140f2f442418568bf17615"};

Hooked<void(__thiscall*)(void*, int)> leavePlanet{0xfde3d0, "83ec588b815c0100008348500153558b"};
Hooked<void(__thiscall*)(void*)> playerShipDestroyed{0xc3c680, "81ec9c000000535556578bd9e86f0df0"};
const Function<void (*)()> callReinforcements{0xff8980, "83ec40535657e8658702008bf88bcf89"};

Hooked<TakeDamageFunction> combatantTakeDamage{0xbfcf10, "83ec48538b5c246055568bf1578b7e28"};
Slot<TakeDamageFunction> shipTakeDamage{0x146bb48, 0x18, 0xc3cec0};
Slot<InRangeFunction> shipInRange{0x146bb48, 0x40, 0xbfc8e0};
Slot<void(__thiscall*)(void*, void*)> shipSetTarget{0x146bb48, 0x50, 0xc3a990};
Slot<void(__thiscall*)(void*, void*)> shipAddHostile{0x146bb48, 0x1c, 0xc37b00};
Hooked<void(__thiscall*)(void*, void*, int)> shipSetTargetInBand{0xc3aa90, "535556578bf98bb7900700008daf9007"};
const Function<void*(__thiscall*)(void*, void*)> shipSetAiTarget{0xc3b860, "515355568b742414578bf9897c241085"};
const Function<void(__thiscall*)(void*)> shipClearAiTarget{0xc38cf0, "5356578bf98b8fac07000085c974108b"};
using BehaviourTickFunction = bool(__cdecl*)(void*, uint32_t, uint32_t, uint32_t, uint32_t, float*, void*, float);
const Function<BehaviourTickFunction> roseTick{0xdbfa60, "83ec4053568b742460578b7c245083bf"};
const Location roseEscortGate{0xdbfbb8, "0f8447020000e83dc123008bc8e806c6"};
const Global<void*> escortRoseBehaviour{0x159e880, "70d65901"};
const Global<void*> roseBehaviour{0x159d670, "6c25870550f9db0060fadb00c0eedb00"};
Hooked<bool(__cdecl*)(void*, void*, bool)> shipHostile{0x102aa60, "51578b7c240c85ff74128b078b502c8b"};
Hooked<void*(__cdecl*)(void*, bool, bool)> nearestHostileShip{0x102bca0, "83ec148b0d18755b010f57c0f30f1144"};
const Global<char*> simulatorUniverse{0x16dc798};
using DecisionFunction = float(__cdecl*)(void*, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, void*, void*);
Global<void*> reinforcementLeavesDecision{0x159edf0, "00eedb00"};
const Function<void*(__thiscall*)(void*, uint32_t, uint32_t)> activeOrder{0xbc9810, "8b81c00100008b91c4010000538b5c24"};

Slot<ModeUpdateFunction> spaceGameUpdate{0x1495454, 0x10, 0x100d140};

Hooked<bool(__thiscall*)(void*, void*)> toolTargetCheck{0x104cdc0, "53568bf1f6867601000001578b7c2410"};
Slot<bool(__thiscall*)(void*, char*, bool, const char16_t**)> placeObjectUpdate{0x149b708, 0x20, 0x1057ba0};
const Function<bool(__thiscall*)(void*, char*, bool, const char16_t**)> toolStrategyUpdate{
    0x1056130, "57e8ca5bfaff8b7c240885c07430e8bd"};
const Function<void*(__cdecl*)(char*, uint32_t, bool)> toolShowEvent{0x1052800, "33c03844240c74798b4c2404568b7424"};
Hooked<bool(__thiscall*)(void*, void*, void*, const game::Vector*, int, int, void*, int)> fireTool{
    0x1050080, "83ec0c5355568b74241c8b8678010000"};
Slot<int(__thiscall*)(void*, void*, const void*, int)> repairAreaFire{0x149ba98, 0x34, 0x1057ff0};
Slot<int(__thiscall*)(void*, void*, const void*, int)> rallyCallFire{0x149bae0, 0x34, 0x105bf80};
Slot<int(__thiscall*)(void*, void*, const void*, int)> missileFire{0x149b630, 0x34, 0x10571e0};
Slot<int(__thiscall*)(void*, int, int)> beamViewUpdate{0x1476750, 0x4, 0xcc43f0};
const Function<void*(*)()> toolManager{0xb3d490, "a108eb6701c3"};
const Function<void(__thiscall*)(void*, const game::Key*, void**)> loadTool{0x104e350, "515657c744240800000000e8"};
const Function<void(__thiscall*)(void*)> stopTimer{0xbc3280, "568bf1807e180074158b461cffd02b46"};
const Function<int(__thiscall*)(void*)> empireLevel{0xc31900, "83ec08568bf183bed8000000000f8dcf"};
Hooked<void(__thiscall*)(void*, const void*, void*)> missileLaunch{0xcb88b0, "5355568bf18b9ee4050000578b7c2418"};
Hooked<void (*)(void*, void*, const game::Vector*, const game::Vector*)> launchProjectile{
    0x1054390, "83ec4c5355568b74245c578bcee82e27"};
const Function<void(__thiscall*)(void*, float, void*, const game::Vector*)> homeProjectile{
    0xcc09f0, "83ec24535556578b7c243c33ed8bf189"};
const Function<void(__thiscall*)(void*, const game::Vector*)> shipSnapTo{
    0xc3dc40, "568bf1e8e8323e0085c07545578b7c24"};
Hooked<float(__thiscall*)(void*)> shipTopSpeed{0xc389a0, "51568bf18b861407000083f8047749ff"};
Hooked<float(__thiscall*)(void*)> shipAcceleration{0xc38a60, "51568bf183be1407000000578b3d68df"};

const Noun ships{0x18ebadc, 0xad4830};
}

