
Beatuy!SpaceStage 1.0.1
Copyright © 2026 Sheroll s.r.o® All rights reserved.

Beatuy!SpaceStage is licensed under the Sheroll Non-Commercial License 1.0. This covers its source code, its build
scripts, the compiled BeatuySpaceStage.dll, the game package BeatuySpaceStage.package and the models, effects,
pictures and texts made for it. The summary and the list of material of others are in LICENSE. The full license text
is in licenses/Sheroll-NC-1.0.md.

Derived works name the source as "Based on Beatuy!SpaceStage by Sheroll s.r.o®" in their credits and wherever they
are published. Sharing unchanged copies of the whole mod needs Sheroll's permission (§3.4 of the license).

Microsoft Detours: Copyright Microsoft Corporation; MIT License.
The compiled DLL links Detours v4.0.1 (commit e4bfd6b) built from its sources, which cmake/setup.sh fetches into
deps/Detours. The full license is in licenses/Detours-MIT.md.
Source: https://github.com/microsoft/Detours

Microsoft C/C++ runtime and Windows SDK: Copyright Microsoft Corporation.
The build links against the MSVC CRT 14.44 and the Windows SDK 10.0.26100, which cmake/setup.sh downloads with xwin
only after the Microsoft license is accepted. They are not part of this repository. The DLL imports the runtime from
VCRUNTIME140.dll and the universal CRT (api-ms-win-crt-*), which come with the Microsoft Visual C++ Redistributable
and with Windows.

Spore and Galactic Adventures are owned by Electronic Arts/Maxis. Beatuy!SpaceStage is not made, approved or
supported by Electronic Arts.
- This repository contains no game executable, game assets, save data or decompiled game code.
- Some files of the released package are made from the game's own data, and that data belongs to Electronic Arts Inc.:
  - the textures and the model layout of the Glow Generator;
  - the game's tool lists that the package extends (spacetools~/ToolConfig, spacetoolpalette~/ColonizePalette);
  - the escort's rally voice lines, copied without their start delay;
  - the attack reticle effect, recoloured in the empires' colours.
- The package's sources and the tools that make them are not published.

