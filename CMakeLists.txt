cmake_minimum_required(VERSION 3.17)
project(lamphttp C CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_STANDARD 99)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

include_directories(${PROJECT_SOURCE_DIR})

# check epoll and add config.h for the macro compilation
include(CheckSymbolExists)
if (EPOLL_EXISTS)
    # set epoll in linux
    set(EPOLL_ENABLE 1 CACHE INTERNAL "enable epoll")
else ()
    set(EPOLL_ENABLE "" CACHE INTERNAL "not enable epoll")
endif ()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/include/config.h)

include_directories(${CMAKE_CURRENT_BINARY_DIR}/include)


#file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
#foreach(file ${files})
#    get_filename_component(name ${file} NAME)
##    if (EPOLL_EXISTS AND ("${file}" STREQUAL "epoll_dispatcher.c"))
##        add_executable(${name} ${file})
##    elseif ("${file}" STREQUAL "epoll_dispatcher.c")
##        continue()
##    endif()
#
#    add_executable(${name} ${file})
#endforeach()

if (EPOLL_EXISTS)
    set(lamphttp_lib
            log.c
            sock_ntop.c
            read.c
            tcp_server.c
            tcp_client.c
            event_loop.c
            channel.c
            acceptor.c
            channel_map.c
            poll_dispatcher.c
            thread_pool.c
            event_loop_thread.c
            utils.c
            epoll_dispatcher.c
            buffer.c
            tcp_connection.c
            http_server.c
            http_request.c
            http_response.c
            )
else()
    set(lamphttp_lib
            log.c
            sock_ntop.c
            read.c
            tcp_server.c
            tcp_client.c
            event_loop.c
            channel.c
            acceptor.c
            poll_dispatcher.c
            thread_pool.c
            event_loop_thread.c
            utils.c
            buffer.c
            tcp_connection.c
            http_server.c
            http_request.c
            http_response.c
            )
endif()

add_library(lamphttp ${lamphttp_lib})
target_link_libraries(lamphttp pthread)

install(TARGETS lamphttp DESTINATION lib)