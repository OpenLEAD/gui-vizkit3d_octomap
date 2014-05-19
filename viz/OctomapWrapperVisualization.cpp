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

OctomapWrapperVisualization::OctomapWrapperVisualization() {
	treeOrientation = Eigen::Quaterniond::Identity();
	treePosition.setZero();
	newmap = true;
	colorize = false;

	vertices = new osg::Vec3Array();
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

	treeGeom = new osg::Geometry();

	osg::Vec4 color_occ_thres = osg::Vec4(0.0f, 1.0f, 0.59f, 1.0f);
	osg::Vec4 color_occ = osg::Vec4(0.0f, 0.59f, 0.59f, 1.0f);

	osg::Vec4 color_emp_thres;
	osg::Vec4 color_emp;

	if (colorize) {
		color_emp_thres = osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f);
		color_emp = osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f);
	} else {
		osg::Vec4 color_emp_thres = osg::Vec4(1.0f, 0.0f, 1.0f, 0.1f);
		osg::Vec4 color_emp = osg::Vec4(1.0f, 0.0f, 1.0f, 0.1f);
	}

	vertices->clear();
	osg::ref_ptr < osg::Vec4Array > colors(new osg::Vec4Array());
	for (octomap::OcTree::tree_iterator it = tree->begin_tree(0), end =
			tree->end_tree(); it != end; ++it) {
		if (it.isLeaf()) {
			if (tree->isNodeOccupied(*it)) { // occupied voxels
				if (tree->isNodeAtThreshold(*it)) {
					vertices->push_back(
							osg::Vec3(it.getCoordinate().x(),
									it.getCoordinate().y(),
									it.getCoordinate().z()));
					// set a color
					colors->push_back(color_occ_thres);
				} else {
					vertices->push_back(
							osg::Vec3(it.getCoordinate().x(),
									it.getCoordinate().y(),
									it.getCoordinate().z()));
					// set a color
					colors->push_back(color_occ);

				}
			} else {
				if (tree->isNodeAtThreshold(*it)) {
					vertices->push_back(
							osg::Vec3(it.getCoordinate().x(),
									it.getCoordinate().y(),
									it.getCoordinate().z()));
					// set a color
					colors->push_back(color_emp_thres);
				} else {
					vertices->push_back(
							osg::Vec3(it.getCoordinate().x(),
									it.getCoordinate().y(),
									it.getCoordinate().z()));
					// set a color
					colors->push_back(color_emp);
				}
			}
		}
	}

	treeGeom->setVertexArray(vertices.get());
	treeGeom->setColorArray(colors.get());
	treeGeom->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

	treeGeom->addPrimitiveSet(
			new osg::DrawArrays(osg::PrimitiveSet::POINTS, 0,
					vertices->size()));

	treeNode->addDrawable(treeGeom.get());
	osg::StateSet* state = treeGeom->getOrCreateStateSet();
	state->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

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

