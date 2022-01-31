#include "modelloader.hpp"
#include "../external/iqm.h"

#include<GL/glew.h>
#include<glm/gtc/type_ptr.hpp>
#include<glm/gtx/transform.hpp>
#include<glm/gtc/quaternion.hpp>
#include<glm/gtx/quaternion.hpp>
#include<cstdio>
#include<vector>
#include<iostream>

//Buffers
static std::vector<glm::vec3> positionBuffer;
static std::vector<glm::vec3> normalBuffer;
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
    glGenBuffers(2, modelVBO);
    glBindBuffer(GL_ARRAY_BUFFER, modelVBO[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, modelVBO[1]);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *)0);
    glEnableVertexAttribArray(1);

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

    //setup attribute layout
    // glBindAttribLocation(flatShader.id, 0, "pos");
    // glBindAttribLocation(flatShader.id, 1, "tx");
    // glBindAttribLocation(flatShader.id, 2, "normal");

    glLinkProgram(defaultShader);
    glGetProgramiv(defaultShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(defaultShader, 512, NULL, infoLog);
        printf("Shader Program Error:%s", infoLog);
    }

    glDeleteShader(vertShader);
    glDeleteShader(fragShader);

    //GL settings
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}


void Model::draw() {
    glLoadIdentity();


    glUseProgram(defaultShader);
    glm::vec3 eye = glm::vec3(.0f, 2.0f, 5.0f);
    glm::mat4 viewProj = glm::perspective(glm::radians(45.0f), 16.0f / 9.0f, .7f, 20.0f) *
        glm::lookAt(eye, glm::vec3(.0f, 1.0f, .0f), glm::vec3(.0f, 1.0f, .0f));
    glm::mat4 model = glm::rotate(glm::radians(45.0f), glm::vec3(.0f, 1.0f, .0f)) * glm::rotate(glm::radians(-90.0f), glm::vec3(1.0f, .0f, .0f));
    glUniformMatrix4fv(glGetUniformLocation(defaultShader, "model"), 1, false, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(defaultShader, "viewproj"), 1, false, glm::value_ptr(viewProj));
    glUniform3f(glGetUniformLocation(defaultShader, "viewPos"), eye.x, eye.y, eye.z);
    glBindVertexArray(modelVAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, modelElementBuffer);
    glDrawElements(GL_TRIANGLES, vertexCount, GL_UNSIGNED_INT, (void *)(vertexOffset * sizeof(unsigned int)));
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


    //read vertex arrays
    if (header.ofs_vertexarrays > 0) {
        fseek(file, header.ofs_vertexarrays, SEEK_SET);
        iqmvertexarray *vertArray = new iqmvertexarray[header.num_vertexarrays];
        fread(vertArray, sizeof(iqmvertexarray), header.num_vertexarrays, file);

        float positions[header.num_vertexes * 3];
        float normals[header.num_vertexes * 3];
        for (int i = 0;i < header.num_vertexarrays;i++) {
            if (vertArray[i].type == IQM_POSITION) {
                fseek(file, vertArray[i].offset, SEEK_SET);
                fread(positions, sizeof(float), header.num_vertexes * 3, file);
            }
            else if (vertArray[i].type == IQM_NORMAL) {
                fseek(file, vertArray[i].offset, SEEK_SET);
                fread(normals, sizeof(float), header.num_vertexes * 3, file);
            }
        }
        for (int i = 0;i < header.num_vertexes;i++) {
            positionBuffer.push_back(glm::vec3(positions[3 * i], positions[3 * i + 1], positions[3 * i + 2]));
            normalBuffer.push_back(glm::vec3(normals[3 * i], normals[3 * i + 1], normals[3 * i + 2]));
        }
        delete vertArray;
    }

    m.vertexOffset = indexBuffer.size();
    //read face indices

    if (header.ofs_triangles > 0) {
        unsigned int *indices = new unsigned int[header.num_triangles * 3];
        fseek(file, header.ofs_triangles, SEEK_SET);
        fread(indices, sizeof(unsigned int), 3 * header.num_triangles, file);
        for (int i = 0;i < header.num_triangles;i++) {
            indexBuffer.push_back(indices[3 * i + 2]);
            indexBuffer.push_back(indices[3 * i + 1]);
            indexBuffer.push_back(indices[3 * i]);
        }
        delete indices;
    }
    m.vertexCount = indexBuffer.size() - m.vertexOffset;

    //read joints
    if(header.ofs_joints>0){
        iqmjoint joints[header.num_joints];
        fseek(file,header.ofs_joints,SEEK_SET);
        fread(joints,sizeof(iqmjoint),header.num_joints,file);
        m.joints.resize(header.num_joints);
        
        for(int i=0;i<header.num_joints;i++){
            m.joints[i].offset=glm::translate(glm::mat4(1.0f),glm::vec3(joints[i].translate[0],joints[i].translate[1],joints[i].translate[2]))
            *glm::toMat4(glm::quat(joints[i].rotate[0],joints[i].rotate[1],joints[i].rotate[2],joints[i].rotate[3]))*
            glm::scale(glm::mat4(1.0f),glm::vec3(joints[i].scale[0],joints[i].scale[1],joints[i].scale[2]));
            m.joints[i].parent=joints[i].parent;
            if(joints[i].parent<0){
                m.rootJoint=i;
            }
        }
    }



    fclose(file);
    return m;
}


void uploadBuffers() {
    //vertex buffers
    glBindBuffer(GL_ARRAY_BUFFER, modelVBO[0]);
    glBufferData(GL_ARRAY_BUFFER, positionBuffer.size() * sizeof(glm::vec3), &positionBuffer[0], GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, modelVBO[1]);
    glBufferData(GL_ARRAY_BUFFER, normalBuffer.size() * sizeof(glm::vec3), &normalBuffer[0], GL_DYNAMIC_DRAW);
    //element buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, modelElementBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBuffer.size() * sizeof(unsigned int), &indexBuffer[0], GL_STATIC_DRAW);
}