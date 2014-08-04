#version 130
#extension GL_ARB_draw_instanced : enable
#extension GL_ARB_texture_rectangle : enable

const int TEX_SIZE        = 1024;
const int TEX_SAMPLES_PER_CELL  = 2;
const int TEX_LINE_SIZE_IN_CELLS = TEX_SIZE / TEX_SAMPLES_PER_CELL;
const int FLOATS_PER_TEX_SAMPLE = 3;
const int FLOATS_PER_CELL       =
    TEX_SAMPLES_PER_CELL * FLOATS_PER_TEX_SAMPLE;
const int MAX_FLOATS_PER_TEX    =
    TEX_SIZE * TEX_SIZE * FLOATS_PER_TEX_SAMPLE;

uniform vec4 colorEmpty;
uniform vec4 colorFull;
uniform float occupiedThreshold;
uniform float resolution;

uniform sampler2DRect cellData;

in vec3 in_Position;
out vec4 ex_Color;

void main() {
    vec2 texCoord = vec2(
      (gl_InstanceID % TEX_LINE_SIZE_IN_CELLS) * TEX_SAMPLES_PER_CELL,
      gl_InstanceID / TEX_LINE_SIZE_IN_CELLS);
    vec4 data0 = texture2DRect(cellData, texCoord + vec2(0.0, 0.0));
    vec4 data1 = texture2DRect(cellData, texCoord + vec2(1.0, 0.0));

    vec4 position = vec4(in_Position * data1.x, 1) + vec4(data0.x, data0.y, data0.z, 0);
    gl_Position = gl_ModelViewProjectionMatrix * position;
    if (data1.y >= occupiedThreshold)
    {
        ex_Color = colorFull;
    }
    else
    {
        ex_Color = colorEmpty;
    }
}

