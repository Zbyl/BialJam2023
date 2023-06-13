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
    target_compile_definitions(${APP_NAME} PUBLIC _CRT_SECURE_NO_WARNINGS)
endif()

find_package(raylib CONFIG REQUIRED)
target_link_libraries(${APP_NAME} PRIVATE raylib)
target_include_directories(${APP_NAME} PRIVATE ${RAYLIB_INCLUDE_DIRS})

target_include_directories(${APP_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/Build/raylib-cpp/include")
target_include_directories(${APP_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/Build/raygui/src")

find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(${APP_NAME} PRIVATE nlohmann_json::nlohmann_json)

set_target_properties(${APP_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Runtime)
