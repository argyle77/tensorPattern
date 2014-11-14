# Tensor Control src/CMAKELists.txt 
# Created 2014_11_14 by Argyle

# Define the executable source - same as project name.
add_executable (tensor test1.c)

# Configure the version header - not used yet.
configure_file("version.h.in" "version.h")
add_library(drv-tensor drv-tensor.c)

# Included directories
include_directories(${PROJECT_BINARY_DIR}/bin ${SDL_INCLUDE_DIR})

# Linked Libraries
target_link_libraries(tensor drv-tensor ${SDL_LIBRARY})

# file(COPY font.ttf Fbyte-01.jpg DESTINATION ${PROJECT_BINARY_DIR}/bin) - not yet

