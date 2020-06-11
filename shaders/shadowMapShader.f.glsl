#version 330 core

void main() {
    gl_FragDepth = gl_FragCoord.z + (gl_FrontFacing ? 0.005 : 0.0);
}
