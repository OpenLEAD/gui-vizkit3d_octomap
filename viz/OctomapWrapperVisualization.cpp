#include <iostream>
#include "OctomapWrapperVisualization.hpp"
#include <cstddef>

#include <osg/PositionAttitudeTransform>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Point>
#include <base/eigen.h>

#include <osg/ShapeDrawable>

#include <vizkit/Vizkit3DHelper.hpp>

using namespace vizkit;

void drawBox(osg::ref_ptr<osg::Vec3Array> vertices,
		osg::ref_ptr<osg::Vec3Array> normals,
		osg::ref_ptr<osg::Vec4Array> colors, const osg::Vec3& position,
		double size, const osg::Vec4& color, const osg::Vec3& normal) {
	const double xp = position.x();
	const double yp = position.y();
	const double zp = position.z();

	double eps = 1e-5;

	const double xs = size - eps;
	const double ys = size - eps;
	const double zs = size - eps;

	vertices->push_back(osg::Vec3(xp - xs * 0.5, yp - ys * 0.5, zp + zs * 0.5));
	vertices->push_back(osg::Vec3(xp + xs * 0.5, yp - ys * 0.5, zp + zs * 0.5));
	vertices->push_back(osg::Vec3(xp + xs * 0.5, yp + ys * 0.5, zp + zs * 0.5));
	vertices->push_back(osg::Vec3(xp - xs * 0.5, yp + ys * 0.5, zp + zs * 0.5));
	for (size_t i = 0; i < 4; i++) {
		normals->push_back(normal);
		colors->push_back(color);
	}

	if (zs > 0.0) {
		vertices->push_back(
				osg::Vec3(xp - xs * 0.5, yp - ys * 0.5, zp + zs * 0.5));
		vertices->push_back(
				osg::Vec3(xp + xs * 0.5, yp - ys * 0.5, zp + zs * 0.5));
		vertices->push_back(
				osg::Vec3(xp + xs * 0.5, yp - ys * 0.5, zp - zs * 0.5));
		vertices->push_back(
				osg::Vec3(xp - xs * 0.5, yp - ys * 0.5, zp - zs * 0.5));
		for (size_t i = 0; i < 4; i++) {
			normals->push_back(osg::Vec3(0, -1.0, 0));
			colors->push_back(color);
		}

		vertices->push_back(
				osg::Vec3(xp + xs * 0.5, yp - ys * 0.5, zp + zs * 0.5));
		vertices->push_back(
				osg::Vec3(xp + xs * 0.5, yp + ys * 0.5, zp + zs * 0.5));
		vertices->push_back(
				osg::Vec3(xp + xs * 0.5, yp + ys * 0.5, zp - zs * 0.5));
		vertices->push_back(
				osg::Vec3(xp + xs * 0.5, yp - ys * 0.5, zp - zs * 0.5));
		for (size_t i = 0; i < 4; i++) {
			normals->push_back(osg::Vec3(1.0, 0, 0));
			colors->push_back(color);
		}

		vertices->push_back(
				osg::Vec3(xp + xs * 0.5, yp + ys * 0.5, zp + zs * 0.5));
		vertices->push_back(
				osg::Vec3(xp - xs * 0.5, yp + ys * 0.5, zp + zs * 0.5));
		vertices->push_back(
				osg::Vec3(xp - xs * 0.5, yp + ys * 0.5, zp - zs * 0.5));
		vertices->push_back(
				osg::Vec3(xp + xs * 0.5, yp + ys * 0.5, zp - zs * 0.5));
		for (size_t i = 0; i < 4; i++) {
			normals->push_back(osg::Vec3(0, 1.0, 0));
			colors->push_back(color);
		}

		vertices->push_back(
				osg::Vec3(xp - xs * 0.5, yp + ys * 0.5, zp + zs * 0.5));
		vertices->push_back(
				osg::Vec3(xp - xs * 0.5, yp - ys * 0.5, zp + zs * 0.5));
		vertices->push_back(
				osg::Vec3(xp - xs * 0.5, yp - ys * 0.5, zp - zs * 0.5));
		vertices->push_back(
				osg::Vec3(xp - xs * 0.5, yp + ys * 0.5, zp - zs * 0.5));
		for (size_t i = 0; i < 4; i++) {
			normals->push_back(osg::Vec3(-1.0, 0, 0));
			colors->push_back(color);
		}

		vertices->push_back(
				osg::Vec3(xp - xs * 0.5, yp - ys * 0.5, zp - zs * 0.5));
		vertices->push_back(
				osg::Vec3(xp + xs * 0.5, yp - ys * 0.5, zp - zs * 0.5));
		vertices->push_back(
				osg::Vec3(xp + xs * 0.5, yp + ys * 0.5, zp - zs * 0.5));
		vertices->push_back(
				osg::Vec3(xp - xs * 0.5, yp + ys * 0.5, zp - zs * 0.5));
		for (size_t i = 0; i < 4; i++) {
			normals->push_back(osg::Vec3(0, 0, -1.0));
			colors->push_back(color);
		}
	}
}

OctomapWrapperVisualization::OctomapWrapperVisualization() {
	treeOrientation = Eigen::Quaterniond::Identity();
	treePosition.setZero();
	newmap = true;
	colorize = false;

	std::cout << "constructor" << std::endl;
}

OctomapWrapperVisualization::~OctomapWrapperVisualization() {
	delete tree;
}

osg::ref_ptr<osg::Node> OctomapWrapperVisualization::createMainNode() {

	std::cout << "creating main node" << std::endl;

	transformNode = new osg::PositionAttitudeTransform();
	treeNode = new osg::Geode();
	transformNode->addChild(treeNode);

	return transformNode;

}

void OctomapWrapperVisualization::updateMainNode(osg::Node* node) {
	//conversion from wrapper to octomap

	if (!newmap) {
		treeNode->removeDrawable(treeNode->getDrawable(0));
	}
	newmap = false;

	tree = octomap_wrapper::binaryMsgToMap(wrapper);

	//delete wrapper;

	osg::Vec3 hnormal = osg::Vec3(0, 0, 1.0);

	osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
	osg::ref_ptr < osg::Vec4Array > color = new osg::Vec4Array;
	osg::ref_ptr < osg::Vec3Array > vertices = new osg::Vec3Array;
	osg::ref_ptr < osg::Vec3Array > normals = new osg::Vec3Array;

	osg::Vec4 color_occ_thres = osg::Vec4(1.0f, 0, 0, 1.0f);
	osg::Vec4 color_occ = osg::Vec4(1.0f, 0, 0, 1.0f);

	osg::Vec4 color_emp_thres = osg::Vec4(0.0f, 1.0f, 0.0f, 1.0f);
	osg::Vec4 color_emp = osg::Vec4(0.0f, 1.0f, 0.0f, 1.0f);

	for (octomap::OcTree::tree_iterator it = tree->begin_tree(0), end =
			tree->end_tree(); it != end; ++it) {
		//double x = it->getOccupancy();
		osg::Vec3 coordinate = osg::Vec3(it.getX(), it.getY(), it.getZ());

		if (it.isLeaf()) {

			if (tree->isNodeOccupied(*it)) { // occupied voxels
				if (tree->isNodeAtThreshold(*it)) {
					drawBox(vertices, normals, color, coordinate, it.getSize(),
							color_occ_thres, hnormal);

				} else {
					drawBox(vertices, normals, color, coordinate, it.getSize(),
							color_occ, hnormal);
				}
			} else {
				if (tree->isNodeAtThreshold(*it)) {
					drawBox(vertices, normals, color, coordinate, it.getSize(),
							color_emp_thres, hnormal);
				} else {
					drawBox(vertices, normals, color, coordinate, it.getSize(),
							color_emp, hnormal);
				}
			}
		}
	}
	geom->setVertexArray(vertices);
	osg::ref_ptr < osg::DrawArrays > drawArrays = new osg::DrawArrays(
			osg::PrimitiveSet::QUADS, 0, vertices->size());
	geom->addPrimitiveSet(drawArrays.get());

	geom->setNormalArray(normals);
	geom->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);

	geom->setColorArray(color.get());
	geom->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

	treeNode->addDrawable(geom.get());

	delete tree;

}

void OctomapWrapperVisualization::updateDataIntern(
		octomap_wrapper::OctomapWrapper const& value) {

	wrapper.binary = value.binary;
	wrapper.id = value.id;
	wrapper.resolution = value.resolution;
	wrapper.data = value.data;
}

//Macro that makes this plugin loadable in ruby, this is optional.
VizkitQtPlugin(OctomapWrapperVisualization)

