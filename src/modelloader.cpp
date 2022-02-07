#include "modelloader.hpp"
#include "../external/iqm.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../external/stb_image.h"

#include<GL/glew.h>
#include<glm/gtc/type_ptr.hpp>
#include<glm/gtx/transform.hpp>
#include<glm/gtc/quaternion.hpp>
#include<glm/gtx/quaternion.hpp>
#include<cstdio>
#include<vector>
#include<iostream>

#include "animation.h"

//Buffers
static std::vector<glm::vec3> positionBuffer;
static std::vector<glm::vec3> normalBuffer;
static std::vector<glm::vec2> texCoordBuffer;
static std::vector<unsigned int>indexBuffer;


static GLuint modelVAO;
static GLuint modelVBO[3];
static GLuint modelElementBuffer;

//Shading
static GLuint defaultShader;

char *loadAscii(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("ERROR: Failed to open file %s\n", filename);
        return NULL;
    }
    int bufsize = 1024, bufused = 0;
    char *contents = (char *)malloc(bufsize * sizeof(char));
    while (!feof(file)) {
        bufused += fread(contents + bufused, sizeof(char), bufsize - bufused, file);
        if (bufused >= bufsize - 1) {
            bufsize *= 2;
            contents = (char *)realloc(contents, bufsize * sizeof(char));
        }
    }
    fclose(file);
    contents[bufused] = '\0';
    contents = (char *)realloc(contents, (bufused + 1) * sizeof(char));
    return contents;
}

void graphicsInit() {
    //create vertex array object
    glGenVertexArrays(1, &modelVAO);
    glBindVertexArray(modelVAO);

    //create vertex buffers
    glGenBuffers(3, modelVBO);
    glBindBuffer(GL_ARRAY_BUFFER, modelVBO[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, modelVBO[1]);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *)0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, modelVBO[2]);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void *)0);
    glEnableVertexAttribArray(2);

    //create element buffer
    glGenBuffers(1, &modelElementBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, modelElementBuffer);

    //create shader
    int success;
    char infoLog[512];

    //create vertex shader
    const char *vertText = loadAscii(ROOTDIR "/shaders/base.vert");
    GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertShader, 1, (const GLchar **)&vertText, NULL);
    glCompileShader(vertShader);
    glGetShaderiv(vertShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertShader, 512, NULL, infoLog);
        printf("Vertex Shader Error:%s", infoLog);
    }
    free((void *)vertText);


    //create fragment shader
    char *fragText = loadAscii(ROOTDIR "/shaders/base.frag");
    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShader, 1, (const GLchar **)&fragText, NULL);
    glCompileShader(fragShader);
    glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragShader, 512, NULL, infoLog);
        printf("Fragment Shader Error:%s", infoLog);
    }
    free((void *)fragText);


    //create flat color shader program
    defaultShader = glCreateProgram();
    glAttachShader(defaultShader, vertShader);
    glAttachShader(defaultShader, fragShader);

    glLinkProgram(defaultShader);
    glGetProgramiv(defaultShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(defaultShader, 512, NULL, infoLog);
        printf("Shader Program Error:%s", infoLog);
    }

    glDeleteShader(vertShader);
    glDeleteShader(fragShader);

    //GL settings
    glDisable(GL_NORMALIZE);
    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    //glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}

static glm::vec3 cameraCenter = glm::vec3(.0f, 1.0f, .0f);
static glm::vec3 eye = glm::vec3(.0f, 2.0f, 5.0f);
static float aspectRatio=16.0f / 9.0f;
void setAspectRatio(float ratio){
    aspectRatio=ratio;
}

void Model::draw() {
    glLoadIdentity();


    glUseProgram(defaultShader);
    glm::mat4 viewProj = glm::perspective(glm::radians(45.0f), aspectRatio, .7f, 20.0f) *
        glm::lookAt(eye, cameraCenter, glm::vec3(.0f, 1.0f, .0f));
    glm::mat4 model =  glm::rotate(glm::radians(-5.0f), glm::vec3(.0f, 1.0f, .0f))*glm::rotate(glm::radians(-90.0f), glm::vec3(1.0f, .0f, .0f));
    glUniformMatrix4fv(glGetUniformLocation(defaultShader, "model"), 1, false, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(defaultShader, "viewproj"), 1, false, glm::value_ptr(viewProj));
    glUniform3f(glGetUniformLocation(defaultShader, "viewPos"), eye.x, eye.y, eye.z);
    glUniform1i(glGetUniformLocation(defaultShader, "textured"), textured);
    glBindTexture(GL_TEXTURE_2D, texture);
    glBindVertexArray(modelVAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, modelElementBuffer);
    glDrawElements(GL_TRIANGLES, vertexCount, GL_UNSIGNED_INT, (void *)(vertexOffset * sizeof(unsigned int)));
}

void Model::animate(float frame){
    //compute new positions
    animationData.deformPositionLBS(&positionBuffer[bufferOffset],frame);
    animationData.deformNormalLBS(&normalBuffer[bufferOffset],frame);
    //reupload buffer
    glBindBuffer(GL_ARRAY_BUFFER, modelVBO[0]);
    glBufferSubData(GL_ARRAY_BUFFER, bufferOffset*sizeof(glm::vec3),animationData.baseVertices.size() * sizeof(glm::vec3), &positionBuffer[bufferOffset]);
    glBindBuffer(GL_ARRAY_BUFFER, modelVBO[1]);
    glBufferSubData(GL_ARRAY_BUFFER, bufferOffset*sizeof(glm::vec3),animationData.baseVertices.size() * sizeof(glm::vec3), &normalBuffer[bufferOffset]);
}


Model loadIQM(const char *filename) {
    Model m;
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        std::cerr << "File not found: " << filename << "\n";
        return m;
    }

    iqmheader header;
    fread(&header, sizeof(header), 1, file);

    m.bufferOffset=positionBuffer.size();

    //read vertex arrays
    if (header.ofs_vertexarrays > 0) {
        fseek(file, header.ofs_vertexarrays, SEEK_SET);
        iqmvertexarray *vertArray = new iqmvertexarray[header.num_vertexarrays];
        fread(vertArray, sizeof(iqmvertexarray), header.num_vertexarrays, file);

        float positions[header.num_vertexes * 3];
        float normals[header.num_vertexes * 3];
        float texCoords[header.num_vertexes * 2];
        unsigned char *blendWeights=nullptr;
        unsigned char *blendIndices=nullptr;
        for (int i = 0;i < header.num_vertexarrays;i++) {
            if (vertArray[i].type == IQM_POSITION) {
                fseek(file, vertArray[i].offset, SEEK_SET);
                fread(positions, sizeof(float), header.num_vertexes * 3, file);
            }
            else if (vertArray[i].type == IQM_NORMAL) {
                fseek(file, vertArray[i].offset, SEEK_SET);
                fread(normals, sizeof(float), header.num_vertexes * 3, file);
            }
            else if (vertArray[i].type == IQM_TEXCOORD) {
                fseek(file, vertArray[i].offset, SEEK_SET);
                fread(texCoords, sizeof(float), header.num_vertexes * 2, file);
            }
            else if (vertArray[i].type == IQM_BLENDWEIGHTS){
                fseek(file,vertArray[i].offset,SEEK_SET);
                m.animationData.weightsPerVertex=vertArray[i].size;
                blendWeights=new unsigned char[header.num_vertexes*vertArray[i].size];
                fread(blendWeights,sizeof(unsigned char),header.num_vertexes*vertArray[i].size,file);
            }
            else if(vertArray[i].type==IQM_BLENDINDEXES){
                fseek(file,vertArray[i].offset,SEEK_SET);
                m.animationData.weightsPerVertex=vertArray[i].size;
                blendIndices=new unsigned char[header.num_vertexes*vertArray[i].size];
                fread(blendIndices,sizeof(unsigned char),header.num_vertexes*vertArray[i].size,file);
            }
        }


        for (int i = 0;i < header.num_vertexes;i++) {
            positionBuffer.push_back(glm::vec3(positions[3 * i], positions[3 * i + 1], positions[3 * i + 2]));
            m.animationData.baseVertices.push_back(glm::vec3(positions[3 * i], positions[3 * i + 1], positions[3 * i + 2]));
            normalBuffer.push_back(glm::vec3(normals[3 * i], normals[3 * i + 1], normals[3 * i + 2]));
            m.animationData.baseNormals.push_back(glm::vec3(normals[3 * i], normals[3 * i + 1], normals[3 * i + 2]));
            texCoordBuffer.push_back(glm::vec2(texCoords[2 * i], texCoords[2 * i + 1]));
            if(blendWeights!=nullptr&&blendIndices!=nullptr){
                for(int j=0;j<m.animationData.weightsPerVertex;j++){
                    m.animationData.vertexWeights.push_back(blendWeights[i*m.animationData.weightsPerVertex+j]/255.0f);
                    m.animationData.weightIndices.push_back(blendIndices[i*m.animationData.weightsPerVertex+j]);
                }
            }
        }
        if(blendWeights!=nullptr)
            delete[] blendWeights;
        if(blendIndices!=nullptr)
            delete[] blendIndices;
        delete[] vertArray;
    }

    //read face indices
    m.vertexOffset = indexBuffer.size();
    if (header.ofs_triangles > 0) {
        unsigned int *indices = new unsigned int[header.num_triangles * 3];
        fseek(file, header.ofs_triangles, SEEK_SET);
        fread(indices, sizeof(unsigned int), 3 * header.num_triangles, file);
        for (int i = 0;i < header.num_triangles;i++) {
            indexBuffer.push_back(indices[3 * i + 2]);
            indexBuffer.push_back(indices[3 * i + 1]);
            indexBuffer.push_back(indices[3 * i]);
        }
        delete[] indices;
    }
    m.vertexCount = indexBuffer.size() - m.vertexOffset;

    //read joints
    if(header.ofs_joints>0){
        iqmjoint joints[header.num_joints];
        fseek(file,header.ofs_joints,SEEK_SET);
        fread(joints,sizeof(iqmjoint),header.num_joints,file);
        
        //evaluate local-space offsets
        glm::mat4 localJoints[header.num_joints];
        glm::mat4 inverseLocalJoints[header.num_joints];

        m.joints.reserve(header.num_joints);

        for(int i=0;i<header.num_joints;i++){
            localJoints[i]=glm::translate(glm::vec3(joints[i].translate[0],joints[i].translate[1],joints[i].translate[2]))
                *glm::toMat4(glm::quat(joints[i].rotate[3],joints[i].rotate[0],joints[i].rotate[1],joints[i].rotate[2]))*
                glm::scale(glm::vec3(joints[i].scale[0],joints[i].scale[1],joints[i].scale[2]));
            inverseLocalJoints[i]=glm::inverse(localJoints[i]);

            if(joints[i].parent>=0){
                //add parent offset
                localJoints[i]=localJoints[joints[i].parent]*localJoints[i];    
                inverseLocalJoints[i]=inverseLocalJoints[i]*inverseLocalJoints[joints[i].parent];
            }

            m.joints[i].matrix=localJoints[i];
            m.joints[i].inverse=inverseLocalJoints[i];
            
        }
    }

    if(header.ofs_poses>0){
        iqmpose poses[header.num_poses];
        fseek(file,header.ofs_poses,SEEK_SET);
        fread(poses,sizeof(iqmpose),header.num_poses,file);

        unsigned short framedata[header.num_frames*header.num_framechannels];
        int framedataIterator=0;
        fseek(file,header.ofs_frames,SEEK_SET);
        fread(framedata,sizeof(unsigned short),header.num_frames*header.num_framechannels,file);
        
        m.animationData.posesPerFrame=header.num_poses;
        m.animationData.poses.resize(header.num_poses*header.num_frames);

        for(int i=0;i<header.num_frames;i++){
            for(int j=0;j<header.num_poses;j++){

                //compute offsets
                glm::vec3 position=glm::vec3(poses[j].channeloffset[0],poses[j].channeloffset[1],poses[j].channeloffset[2]);
                glm::quat rotation=glm::quat(poses[j].channeloffset[6],poses[j].channeloffset[3],
                    poses[j].channeloffset[4],poses[j].channeloffset[5]);
                glm::vec3 scale=glm::vec3(poses[j].channeloffset[7],poses[j].channeloffset[8],poses[j].channeloffset[9]);

                //add scaled data
                if(poses[j].mask&0x01){
                    position[0]+=framedata[framedataIterator]*poses[j].channelscale[0];
                    framedataIterator++;
                }
                if(poses[j].mask&0x02){
                    position[1]+=framedata[framedataIterator]*poses[j].channelscale[1];
                    framedataIterator++;
                }
                if(poses[j].mask&0x04){
                    position[2]+=framedata[framedataIterator]*poses[j].channelscale[2];
                    framedataIterator++;
                }
                if(poses[j].mask&0x08){
                    rotation.x+=framedata[framedataIterator]*poses[j].channelscale[3];
                    framedataIterator++;
                }
                if(poses[j].mask&0x10){
                    rotation.y+=framedata[framedataIterator]*poses[j].channelscale[4];
                    framedataIterator++;
                }
                if(poses[j].mask&0x20){
                    rotation.z+=framedata[framedataIterator]*poses[j].channelscale[5];
                    framedataIterator++;
                }
                if(poses[j].mask&0x40){
                    rotation.w+=framedata[framedataIterator]*poses[j].channelscale[6];
                    framedataIterator++;
                }
                if(poses[j].mask&0x80){
                    scale[0]+=framedata[framedataIterator]*poses[j].channelscale[7];
                    framedataIterator++;
                }
                if(poses[j].mask&0x100){
                    scale[1]+=framedata[framedataIterator]*poses[j].channelscale[8];
                    framedataIterator++;
                }
                if(poses[j].mask&0x200){
                    scale[2]+=framedata[framedataIterator]*poses[j].channelscale[9];
                    framedataIterator++;
                }




                glm::mat4 pose=glm::translate(position)*glm::toMat4(glm::normalize(rotation))*glm::scale(scale)*m.joints[j].inverse;
                if(poses[j].parent>=0)
                    pose=(glm::mat4x3)m.animationData.poses[i*m.animationData.posesPerFrame+poses[j].parent]*m.joints[poses[j].parent].matrix*pose;

                m.animationData.poses[i*m.animationData.posesPerFrame+j].rotscale=glm::mat3(pose);
                m.animationData.poses[i*m.animationData.posesPerFrame+j].translate=glm::mat4x3(pose)[3];
            }
        }
        
    }



    fclose(file);
    return m;
}

GLuint loadTexture(const char *filename) {
    int width, height, compCount;
    unsigned char *texData = stbi_load(filename, &width, &height, &compCount, 0);
    if (texData == NULL) {
        std::cout << "Failed to load texture " << filename << "\n";
        return 0;
    }
    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    stbi_image_free(texData);
    return texID;
}


void uploadBuffers() {
    //vertex buffers
    glBindBuffer(GL_ARRAY_BUFFER, modelVBO[0]);
    glBufferData(GL_ARRAY_BUFFER, positionBuffer.size() * sizeof(glm::vec3), &positionBuffer[0], GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, modelVBO[1]);
    glBufferData(GL_ARRAY_BUFFER, normalBuffer.size() * sizeof(glm::vec3), &normalBuffer[0], GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, modelVBO[2]);
    glBufferData(GL_ARRAY_BUFFER, texCoordBuffer.size() * sizeof(glm::vec2), &texCoordBuffer[0], GL_DYNAMIC_DRAW);
    //element buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, modelElementBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBuffer.size() * sizeof(unsigned int), &indexBuffer[0], GL_STATIC_DRAW);
}