#version 130
#extension GL_ARB_draw_instanced: enable

uniform mat4 osg_ViewMatrix;
uniform vec4 colorEmpty;
uniform vec4 colorFull;
uniform float occupiedThreshold;

in vec3 in_Position;
in float in_Probability;
out vec4 ex_Color;

void main() {
    gl_Position = gl_ModelViewProjectionMatrix * vec4(in_Position, 1);
    if (in_Probability >= occupiedThreshold)
    {
        ex_Color = colorFull;
    }
    else
    {
        ex_Color = colorEmpty;
    }
}

