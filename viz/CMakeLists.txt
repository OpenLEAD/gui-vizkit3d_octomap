rock_vizkit_plugin(vizkit_3d_octomap-viz
    OctomapWrapperVisualization.cpp
    DEPS vizkit_3d_octomap
    DEPS_PKGCONFIG base-types
    MOC OctomapWrapperVisualization.hpp
    HEADERS OctomapWrapperVisualization.hpp)

