cmake_minimum_required(VERSION 3.10)

# set the project name
project(RayGame LANGUAGES CXX)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(APP_NAME RayGame)

# add the executable
add_executable(${APP_NAME}
    main.cpp

    Game.h
    Game.cpp
    Menu.h
    Menu.cpp
    Player.h
    Player.cpp
    Animation.h
    Animation.cpp
    Level.h
    Level.cpp
    Collectible.h
    Collectible.cpp
    Scene.h
    Scene.cpp
    Utilities.h
    Utilities.cpp
    ResourceCache.h
    ResourceCache.cpp

    zerrors.h
    zstr.h

    README.md
)

if(MSVC)
    # Enable Edit & Continue: https://learn.microsoft.com/en-us/cpp/build/cmake-projects-in-visual-studio?view=msvc-170#edit-and-continue-for-cmake-projects
    # Still doesn't really work...
    target_compile_options(${APP_NAME} PUBLIC "/ZI")
    target_link_options(${APP_NAME} PUBLIC "/INCREMENTAL")

    # Remove useless warnings.
    target_compile_definitions(${APP_NAME} PUBLIC _CRT_SECURE_NO_WARNINGS _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING)
endif()

if (EMSCRIPTEN)
    target_compile_definitions(${APP_NAME} PUBLIC PLATFORM_WEB)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fwasm-exceptions")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fwasm-exceptions")
    # @note STACK_SIZE and TOTAL_MEMORY are overkill, but defaults were way to small!
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s USE_GLFW=3 -s ASSERTIONS=1 -s WASM=1 -fwasm-exceptions -s STACK_SIZE=67108864 -s ALLOW_MEMORY_GROWTH=1 -s TOTAL_MEMORY=1073741824 -s FORCE_FILESYSTEM=1 --preload-file ${CMAKE_SOURCE_DIR}/Runtime@/ --shell-file ${CMAKE_SOURCE_DIR}/Build/emscripten-shell.html")
    set(CMAKE_EXECUTABLE_SUFFIX ".html") # This line is used to set your executable to build with the emscripten html template so that you can directly open it.
endif()

find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(${APP_NAME} PRIVATE nlohmann_json::nlohmann_json)

find_package(raylib CONFIG REQUIRED)
target_link_libraries(${APP_NAME} PRIVATE raylib)
target_include_directories(${APP_NAME} PRIVATE ${RAYLIB_INCLUDE_DIRS})

target_include_directories(${APP_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/Build/raylib-cpp/include")
target_include_directories(${APP_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/Build/raygui/src")

set_target_properties(${APP_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Runtime)
