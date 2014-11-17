# Tensor Control src/CMAKELists.txt
# Created 2014_11_14 by Argyle

# Define the executable source - same as project name.
add_executable (tensorSDL tensorSDL.c)
set_target_properties (tensorSDL PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

# Configure the version header - not used yet.
configure_file("version.h.in" "version.h")
add_library(drv-tensor drv-tensor.c)

# Included directories
include_directories(${PROJECT_BINARY_DIR} ${SDL2_INCLUDE_DIRS} ${SDL2TTF_INCLUDE_DIRS} ${SDL2IMAGE_INCLUDE_DIRS})

# Linked Libraries
target_link_libraries(tensorSDL drv-tensor ${SDL2_LIBRARIES} ${SDL2TTF_LIBRARIES} ${SDL2IMAGE_LIBRARIES} SDL2_gfx)


#message(STATUS "sdl includes: ${SDL2_INCLUDE_DIRS}")
#message(STATUS "sdl libraries: ${SDL2_LIBRARIES}")

#message(STATUS "sdl ttf incl: ${SDL2TTF_INCLUDE_DIRS}")
#message(STATUS "sdl ttf lib: ${SDL2TTF_LIBRARIES}")

#message(STATUS "sdl image incle: ${SDL2IMAGE_INCLUDE_DIRS}")
#message(STATUS "sdl lib innluide: ${SDL2IMAGE_LIBRARIES}")
