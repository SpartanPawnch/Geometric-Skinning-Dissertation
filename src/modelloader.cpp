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
#include "camera.hpp"

//Buffers
static std::vector<glm::vec3> positionBuffer;
static std::vector<glm::vec3> normalBuffer;
static std::vector<glm::vec2> texCoordBuffer;
static std::vector<glm::ivec4> weightIndexBuffer;
static std::vector<glm::vec4> weightBuffer;

static std::vector<unsigned int>indexBuffer;


static GLuint modelVAO;
static GLuint modelVBO[5];
static GLuint modelElementBuffer;

//Shading
static GLuint defaultShader;
static GLuint skinnedShader;

//Camera
Camera sceneCamera;


char* loadAscii(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("ERROR: Failed to open file %s\n", filename);
        return NULL;
    }
    int bufsize = 1024, bufused = 0;
    char* contents = (char*)malloc(bufsize * sizeof(char));
    while (!feof(file)) {
        bufused += fread(contents + bufused, sizeof(char), bufsize - bufused, file);
        if (bufused >= bufsize - 1) {
            bufsize *= 2;
            contents = (char*)realloc(contents, bufsize * sizeof(char));
        }
    }
    fclose(file);
    contents[bufused] = '\0';
    contents = (char*)realloc(contents, (bufused + 1) * sizeof(char));
    return contents;
}

void graphicsInit() {

    //create vertex array object
    glGenVertexArrays(1, &modelVAO);
    glBindVertexArray(modelVAO);

    //create vertex buffers
    glGenBuffers(5, modelVBO);
    glBindBuffer(GL_ARRAY_BUFFER, modelVBO[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, modelVBO[1]);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, modelVBO[2]);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, modelVBO[3]);
    glVertexAttribPointer(3, 4, GL_INT, GL_FALSE, sizeof(glm::ivec4), (void*)0);
    glEnableVertexAttribArray(3);

    glBindBuffer(GL_ARRAY_BUFFER, modelVBO[4]);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
    glEnableVertexAttribArray(4);

    //create element buffer
    glGenBuffers(1, &modelElementBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, modelElementBuffer);

    //create shader
    int success;
    char infoLog[512];

    //create regular vertex shader
    char* vertText = loadAscii(ROOTDIR "/shaders/base.vert");
    GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertShader, 1, (const GLchar**)&vertText, NULL);
    glCompileShader(vertShader);
    glGetShaderiv(vertShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertShader, 512, NULL, infoLog);
        printf("Vertex Shader Error:%s", infoLog);
    }
    free((void*)vertText);

    //create skinned vertex shader
    vertText = loadAscii(ROOTDIR "/shaders/skinned.vert");
    GLuint skinnedVertShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(skinnedVertShader, 1, (const GLchar**)&vertText, NULL);
    glCompileShader(skinnedVertShader);
    glGetShaderiv(skinnedVertShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(skinnedVertShader, 512, NULL, infoLog);
        printf("Skinned Vertex Shader Error:%s", infoLog);
    }
    free((void*)vertText);


    //create fragment shader
    char* fragText = loadAscii(ROOTDIR "/shaders/base.frag");
    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShader, 1, (const GLchar**)&fragText, NULL);
    glCompileShader(fragShader);
    glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragShader, 512, NULL, infoLog);
        printf("Fragment Shader Error:%s", infoLog);
    }
    free((void*)fragText);


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

    //create skinned shader program
    skinnedShader = glCreateProgram();
    glAttachShader(skinnedShader, skinnedVertShader);
    glAttachShader(skinnedShader, fragShader);

    glLinkProgram(skinnedShader);
    glGetProgramiv(skinnedShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(skinnedShader, 512, NULL, infoLog);
        printf("Shader Program Error:%s", infoLog);
    }

    glDeleteShader(vertShader);
    glDeleteShader(skinnedVertShader);
    glDeleteShader(fragShader);

    //GL settings
    glDisable(GL_NORMALIZE);
    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    //glPixelStorei(GL_UNPACK_ALIGNMENT, 1);


    //camera setup
    sceneCamera = Camera(glm::vec3(.0f, 2.0f, 4.0f), glm::vec3(.0f, 1.0f, .0f), glm::vec3(.0f, 1.0f, .0f));
    sceneCamera.screenWidth = 1600;
    sceneCamera.screenHeight = 900;
}
void setScreen(float width, float height) {
    sceneCamera.screenWidth = width;
    sceneCamera.screenHeight = height;
}

//TODO: refactor to remove this
static float targetFrame = .0f;

void Model::draw() {
    glLoadIdentity();

    GLuint activeShader = defaultShader;
    if (skinningType == SkinningTypeGPU)
        activeShader = skinnedShader;


    glUseProgram(activeShader);

    if (skinningType == SkinningTypeGPU) {
        if (currentClip >= 0)
            animationData.uploadPose(targetFrame, skinnedShader, clips[currentClip]);
        else
            animationData.uploadPose(targetFrame, skinnedShader);
    }

    glm::mat4 viewProj = sceneCamera.getMatrix();
    glm::mat4 model = glm::rotate(glm::radians(-5.0f), glm::vec3(.0f, 1.0f, .0f)) * glm::rotate(glm::radians(-90.0f), glm::vec3(1.0f, .0f, .0f));
    glUniformMatrix4fv(glGetUniformLocation(activeShader, "model"), 1, false, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(activeShader, "viewproj"), 1, false, glm::value_ptr(viewProj));
    glUniform3f(glGetUniformLocation(activeShader, "viewPos"), sceneCamera.getEye().x, sceneCamera.getEye().y, sceneCamera.getEye().z);
    glUniform1i(glGetUniformLocation(activeShader, "textured"), textured);
    glBindTexture(GL_TEXTURE_2D, texture);
    glBindVertexArray(modelVAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, modelElementBuffer);
    glDrawElements(GL_TRIANGLES, vertexCount, GL_UNSIGNED_INT, (void*)(vertexOffset * sizeof(unsigned int)));
}

void Model::animate(float frame) {
    if (skinningType == SkinningTypeCPU) {
        //compute new positions
        if (currentClip >= 0) {
            animationData.deformPositionLBS(&positionBuffer[bufferOffset], frame, clips[currentClip]);
            animationData.deformNormalLBS(&normalBuffer[bufferOffset], frame, clips[currentClip]);
        }
        else {
            animationData.deformPositionLBS(&positionBuffer[bufferOffset], frame);
            animationData.deformNormalLBS(&normalBuffer[bufferOffset], frame);
        }
        //reupload buffer
        glBindBuffer(GL_ARRAY_BUFFER, modelVBO[0]);
        glBufferSubData(GL_ARRAY_BUFFER, bufferOffset * sizeof(glm::vec3), animationData.baseVertices.size() * sizeof(glm::vec3), &positionBuffer[bufferOffset]);
        glBindBuffer(GL_ARRAY_BUFFER, modelVBO[1]);
        glBufferSubData(GL_ARRAY_BUFFER, bufferOffset * sizeof(glm::vec3), animationData.baseVertices.size() * sizeof(glm::vec3), &normalBuffer[bufferOffset]);
    }
    else {
        targetFrame = frame;
    }
}

void Model::resetBuffers() {
    animationData.resetVertices(&positionBuffer[bufferOffset]);
    animationData.resetNormals(&normalBuffer[bufferOffset]);
    //reupload buffer
    glBindBuffer(GL_ARRAY_BUFFER, modelVBO[0]);
    glBufferSubData(GL_ARRAY_BUFFER, bufferOffset * sizeof(glm::vec3), animationData.baseVertices.size() * sizeof(glm::vec3), &positionBuffer[bufferOffset]);
    glBindBuffer(GL_ARRAY_BUFFER, modelVBO[1]);
    glBufferSubData(GL_ARRAY_BUFFER, bufferOffset * sizeof(glm::vec3), animationData.baseVertices.size() * sizeof(glm::vec3), &normalBuffer[bufferOffset]);
}

void Model::clear() {
    vertexOffset = 0;
    vertexCount = 0;
    bufferOffset = 0;
    textured = false;
    glDeleteTextures(1, &texture);
    texture = 0;
    joints.clear();
    clips.clear();
    clipNames.clear();
    currentClip = -1;
    animationData.clear();
}


Model loadIQM(const char* filename) {
    Model m;
    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        std::cerr << "File not found: " << filename << "\n";
        return m;
    }

    iqmheader header;
    fread(&header, sizeof(header), 1, file);
    if (strcmp(header.magic, "INTERQUAKEMODEL") != 0) {
        std::cout << "Error loading file \"" << filename << "\": Incorrect magic identifier";
        return m;
    }

    m.bufferOffset = positionBuffer.size();

    //read vertex arrays
    if (header.ofs_vertexarrays > 0) {
        fseek(file, header.ofs_vertexarrays, SEEK_SET);
        iqmvertexarray* vertArray = new iqmvertexarray[header.num_vertexarrays];
        fread(vertArray, sizeof(iqmvertexarray), header.num_vertexarrays, file);

        float* positions = new float[header.num_vertexes * 3];
        float* normals = new float[header.num_vertexes * 3];
        float* texCoords = new float[header.num_vertexes * 2];
        unsigned char* blendWeights = NULL;
        unsigned char* blendIndices = NULL;
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
            else if (vertArray[i].type == IQM_BLENDWEIGHTS) {
                fseek(file, vertArray[i].offset, SEEK_SET);
                m.animationData.weightsPerVertex = vertArray[i].size;
                blendWeights = new unsigned char[header.num_vertexes * vertArray[i].size];
                fread(blendWeights, sizeof(unsigned char), header.num_vertexes * vertArray[i].size, file);
            }
            else if (vertArray[i].type == IQM_BLENDINDEXES) {
                fseek(file, vertArray[i].offset, SEEK_SET);
                m.animationData.weightsPerVertex = vertArray[i].size;
                blendIndices = new unsigned char[header.num_vertexes * vertArray[i].size];
                fread(blendIndices, sizeof(unsigned char), header.num_vertexes * vertArray[i].size, file);
            }
        }


        for (int i = 0;i < header.num_vertexes;i++) {
            positionBuffer.push_back(glm::vec3(positions[3 * i], positions[3 * i + 1], positions[3 * i + 2]));
            m.animationData.baseVertices.push_back(glm::vec3(positions[3 * i], positions[3 * i + 1], positions[3 * i + 2]));
            normalBuffer.push_back(glm::vec3(normals[3 * i], normals[3 * i + 1], normals[3 * i + 2]));
            m.animationData.baseNormals.push_back(glm::vec3(normals[3 * i], normals[3 * i + 1], normals[3 * i + 2]));
            texCoordBuffer.push_back(glm::vec2(texCoords[2 * i], texCoords[2 * i + 1]));
            if (blendWeights != NULL && blendIndices != NULL) {
                for (int j = 0;j < m.animationData.weightsPerVertex;j++) {
                    m.animationData.vertexWeights.push_back(blendWeights[i * m.animationData.weightsPerVertex + j] / 255.0f);
                    m.animationData.weightIndices.push_back(blendIndices[i * m.animationData.weightsPerVertex + j]);
                }
            }
        }

        delete[] positions;
        delete[] normals;
        delete[] texCoords;

        if (blendWeights != NULL)
            delete[] blendWeights;
        if (blendIndices != NULL)
            delete[] blendIndices;
        delete[] vertArray;
    }

    //read face indices
    m.vertexOffset = indexBuffer.size();
    if (header.ofs_triangles > 0) {
        unsigned int* indices = new unsigned int[header.num_triangles * 3];
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
    if (header.ofs_joints > 0) {
        iqmjoint joints[header.num_joints];
        fseek(file, header.ofs_joints, SEEK_SET);
        fread(joints, sizeof(iqmjoint), header.num_joints, file);

        //evaluate local-space offsets
        glm::mat4 localJoints[header.num_joints];
        glm::mat4 inverseLocalJoints[header.num_joints];

        m.joints.reserve(header.num_joints);

        for (int i = 0;i < header.num_joints;i++) {
            localJoints[i] = glm::translate(glm::vec3(joints[i].translate[0], joints[i].translate[1], joints[i].translate[2]))
                * glm::toMat4(glm::quat(joints[i].rotate[3], joints[i].rotate[0], joints[i].rotate[1], joints[i].rotate[2])) *
                glm::scale(glm::vec3(joints[i].scale[0], joints[i].scale[1], joints[i].scale[2]));
            inverseLocalJoints[i] = glm::inverse(localJoints[i]);

            if (joints[i].parent >= 0) {
                //add parent offset
                localJoints[i] = localJoints[joints[i].parent] * localJoints[i];
                inverseLocalJoints[i] = inverseLocalJoints[i] * inverseLocalJoints[joints[i].parent];
            }

            m.joints[i].matrix = localJoints[i];
            m.joints[i].inverse = inverseLocalJoints[i];

        }
    }

    if (header.ofs_joints > 0 && header.ofs_poses > 0) {
        iqmpose poses[header.num_poses];
        fseek(file, header.ofs_poses, SEEK_SET);
        fread(poses, sizeof(iqmpose), header.num_poses, file);

        unsigned short framedata[header.num_frames * header.num_framechannels];
        int framedataIterator = 0;
        fseek(file, header.ofs_frames, SEEK_SET);
        fread(framedata, sizeof(unsigned short), header.num_frames * header.num_framechannels, file);

        m.animationData.posesPerFrame = header.num_poses;
        m.animationData.poses.resize(header.num_poses * header.num_frames);

        for (int i = 0;i < header.num_frames;i++) {
            for (int j = 0;j < header.num_poses;j++) {

                //compute offsets
                glm::vec3 position = glm::vec3(poses[j].channeloffset[0], poses[j].channeloffset[1], poses[j].channeloffset[2]);
                glm::quat rotation = glm::quat(poses[j].channeloffset[6], poses[j].channeloffset[3],
                    poses[j].channeloffset[4], poses[j].channeloffset[5]);
                glm::vec3 scale = glm::vec3(poses[j].channeloffset[7], poses[j].channeloffset[8], poses[j].channeloffset[9]);

                //add scaled data
                if (poses[j].mask & 0x01) {
                    position[0] += framedata[framedataIterator] * poses[j].channelscale[0];
                    framedataIterator++;
                }
                if (poses[j].mask & 0x02) {
                    position[1] += framedata[framedataIterator] * poses[j].channelscale[1];
                    framedataIterator++;
                }
                if (poses[j].mask & 0x04) {
                    position[2] += framedata[framedataIterator] * poses[j].channelscale[2];
                    framedataIterator++;
                }
                if (poses[j].mask & 0x08) {
                    rotation.x += framedata[framedataIterator] * poses[j].channelscale[3];
                    framedataIterator++;
                }
                if (poses[j].mask & 0x10) {
                    rotation.y += framedata[framedataIterator] * poses[j].channelscale[4];
                    framedataIterator++;
                }
                if (poses[j].mask & 0x20) {
                    rotation.z += framedata[framedataIterator] * poses[j].channelscale[5];
                    framedataIterator++;
                }
                if (poses[j].mask & 0x40) {
                    rotation.w += framedata[framedataIterator] * poses[j].channelscale[6];
                    framedataIterator++;
                }
                if (poses[j].mask & 0x80) {
                    scale[0] += framedata[framedataIterator] * poses[j].channelscale[7];
                    framedataIterator++;
                }
                if (poses[j].mask & 0x100) {
                    scale[1] += framedata[framedataIterator] * poses[j].channelscale[8];
                    framedataIterator++;
                }
                if (poses[j].mask & 0x200) {
                    scale[2] += framedata[framedataIterator] * poses[j].channelscale[9];
                    framedataIterator++;
                }




                glm::mat4 pose = glm::translate(position) * glm::toMat4(glm::normalize(rotation)) * glm::scale(scale) * m.joints[j].inverse;
                if (poses[j].parent >= 0)
                    pose = (glm::mat4)(glm::mat4x3)m.animationData.poses[i * m.animationData.posesPerFrame + poses[j].parent] * m.joints[poses[j].parent].matrix * pose;

                m.animationData.poses[i * m.animationData.posesPerFrame + j].rotscale = glm::mat3(pose);
                m.animationData.poses[i * m.animationData.posesPerFrame + j].translate = glm::mat4x3(pose)[3];
            }
        }
        m.animatable = true;
    }

    //load animation names
    char animNames[header.num_text];
    if (header.ofs_text > 0) {
        fseek(file, header.ofs_text, SEEK_SET);
        fread(animNames, sizeof(char), header.num_text, file);
    }

    //read animations
    if (header.ofs_anims > 0) {
        iqmanim anims[header.num_anims];
        fseek(file, header.ofs_anims, SEEK_SET);
        fread(anims, sizeof(iqmanim), header.num_anims, file);
        m.clips.reserve(header.num_anims);
        m.clipNames.reserve(header.num_anims);
        for (int i = 0;i < header.num_anims;i++) {
            AnimationClip clip = {
                .offset = anims[i].first_frame,
                .length = anims[i].num_frames,
                .framerate = anims[i].framerate
            };
            m.clips.push_back(clip);
            m.clipNames.push_back(std::string(&animNames[anims[i].name]));
        }
    }



    if (m.animatable) {
        weightBuffer.resize(m.bufferOffset + m.animationData.baseVertices.size());
        weightIndexBuffer.resize(m.bufferOffset + m.animationData.baseVertices.size());
        m.animationData.copyWeights(&weightBuffer[m.bufferOffset], VertexWeightSetBase);
        m.animationData.copyIndices(&weightIndexBuffer[m.bufferOffset]);
    }
    else {
        for (int i = 0;i < m.animationData.baseVertices.size();i++) {
            weightIndexBuffer.push_back(glm::ivec4(0));
            weightBuffer.push_back(glm::vec4(.0f));
        }
    }


    fclose(file);
    return m;
}

GLuint loadTexture(const char* filename) {
    int width, height, compCount;
    unsigned char* texData = stbi_load(filename, &width, &height, &compCount, 0);
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

    //skinning weight data
    glBindBuffer(GL_ARRAY_BUFFER, modelVBO[3]);
    glBufferData(GL_ARRAY_BUFFER, weightIndexBuffer.size() * sizeof(glm::ivec4), &weightIndexBuffer[0], GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, modelVBO[4]);
    glBufferData(GL_ARRAY_BUFFER, weightBuffer.size() * sizeof(glm::vec4), &weightBuffer[0], GL_DYNAMIC_DRAW);

    //element buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, modelElementBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBuffer.size() * sizeof(unsigned int), &indexBuffer[0], GL_STATIC_DRAW);
}

void clearBuffers() {
    positionBuffer.clear();
    normalBuffer.clear();
    texCoordBuffer.clear();
    weightIndexBuffer.clear();
    weightBuffer.clear();
    indexBuffer.clear();
}
