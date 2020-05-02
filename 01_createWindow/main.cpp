#include<iostream>
#include<glad/glad.h>
#include<GLFW/glfw3.h>

static const int WND_WIDTH = 1920;
static const int WND_HEIGHT = 1080;

void onFrameBufferResized(GLFWwindow* window, int width, int height);

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE;  // for mac os

    GLFWwindow* window = glfwCreateWindow(WND_WIDTH, WND_HEIGHT, "TriangleDemo", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);


    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glfwSetFramebufferSizeCallback(window, onFrameBufferResized);

    glViewport(0, 0, WND_WIDTH, WND_HEIGHT);

    while (!glfwWindowShouldClose(window))
    {  
        // process input
        glfwPollEvents();

        //rendering
        glClearColor(0.2, 0.3, 0.3, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers(window);
    }

    glfwTerminate();

	return 0;
}


void onFrameBufferResized(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}