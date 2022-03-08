#version 300 es
precision lowp float;

layout(location=0) in vec3 pos;
layout(location=1) in vec3 normal;
layout(location=2) in vec2 tx;
layout(location=3) in ivec4 vertIndices;
layout(location=4) in vec4 vertWeights;

//separate model matrix for normals
uniform mat4 model;
uniform mat4 viewproj;

//skinning data
uniform mat4x3 poses[100];

out vec3 fragPos; //world space position
out vec3 modNormal;
out vec2 texCoord;

vec3 deformVertLBS(vec3 target, ivec4 indices,vec4 weights){
    vec3 result=vec3(.0);
    for(int i=0;i<4;i++){
        result+=weights[i]*poses[indices[i]]*vec4(target,1.0);
    }
    return result;
}

vec3 deformNormalLBS(vec3 target, ivec4 indices,vec4 weights){
    vec3 result=vec3(.0);
    for(int i=0;i<4;i++){

        result+=weights[i]*mat3(poses[indices[i]])*target;
    }
    return result;
}

void main(){
    vec4 worldPos=model*vec4(deformVertLBS(pos,vertIndices,vertWeights),1.0f);
    gl_Position=viewproj*worldPos;
    fragPos=vec3(worldPos);
    modNormal=mat3(model)*(deformNormalLBS(normal,vertIndices,vertWeights));
    texCoord=tx;
}