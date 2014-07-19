#include <iostream>
#include "OctomapWrapperVisualization.hpp"
#include <cstddef>

#include <osg/PositionAttitudeTransform>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Point>
#include <base/Eigen.hpp>
#include <octomap_wrapper/Conversion.hpp>

#include <osg/ShapeDrawable>

#include <vizkit3d/Vizkit3DHelper.hpp>

using namespace vizkit3d;

void drawBox(osg::Vec3Array& vertices,
		osg::Vec4Array& colors,
                osg::DrawElementsUInt& indices,
                const osg::Vec3& position,
		double size, const osg::Vec4& color) {
	const double xp = position.x();
	const double yp = position.y();
	const double zp = position.z();

	const double eps = 1e-5;

	const double xs = size - eps;
	const double ys = size - eps;
	const double zs = size - eps;

        const unsigned int baseIndex = vertices.size();
	vertices.push_back(osg::Vec3(xp - xs * 0.5, yp - ys * 0.5, zp - zs * 0.5));
	vertices.push_back(osg::Vec3(xp + xs * 0.5, yp - ys * 0.5, zp - zs * 0.5));
	vertices.push_back(osg::Vec3(xp - xs * 0.5, yp + ys * 0.5, zp - zs * 0.5));
	vertices.push_back(osg::Vec3(xp + xs * 0.5, yp + ys * 0.5, zp - zs * 0.5));
	vertices.push_back(osg::Vec3(xp - xs * 0.5, yp - ys * 0.5, zp + zs * 0.5));
	vertices.push_back(osg::Vec3(xp + xs * 0.5, yp - ys * 0.5, zp + zs * 0.5));
	vertices.push_back(osg::Vec3(xp - xs * 0.5, yp + ys * 0.5, zp + zs * 0.5));
	vertices.push_back(osg::Vec3(xp + xs * 0.5, yp + ys * 0.5, zp + zs * 0.5));

	for (size_t i = 0; i < 8; i++) {
            colors.push_back(color);
	}

        unsigned int relativeIndices[] = {
            0, 2, 1,
            2, 3, 1,
            1, 3, 5,
            3, 7, 5,
            7, 4, 5,
            7, 6, 4,
            6, 0, 4,
            6, 2, 0,
            0, 4, 1,
            4, 5, 1,
            3, 7, 2,
            7, 6, 2
        };
        for (int i = 0; i < 12 * 3; ++i)
            indices.addElement(baseIndex + relativeIndices[i]);
}

OctomapWrapperVisualization::OctomapWrapperVisualization()
        : tree(0) {}

OctomapWrapperVisualization::~OctomapWrapperVisualization() {
        delete tree;
}

osg::ref_ptr<osg::Node> OctomapWrapperVisualization::createMainNode() {
        osg::Geode* root = new osg::Geode;
	osg::Geometry* geom = new osg::Geometry;
        root->addDrawable(geom);
	return root;

}
void OctomapWrapperVisualization::updateMainNode(osg::Node* node) {
        osg::Geode* treeNode = dynamic_cast<osg::Geode*>(node);
        if (!tree)
            return;

	osg::ref_ptr<osg::Geometry> geom = dynamic_cast<osg::Geometry*>(treeNode->getDrawable(0));

	osg::ref_ptr < osg::Vec3Array > vertices = new osg::Vec3Array;
	osg::ref_ptr < osg::Vec4Array > color = new osg::Vec4Array;
        color->setBinding(osg::Array::BIND_PER_VERTEX);

	osg::Vec4 color_occ_thres = osg::Vec4(1.0f, 0, 0, 1.0f);
	osg::Vec4 color_occ = osg::Vec4(1.0f, 0, 0, 1.0f);

	osg::Vec4 color_emp_thres = osg::Vec4(0.0f, 1.0f, 0.0f, 1.0f);
	osg::Vec4 color_emp = osg::Vec4(0.0f, 1.0f, 0.0f, 1.0f);
	osg::ref_ptr < osg::DrawElementsUInt > draw =
            new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES);

        unsigned int count = 0;

        // Documentation states that getting the end leaf iterator is expensive
        // and should be done really only once
        octomap::OcTree::leaf_iterator const end_it = tree->end_leafs();
	for (octomap::OcTree::leaf_iterator it = tree->begin_leafs(0); it != end_it; ++it) {
                osg::Vec3 coordinate = osg::Vec3(it.getX(), it.getY(), it.getZ());
                osg::Vec4 cell_color;

                if (tree->isNodeOccupied(*it)) { // occupied voxels
                        if (tree->isNodeAtThreshold(*it))
                                cell_color = color_occ_thres;
                        else
                                cell_color = color_occ;
                } else {
                        if (tree->isNodeAtThreshold(*it)) {
                                cell_color = color_emp_thres;
                        } else {
                                cell_color = color_emp;
                        }
                }
                drawBox(*vertices, *color, *draw, coordinate, it.getSize(), cell_color);

                if (++count > 50000)
                    break;
	}
        std::cout << "cell count: " << count << std::endl;
	geom->setVertexArray(vertices);
	geom->setColorArray(color.get());
	geom->addPrimitiveSet(draw.get());
}

void OctomapWrapperVisualization::updateDataIntern(
		octomap_wrapper::OctomapWrapper const& value) {

        delete tree;
	tree = octomap_wrapper::binaryMsgToMap(value);
}

//Macro that makes this plugin loadable in ruby, this is optional.
VizkitQtPlugin(OctomapWrapperVisualization)

