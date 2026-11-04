
# Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
# Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_COMPILER_TARGET i686-pc-windows-msvc)
set(CMAKE_LINKER_TYPE LLD)
find_program(CMAKE_AR NAMES llvm-ar llvm-ar-21 REQUIRED)
find_program(CMAKE_RANLIB NAMES llvm-ranlib llvm-ranlib-21 REQUIRED)
find_program(CMAKE_MT NAMES llvm-mt llvm-mt-21 REQUIRED)
find_program(CMAKE_RC_COMPILER NAMES llvm-rc llvm-rc-21 REQUIRED)
find_program(CMAKE_CXX_COMPILER_CLANG_SCAN_DEPS NAMES clang-scan-deps clang-scan-deps-21 REQUIRED)
if("${CIV_MSVC_SDK}" STREQUAL "" AND NOT "${CIV_DEPS}" STREQUAL "")
  set(CIV_MSVC_SDK "${CIV_DEPS}/msvc-sdk")
elseif("${CIV_MSVC_SDK}" STREQUAL "")
  get_filename_component(CIV_MSVC_SDK "${CMAKE_CURRENT_LIST_DIR}/../deps/msvc-sdk" ABSOLUTE)
endif()
set(_civ_libs "-L\"${CIV_MSVC_SDK}/crt/lib/x86\" -L\"${CIV_MSVC_SDK}/sdk/lib/um/x86\" -L\"${CIV_MSVC_SDK}/sdk/lib/ucrt/x86\"")
set(_civ_includes "-idirafter \"${CIV_MSVC_SDK}/crt/include\" -idirafter \"${CIV_MSVC_SDK}/sdk/include/ucrt\" -idirafter \"${CIV_MSVC_SDK}/sdk/include/um\" -idirafter \"${CIV_MSVC_SDK}/sdk/include/shared\"")
set(CMAKE_CXX_FLAGS_INIT "${_civ_includes}")
set(CMAKE_EXE_LINKER_FLAGS_INIT "${_civ_libs}")
set(CMAKE_SHARED_LINKER_FLAGS_INIT "${_civ_libs}")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL" CACHE STRING "" FORCE)
set(CMAKE_TRY_COMPILE_CONFIGURATION Release)

