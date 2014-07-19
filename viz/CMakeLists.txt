rock_vizkit_plugin(vizkit_3d_octomap-viz
    OctomapWrapperVisualization.cpp
    DEPS vizkit_3d_octomap
    MOC OctomapWrapperVisualization.hpp
    HEADERS OctomapWrapperVisualization.hpp
    DEPS_PKGCONFIG eigen3 base-types)

install(FILES octomap_display_file DESTINATION bin)
