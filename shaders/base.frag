#version 300 es
precision lowp float;
in vec3 fragPos;
in vec3 modNormal;
in vec2 texCoord;

uniform vec3 viewPos;
uniform bool textured;
uniform sampler2D tex;

out vec4 fragColor;

void main() {
    vec4 color=vec4(.7,.7,.7,1.0);
    if(textured){
        color=texture(tex,texCoord);   
    }


    //specular
    vec3 viewDir=normalize(viewPos-fragPos);


    fragColor=(dot(viewDir,modNormal))*color;
}