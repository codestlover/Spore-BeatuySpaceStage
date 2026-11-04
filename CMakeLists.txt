
# Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
# Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

cmake_minimum_required(VERSION 3.28)
file(REAL_PATH "${CMAKE_CURRENT_SOURCE_DIR}" CIV_SOURCE_REAL)
file(REAL_PATH "${CMAKE_CURRENT_BINARY_DIR}" CIV_BINARY_REAL)
if(CIV_BINARY_REAL STREQUAL CIV_SOURCE_REAL)
  message(FATAL_ERROR "In-source builds are not supported; delete CMakeCache.txt and CMakeFiles from the source folder "
    "and configure a separate build folder with -B build")
endif()
set(CIV_DEPS_DEFAULT "${CMAKE_CURRENT_SOURCE_DIR}/deps")
if(NOT "$ENV{CIV_DEPS}" STREQUAL "")
  set(CIV_DEPS_DEFAULT "$ENV{CIV_DEPS}")
endif()
set(CIV_DEPS "${CIV_DEPS_DEFAULT}" CACHE PATH "Dependencies prepared by cmake/setup.sh")
if(DEFINED CACHE{CIV_MSVC_SDK})
  set(CIV_MSVC_SDK "${CIV_MSVC_SDK}" CACHE PATH "An existing MSVC CRT and Windows SDK tree in the xwin splat layout")
endif()
if(CIV_DEPS STREQUAL "")
  unset(CIV_DEPS CACHE)
  message(FATAL_ERROR "CIV_DEPS must not be empty")
endif()
foreach(CIV_PATH_VARIABLE CIV_DEPS CIV_MSVC_SDK)
  if(NOT "${${CIV_PATH_VARIABLE}}" STREQUAL "" AND NOT IS_ABSOLUTE "${${CIV_PATH_VARIABLE}}")
    set(CIV_BAD_PATH "${${CIV_PATH_VARIABLE}}")
    unset(${CIV_PATH_VARIABLE} CACHE)
    message(FATAL_ERROR "${CIV_PATH_VARIABLE} must be an absolute path, not ${CIV_BAD_PATH}")
  endif()
endforeach()
option(CIV_ACCEPT_MSVC_LICENSE "Accept the Microsoft license and download the MSVC CRT and Windows SDK with xwin" OFF)
set(CIV_MSVC_TREE "${CIV_DEPS}/msvc-sdk")
set(CIV_MSVC_EXTERNAL "")
if(NOT "${CIV_MSVC_SDK}" STREQUAL "")
  set(CIV_MSVC_EXTERNAL "${CIV_MSVC_SDK}")
  set(CIV_MSVC_TREE "${CIV_MSVC_SDK}")
endif()
if(EXISTS "${CMAKE_BINARY_DIR}/CMakeCache.txt" AND DEFINED CACHE{CMAKE_CXX_FLAGS})
  string(FIND "${CMAKE_CXX_FLAGS}" "-idirafter \"${CIV_MSVC_TREE}/crt/include\"" CIV_FLAGS_TREE)
  if(CIV_FLAGS_TREE EQUAL -1)
    message(FATAL_ERROR "CMAKE_CXX_FLAGS of this build folder lack the toolchain's -idirafter flags for ${CIV_MSVC_TREE}: "
      "CIV_DEPS or CIV_MSVC_SDK changed, CMAKE_CXX_FLAGS was set by hand, or the folder was first configured without "
      "the toolchain file; configure a new build folder")
  endif()
endif()
file(SHA256 "${CMAKE_CURRENT_SOURCE_DIR}/cmake/setup.sh" CIV_SETUP_HASH)
set(CIV_DEPS_STAMP "")
if(EXISTS "${CIV_DEPS}/.setup-stamp")
  file(STRINGS "${CIV_DEPS}/.setup-stamp" CIV_DEPS_STAMP LIMIT_COUNT 1)
endif()
set(CIV_NEED_SETUP OFF)
if(NOT CIV_DEPS_STAMP STREQUAL CIV_SETUP_HASH)
  set(CIV_NEED_SETUP ON)
endif()
foreach(required "${CIV_DEPS}/Detours/src/detours.h" "${CIV_MSVC_TREE}/crt/include" "${CIV_MSVC_TREE}/sdk/lib/um/x86")
  if(NOT EXISTS "${required}")
    set(CIV_NEED_SETUP ON)
  endif()
endforeach()
if(CIV_NEED_SETUP)
  set(CIV_SETUP_ARGS "")
  if(NOT CIV_MSVC_EXTERNAL STREQUAL "")
    if(NOT EXISTS "${CIV_MSVC_EXTERNAL}/crt/include" OR NOT EXISTS "${CIV_MSVC_EXTERNAL}/sdk/lib/um/x86")
      message(FATAL_ERROR "CIV_MSVC_SDK=${CIV_MSVC_EXTERNAL} is not an xwin splat tree (crt/include, sdk/lib/um/x86)")
    endif()
    list(APPEND CIV_SETUP_ARGS --msvc-sdk "${CIV_MSVC_EXTERNAL}")
  elseif(CIV_ACCEPT_MSVC_LICENSE)
    list(APPEND CIV_SETUP_ARGS --accept-msvc-license)
  elseif(NOT EXISTS "${CIV_DEPS}/msvc-sdk/crt/include" OR NOT EXISTS "${CIV_DEPS}/msvc-sdk/sdk/lib/um/x86")
    message(FATAL_ERROR "The MSVC CRT and Windows SDK are under the Microsoft license and are not redistributed. "
      "Configure with -DCIV_ACCEPT_MSVC_LICENSE=ON to download them with xwin, "
      "or with -DCIV_MSVC_SDK=PATH to use an existing xwin splat tree.")
  endif()
  find_program(CIV_BASH NAMES bash REQUIRED NO_CMAKE_FIND_ROOT_PATH)
  message(STATUS "Preparing the dependencies in ${CIV_DEPS} with cmake/setup.sh")
  execute_process(COMMAND "${CMAKE_COMMAND}" -E env "CIV_DEPS=${CIV_DEPS}"
    "${CIV_BASH}" "${CMAKE_CURRENT_SOURCE_DIR}/cmake/setup.sh" ${CIV_SETUP_ARGS}
    RESULT_VARIABLE CIV_SETUP_RESULT)
  if(NOT CIV_SETUP_RESULT EQUAL 0)
    message(FATAL_ERROR "cmake/setup.sh failed; if it asks for the Microsoft license, configure with "
      "-DCIV_ACCEPT_MSVC_LICENSE=ON, or with -DCIV_MSVC_SDK=PATH to use an existing xwin splat tree")
  endif()
endif()
project(BeatuySpaceStage LANGUAGES CXX)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS cmake/setup.sh)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if("${CIV_MSVC_SDK}" STREQUAL "")
  set(CIV_MSVC_SDK "${CIV_DEPS}/msvc-sdk")
endif()
if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR NOT CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "GNU"
    OR NOT CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC" OR NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
  message(FATAL_ERROR "This is not the 32-bit clang++ toolchain for the MSVC ABI; configure a new build folder with "
    "-G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-clang-i686.cmake")
endif()
if(NOT CMAKE_GENERATOR MATCHES "Ninja")
  message(FATAL_ERROR "C++20 modules build only with Ninja; configure a new build folder with -G Ninja")
endif()
string(FIND "${CMAKE_CXX_FLAGS}" "-idirafter \"${CIV_MSVC_SDK}/crt/include\"" CIV_FLAGS_TREE)
if(CIV_FLAGS_TREE EQUAL -1)
  message(FATAL_ERROR "CMAKE_CXX_FLAGS lack the toolchain's -idirafter flags for ${CIV_MSVC_SDK}; keep extra flags in "
    "CMAKE_CXX_FLAGS_RELEASE and configure a new build folder")
endif()
set_property(GLOBAL PROPERTY JOB_POOLS one=1)
set(CMAKE_JOB_POOL_COMPILE one)
set(CMAKE_JOB_POOL_LINK one)
foreach(required "${CIV_DEPS}/Detours/src/detours.h" "${CIV_MSVC_SDK}/crt/include" "${CIV_MSVC_SDK}/sdk/lib/um/x86")
  if(NOT EXISTS "${required}")
    message(FATAL_ERROR "${required} is missing; run bash cmake/setup.sh or reconfigure")
  endif()
endforeach()
find_program(CIV_BASH NAMES bash REQUIRED NO_CMAKE_FIND_ROOT_PATH)
set(CIV_LAYOUT_GUARD "${CMAKE_CURRENT_SOURCE_DIR}/cmake/layout.sh")
execute_process(COMMAND "${CIV_BASH}" "${CIV_LAYOUT_GUARD}" --skip "${CMAKE_BINARY_DIR}" --skip "${CIV_DEPS}"
  --skip "${CIV_MSVC_SDK}"
  RESULT_VARIABLE CIV_LAYOUT_RESULT)
if(NOT CIV_LAYOUT_RESULT EQUAL 0)
  message(FATAL_ERROR "Project layout guard failed; bash cmake/layout.sh --fix reframes the files")
endif()
add_custom_target(layout_guard ALL COMMAND "${CIV_BASH}" "${CIV_LAYOUT_GUARD}" --skip "${CMAKE_BINARY_DIR}" --skip "${CIV_DEPS}"
  --skip "${CIV_MSVC_SDK}"
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" VERBATIM)
add_compile_options("-ffile-prefix-map=${CMAKE_CURRENT_SOURCE_DIR}/=" -mno-incremental-linker-compatible)
add_link_options(LINKER:/Brepro)
add_compile_definitions(WIN32 _WINDOWS)
add_compile_options(-fms-extensions -fms-compatibility -fms-compatibility-version=19.29 -fno-strict-aliasing
  -ffunction-sections -finline-functions -fstack-protector-strong "SHELL:-Xclang -fexternc-nounwind")
set(DETOURS_SRC "${CIV_DEPS}/Detours/src")
add_library(civ_detours STATIC "${DETOURS_SRC}/detours.cpp" "${DETOURS_SRC}/modules.cpp"
  "${DETOURS_SRC}/disasm.cpp" "${DETOURS_SRC}/image.cpp" "${DETOURS_SRC}/creatwth.cpp")
target_compile_definitions(civ_detours PRIVATE WIN32_LEAN_AND_MEAN _WIN32_WINNT=0x0601)
target_compile_options(civ_detours PRIVATE -w)
add_library(beatuy_game OBJECT src/game/map/Map.cpp src/game/World.cpp)
target_sources(beatuy_game PUBLIC FILE_SET CXX_MODULES BASE_DIRS src FILES
  src/core/Language.cppm
  src/game/Game.cppm src/game/Math.cppm src/game/Memory.cppm src/game/World.cppm
  src/game/map/Map.cppm src/game/map/Address.cppm src/game/map/Application.cppm src/game/map/Civilization.cppm
  src/game/map/Identifiers.cppm src/game/map/Planet.cppm src/game/map/Space.cppm src/game/map/Tribe.cppm
  src/game/map/Saving.cppm src/game/map/UserInterface.cppm
  src/game/objects/Objects.cppm src/game/objects/View.cppm src/game/objects/Application.cppm
  src/game/objects/Belief.cppm src/game/objects/Camera.cppm src/game/objects/City.cppm src/game/objects/Combat.cppm
  src/game/objects/Creature.cppm src/game/objects/Empire.cppm src/game/objects/Event.cppm
  src/game/objects/GameData.cppm src/game/objects/Interface.cppm src/game/objects/Planet.cppm
  src/game/objects/Ship.cppm src/game/objects/Vehicle.cppm)
target_compile_options(beatuy_game PRIVATE -Wall)
target_compile_definitions(beatuy_game PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX UNICODE _UNICODE)
add_library(BeatuySpaceStage SHARED src/core/Main.cpp src/core/Mod.cpp
  src/space/civilization/Civilization.cpp src/space/civilization/Designs.cpp src/space/civilization/Marker.cpp
  src/space/civilization/Text.cpp src/space/colony/Colony.cpp src/space/escort/Escort.cpp
  src/space/exit/QuickExit.cpp src/space/generator/Generator.cpp src/space/generator/Picker.cpp
  src/space/glow/Glow.cpp src/space/orders/Behaviour.cpp src/space/orders/Mark.cpp src/space/orders/Orders.cpp
  src/space/orders/Panel.cpp src/space/orders/Reply.cpp src/space/orders/Saving.cpp
  src/space/radio/Radio.cpp src/space/travel/Travel.cpp src/space/tribe/Tribe.cpp)
target_sources(BeatuySpaceStage PRIVATE FILE_SET CXX_MODULES BASE_DIRS src FILES
  src/core/Feature.cppm src/core/Hooks.cppm src/core/Mod.cppm src/core/Name.cppm
  src/space/civilization/Civilization.cppm src/space/civilization/Designs.cppm src/space/civilization/Marker.cppm
  src/space/civilization/Render.cppm src/space/civilization/Settings.cppm src/space/civilization/Text.cppm
  src/space/civilization/Unit.cppm src/space/colony/Colony.cppm src/space/escort/Escort.cppm
  src/space/exit/QuickExit.cppm src/space/generator/Generator.cppm src/space/generator/Picker.cppm
  src/space/generator/Texts.cppm src/space/generator/WindowProcedure.cppm src/space/glow/Color.cppm
  src/space/glow/Glow.cppm src/space/orders/Behaviour.cppm src/space/orders/Book.cppm src/space/orders/Mark.cppm
  src/space/orders/Orders.cppm src/space/orders/Panel.cppm src/space/orders/Reply.cppm src/space/orders/Saving.cppm
  src/space/orders/Texts.cppm src/space/radio/Radio.cppm src/space/travel/Travel.cppm
  src/space/tribe/Spear.cppm src/space/tribe/Tribe.cppm src/space/tribe/Village.cppm)
target_include_directories(BeatuySpaceStage SYSTEM PRIVATE "${DETOURS_SRC}")
target_compile_options(BeatuySpaceStage PRIVATE -Wall)
target_compile_definitions(BeatuySpaceStage PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX UNICODE _UNICODE)
target_link_directories(BeatuySpaceStage PRIVATE
  "${CIV_MSVC_SDK}/crt/lib/x86" "${CIV_MSVC_SDK}/sdk/lib/um/x86" "${CIV_MSVC_SDK}/sdk/lib/ucrt/x86")
target_link_libraries(BeatuySpaceStage PRIVATE beatuy_game civ_detours kernel32 user32 msvcrt ucrt vcruntime)
target_link_options(BeatuySpaceStage PRIVATE "LINKER:/map:${CMAKE_BINARY_DIR}/BeatuySpaceStage.map")
set_target_properties(BeatuySpaceStage PROPERTIES PREFIX "" SUFFIX ".dll" RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
add_custom_command(TARGET BeatuySpaceStage POST_BUILD
  COMMAND "${CIV_BASH}" "${CMAKE_CURRENT_SOURCE_DIR}/cmake/imports.sh" "$<TARGET_FILE:BeatuySpaceStage>" VERBATIM)
set(CIV_TARGETS civ_detours beatuy_game BeatuySpaceStage)
foreach(target ${CIV_TARGETS})
  add_dependencies(${target} layout_guard)
endforeach()

