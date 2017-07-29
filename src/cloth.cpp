#include "cloth.h"

#include <cstdio>
#include <cmath>

// call this inside a Cloth method
#define IDX(__x, __y) (__x+__y*this->width)

Cloth::Cloth(int width, int height, float pointMass) :
        width(width), height(height), pointMass(pointMass) {

    pointMass = 3;
    gravityConstant = 3.0;
    springConstant = 15.0;
    airResistance = 0.015;

    position     = new glm::vec3[width*height];
    prevPosition = new glm::vec3[width*height];
    initPosition = new glm::vec3[width*height];
    velocity     = new glm::vec3[width*height];
    accel        = new glm::vec3[width*height];

    // initialize positions

    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            int idx = IDX(x, y);
            position[idx] = glm::vec3((x-width/2)/2.0, (y-height/2)/2.0, 0);
            prevPosition[idx] = position[idx];
            initPosition[idx] = position[idx];
            velocity[idx] = glm::vec3(0.0f);
            accel[idx] = glm::vec3(0.0f);
        }
    }
}

glm::vec3 Cloth::calcSpringForce(int x1, int y1, int x2, int y2) {
    glm::vec3 pos1 = position[IDX(x1, y1)];
    glm::vec3 pos2 = position[IDX(x2, y2)];

    glm::vec3 diff = pos2 - pos1;
    float rest = glm::length(initPosition[IDX(x2,y2)] - initPosition[IDX(x1,y1)]);
    return springConstant * (glm::length(diff) - rest) * glm::normalize(diff);
}

void Cloth::step(float dt) {
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            // the top should be fixed
            // if (y == 0 && x == 0) continue;
            // if (y == 0 && x == width-1) continue;
            // if (y == height-1 && x == 0) continue;
            // if (y == height-1 && x == width-1) continue;
            //if (y == 0) continue;

            glm::vec3 prevPos = prevPosition[IDX(x, y)];
            glm::vec3 currPos = position[IDX(x, y)];

            glm::vec3 Fgravity(0.0f, 0.0f, -gravityConstant * pointMass);
            glm::vec3 Fspring(0.0f, 0.0f, 0.0f);
            if (x < width - 1)
                Fspring += calcSpringForce(x, y, x+1, y);
            if (x > 0)
                Fspring += calcSpringForce(x, y, x-1, y);
            if (y < height - 1)
                Fspring += calcSpringForce(x, y, x, y+1);
            if (y > 0)
                Fspring += calcSpringForce(x, y, x, y-1);
            if (x < width - 1 && y < height - 1)
                Fspring += calcSpringForce(x, y, x+1, y+1);
            if (x < width - 1 && y > 0)
                Fspring += calcSpringForce(x, y, x+1, y-1);
            if (x > 0 && y < height - 1)
                Fspring += calcSpringForce(x, y, x-1, y+1);
            if (x > 0 && y > 0)
                Fspring += calcSpringForce(x, y, x-1, y-1);
            
            glm::vec3 v = velocity[IDX(x, y)];
            float vMag = glm::length(v);
            glm::vec3 Fdrag;
            if (vMag != 0)
                Fdrag = -airResistance * (vMag*vMag) * normalize(v);
            else
                Fdrag = glm::vec3(0.0f);
            
            glm::vec3 a = (Fgravity + Fspring + Fdrag) / pointMass;
            accel[IDX(x, y)] = a;

            glm::vec3 newPos = 2.0f*currPos - prevPos + a*(dt*dt);
            prevPosition[IDX(x, y)] = currPos;

            for (int i = 0; i < obstacles.size(); i++) {
                //std::printf("inside!\n");
                if (obstacles[i]->isInside(newPos))
                    newPos = obstacles[i]->project(newPos);
            }

            position[IDX(x, y)] = newPos;
        
            velocity[IDX(x, y)] += a * dt;
            accel[IDX(x, y)] = a;
        }
    }

    //glm::vec3 v = accel[IDX(width-1, height-1)];
    //printf("vert %.03f %.03f %.03f\n", v[0], v[1], v[2]);
}

void Cloth::reset() {
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            position[IDX(x, y)] = glm::vec3((x-width/2)/2.0, (y-height/2)/2.0, 0);
            prevPosition[IDX(x, y)] = position[IDX(x, y)];
            velocity[IDX(x, y)] = glm::vec3(0.0f);
            accel[IDX(x, y)] = glm::vec3(0.0f);
        }
    } 
}

ClothMesh::ClothMesh(Cloth *cloth) : cloth(cloth) {
    glGenVertexArrays(1, &VAO);

    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    // generate element buffer
    int width = cloth->width, height = cloth->height;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    indicesCount = 3 * 2 * (width - 1) * (height - 1);
    int *indices = new int[indicesCount];
    int i = 0;
    for (int x = 0; x < width - 1; x++) {
        for (int y = 0; y < height - 1; y++) {
            indices[i++] = x + y*width;
            indices[i++] = (x+1) + (y+1)*width;
            indices[i++] = x + (y+1)*width;
            indices[i++] = x + y*width;
            indices[i++] = (x+1) + y*width;
            indices[i++] = (x+1) + (y+1) * width;
        }
    }
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesCount * sizeof(int), indices, GL_STATIC_DRAW);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, 5*sizeof(float), (void*) 0);
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, false, 5*sizeof(float), (void*) (3*sizeof(float)));

    glBindVertexArray(0);
}

void ClothMesh::updateVertexBuffer() {
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    float *data = new float[5 * cloth->width * cloth->height];
    int j = 0;
    for (int i = 0; i < cloth->width * cloth->height; i++) {
        data[j++] = cloth->position[i].x;
        data[j++] = cloth->position[i].y;
        data[j++] = cloth->position[i].z;
        data[j++] = i % 2;
        data[j++] = 0.0f;
    }
    glBufferData(GL_ARRAY_BUFFER, 5 * cloth->width * cloth->height * sizeof(float), data, GL_STREAM_DRAW);
}

void ClothMesh::draw() {
    glBindVertexArray(VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glDrawElements(GL_TRIANGLES, indicesCount, GL_UNSIGNED_INT, NULL);
    glBindVertexArray(0);
}

SphereObstacle::SphereObstacle(glm::vec3 center, float radius) :
        center(center), radius(radius) {}

bool SphereObstacle::isInside(glm::vec3 pt) {
    return glm::length(pt - center) <= 1.001f*radius;
}

glm::vec3 SphereObstacle::project(glm::vec3 pt) {
    glm::vec3 dir = glm::normalize(pt - center);
    return center + radius * dir;
}