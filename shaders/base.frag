#version 300 es
precision lowp float;
in vec3 fragPos;
in vec3 modNormal;

uniform vec3 viewPos;


out vec4 fragColor;

void main() {
    vec4 color=vec4(.7,.7,.7,1.0);

    //specular
    vec3 viewDir=normalize(viewPos-fragPos);


    fragColor=(dot(viewDir,modNormal))*color;
}