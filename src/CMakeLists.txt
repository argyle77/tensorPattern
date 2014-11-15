# Tensor Control src/CMAKELists.txt 
# Created 2014_11_14 by Argyle

# Define the executable source - same as project name.
add_executable (tensorSDL tensorSDL.c)

# Configure the version header - not used yet.
configure_file("version.h.in" "version.h")
add_library(drv-tensor drv-tensor.c)
add_library(pattern pattern.c)

# Included directories
include_directories(${PROJECT_BINARY_DIR}/bin ${SDL_INCLUDE_DIR} ${SDL_TTF_INCLUDE_DIRS} ${SDL_IMAGE_INCLUDE_DIRS} ${GTK2_INCLUDE_DIRS} )
message(STATUS ${GTK2_INCLUDE_DIRS})

# Linked Libraries
target_link_libraries(tensorSDL drv-tensor ${SDL_LIBRARY} ${SDL_TTF_LIBRARIES} ${SDL_IMAGE_LIBRARIES} SDL_gfx ${GTK2_LIBRARIES} pattern)



