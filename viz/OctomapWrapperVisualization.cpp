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
#include <osg/TextureRectangle>

#include <vizkit3d/Vizkit3DHelper.hpp>

using namespace vizkit3d;

void encodeData(osg::StateSet& stateSet,
        unsigned int numInstances,
        std::vector<float> const& cellData)
{
    unsigned int const texLineSize = 1024u;
    unsigned int const vectorsPerSample = 2;
    unsigned int const dataSamplesPerLine = texLineSize / vectorsPerSample;

    if (cellData.size() != numInstances * vectorsPerSample * 4)
        throw std::logic_error("cellData and numInstances mismatch");

    unsigned int height = (numInstances + dataSamplesPerLine - 1) / dataSamplesPerLine;
    osg::ref_ptr<osg::Image> image = new osg::Image;
    image->allocateImage(texLineSize, height, 1, GL_RGBA, GL_FLOAT);
    image->setInternalTextureFormat(GL_RGBA32F_ARB);

    float * data = (float*)image->data(0);
    memcpy(data, &cellData[0], sizeof(float) * cellData.size());

    osg::ref_ptr<osg::TextureRectangle> texture = new osg::TextureRectangle(image);
    texture->setInternalFormat(GL_RGBA32F_ARB);
    texture->setSourceFormat(GL_RGBA);
    texture->setSourceType(GL_FLOAT);
    texture->setTextureSize(2, numInstances);
    texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::NEAREST);
    texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::NEAREST);
    texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_BORDER);
    texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_BORDER);
    stateSet.setTextureAttributeAndModes(1, texture, osg::StateAttribute::ON);
    stateSet.addUniform(new osg::Uniform("cellData", 1));
}

void drawBox(osg::Vec3Array& vertices, osg::DrawElementsUInt& indices) {
    vertices.push_back(osg::Vec3(-0.5, -0.5, -0.5));
    vertices.push_back(osg::Vec3(+0.5, -0.5, -0.5));
    vertices.push_back(osg::Vec3(-0.5, +0.5, -0.5));
    vertices.push_back(osg::Vec3(+0.5, +0.5, -0.5));
    vertices.push_back(osg::Vec3(-0.5, -0.5, +0.5));
    vertices.push_back(osg::Vec3(+0.5, -0.5, +0.5));
    vertices.push_back(osg::Vec3(-0.5, +0.5, +0.5));
    vertices.push_back(osg::Vec3(+0.5, +0.5, +0.5));

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
        indices.addElement(relativeIndices[i]);
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
    osg::Uniform* resolution   =
        new osg::Uniform( "resolution", 0.1f);
    ss->addUniform( resolution );
}

osg::ref_ptr<osg::Node> OctomapWrapperVisualization::createMainNode() {
    osg::Geode* root = new osg::Geode;
    osg::Geometry* geom = new osg::Geometry;
    setupShaders130(geom);
    // Build the template geometry (a cube !)
    osg::ref_ptr < osg::DrawElementsUInt > draw =
        new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES);
    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
    drawBox(*vertices, *draw);
    geom->setVertexAttribArray(0, vertices);
    geom->setVertexAttribBinding(0, osg::Geometry::BIND_PER_VERTEX);
    draw->setNumInstances(0);
    geom->addPrimitiveSet(draw.get());
    root->addDrawable(geom);
    return root;
}

void OctomapWrapperVisualization::updateMainNode(osg::Node* node) {
    osg::Geode* treeNode = dynamic_cast<osg::Geode*>(node);
    if (!tree)
        return;

    osg::ref_ptr<osg::Geometry> geom =
        dynamic_cast<osg::Geometry*>(treeNode->getDrawable(0));

    // We encode the data as [x,y,z,s,p,0] where s is the size in cells and p the
    // probability
    std::vector<float> cellData;

    unsigned int count = 0;

    // Documentation states that getting the end leaf iterator is expensive
    // and should be done really only once
    octomap::OcTree::leaf_iterator const end_it = tree->end_leafs();
    for (octomap::OcTree::leaf_iterator it = tree->begin_leafs(0); it != end_it; ++it) {
        cellData.push_back(it.getX());
        cellData.push_back(it.getY());
        cellData.push_back(it.getZ());
        cellData.push_back(0);

        cellData.push_back(it.getSize());
        cellData.push_back(it->getOccupancy());
        cellData.push_back(0);
        cellData.push_back(0);

        ++count;
    }

    geom->getPrimitiveSet(0)->setNumInstances(count);
    geom->setUseDisplayList(false);
    geom->setUseVertexBufferObjects(true);

    osg::StateSet *ss = geom->getOrCreateStateSet();
    encodeData(*ss, count, cellData);

    // And update the occupation threshold
    osg::Uniform* threshold = ss->getUniform("occupiedThreshold");
    threshold->set(static_cast<float>(tree->getClampingThresMax()) - 0.01f);
    osg::Uniform* resolution = ss->getUniform("resolution");
    resolution->set(static_cast<float>(tree->getResolution()));
}

void OctomapWrapperVisualization::updateDataIntern(
        octomap_wrapper::OctomapWrapper const& value) {

    delete tree;
    tree = octomap_wrapper::binaryMsgToMap(value);
}

//Macro that makes this plugin loadable in ruby, this is optional.
VizkitQtPlugin(OctomapWrapperVisualization)

