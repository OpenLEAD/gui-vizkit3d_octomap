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
		osg::FloatArray& probabilities,
                osg::DrawElementsUInt& indices,
                const osg::Vec3& position,
		double size, double probability) {
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
            probabilities.push_back(probability);
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

static void setupShaders130(osg::Geometry* geometry)
{
    osg::StateSet *ss = geometry->getOrCreateStateSet();
    osg::Program* program = new osg::Program;
    program->setName( "colorize" );
    program->addBindAttribLocation( "in_Position", 0 );
    program->addBindAttribLocation( "in_Probability", 1 );
    program->addShader( osg::Shader::readShaderFile( osg::Shader::VERTEX, SHADER_DIR "/Octomap.vert" ) );
    program->addShader( osg::Shader::readShaderFile( osg::Shader::FRAGMENT, SHADER_DIR "/Octomap.frag" ) );
    ss->setAttributeAndModes(program, osg::StateAttribute::ON);

    osg::Uniform* fullColor   =
        new osg::Uniform( "colorEmpty", osg::Vec4(0, 1, 0, 1));
    ss->addUniform( fullColor );
    osg::Uniform* emptyColor   =
        new osg::Uniform( "colorFull", osg::Vec4(1, 0, 0, 1));
    ss->addUniform( emptyColor );
    osg::Uniform* threshold   =
        new osg::Uniform( "occupiedThreshold", 0.8f);
    ss->addUniform( threshold );
}

osg::ref_ptr<osg::Node> OctomapWrapperVisualization::createMainNode() {
    osg::Geode* root = new osg::Geode;
    osg::Geometry* geom = new osg::Geometry;
    setupShaders130(geom);
    root->addDrawable(geom);
    return root;
}

void OctomapWrapperVisualization::updateMainNode(osg::Node* node) {
    osg::Geode* treeNode = dynamic_cast<osg::Geode*>(node);
    if (!tree)
        return;

    osg::ref_ptr<osg::Geometry> geom =
        dynamic_cast<osg::Geometry*>(treeNode->getDrawable(0));

    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
    osg::ref_ptr<osg::FloatArray> probabilities = new osg::FloatArray;
    probabilities->setBinding(osg::Array::BIND_PER_VERTEX);
    osg::ref_ptr < osg::DrawElementsUInt > draw =
        new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES);

    unsigned int count = 0;

    // Documentation states that getting the end leaf iterator is expensive
    // and should be done really only once
    octomap::OcTree::leaf_iterator const end_it = tree->end_leafs();
    for (octomap::OcTree::leaf_iterator it = tree->begin_leafs(0); it != end_it; ++it) {
        osg::Vec3 coordinate = osg::Vec3(it.getX(), it.getY(), it.getZ());
        drawBox(*vertices, *probabilities, *draw, coordinate, it.getSize(), it->getOccupancy());

        if (++count > 400000)
            break;
    }
    std::cout << "cell count: " << count << std::endl;
    geom->addPrimitiveSet(draw.get());
    geom->setVertexAttribArray(0, vertices, osg::Array::BIND_PER_VERTEX);
    geom->setVertexAttribArray(1, probabilities, osg::Array::BIND_PER_VERTEX);
    osg::StateSet *ss = geom->getOrCreateStateSet();
    osg::Uniform* threshold = ss->getUniform("occupiedThreshold");
    threshold->set(static_cast<float>(tree->getClampingThresMax()));
}

void OctomapWrapperVisualization::updateDataIntern(
        octomap_wrapper::OctomapWrapper const& value) {

    delete tree;
    tree = octomap_wrapper::binaryMsgToMap(value);
}

//Macro that makes this plugin loadable in ruby, this is optional.
VizkitQtPlugin(OctomapWrapperVisualization)

