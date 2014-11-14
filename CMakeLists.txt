# Tensor Control Project CMAKELists.txt 
# Created 2014_11_14 by Argyle
cmake_minimum_required(VERSION 2.6)

# This can be read from ${PROJECT_NAME} after project() is called
project(tensorPattern)

# Let's set a version number.
# This file must be manually changed to effect version change.
set (VERSION_MAJOR 3)
set (VERSION_MINOR 7)
set (VERSION_PATCH 0)
set (VERSION_PRERELEASE)

if ("${VERSION_PRERELEASE}" STREQUAL "")
  message(STATUS "Configuring build version v" ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})
else()  
  message(STATUS "Configuring build version v" ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}-${VERSION_PRERELEASE})
endif()

# gcc flags
add_compile_options(-Wall -g)

# Libraries 
find_package(SDL)
if ( NOT SDL_FOUND )
  message(FATAL_ERROR "SDL not found! Please install libsdl1.2-dev!")
endif ( NOT SDL_FOUND )

find_package(SDL_ttf)
if ( NOT SDL_TTF_FOUND )
  message(FATAL_ERROR "SDL TTF not found! Please install libsdl-ttf2.0-dev!")
endif ( NOT SDL_TTF_FOUND )

# Verbose output (or use: make VERBOSE=1)
#set ( CMAKE_VERBOSE_MAKEFILE on )

# Subdirectories
add_subdirectory (src bin)

# Font support.
file(COPY fonts/DroidSansMono.ttf DESTINATION ${PROJECT_BINARY_DIR}/bin)
file(RENAME ${PROJECT_BINARY_DIR}/bin/DroidSansMono.ttf ${PROJECT_BINARY_DIR}/bin/font.ttf)

