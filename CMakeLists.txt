cmake_minimum_required(VERSION 3.22)

project(LeviModules
    VERSION 0.1.0
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(LEVI_TARGET minecraft)

add_library(${LEVI_TARGET} SHARED
    src/core/Logger.cpp
    src/core/State.cpp
    src/core/Runtime.cpp

    src/memory/Pattern.cpp
    src/memory/PatternScanner.cpp
    src/memory/Hook.cpp
    src/memory/VTable.cpp

    src/minecraft/Camera.cpp
    src/minecraft/ClientInstance.cpp
    src/minecraft/MinecraftProfile.cpp
    src/minecraft/ItemEntity.cpp
    src/minecraft/ItemRenderer.cpp
    src/minecraft/MatrixStack.cpp
    src/minecraft/RenderContext.cpp

    src/modules/ModuleManager.cpp
    src/modules/ViewModel.cpp
    src/modules/Freelook.cpp
    src/modules/ItemPhysics.cpp
)

target_include_directories(${LEVI_TARGET}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_definitions(${LEVI_TARGET}
    PRIVATE
        LEVI_ANDROID=1
        LEVI_AARCH64=1
        LEVI_MINECRAFT_1_26_44_3=1
)

target_compile_options(${LEVI_TARGET}
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -fvisibility=hidden
        -fno-exceptions
        -fno-rtti
)

target_link_libraries(${LEVI_TARGET}
    PRIVATE
        log
        android
        dl
)
