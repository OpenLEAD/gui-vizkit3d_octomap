#ifndef vizkit_3d_octomap_OctomapWrapperVisualization_H
#define vizkit_3d_octomap_OctomapWrapperVisualization_H

#include <vizkit3d/Vizkit3DPlugin.hpp>

#include <octomap_wrapper/OctomapWrapper.hpp>
#include <octomap/OcTree.h>
#include <Eigen/Geometry>

namespace osg {
    class Geometry;
}

namespace vizkit3d
{
    class OctomapWrapperVisualization
        : public Vizkit3DPlugin<octomap_wrapper::OctomapWrapper>
    {
    Q_OBJECT
    public:
        OctomapWrapperVisualization();
        ~OctomapWrapperVisualization();

        Q_INVOKABLE void updateData(octomap_wrapper::OctomapWrapper const &sample)
        {
            vizkit3d::Vizkit3DPlugin<octomap_wrapper::OctomapWrapper>::updateData(sample);
        }


    protected:
        virtual osg::ref_ptr<osg::Node> createMainNode();
        virtual void updateMainNode(osg::Node* node);
        virtual void updateDataIntern(octomap_wrapper::OctomapWrapper const& plan);
        
    private:
        octomap::OcTree* tree;
    };
}
#endif
