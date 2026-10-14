# Builds heron-c: a multi-pane terminal written in C on Win32, ConPTY,
# Direct2D and DirectWrite. Debug builds are console programs so logs show in
# the terminal that started them; Release builds are GUI programs.
cmake_minimum_required(VERSION 3.20)
project(heron_c LANGUAGES C RC)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

# Flags shared by the app and the tests.
add_library(heron_c_options INTERFACE)
target_include_directories(heron_c_options INTERFACE src)
target_compile_definitions(heron_c_options INTERFACE
  UNICODE _UNICODE WIN32_LEAN_AND_MEAN COBJMACROS
  _WIN32_WINNT=0x0A00 NTDDI_VERSION=0x0A000006)
target_compile_options(heron_c_options INTERFACE -Wall -Wextra -Wno-unused-parameter
  -finput-charset=UTF-8 -fexec-charset=UTF-8)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_definitions(heron_c_options INTERFACE HERON_DEV)
endif()

file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS src/*.c)
add_executable(heron-c ${APP_SOURCES} res/heron.rc)
target_link_libraries(heron-c PRIVATE heron_c_options
  user32 gdi32 imm32 dwmapi shell32 ole32 uuid comctl32 d2d1 dwrite)
if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
  set_target_properties(heron-c PROPERTIES WIN32_EXECUTABLE ON)
  target_link_options(heron-c PRIVATE -s)
endif()

# Unit tests for the modules that need no window: base, vt, input, layout,
# claude, agent, hooks, git, app.
file(GLOB_RECURSE CORE_SOURCES CONFIGURE_DEPENDS
  src/base/*.c src/vt/*.c src/input/*.c src/layout/*.c src/claude/*.c src/agent/*.c src/hooks/*.c src/git/*.c src/app/*.c)
file(GLOB TEST_SOURCES CONFIGURE_DEPENDS tests/*.c)
add_executable(heron-c-tests ${CORE_SOURCES} ${TEST_SOURCES})
target_include_directories(heron-c-tests PRIVATE tests)
target_link_libraries(heron-c-tests PRIVATE heron_c_options)
enable_testing()
add_test(NAME unit COMMAND heron-c-tests)
