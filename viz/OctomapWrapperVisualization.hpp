#ifndef vizkit_3d_octomap_OctomapWrapperVisualization_H
#define vizkit_3d_octomap_OctomapWrapperVisualization_H

#include <boost/noncopyable.hpp>
#include <vizkit/Vizkit3DPlugin.hpp>

#include <octomap_wrapper/OctomapWrapper.hpp>
#include <octomap_wrapper/conversion.hpp>
#include "octomap/OcTree.h"
#include "octomap/AbstractOcTree.h"


namespace osg {
    class Geometry;
}

namespace vizkit
{
    class OctomapWrapperVisualization
        : public vizkit::Vizkit3DPlugin<octomap_wrapper::OctomapWrapper>
    {
    Q_OBJECT
    public:
        OctomapWrapperVisualization();
        ~OctomapWrapperVisualization();

    Q_INVOKABLE void updateData(octomap_wrapper::OctomapWrapper const &sample)
    {vizkit::Vizkit3DPlugin<octomap_wrapper::OctomapWrapper>::updateData(sample);}

    protected:
        virtual osg::ref_ptr<osg::Node> createMainNode();
        virtual void updateMainNode(osg::Node* node);
        virtual void updateDataIntern(octomap_wrapper::OctomapWrapper const& plan);
        
    private:

        osg::ref_ptr<osg::Node> drawpoints();

        octomap_wrapper::OctomapWrapper wrapper;
        octomap::OcTree* tree;
        Eigen::Vector3d treePosition;
        Eigen::Quaterniond treeOrientation; // TODO acho q nao vou precisar
        osg::ref_ptr< osg::PositionAttitudeTransform > transformNode;
        osg::ref_ptr<osg::Geode> treeNode;
        osg::ref_ptr<osg::Geometry> treeGeom;

        osg::ref_ptr < osg::Vec3Array > vertices;
        bool newmap;

        bool colorize;
        bool show_polygon;
        double colorize_interval;   // 1/distance

    };
}
#endif
