#version 330 core

void main() {
    gl_FragDepth = gl_FragCoord.z + (gl_FrontFacing ? 0.0005 : 0.0);
}
