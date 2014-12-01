# Tensor Control Project CMAKELists.txt
# Created 2014_11_14 by Argyle
cmake_minimum_required(VERSION 2.6)

# This can be read from ${PROJECT_NAME} after project() is called
project(tensorPattern)

# Let's set a version number.
# This file must be manually changed to effect version change.
set (VERSION_MAJOR 6)
set (VERSION_MINOR 0)
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
include(FindPkgConfig)
PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2>=2.0.2)
PKG_SEARCH_MODULE(SDL2IMAGE REQUIRED SDL2_image>=2.0.0)
PKG_SEARCH_MODULE(SDL2TTF REQUIRED SDL2_ttf>=2.0.0)

# Verbose output (or use: make VERBOSE=1)
#set ( CMAKE_VERBOSE_MAKEFILE on )

# Subdirectories
add_subdirectory (src)

# Font support.
file(COPY fonts/DroidSansMono.ttf DESTINATION ${PROJECT_BINARY_DIR})
file(RENAME ${PROJECT_BINARY_DIR}/DroidSansMono.ttf ${PROJECT_BINARY_DIR}/font.ttf)

# Aux files
file(GLOB IMAGES images/*)
file(COPY ${IMAGES} DESTINATION ${PROJECT_BINARY_DIR})
file(GLOB PRELOAD preload/*.now)
file(COPY ${PRELOAD} DESTINATION ${PROJECT_BINARY_DIR})

