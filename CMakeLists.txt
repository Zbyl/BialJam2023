cmake_minimum_required(VERSION 3.10)

# set the project name
project(RayGame LANGUAGES CXX)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

#add_subdirectory("../LDtkLoader" LDtkLoader)

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
    Utilities.h
    Utilities.cpp

    zerrors.h
    zstr.h

    Notes.txt
)

find_package(raylib CONFIG REQUIRED)
target_link_libraries(${APP_NAME} PRIVATE raylib)

find_package(recastnavigation CONFIG REQUIRED)
target_link_libraries(${APP_NAME} PRIVATE RecastNavigation::Detour RecastNavigation::Recast RecastNavigation::DebugUtils RecastNavigation::DetourCrowd)
    
#find_package(raylib CONFIG REQUIRED)
target_include_directories(${APP_NAME} PRIVATE ${RAYLIB_INCLUDE_DIRS})
#target_link_libraries(${APP_NAME} PRIVATE ${RAYLIB_LIBRARIES})
#target_link_libraries(${APP_NAME} PRIVATE LDtkLoader)

target_include_directories(${APP_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/../raylib-cpp/include")
target_include_directories(${APP_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/../raygui/src")
#target_include_directories(${APP_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/../LDtkLoader/include")

find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(${APP_NAME} PRIVATE nlohmann_json::nlohmann_json)


set_target_properties(${APP_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Runtime)
