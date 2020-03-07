#version 330 core

uniform sampler2D tex1;
uniform sampler2D tex2;
uniform vec3 objectColor;
uniform vec3 lightColor;

in vec3 fNormal;
in vec2 fTexCoord;

out vec4 FragColor;

void main() {
    //FragColor = mix(texture(tex1, fTexCoord), texture(tex2, fTexCoord), 0.2);
    FragColor = vec4(lightColor * objectColor, 1.0);
}
