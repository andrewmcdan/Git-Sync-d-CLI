cmake_minimum_required(VERSION 3.10)
project(Git-Sync-d-CLI)

set(Boost_USE_STATIC_LIBS ON) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME ON) 

set(Boost_INCLUDE_DIR "C:/Users/Andrew/Documents/Projects/boost_1_84_0")
set(Boost_LIBRARY_DIR "C:/Users/Andrew/Documents/Projects/boost_1_84_0/stage/lib/")

find_package(Boost)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
  link_directories(${Boost_LIBRARY_DIRS})
endif()

# Platform-specific settings
if(WIN32)
  add_definitions(-DWINDOWS)
  if(MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
  endif()
elseif(UNIX AND NOT APPLE)
  add_definitions(-DLINUX)
endif()

# Include directories
include_directories(src/include/)

# Source files
file(GLOB SOURCES "src/*.cpp" "src/common/*.cpp")

# Platform-specific sources
if(WIN32)
  list(APPEND SOURCES "src/windows/cli.cpp")
elseif(UNIX AND NOT APPLE)
  list(APPEND SOURCES "src/linux/cli.cpp")
endif()

add_executable(Git-Sync-d-CLI main.cpp ${SOURCES})