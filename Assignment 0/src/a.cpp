#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <bits/stdc++.h>
#include <unistd.h>
#define PI 2 * acos(0.0)
using namespace std;
int flag_r = 0, flag_t_key = 0;
float angle = 0;
float transx = 0, transy = 0, transz = 0;
float last_toggle = 0;
float cameraSpeed = 0.01;

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
    {
        angle += 1.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        transy += 0.01;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        transy -= 0.01;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    {
        transx -= 0.01;
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    {
        transx += 0.01;
    }
    if (glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS)
    {
        transz += 0.008;
    }
    if (glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS)
    {
        transz -= 0.008;
    }
    if (glfwGetTime() - last_toggle >= 0.5 && glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
    {
        flag_t_key = 1 - flag_t_key;
        last_toggle = glfwGetTime();
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        cameraPos += cameraSpeed * cameraFront;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        cameraPos -= cameraSpeed * cameraFront;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        cameraPos += cameraSpeed * glm::normalize(glm::cross(cameraUp,cameraFront));
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        cameraPos -= cameraSpeed * glm::normalize(glm::cross(cameraUp,cameraFront));
    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        cameraPos += cameraSpeed * cameraUp;
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        cameraPos -= cameraSpeed * cameraUp;
    }
}
const char *vertexShaderSource = "#version 330 core\n"
                                 "layout (location = 0) in vec3 aPos;\n"
                                 "uniform mat4 transform;\n"
                                 "uniform mat4 projection;\n"
                                 "uniform mat4 view;\n"
                                 "void main()\n"
                                 "{\n"
                                 "   gl_Position = projection*view*transform*vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
                                 "}\0";
const char *fragmentShaderSource = "#version 330 core\n"
                                   "out vec4 FragColor;\n"
                                   "uniform vec4 Col;\n"
                                   "void main()\n"
                                   "{\n"
                                   "   FragColor = Col;\n"
                                   "}\n\0";
int main()
{
    int n;
    cin >> n;
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow *window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
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
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    }
    // fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    }
    // link shaders
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
                  << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    unsigned int VBO;
    unsigned int VAO;
    unsigned int EBO[3];
    glGenBuffers(3, EBO);
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    unsigned indices[] = {
        2, 3, 4,
        3, 4, 5};
    unsigned indices2[] = {
        2, 3, 0};
    unsigned indices3[] = {
        1, 4, 5};
    unsigned int indices4[] = {
        1, 2, 3};
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices2), indices2, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[2]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices3), indices3, GL_STATIC_DRAW);

    float vertices[18];
    vertices[0] = 0;
    vertices[1] = 0;
    vertices[3] = 0;
    vertices[4] = 0;
    vertices[2] = vertices[8] = vertices[11] = 0.5;
    vertices[5] = vertices[14] = vertices[17] = -0.5;
    glEnable(GL_DEPTH_TEST);
    while (!glfwWindowShouldClose(window))
    {
        processInput(window);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shaderProgram);

        // rotation on R
        glm::mat4 trans = glm::mat4(1.0f);
        glm::mat4 proj = glm::mat4(1.0f);
        glm::mat4 view = glm::mat4(1.0f);

        view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

        proj = glm::perspective(glm::radians(90.0f), (float)800 / (float)600, 0.1f, 100.0f);
        trans = glm::translate(trans, glm::vec3(transx, 0, 0));
        trans = glm::translate(trans, glm::vec3(0, transy, 0));
        trans = glm::translate(trans, glm::vec3(0, 0, transz));
        trans = glm::rotate(trans, glm::radians(angle), glm::vec3(0.0, 1.0, 0.0));
        unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
        unsigned int transformLoc = glGetUniformLocation(shaderProgram, "transform");
        unsigned int projectLoc = glGetUniformLocation(shaderProgram, "projection");
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans));
        glUniformMatrix4fv(projectLoc, 1, GL_FALSE, glm::value_ptr(proj));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glBindVertexArray(VAO);
        int colloc = glGetUniformLocation(shaderProgram, "Col");
        for (int i = 1; i <= n; i++)
        {
            float a = (float)(2 * PI) * ((float)(i) / (float)n);
            float a2 = (float)(2 * PI) * ((float)(i + 1) / (float)n);
            vertices[6] = cos(a);
            vertices[7] = sin(a);
            vertices[12] = cos(a);
            vertices[13] = sin(a);
            vertices[9] = cos(a2);
            vertices[10] = sin(a2);
            vertices[15] = cos(a2);
            vertices[16] = sin(a2);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[0]);
            glUniform4f(colloc, 1 - (float)i / (float)n, 1 - (float)i / (float)n, (float)i / (float)(n), 1.0);
            if (flag_t_key == 0)
            {
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void *)0);
            }
            else
            {
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices4), indices4, GL_STATIC_DRAW);
                glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void *)0);
            }
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[1]);
            glUniform4f(colloc, 1.0, 0.0, 0.0, 1.0);
            glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void *)0);
            if (flag_t_key == 0)
            {
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[2]);
                glUniform4f(colloc, 0.0, 1.0, 0.0, 1.0);
                glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void *)0);
            }
        }
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}