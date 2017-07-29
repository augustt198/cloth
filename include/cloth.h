#include <glm/glm.hpp>
#include <GL/gl3w.h>

#include <vector>

class Obstacle {
    public:
    Obstacle() {}
 
    virtual bool isInside(glm::vec3 pt) = 0;
    virtual glm::vec3 project(glm::vec3 pt) = 0;
};

class Cloth {

    public:
    Cloth(int width=100, int height=100, float pointMass=0.1f);

    void step(float dt);

    int width, height;
    float pointMass;
    float gravityConstant;
    float springConstant;
    float airResistance;
    glm::vec3 *position;
    glm::vec3 *prevPosition;
    glm::vec3 *initPosition;
    glm::vec3 *velocity;
    glm::vec3 *accel;

    std::vector<Obstacle*> obstacles;

    glm::vec3 calcSpringForce(int x1, int y1, int x2, int y2);
    void reset();
};

class ClothMesh {

    public:
    ClothMesh(Cloth *cloth);

    void updateVertexBuffer();
    void draw();

    private:
    GLuint VAO, VBO, EBO;
    int width, height;
    int indicesCount;
    Cloth *cloth;
};

class SphereObstacle : public Obstacle {
    public:
    SphereObstacle(glm::vec3 center, float radius);

    virtual bool isInside(glm::vec3 pt);
    virtual glm::vec3 project(glm::vec3 pt);

    glm::vec3 center;
    float radius;
};