cmake_minimum_required(VERSION 3.12)

project(json-fmt)

option(PARABIX_INCLUDE "Include directory for parabix framework")
option(PARABIX_LIB "Library directory for parabix framework")

set(PARABIX_OBJECT_CACHE "$ENV{HOME}/.cache/parabix/")
if (NOT EXISTS(${PARABIX_OBJECT_CACHE}))
    file(MAKE_DIRECTORY ${PARABIX_OBJECT_CACHE})
endif()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DPARABIX_OBJECT_CACHE='\"${PARABIX_OBJECT_CACHE}\"'")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION} in: ${LLVM_DIR}")
set(LLVM_ALL_TARGETS X86)
llvm_map_components_to_libnames(REQ_LLVM_LIBRARIES ${LLVM_ALL_TARGETS} mcjit native IRReader Linker)
include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})
add_definitions(${LLVM_DEFINITIONS})
math(EXPR LLVM_VERSION_INTEGER "${LLVM_VERSION_MAJOR} * 10000 + ${LLVM_VERSION_MINOR} * 100 + ${LLVM_VERSION_PATCH}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DLLVM_VERSION_INTEGER=${LLVM_VERSION_INTEGER}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DLLVM_VERSION_major=${LLVM_VERSION_MAJOR} -DLLVM_VERSION_MINOR=${LLVM_VERSION_MINOR}")
message(STATUS "LLVM Version Integer: ${LLVM_VERSION_INTEGER}")

if (NOT LLVM_ENABLE_RTTI)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
endif()

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED OFF)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.61 REQUIRED COMPONENTS system filesystem iostreams)
message(STATUS "Found Boost_LIBRARY_DIR: ${Boost_LIBRARY_DIR}")
include_directories("${Boost_INCLUDE_DIRS}")
link_directories(${Boost_LIBRARY_DIR})

# Creates a pablo source target which copies pablo source files to the parabix
# cache directory.
#
# Usage: parabix_add_pablo_src(target-name source-files...)
function(parabix_add_pablo_src NAME)
    set(FILE_LIST "")
    foreach(PABLO_FILE ${ARGN})
        list(APPEND FILE_LIST "${CMAKE_CURRENT_SOURCE_DIR}/${PABLO_FILE}")
    endforeach(PABLO_FILE)

    add_custom_target(${NAME}
            COMMAND cp ${FILE_LIST} ${PARABIX_OBJECT_CACHE}
            DEPENDS ${ARGN})
endfunction(parabix_add_pablo_src)


include_directories(${PARABIX_INCLUDE})

link_directories("${PARABIX_LIB}")

parabix_add_pablo_src(json-fmt-pablo "pablo/json_fmt.pablo")

add_executable(json-fmt
        src/main.cpp
        src/cli.cpp
        src/pipeline.cpp
        src/kernel/lex_json.cpp
        src/kernel/lex_json.hpp
        src/kernel/indent_bixnum.cpp
        src/kernel/indent_bixnum.hpp
        src/kernel/split.cpp
        src/kernel/analyze_json.cpp
        src/kernel/find_spread_insert_locations.cpp
        src/kernel/insert_chars.cpp)

add_dependencies(json-fmt json-fmt-pablo)

target_link_libraries(json-fmt
        parabix_framework
        ${Boost_LIBRARIES}
        ${REQ_LLVM_LIBRARIES})
