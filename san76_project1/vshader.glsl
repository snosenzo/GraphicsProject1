#version 120

attribute vec4 vPosition;
attribute vec4 vColor;
varying vec4 color;
uniform mat4 scale;
uniform mat4 rotate;

void main()
{
    vec4 white;
    white.x = 1;
    white.y = 1;
    white.z = 1;
    white.w = 1;
    color = vColor;
    
    gl_Position = scale * rotate * vPosition;
}
