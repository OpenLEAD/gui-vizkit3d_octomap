#ifndef _OCTOMAPVISUALIZATION_OSGOCTREEBUILDER_HPP_
#define _OCTOMAPVISUALIZATION_OSGOCTREEBUILDER_HPP_


#include <osg/PolygonMode>
#include <osg/ShapeDrawable>
#include <osg/Geometry>
#include <osg/Geode>
#include <osg/LOD>
#include <osgDB/ReadFile>
#include <osgUtil/PrintVisitor>
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/Viewer>
#include <iostream>
#include <fstream>
#include <sstream>
namespace octomapVisualization{


class OsgOcTreeBuilder
{
public:
OsgOcTreeBuilder() : _maxChildNumber(16), _maxTreeDepth(16),
_maxLevel(0) {}

int getMaxLevel() const { return _maxLevel; }

void setMaxChildNumber( int max ) { _maxChildNumber= max; }

int getMaxChildNumber() const { return _maxChildNumber; }

void setMaxTreeDepth( int max ) { _maxTreeDepth = max; }

int getMaxTreeDepth() const { return _maxTreeDepth; }

typedef std::pair<std::string, osg::BoundingBox>ElementInfo;

osg::Group* build( int depth, const osg::BoundingBox& total, std::vector<ElementInfo>& elements );

protected:

osg::LOD* createNewLevel( int level, const osg::Vec3& center, float radius );
osg::Node* createElement( const std::string& id, const osg::Vec3& center, float side );
int _maxChildNumber;
int _maxTreeDepth;
int _maxLevel;
};

}

#endif
