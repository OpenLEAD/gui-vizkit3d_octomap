rock_library(vizkit_3d_octomap
    SOURCES Dummy.cpp 
    HEADERS Dummy.hpp 
    DEPS_CMAKE octomap)

rock_executable(vizkit_3d_octomap_bin Main.cpp
    DEPS vizkit_3d_octomap)

