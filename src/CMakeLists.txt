# Tensor Control src/CMAKELists.txt 
# Created 2014_11_14 by Argyle

# Define the executable source - same as project name.
add_executable (blue-wave-left_a2 blue-wave-left_a2.c)
add_executable (blue-wave-up_a blue-wave-up_a.c)
add_executable (color-cycle-ugly_colors color-cycle-ugly_colors.c)
add_executable (green-wave-up_adf green-wave-up_adf.c)
add_executable (magenta-green-ortho-scaler_pl magenta-green-ortho-scaler_pl.c)
add_executable (red-parabaloid-modular_a3 red-parabaloid-modular_a3.c)
add_executable (rgb-hash-sort_ick rgb-hash-sort_ick.c)
add_executable (test1_bayard test1_bayard.c)

# Configure the version header - not used yet.
configure_file("version.h.in" "version.h")
add_library(drv-tensor drv-tensor.c)

# Included directories
include_directories(${PROJECT_BINARY_DIR}/bin)

# Linked Libraries
target_link_libraries(blue-wave-left_a2 drv-tensor)
target_link_libraries(blue-wave-up_a drv-tensor)
target_link_libraries(color-cycle-ugly_colors drv-tensor)
target_link_libraries(green-wave-up_adf drv-tensor)
target_link_libraries(magenta-green-ortho-scaler_pl drv-tensor)
target_link_libraries(red-parabaloid-modular_a3 drv-tensor)
target_link_libraries(rgb-hash-sort_ick drv-tensor)
target_link_libraries(test1_bayard drv-tensor)

# file(COPY font.ttf Fbyte-01.jpg DESTINATION ${PROJECT_BINARY_DIR}/bin) - not yet

