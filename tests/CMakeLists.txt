include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

find_package(Check REQUIRED)
include_directories(${CHECK_INCLUDE_DIRS})

set(LIBS bflib ${CHECK_LIBRARIES} m rt pthread)

#if( ${CHECK_MODE_NOFORK} STREQUAL yes )
#    set(CHECK_MODE_NOFORK 1)
#else()
#    set(CHECK_MODE_NOFORK 0)
#endif()
#configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/config.h)

add_executable(check_addrwatch check_addrwatch.c)
target_link_libraries(check_addrwatch ${LIBS})

add_executable(check_heap check_heap.c)
target_link_libraries(check_heap ${LIBS})

add_executable(check_timer check_timer.c)
target_link_libraries(check_timer ${LIBS})

add_executable(check_tree check_tree.c)
target_link_libraries(check_tree ${LIBS})
