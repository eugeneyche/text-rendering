#include "text.hpp"
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

glm::mat4 make_2d_transform(glm::vec2 offset, float rotation)
{
    return glm::rotate(
        glm::translate(
            glm::mat4{1.f},
            glm::vec3{offset, 0}
            ),
        rotation,
        glm::vec3{0.f, 0.f, 1.f}
        );
}

glm::vec3 hue_to_color(float hue)
{
    float h6 = (hue - glm::floor(hue)) * 6;
    float offset = h6 - glm::floor(h6);
    float r, g, b;
    if (h6 < 1.f) {
        r = 1.f;
        g = offset;
        b = 0.f;
    } else if (h6 < 2.f) {
        r = 1 - offset;
        g = 1;
        b = 0;
    } else if (h6 < 3.f) {
        r = 0;
        g = 1;
        b = offset;
    } else if (h6 < 4.f) {
        r = 0;
        g = 1 - offset;
        b = 1;
    } else if (h6 < 5.f) {
        r = offset;
        g = 0;
        b = 1;
    } else if (h6 < 6.f) {
        r = 1;
        g = 0;
        b = 1 - offset;
    }
    return {r, g, b};
}

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

    if (not init_text_rendering()) {
        fprintf(stderr, "Failed to initialize text rendering.\n");
        return -1;
    }

    Font sans, papyrus;
    if (    not make_font(&sans, "fonts/comic_sans.ttf", 48) or
            not make_font(&papyrus, "fonts/papyrus.ttf", 32)
            ) {
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
    "adipiscing elit.";

    while (not glfwWindowShouldClose(window)) {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT);
        draw_text(
            &sans, projection, 
            {0.f, 1.f, 1.f, 1.f},
            {10.f, window_height - 60.f},
            "Text Rendering is cool~~!"
            );
        draw_text(
            &papyrus, projection,
            {1.f, 0.f, 1.f, 1.f},
            {10.f, window_height - 120.f},
            loren_ipsum
            );
        draw_text(
            &sans, projection * make_2d_transform({200.f, 400.f}, 5.f * glfwGetTime()),
            glm::vec4{hue_to_color(-0.5f * glfwGetTime()), 1},
            {-60.f, -10.f},
            "wow~~"
            );
        draw_text(
            &sans, projection * make_2d_transform({window_width - 300.f, 200.f}, -3.f * glfwGetTime()),
            glm::vec4{hue_to_color(glfwGetTime()), 1},
            {-150.f, -10.f},
            "much truasre"
            );
        draw_text(
            &papyrus, projection,
            {1.f, 0.f, 0.f, 1.f},
            {10.f, 30.f},
            "STOP THAT, SANS!"
            );
        glfwSwapBuffers(window);
    }

    glfwTerminate();
}
