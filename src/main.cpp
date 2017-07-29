#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

#include <cstdio>
#include <vector>
#include <fstream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "imgui.h"
#include "imgui_impl_glfw_gl3.h"

#include "cloth.h"
#include "camera.h"
#include "shader.h"

#define IM_ARRAYSIZE(_ARR)  ((int)(sizeof(_ARR)/sizeof(*_ARR)))


GLFWwindow *create_window() {
    int width = 1100;
    int height = 720;
    const char *title = "Cloth";

    return glfwCreateWindow(width, height, title, NULL, NULL);
}

void error_callback(int error, const char *desc) {
    std::printf("[GLFW ERROR] %s (%d)\n", desc, error);
}

glm::vec3 sphereCenter(0.0f, 0.0f, -4.0f);
void keyCallback(GLFWwindow *win, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        float delta = 0.01;
        if (key == GLFW_KEY_A)
            sphereCenter[0] -= delta;
        else if (key == GLFW_KEY_D)
            sphereCenter[0] += delta;
        else if (key == GLFW_KEY_S)
            sphereCenter[1] -= delta;
        else if (key == GLFW_KEY_W)
            sphereCenter[1] += delta;
        else if (key == GLFW_KEY_Z)
            sphereCenter[2] -= delta;
        else if (key == GLFW_KEY_X)
            sphereCenter[2] += delta;
    }
}

int main(int argc, char **argv) {
    if (!glfwInit()) {
        std::printf("Failed to init GLFW\n");
        return -1;
    }

    glfwSetErrorCallback(error_callback);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); 
    GLFWwindow *win = create_window();
    if (!win) {
        std::printf("Couldn't create GLFW window\n");
        return -1;
    }

    ImGui_ImplGlfwGL3_Init(win, true);

    glfwMakeContextCurrent(win);
    glfwSwapInterval(1);
    
    if (gl3wInit()) {
        std::printf("Failed to init OpenGL\n");
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    //ImGuiIO& io = ImGui::GetIO();
    //io.Fonts->AddFontFromFileTTF("Karla-Regular.ttf", 18.0f, NULL, NULL);

    std::printf("OpenGL %s\n", glGetString(GL_VERSION));
    std::printf("GLSL %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    ShaderProgram shader("shaders/shader.vert", "shaders/shader.frag");

    OrbitCamera camera;
    glfwSetWindowUserPointer(win, &camera);
    glfwSetCursorPosCallback(win, OrbitCamera::cursorPosCallback);
    glfwSetMouseButtonCallback(win, OrbitCamera::mouseButtonCallback);
    glfwSetScrollCallback(win, OrbitCamera::scrollCallback);
    camera.setAngles(-M_PI/2, -0.5);

    glfwSetKeyCallback(win, keyCallback);
    
    Cloth cloth(40, 40);

    SphereObstacle *obstacle = new SphereObstacle(sphereCenter, 4.0);
    cloth.obstacles.push_back(obstacle);
    
    ClothMesh mesh(&cloth);

    double prevTime = glfwGetTime();
    static bool run;
    while (!glfwWindowShouldClose(win)) {
        glfwPollEvents();
        ImGui_ImplGlfwGL3_NewFrame();

        glClearColor(0.2, 0.2, 0.2, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shader.program);
        shader.setMat4("cameraDir", camera.getCameraMatrix());
        shader.setVec3("cameraPos", camera.getCameraPos());

        int width, height;
        glfwGetWindowSize(win, &width, &height);
        float aspect = (float) width / height;
        glm::mat4 perspective = glm::perspective(0.5f, aspect, 0.001f, 1000.0f);
        shader.setMat4("perspective", perspective);

        obstacle->center = sphereCenter;

        float dt = (float) (glfwGetTime() - prevTime);
        prevTime = glfwGetTime();
        if (run) {
            for (int i = 0; i < 100; i++)
                cloth.step(dt/25);
        }
        mesh.updateVertexBuffer();
        mesh.draw();
        glUseProgram(0);

        ImGui::Begin("Cloth");
        ImGui::Text("Cloth simulation");
        ImGui::Text("Camera angle: theta %.2f / phi %.2f", camera.theta, camera.phi);
        ImGui::Text("Camera distance: %.2f", camera.distance);
        ImGui::Checkbox("Run", &run);
        if (ImGui::Button("Reset")) {
            cloth.reset();
        }
        ImGui::DragFloat("Point mass", &cloth.pointMass);
        ImGui::DragFloat("Gravity", &cloth.gravityConstant);
        ImGui::DragFloat("Spring", &cloth.springConstant);
        ImGui::DragFloat("Drag", &cloth.airResistance);
        ImGui::End();

        ImGui::Render();


        glfwSwapBuffers(win);
    }

    glfwTerminate();
}
