
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <cstdint>

export module beatuy.game.map:identifiers;

export namespace map::identifiers {
enum class Type : uint32_t {
    Ship = 0xb033b403,
    Planet = 0x3275872,
    City = 0xee9b2232,
    CityHall = 0x1007ae63,
    Building = 0xe9cb8ba,
    Turret = 0x436f315,
    Vehicle = 0x137e8e0,
    Creature = 0xce9f6639,
    Spatial = 0x1186577,
    GameData = 0x17f243b,
    Combatant = 0x13f94d4,
    ToolObject = 0x4e3faaf,
    SpaceCamera = 0x303154cc,
    PlanetCamera = 0x303154cd,
    RaidEvent = 0x3960c0a,
};

enum class Mission : uint32_t { RaidWar = 0xbbae6b99, RaidAlly = 0xd646c543 };

enum class Mode : uint32_t { Space = 0x1654c05 };
enum class Context : int { None = -1, Planet = 0, System = 1, Galaxy = 2 };
enum class Button : int { Left = 1000, Right = 1002 };

enum class Technology : int { None = -1, Tribe = 2, City = 3, Civilization = 4, Empire = 5 };
enum class ShipKind : int { Player = 0, Raider = 2, Escort = 3, Bomber = 4, Reinforcement = 8 };
enum class Atmosphere : int { Red = 1 };
enum class Order : int { Attack = 1, AttackCity = 2, ConvertCity = 3, Move = 6 };
enum class Purpose : int { Military = 0, Cultural = 1, Economic = 2 };
enum class Locomotion : int { Land = 0, Water = 1, Air = 2 };
enum class Damage : int { Artillery = 0, Final = 2, Heal = 10 };
enum class Archetype : int {
    Warrior = 0,
    Trader = 1,
    Scientist = 2,
    Shaman = 3,
    Bard = 4,
    Zealot = 5,
    Diplomat = 6,
    Ecologist = 7,
    Grob = 8,
};

enum class Relation : uint32_t {
    Attacked = 0x526e545,
    DestroyedBuilding = 0x526e53c,
    DeclaredWar = 0x5f8a1ad,
    BeNasty = 0x577909b,
    FoughtEnemy = 0x5da8036,
    GoodToolUse = 0x526e56a,
    MissionCompleted = 0x526e519,
    MissionFailed = 0x526e51c,
    MissionRejected = 0x526e51d,
    MissionAborted = 0x526e51e,
};

enum class Popup : uint32_t { Group = 0x131a9f54, AlliesArrive = 0xa0c7000a };

enum class AudioEvent : uint32_t {
    Message = 0x3a127dc,
    Started = 0x617208b,
    Ended = 0x3a0fd6e,
    Stopping = 0x3a129e8,
    Finished = 0x3a129ee,
};

enum class AudioProperty : uint32_t {
    Spatial = 0x12d2a4d4,
    Virtual = 0xba134192,
    FadeIn = 0x98ac8996,
    FadeOut = 0xe8ab99b9,
    Gain = 0x25df0108,
    SeekTime = 0x14f9bdf6,
    LowPass = 0x647a7836,
    RingFrequency = 0x703d682b,
    RingMix = 0x829c37d7,
};
}

