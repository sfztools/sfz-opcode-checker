cmake_minimum_required(VERSION 3.3)

set(CMAKE_CXX_STANDARD 14)

add_subdirectory(external/abseil-cpp EXCLUDE_FROM_ALL)

project(sfz-opcode-checker)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(USE_STATIC_LINK_DEFAULT ON)
else()
    set(USE_STATIC_LINK_DEFAULT OFF)
endif()

option(USE_SYSTEM_YAML_CPP "Use the yaml-cpp system library" OFF)
option(USE_STATIC_LINK "Add `static` to the linker options" "${USE_STATIC_LINK_DEFAULT}")

#
if(USE_SYSTEM_YAML_CPP)
    find_library(YAMLCPP_LIBRARY "yaml-cpp")
    find_path(YAMLCPP_INCLUDE_DIR "yaml-cpp/yaml.h")
    message(STATUS "yaml-cpp library: ${YAMLCPP_LIBRARY}")
    message(STATUS "yaml-cpp headers: ${YAMLCPP_INCLUDE_DIR}")
    if(NOT YAMLCPP_LIBRARY OR NOT YAMLCPP_INCLUDE_DIR)
        message(FATAL_ERROR "cannot find yaml-cpp")
    endif()
    add_library(yaml-cpp INTERFACE)
    target_include_directories(yaml-cpp INTERFACE "${YAMLCPP_INCLUDE_DIR}")
    target_link_libraries(yaml-cpp INTERFACE "${YAMLCPP_LIBRARY}")
else()
    set(YAML_CPP_BUILD_CONTRIB OFF CACHE STRING "" FORCE)
    set(YAML_CPP_BUILD_TOOLS OFF CACHE STRING "" FORCE)
    set(YAML_CPP_BUILD_TESTS OFF CACHE STRING "" FORCE)
    set(YAML_BUILD_SHARED_LIBS OFF CACHE STRING "" FORCE)
    add_subdirectory(external/yaml-cpp EXCLUDE_FROM_ALL)
endif()

#
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    find_library(WININET_LIBRARY "wininet")
else()
    find_package(CURL REQUIRED)
    add_library(curl INTERFACE)
    target_include_directories(curl INTERFACE ${CURL_INCLUDE_DIRS})
    target_link_libraries(curl INTERFACE ${CURL_LIBRARIES})
endif()

#
add_library(ghc_filesystem INTERFACE)
target_include_directories(ghc_filesystem
    INTERFACE external/ghc)

#
add_library(sfizz_parser STATIC
    external/sfizz/sfizz/Opcode.cpp
    external/sfizz/sfizz/Parser.cpp
    external/sfizz/sfizz/SfzHelpers.cpp)
target_include_directories(sfizz_parser
    PUBLIC external/sfizz)
target_link_libraries(sfizz_parser
    PUBLIC ghc_filesystem
    PUBLIC absl::optional
    PUBLIC absl::strings)

#
add_executable(sfz-opcode-checker
    sources/main.cpp
    sources/sfzformat_db.cpp)
target_link_libraries(sfz-opcode-checker
    PRIVATE sfizz_parser
    PRIVATE yaml-cpp)
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_link_libraries(sfz-opcode-checker
        PRIVATE "${WININET_LIBRARY}")
else()
    target_link_libraries(sfz-opcode-checker
        PRIVATE curl)
endif()
if(USE_STATIC_LINK)
    target_link_options(sfz-opcode-checker
        PRIVATE "-static")
endif()

# Installation
install (TARGETS sfz-opcode-checker DESTINATION "${CMAKE_INSTALL_BINDIR}")
