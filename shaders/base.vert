#version 300 es
precision lowp float;

layout(location=0) in vec3 pos;
layout(location=1) in vec3 normal;

//separate model matrix for normals
uniform mat4 model;
uniform mat4 viewproj;

out vec3 fragPos; //world space position
out vec3 modNormal;
out vec2 texCoord;

void main(){
    vec4 worldPos=model*vec4(pos,1.0f);
    gl_Position=viewproj*worldPos;
    fragPos=vec3(worldPos);
    modNormal=mat3(model)*normal;
    //texCoord=tx;
}