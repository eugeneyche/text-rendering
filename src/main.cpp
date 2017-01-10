#include "text.hpp"
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H


int main()
{
    if (not glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW.\n");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    int window_width = 800;
    int window_height = 600;
    const char* window_title = "Text Rendering";

    GLFWwindow* window = glfwCreateWindow(window_width, window_height, window_title, nullptr, nullptr);
    if (window == nullptr) {
        fprintf(stderr, "Failed to create window.\n");
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (not gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        fprintf(stderr, "Failed to initialize GLAD.\n");
        return -1;
    }

    if (not init_text_technique()) {
        fprintf(stderr, "Failed to initialize text rendering technique.\n");
        return -1;
    }
    Font sans, papyrus;
    if (not load_font(&sans, "fonts/comic_sans.ttf", 36)) {
        return -1;
    }
    if (not load_font(&papyrus, "fonts/papyrus.ttf", 24)) {
        return -1;
    }

    glViewport(0, 0, window_width, window_height);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glm::mat4 projection = glm::ortho(
        0.f, static_cast<float>(window_width), 
        0.f, static_cast<float>(window_height)
        );

    const char* loren_ipsum =
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit.\n"
    "Duis at semper tellus, in dictum ex. Sed risus est,\n"
    "sollicitudin in accumsan non, consectetur eget nisi.\n"
    "Fusce ut nisl sit amet velit tincidunt facilisis non\n"
    "quis sem. Proin non erat id elit fermentum semper sit\n"
    "amet in ante. Lorem ipsum dolor sit amet, consectetur\n"
    "adipiscing elit.n";


    while (not glfwWindowShouldClose(window)) {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT);
        draw_text(
            projection, &sans, 
            {10.f, window_height - 40.f},
            {0.f, 1.f, 1.f, 1.f},
            "Text Rendering is cool~~!"
            );
        draw_text(
            projection, &papyrus,
            {10.f, window_height - 80.f},
            {1.f, 0.f, 1.f, 1.f},
            loren_ipsum
            );
        glfwSwapBuffers(window);
    }

    glfwTerminate();
}
