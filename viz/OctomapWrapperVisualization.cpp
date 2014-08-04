#include <iostream>
#include "OctomapWrapperVisualization.hpp"
#include <cstddef>

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Point>
#include <osg/Version>
#include <osg/TextureRectangle>

#include <base/Eigen.hpp>
#include <octomap_wrapper/Conversion.hpp>

#include <boost/lexical_cast.hpp>
#include <string>
using std::string;
using boost::lexical_cast;

#include <vizkit3d/Vizkit3DHelper.hpp>

using namespace vizkit3d;

/**
 * BIG FAT WARNING
 * BIG FAT WARNING if you change these, you need to copy/paste the whole block
 * BIG FAT WARNING into the Octomap.vert shader, and remove the 'static' prefix
 */
static const int TEX_SIZE        = 1024;
static const int TEX_SAMPLES_PER_CELL  = 2;
static const int TEX_LINE_SIZE_IN_CELLS = TEX_SIZE / TEX_SAMPLES_PER_CELL;
static const int FLOATS_PER_TEX_SAMPLE = 3;
static const int FLOATS_PER_CELL       =
    TEX_SAMPLES_PER_CELL * FLOATS_PER_TEX_SAMPLE;
static const int MAX_FLOATS_PER_TEX    =
    TEX_SIZE * TEX_SIZE * FLOATS_PER_TEX_SAMPLE;
/*
 * BIG FAT WARNING
 * BIG FAT WARNING see comment at the top of the block
 * BIG FAT WARNING
 **/

void encodeData(osg::StateSet& stateSet,
        unsigned int numInstances,
        std::vector<float> const& cellData)
{
    if (cellData.size() != numInstances * FLOATS_PER_TEX_SAMPLE * TEX_SAMPLES_PER_CELL)
        throw std::logic_error("cellData and numInstances mismatch");

    unsigned int height = (numInstances + TEX_LINE_SIZE_IN_CELLS - 1)
        / TEX_LINE_SIZE_IN_CELLS;
    if (height > TEX_SIZE)
        throw std::logic_error("cannot generate textures bigger than " + lexical_cast<string>(TEX_SIZE) + " (got " + lexical_cast<string>(height) + ")");

    osg::ref_ptr<osg::Image> image = new osg::Image;
    image->allocateImage(TEX_SIZE, TEX_SIZE, 1, GL_RGB, GL_FLOAT);
    image->setInternalTextureFormat(GL_RGB32F_ARB);

    float * data = (float*)image->data(0);
    memcpy(data, &cellData[0], sizeof(float) * cellData.size());

    osg::ref_ptr<osg::TextureRectangle> texture = new osg::TextureRectangle(image);
    texture->setInternalFormat(GL_RGB32F_ARB);
    texture->setSourceFormat(GL_RGB);
    texture->setSourceType(GL_FLOAT);
    texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::NEAREST);
    texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::NEAREST);
    texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_BORDER);
    texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_BORDER);
    stateSet.setTextureAttributeAndModes(1, texture, osg::StateAttribute::ON);
    stateSet.addUniform(new osg::Uniform("cellData", 1));
}

void drawBox(osg::Vec3Array& vertices, osg::Vec3Array& normals)
{
    osg::Vec3 corners[] = {
        osg::Vec3(-0.5, -0.5, -0.5), // 0
        osg::Vec3(+0.5, -0.5, -0.5), // 1
        osg::Vec3(-0.5, +0.5, -0.5), // 2
        osg::Vec3(+0.5, +0.5, -0.5), // 3
        osg::Vec3(-0.5, -0.5, +0.5), // 4
        osg::Vec3(+0.5, -0.5, +0.5), // 5
        osg::Vec3(-0.5, +0.5, +0.5), // 6
        osg::Vec3(+0.5, +0.5, +0.5)  // 7
    };

    osg::Vec3 face_normals[] = {
        osg::Vec3( 0,  0, -1),
        osg::Vec3( 1,  0,  0),
        osg::Vec3( 0,  0,  1),
        osg::Vec3(-1,  0,  0),
        osg::Vec3( 0, -1,  0),
        osg::Vec3( 0,  1,  0)
    };

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

    for (int i = 0; i < 12; ++i)
    {
        vertices.push_back(corners[relativeIndices[i * 3]]);
        vertices.push_back(corners[relativeIndices[i * 3 + 1]]);
        vertices.push_back(corners[relativeIndices[i * 3 + 2]]);
        normals.push_back(face_normals[i / 2]);
        normals.push_back(face_normals[i / 2]);
        normals.push_back(face_normals[i / 2]);
    }
}

OctomapWrapperVisualization::OctomapWrapperVisualization()
        : tree(0) {}

OctomapWrapperVisualization::~OctomapWrapperVisualization() {
        delete tree;
}


static void setupGeom(osg::Geometry& geometry)
{
    // Build the template geometry (a cube !)
    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
    osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array;
    drawBox(*vertices, *normals);

    osg::ref_ptr < osg::DrawArrays > draw =
        new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES, 0, vertices->size());
#if OSG_MIN_VERSION_REQUIRED(3,2,0)
    geometry.setVertexAttribArray(0, vertices, osg::Array::BIND_PER_VERTEX);
    geometry.setVertexAttribArray(1, normals, osg::Array::BIND_PER_VERTEX);
#else
    geometry.setVertexAttribArray(0, vertices);
    geometry.setVertexAttribBinding(0, osg::Geometry::BIND_PER_VERTEX);
    geometry.setVertexAttribArray(1, normals);
    geometry.setVertexAttribBinding(1, osg::Geometry::BIND_PER_VERTEX);
#endif
    geometry.setUseDisplayList(false);
    geometry.setUseVertexBufferObjects(true);
    geometry.addPrimitiveSet(draw);
}

osg::ref_ptr<osg::Node> OctomapWrapperVisualization::createMainNode() {
    osg::Geode* geode = new osg::Geode;
    osg::StateSet *ss = geode->getOrCreateStateSet();

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

    osg::Program* program = new osg::Program;
    program->setName( "colorize" );
    program->addBindAttribLocation( "in_Position", 0 );
    program->addBindAttribLocation( "in_Normal", 1 );
    program->addShader( osg::Shader::readShaderFile( osg::Shader::VERTEX, SHADER_DIR "/Octomap.vert" ) );
    program->addShader( osg::Shader::readShaderFile( osg::Shader::FRAGMENT, SHADER_DIR "/Octomap.frag" ) );
    ss->setAttributeAndModes(program, osg::StateAttribute::ON);
    return geode;
}

static void emitGeom(osg::Geode& root, unsigned int current_geometry, std::vector<float> const& cellData, unsigned int count)
{
    if (root.getNumDrawables() == current_geometry)
    {
        osg::Geometry* geom = new osg::Geometry;
        setupGeom(*geom);
        root.addDrawable(geom);
    }
    osg::ref_ptr<osg::Geometry> geom = dynamic_cast<osg::Geometry*>(root.getDrawable(current_geometry));
    geom->getPrimitiveSet(0)->setNumInstances(count);

    osg::StateSet *ss = geom->getOrCreateStateSet();
    encodeData(*ss, count, cellData);
}

void OctomapWrapperVisualization::updateMainNode(osg::Node* node) {
    osg::Geode* root = dynamic_cast<osg::Geode*>(node);
    if (!tree)
        return;

    // We encode the data as [x,y,z,s,p,0] where s is the size in cells and p the
    // probability
    std::vector<float> cellData;
    cellData.reserve(MAX_FLOATS_PER_TEX);

    unsigned int count = 0;
    unsigned int current_geometry = 0;

    // Documentation states that getting the end leaf iterator is expensive
    // and should be done really only once
    octomap::OcTree::leaf_iterator const end_it = tree->end_leafs();
    for (octomap::OcTree::leaf_iterator it = tree->begin_leafs(0); it != end_it; ++it) {
        cellData.push_back(it.getX());
        cellData.push_back(it.getY());
        cellData.push_back(it.getZ());

        cellData.push_back(it.getSize());
        cellData.push_back(it->getOccupancy());
        cellData.push_back(0);
        ++count;

        if (cellData.size() == MAX_FLOATS_PER_TEX)
        {
            emitGeom(*root, current_geometry, cellData, count);
            ++current_geometry;
            cellData.clear();
            count = 0;
        }
    }
    if (count != 0)
        emitGeom(*root, current_geometry, cellData, count);

    // And update the occupation threshold
    osg::StateSet *ss = root->getOrCreateStateSet();
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

