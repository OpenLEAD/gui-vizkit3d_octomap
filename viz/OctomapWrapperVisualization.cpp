#include <iostream>
#include "OctomapWrapperVisualization.hpp"

using namespace vizkit;

struct OctomapWrapperVisualization::Data {
    // Copy of the value given to updateDataIntern.
    //
    // Making a copy is required because of how OSG works
    octomap_wrapper::OctomapWrapper data;
};


OctomapWrapperVisualization::OctomapWrapperVisualization()
    : p(new Data)
{
}

OctomapWrapperVisualization::~OctomapWrapperVisualization()
{
    delete p;
}

osg::ref_ptr<osg::Node> OctomapWrapperVisualization::createMainNode()
{
    // Geode is a common node used for vizkit plugins. It allows to display
    // "arbitrary" geometries
    return new osg::Geode();
}

void OctomapWrapperVisualization::updateMainNode ( osg::Node* node )
{
    osg::Geode* geode = static_cast<osg::Geode*>(node);
    // Update the main node using the data in p->data
}

void OctomapWrapperVisualization::updateDataIntern(octomap_wrapper::OctomapWrapper const& value)
{
    p->data = value;
    std::cout << "got new sample data" << std::endl;
}

//Macro that makes this plugin loadable in ruby, this is optional.
VizkitQtPlugin(OctomapWrapperVisualization)

