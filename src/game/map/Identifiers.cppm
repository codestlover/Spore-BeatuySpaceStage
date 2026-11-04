
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
    ToolObject = 0x4e3faaf,
    SpaceCamera = 0x303154cc,
    PlanetCamera = 0x303154cd,
};

enum class Mode : uint32_t { Space = 0x1654c05 };
enum class Context : int { None = -1, Planet = 0, System = 1, Galaxy = 2 };
enum class Button : int { Left = 1000, Right = 1002 };

enum class Technology : int { None = -1, Tribe = 2, City = 3, Civilization = 4, Empire = 5 };
enum class ShipKind : int { Escort = 3, Reinforcement = 8 };
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
};

enum class Popup : uint32_t { Group = 0x131a9f54, AlliesArrive = 0xa0c7000a };
}

