#ifndef vizkit_3d_octomap_OctomapWrapperVisualization_H
#define vizkit_3d_octomap_OctomapWrapperVisualization_H

#include <boost/noncopyable.hpp>
#include <vizkit/Vizkit3DPlugin.hpp>
#include <osg/Geode>
#include <slam/octomap_wrapper/OctomapWrapper.hpp>

namespace vizkit
{
    class OctomapWrapperVisualization
        : public vizkit::Vizkit3DPlugin<octomap_wrapper::OctomapWrapper>
        , boost::noncopyable
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
        struct Data;
        Data* p;
    };
}
#endif
