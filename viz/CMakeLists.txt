add_definitions(-DSHADER_DIR="${CMAKE_INSTALL_PREFIX}/share/vizkit3d_octomap")
rock_vizkit_plugin(vizkit_3d_octomap-viz
    OctomapWrapperVisualization.cpp
    DEPS vizkit_3d_octomap
    MOC OctomapWrapperVisualization.hpp
    HEADERS OctomapWrapperVisualization.hpp
    DEPS_PKGCONFIG eigen3 base-types)

install(FILES Octomap.vert Octomap.frag DESTINATION share/vizkit3d_octomap)
install(FILES octomap_display_file DESTINATION bin)
