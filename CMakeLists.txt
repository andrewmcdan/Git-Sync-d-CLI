cmake_minimum_required(VERSION 3.10)
project(Git-Sync-d-CLI)

# Set up Boost
set(Boost_USE_STATIC_LIBS ON) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME ON) 

set(Boost_INCLUDE_DIR "C:/Users/Andrew/Documents/Projects/boost_1_84_0")
set(Boost_LIBRARY_DIR "C:/Users/Andrew/Documents/Projects/boost_1_84_0/stage/lib/")

find_package(Boost)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
  link_directories(${Boost_LIBRARY_DIRS})
  if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0601)
  endif()
endif()

# Set up FTXUI
include(FetchContent)

FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
  GIT_TAG v5.0.0
)

FetchContent_GetProperties(ftxui)
if(NOT ftxui_POPULATED)
  FetchContent_Populate(ftxui)
  add_subdirectory(${ftxui_SOURCE_DIR} ${ftxui_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

# Platform-specific settings
if(WIN32)
  add_definitions(-DWINDOWS)
  if(MSVC)
    # set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
    # set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
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
  list(APPEND SOURCES "src/windows/ipc.cpp")
elseif(UNIX AND NOT APPLE)
  list(APPEND SOURCES "src/unix/ipc.cpp")
endif()


add_executable(Git-Sync-d-CLI main.cpp ${SOURCES})

target_link_libraries(Git-Sync-d-CLI 
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
  PRIVATE ftxui::screen
)

# add_custom_command(
#   TARGET Git-Sync-d-CLI POST_BUILD
#   COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_PDB_OUTPUT_DIRECTORY}/Git-Sync-d-CLI.exe ${CMAKE_SOURCE_DIR}/../Git-Sync-d/build/Debug/Git-Sync-d-CLI.exe
# )