cmake_minimum_required(VERSION 3.7)
project(williams_ethan_assignment_1_10)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        character.cpp
        character.h
        descriptions.cpp
        descriptions.h
        dice.cpp
        dice.h
        dims.h
        dungeon.cpp
        dungeon.h
        event.cpp
        event.h
        heap.cpp
        heap.h
        io.cpp
        io.h
        macros.h
        move.cpp
        move.h
        npc.cpp
        npc.h
        object.cpp
        object.h
        path.cpp
        path.h
        pc.cpp
        pc.h
        rlg327.cpp
        utils.cpp
        utils.h)

add_executable(williams_ethan_assignment_1_10 ${SOURCE_FILES})