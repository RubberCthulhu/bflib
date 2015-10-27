cmake_minimum_required(VERSION 2.8)
set(PROJECT bflib)
project(${PROJECT} C)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

set(CMAKE_CONFIGURATION_TYPES Debug Release)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

message("PROJECT ${PROJECT}: Build type: ${CMAKE_BUILD_TYPE}")

if( ${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR} )
    set(CMAKE_C_FLAGS "-Wall")
    set(CMAKE_C_FLAGS_DEBUG "-g3 -O0")
    set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG")
endif()

set(SRC
    addrwatch.c addrwatch.h
    bits.c bits.h
    counter.c counter.h
    die.c die.h
    heap.c heap.h
    list.c list.h
    netint.c netint.h
    octstr.c octstr.h
    timer.c timer.h
    tree.c tree.h
    uintvar.c uintvar.h
)

add_library(${PROJECT} ${SRC})

if( ${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR} )
    if(${CMAKE_BUILD_TYPE} STREQUAL Debug)
        enable_testing()
    endif()
endif()

if( ${CMAKE_TESTING_ENABLED} )
    add_subdirectory(tests)
    if( DEFINED ${CHECK_MODE_NOFORK} AND ${CHECK_MODE_NOFORK} STREQUAL yes )
        set(CHECK_MODE_NOFORK 1)
    else()
        set(CHECK_MODE_NOFORK 0)
    endif()
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/tests/config.h.in
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/config.h)
    add_test(NAME check_addrwatch COMMAND check_addrwatch)
    add_test(NAME check_heap COMMAND check_heap)
    add_test(NAME check_timer COMMAND check_timer)
    add_test(NAME check_tree COMMAND check_tree)
endif()
