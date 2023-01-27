#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "stb_image.h"
#include <bits/stdc++.h>
#include <unistd.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include "shader.h"
using namespace std;
float velocity = 0, transy = 0, k = 0, trans_coin = 0, trans_zap = 0, zap_osc = 0, flag_zap = 1;
float lasttime, velZap = 1.0, velTrans = 1.0;
unsigned int fb_texture, rbo;
unsigned int fbo;
int flag_vel = 0;
int flag_fly = 0;
int coin_ctr = 0;
int flag_nex_lev = 0;
int level_comp = 0;
int flag_restart = 0;
int flag_win=0;
bool bloom = true;
float exposure = 1.0f;
int flag_dead = 0;
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);

    glBindTexture(GL_TEXTURE_2D, fb_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}
struct Character
{
    unsigned int TextureID; // ID handle of the glyph texture
    glm::ivec2 Size;        // Size of glyph
    glm::ivec2 Bearing;     // Offset from baseline to left/top of glyph
    unsigned int Advance;   // Horizontal offset to advance to next glyph
};
std::map<GLchar, Character> Characters;
unsigned int VAO, VBO;
void RenderText(Shader &shader, std::string text, float x, float y, float scale, glm::vec3 color)
{
    // activate corresponding render state
    shader.use();
    glUniform3f(glGetUniformLocation(shader.ID, "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        Character ch = Characters[*c];

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;
        //     // update VBO for each character
        float vertices[6][4] = {
            {xpos, ypos + h, 0.0f, 0.0f},
            {xpos, ypos, 0.0f, 1.0f},
            {xpos + w, ypos, 1.0f, 1.0f},

            {xpos, ypos + h, 0.0f, 0.0f},
            {xpos + w, ypos, 1.0f, 1.0f},
            {xpos + w, ypos + h, 1.0f, 0.0f}};
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        //     // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        //     // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        //     // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        flag_fly = 1;
        if (transy < 1.4)
        {
            if (glfwGetTime() - lasttime > 0.005)
            {
                velocity += 0.06 * (glfwGetTime() - lasttime);
                transy += velocity;
            }
        }
    }
    else
    {
        flag_fly = 0;
        velocity = 0;
        if (transy > 0.0)
        {
            if (glfwGetTime() - lasttime > 0.005)
            {
                transy -= 1.8 * (glfwGetTime() - lasttime);
            }
        }
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS && level_comp == 1)
    {
        flag_nex_lev = 1;
    }
    if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS&&(flag_dead==1||flag_win==1) )
    {
        flag_restart = 1;
    }
}
int main()
{
    lasttime = glfwGetTime();
    srand(time(0));
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
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Shader shader("../src/text.vs", "../src/text.fs");
    Shader fbShader("../src/fb.vs", "../src/fb.fs");
    Shader shaderBlur("../src/fb.vs", "../src/bloom.fs");
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(SCR_WIDTH), 0.0f, static_cast<float>(SCR_HEIGHT));
    shader.use();
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    FT_Library ft;
    // All functions return a value different than 0 whenever an error occurred
    if (FT_Init_FreeType(&ft))
    {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return -1;
    }

    // find path to font
    std::string font_name = "../fonts/Antonio-Bold.ttf";
    if (font_name.empty())
    {
        std::cout << "ERROR::FREETYPE: Failed to load font_name" << std::endl;
        return -1;
    }

    // load font as face
    FT_Face face;
    if (FT_New_Face(ft, font_name.c_str(), 0, &face))
    {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
        return -1;
    }
    else
    {
        // set size to load glyphs as
        FT_Set_Pixel_Sizes(face, 0, 48);

        // disable byte-alignment restriction
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        // load first 128 characters of ASCII set
        for (unsigned char c = 0; c < 128; c++)
        {
            // Load character glyph
            if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            {
                std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
                continue;
            }
            // generate texture
            unsigned int texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED,
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer);
            // set texture options
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            // now store character for later use
            Character character = {
                texture,
                glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                static_cast<unsigned int>(face->glyph->advance.x)};
            Characters.insert(std::pair<char, Character>(c, character));
        }
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    // destroy FreeType once we're finished
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    // creating FrameBuffer
    //
    //
    //
    //
    //
    //
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    unsigned int colorBuffers[2];
    glGenTextures(2, colorBuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
    }
    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    unsigned int attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, attachments);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    unsigned int pingpongFBO[2];
    unsigned int pingpongBuffers[2];
    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpongBuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, pingpongBuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongBuffers[i], 0);
        // also check if framebuffers are complete (no need for depth buffer)
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer not complete!" << std::endl;
    }
    // done
    //
    //
    //
    //
    //
    //
    Shader shaderProgram("../src/shader.vs", "../src/shader.fs");

    float vertices[] = {
        -0.8, -0.8, 0.0, 0.0, 1.0,
        -0.5, -0.8, 0.0, 1.0, 1.0,
        -0.5, -0.5, 0.0, 1.0, 0.0,
        -0.8, -0.5, 0.0, 0.0, 0.0};
    float vertices2[] = {
        -1.0, -1.0, 0.0, 0.0, 1.0,
        1.0, -1.0, 0.0, 1.0, 1.0,
        1.0, -0.8, 0.0, 1.0, 0.0,
        -1.0, -0.8, 0.0, 0.0, 0.0};
    float coin[] = {
        1.05, 0.0, 0.0, 0.0, 1.0,
        1.15, 0.0, 0.0, 1.0, 1.0,
        1.15, 0.1, 0.0, 1.0, 0.0,
        1.05, 0.1, 0.0, 0.0, 0.0};
    float zap[] = {
        1.3, 0.0, 0.0, 0.0, 1.0,
        1.33, 0.0, 0.0, 1.0, 1.0,
        1.33, 0.4, 0.0, 1.0, 0.0,
        1.3, 0.4, 0.0, 0.0, 0.0};
    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0};

    // Coordinates for FB
    //
    //
    //
    //
    //
    //
    float fb_vertices[] = {
        1.f, 1.f, 0.0f, 1.0f, 1.0f,   // top right
        1.f, -1.f, 0.0f, 1.0f, 0.0f,  // bottom right
        -1.f, -1.f, 0.0f, 0.0f, 0.0f, // bottom left
        -1.f, 1.f, 0.0f, 0.0f, 1.0f   // top left

    };
    // done
    //
    //
    //
    //
    //
    //
    unsigned int VBO_1;
    unsigned int VAO_1, EBO;
    glGenVertexArrays(1, &VAO_1);
    glGenBuffers(1, &VBO_1);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO_1);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_1);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    //
    //
    //
    // Designing Fb VBO and EBO
    //
    //
    //
    //
    //
    unsigned int VBO_fb;
    unsigned int VAO_fb, EBO_fb;
    glGenVertexArrays(1, &VAO_fb);
    glGenBuffers(1, &VBO_fb);
    glGenBuffers(1, &EBO_fb);
    glBindVertexArray(VAO_fb);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_fb);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_fb);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glBufferData(GL_ARRAY_BUFFER, sizeof(fb_vertices), fb_vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    //
    //
    //
    // Done
    //
    //
    //
    //
    //
    unsigned int texture, texture_floor, texture_coin, texture_zap;
    glGenTextures(1, &texture);
    glGenTextures(1, &texture_floor);
    glGenTextures(1, &texture_coin);
    glGenTextures(1, &texture_zap);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int width, height, nrChannels;
    unsigned char *data = stbi_load("../textures/flying.png", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, texture_floor);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width_f, height_f, nrChannels_f;
    unsigned char *floor = stbi_load("../textures/floor.jpg", &width_f, &height_f, &nrChannels_f, 0);
    if (floor)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width_f, height_f, 0, GL_RGB, GL_UNSIGNED_BYTE, floor);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(floor);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, texture_coin);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int width_c, height_c, nrChannels_c;
    unsigned char *coin_tex = stbi_load("../textures/coin.png", &width_c, &height_c, &nrChannels_c, 0);
    if (coin_tex)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_c, height_c, 0, GL_RGBA, GL_UNSIGNED_BYTE, coin_tex);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(coin_tex);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, texture_zap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int width_z, height_z, nrChannels_z;
    unsigned char *zap_tex = stbi_load("../textures/zap.png", &width_z, &height_z, &nrChannels_z, 0);
    if (zap_tex)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width_z, height_z, 0, GL_RGB, GL_UNSIGNED_BYTE, zap_tex);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(zap_tex);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
restart:
    flag_restart = 0;
    string per = "Distance Travelled: ";
    string coin_str = "Coins Collected: ";
    coin[1] = coin[6] = (double)(2 * (rand() % 10)) / (double)13 - 0.65;
    coin[11] = coin[16] = coin[1] + 0.1;
    zap[1] = zap[6] = (double)(1.4 * (double)(rand() % 10)) / (double)10 - 0.7;
    zap[11] = zap[16] = zap[1] + 0.3;
    float starttime = glfwGetTime();

    shaderProgram.use();
    shaderProgram.setInt("diffuseTexture", 0);
    shaderBlur.use();
    shaderBlur.setInt("image", 0);
    fbShader.use();
    fbShader.setInt("scene", 0);
    fbShader.setInt("bloomBlur", 1);

    while (!glfwWindowShouldClose(window) && flag_dead != 1)
    {

        if (glfwGetTime() - starttime > 10)
        {
            level_comp = 1;
            break;
        }
        // Enable FB
        //
        //
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glEnable(GL_DEPTH_TEST);
        //
        //
        //
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        RenderText(shader, "Level: 1", 50.0f, 580.0f, 0.4f, glm::vec3(1.0, 1.0f, 1.0f));
        float distance = 100 * (glfwGetTime() - starttime);
        per += to_string((int)distance);
        per += "m / 1000m";
        RenderText(shader, per, 200.0f, 580.0f, 0.4f, glm::vec3(1.0, 1.0f, 1.0f));
        per = "Distance Travelled: ";
        coin_str += to_string(coin_ctr);
        RenderText(shader, coin_str, 500.0f, 580.0f, 0.4f, glm::vec3(1.0, 1.0f, 1.0f));
        coin_str = "Coins Collected: ";
        shaderProgram.use();
        if (glfwGetTime() - lasttime > 0.005)
            flag_vel = 1;
        if (zap_osc + zap[11] > 0.95)
        {
            flag_zap = -1;
        }
        if (zap_osc + zap[1] < -0.78)
        {
            flag_zap = 1;
        }
        if (trans_coin < -2.15)
        {
            trans_coin = 0;
            coin[1] = coin[6] = (double)(2 * (rand() % 10)) / (double)13 - 0.65;
            coin[11] = coin[16] = coin[1] + 0.1;
        }
        if (trans_zap < -2.4)
        {
            trans_zap = 0;
            zap[1] = zap[6] = (double)(1.4 * (double)(rand() % 10)) / (double)10 - 0.7;
            zap[11] = zap[16] = zap[1] + 0.3;
            int random = rand() % 2;
            if (random == 0)
                flag_zap = 1;
            else
                flag_zap = -1;
        }
        if ((coin[11] > vertices[1] + transy && coin[1] < vertices[11] + transy) &&
            (coin[5] + trans_coin > vertices[0] && coin[0] + trans_coin < vertices[5]))
        {
            trans_coin = 0;
            coin[1] = coin[6] = (double)(2 * (rand() % 10)) / (double)13 - 0.65;
            coin[11] = coin[16] = coin[1] + 0.1;
            coin_ctr++;
        }
        if ((zap[11] + zap_osc > vertices[1] + transy && zap[1] + zap_osc < vertices[11] + transy) &&
            (zap[5] + trans_zap > vertices[0] && zap[0] + trans_zap < vertices[5]))
        {
            flag_dead = 1;
        }
        processInput(window);
        if (glfwGetTime() - lasttime > 0.005)
        {
            trans_coin -= velTrans * (glfwGetTime() - lasttime);
            trans_zap -= velTrans * (glfwGetTime() - lasttime);
            zap_osc += flag_zap * velZap * (glfwGetTime() - lasttime);
        }
        glBindVertexArray(VAO_1);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_1);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glm::mat4 trans = glm::mat4(1.0f);
        trans = glm::translate(trans, glm::vec3(0.0, transy, 0.0));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram.ID, "transform"), 1, GL_FALSE, glm::value_ptr(trans));

        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, texture_floor);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, texture_coin);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, texture_zap);

        shaderProgram.setBool("iszap", flag_fly);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

        glUniform1i(glGetUniformLocation(shaderProgram.ID, "ourTexture"), 1);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void *)0);
        trans = glm::mat4(1.0f);
        shaderProgram.setBool("iszap", 0);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram.ID, "transform"), 1, GL_FALSE, glm::value_ptr(trans));
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_DYNAMIC_DRAW);
        glUniform1i(glGetUniformLocation(shaderProgram.ID, "ourTexture"), 2);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void *)0);

        trans = glm::translate(trans, glm::vec3(trans_coin, 0.0, 0.0));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram.ID, "transform"), 1, GL_FALSE, glm::value_ptr(trans));
        glBufferData(GL_ARRAY_BUFFER, sizeof(coin), coin, GL_DYNAMIC_DRAW);
        glUniform1i(glGetUniformLocation(shaderProgram.ID, "ourTexture"), 3);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void *)0);

        trans = glm::mat4(1.0f);
        trans = glm::translate(trans, glm::vec3(trans_zap, zap_osc, 0.0));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram.ID, "transform"), 1, GL_FALSE, glm::value_ptr(trans));
        shaderProgram.setBool("iszap", 1);
        glBufferData(GL_ARRAY_BUFFER, sizeof(zap), zap, GL_DYNAMIC_DRAW);
        glUniform1i(glGetUniformLocation(shaderProgram.ID, "ourTexture"), 4);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void *)0);
        shaderProgram.setBool("iszap", 0);
        //
        //
        //
        // Disable FB and Render Quad
        //
        //
        //
        //
        //
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        bool horizontal = true, first_iteration = true;
        int amount = 10;
        shaderBlur.use();
        for (unsigned int i = 0; i < amount; i++)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
            shaderBlur.setInt("horizontal", horizontal);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(
                GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : pingpongBuffers[!horizontal]);
            shaderBlur.setInt("image", 0);
            glBindVertexArray(VAO_fb);
            glBindBuffer(GL_ARRAY_BUFFER, VBO_fb);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_fb);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            horizontal = !horizontal;
            if (first_iteration)
                first_iteration = false;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST);
        glClearColor(1, 1, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        fbShader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, pingpongBuffers[!horizontal]);
        fbShader.setInt("bloom", bloom);
        fbShader.setFloat("exposure", exposure);
        glBindVertexArray(VAO_fb);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_fb);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_fb);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        //
        //
        //
        // Done
        //
        //
        //
        //
        //
        glfwSwapBuffers(window);
        glfwPollEvents();
        if (glfwGetTime() - lasttime > 0.005)
        {
            vertices2[3] += velTrans * (glfwGetTime() - lasttime);
            vertices2[8] += velTrans * (glfwGetTime() - lasttime);
            vertices2[13] += velTrans * (glfwGetTime() - lasttime);
            vertices2[18] += velTrans * (glfwGetTime() - lasttime);
        }
        if (flag_vel == 1)
        {
            lasttime = glfwGetTime();
            flag_vel = 0;
        }
    }
    if (level_comp == 1)
    {
        while (!glfwWindowShouldClose(window) && flag_nex_lev == 0)
        {
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            processInput(window);
            RenderText(shader, "Level 1 Complete", 100.0f, 350.0f, 2.0f, glm::vec3(1.0, 1.0f, 1.0f));
            RenderText(shader, "Press right arrow for next level", 500.0f, 100.0f, 0.4f, glm::vec3(1.0, 1.0f, 1.0f));
            glfwSwapBuffers(window);
            glfwPollEvents();
        }
        if (flag_nex_lev == 1)
        {
            level_comp = 0;
            flag_nex_lev = 0;
            starttime = glfwGetTime();
            velocity = 0, transy = 0, k = 0, trans_coin = 0, trans_zap = 0, zap_osc = 0, flag_zap = 1;
            lasttime = glfwGetTime(), velZap = 1.4, velTrans = 1.4;
            int flag_vel = 0;
            coin[1] = coin[6] = (double)(2 * (rand() % 10)) / (double)13 - 0.65;
            coin[11] = coin[16] = coin[1] + 0.1;
            zap[1] = zap[6] = (double)(1.2 * (double)(rand() % 10)) / (double)10 - 0.7;
            zap[11] = zap[16] = zap[1] + 0.5;
            float starttime = glfwGetTime();

            while (!glfwWindowShouldClose(window) && flag_dead != 1)
            {
                if (glfwGetTime() - starttime > 15)
                {
                    level_comp = 1;
                    break;
                }
                // Enable FB
                //
                //
                glBindFramebuffer(GL_FRAMEBUFFER, fbo);
                //
                //
                //
                glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT);
                RenderText(shader, "Level: 2", 50.0f, 580.0f, 0.4f, glm::vec3(1.0, 1.0f, 1.0f));
                float distance = 100 * (glfwGetTime() - starttime);
                per += to_string((int)distance);
                per += "m / 1500m";
                RenderText(shader, per, 200.0f, 580.0f, 0.4f, glm::vec3(1.0, 1.0f, 1.0f));
                per = "Distance Travelled: ";
                coin_str += to_string(coin_ctr);
                RenderText(shader, coin_str, 500.0f, 580.0f, 0.4f, glm::vec3(1.0, 1.0f, 1.0f));
                coin_str = "Coins Collected: ";
                shaderProgram.use();
                if (glfwGetTime() - lasttime > 0.005)
                    flag_vel = 1;
                if (zap_osc + zap[11] > 0.95)
                {
                    flag_zap = -1;
                }
                if (zap_osc + zap[1] < -0.78)
                {
                    flag_zap = 1;
                }
                if (trans_coin < -2.15)
                {
                    trans_coin = 0;
                    coin[1] = coin[6] = (double)(2 * (rand() % 10)) / (double)13 - 0.65;
                    coin[11] = coin[16] = coin[1] + 0.1;
                }
                if (trans_zap < -2.4)
                {
                    trans_zap = 0;
                    zap[1] = zap[6] = (double)(1.2 * (double)(rand() % 10)) / (double)10 - 0.7;
                    zap[11] = zap[16] = zap[1] + 0.5;
                    int random = rand() % 2;
                    if (random == 0)
                        flag_zap = 1;
                    else
                        flag_zap = -1;
                }
                if ((coin[11] > vertices[1] + transy && coin[1] < vertices[11] + transy) &&
                    (coin[5] + trans_coin > vertices[0] && coin[0] + trans_coin < vertices[5]))
                {
                    trans_coin = 0;
                    coin[1] = coin[6] = (double)(2 * (rand() % 10)) / (double)13 - 0.65;
                    coin[11] = coin[16] = coin[1] + 0.1;
                    coin_ctr++;
                }
                if ((zap[11] + zap_osc > vertices[1] + transy && zap[1] + zap_osc < vertices[11] + transy) &&
                    (zap[5] + trans_zap > vertices[0] && zap[0] + trans_zap < vertices[5]))
                {
                    flag_dead = 1;
                }
                processInput(window);
                if (glfwGetTime() - lasttime > 0.005)
                {
                    trans_coin -= velTrans * (glfwGetTime() - lasttime);
                    trans_zap -= velTrans * (glfwGetTime() - lasttime);
                    zap_osc += flag_zap * velZap * (glfwGetTime() - lasttime);
                }
                glBindVertexArray(VAO_1);
                glBindBuffer(GL_ARRAY_BUFFER, VBO_1);
                glm::mat4 trans = glm::mat4(1.0f);
                trans = glm::translate(trans, glm::vec3(0.0, transy, 0.0));
                glUniformMatrix4fv(glGetUniformLocation(shaderProgram.ID, "transform"), 1, GL_FALSE, glm::value_ptr(trans));

                glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, texture_floor);
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, texture);
                glActiveTexture(GL_TEXTURE3);
                glBindTexture(GL_TEXTURE_2D, texture_coin);
                glActiveTexture(GL_TEXTURE4);
                glBindTexture(GL_TEXTURE_2D, texture_zap);

                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
                shaderProgram.setBool("iszap", flag_fly);
                glUniform1i(glGetUniformLocation(shaderProgram.ID, "ourTexture"), 1);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void *)0);
                shaderProgram.setBool("iszap", 0);
                trans = glm::mat4(1.0f);
                glUniformMatrix4fv(glGetUniformLocation(shaderProgram.ID, "transform"), 1, GL_FALSE, glm::value_ptr(trans));
                glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_DYNAMIC_DRAW);
                glUniform1i(glGetUniformLocation(shaderProgram.ID, "ourTexture"), 2);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void *)0);

                trans = glm::translate(trans, glm::vec3(trans_coin, 0.0, 0.0));
                glUniformMatrix4fv(glGetUniformLocation(shaderProgram.ID, "transform"), 1, GL_FALSE, glm::value_ptr(trans));
                glBufferData(GL_ARRAY_BUFFER, sizeof(coin), coin, GL_DYNAMIC_DRAW);
                glUniform1i(glGetUniformLocation(shaderProgram.ID, "ourTexture"), 3);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void *)0);

                trans = glm::mat4(1.0f);
                shaderProgram.setBool("iszap", 1);
                trans = glm::translate(trans, glm::vec3(trans_zap, zap_osc, 0.0));
                glUniformMatrix4fv(glGetUniformLocation(shaderProgram.ID, "transform"), 1, GL_FALSE, glm::value_ptr(trans));
                glBufferData(GL_ARRAY_BUFFER, sizeof(zap), zap, GL_DYNAMIC_DRAW);
                glUniform1i(glGetUniformLocation(shaderProgram.ID, "ourTexture"), 4);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void *)0);
                shaderProgram.setBool("iszap", 0);
                //
                //
                //
                // Disable FB and Render Quad
                //
                //
                //
                //
                //
                glBindFramebuffer(GL_FRAMEBUFFER, 0);

                bool horizontal = true, first_iteration = true;
                int amount = 10;
                shaderBlur.use();
                for (unsigned int i = 0; i < amount; i++)
                {
                    glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
                    shaderBlur.setInt("horizontal", horizontal);
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(
                        GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : pingpongBuffers[!horizontal]);
                    shaderBlur.setInt("image", 0);
                    glBindVertexArray(VAO_fb);
                    glBindBuffer(GL_ARRAY_BUFFER, VBO_fb);
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_fb);
                    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                    horizontal = !horizontal;
                    if (first_iteration)
                        first_iteration = false;
                }
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                glDisable(GL_DEPTH_TEST);
                glClearColor(1, 1, 1, 1);
                glClear(GL_COLOR_BUFFER_BIT);
                fbShader.use();
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, pingpongBuffers[!horizontal]);
                fbShader.setInt("bloom", bloom);
                fbShader.setFloat("exposure", exposure);
                glBindVertexArray(VAO_fb);
                glBindBuffer(GL_ARRAY_BUFFER, VBO_fb);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_fb);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                //
                //
                //
                // Done
                //
                //
                //
                //
                //
                glfwSwapBuffers(window);
                glfwPollEvents();
                if (glfwGetTime() - lasttime > 0.005)
                {
                    vertices2[3] += velTrans * (glfwGetTime() - lasttime);
                    vertices2[8] += velTrans * (glfwGetTime() - lasttime);
                    vertices2[13] += velTrans * (glfwGetTime() - lasttime);
                    vertices2[18] += velTrans * (glfwGetTime() - lasttime);
                }
                if (flag_vel == 1)
                {
                    lasttime = glfwGetTime();
                    flag_vel = 0;
                }
            }
            if (level_comp == 1)
            {
                while (!glfwWindowShouldClose(window) && flag_nex_lev == 0)
                {
                    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
                    glClear(GL_COLOR_BUFFER_BIT);
                    processInput(window);
                    RenderText(shader, "Level 2 Complete", 100.0f, 350.0f, 2.0f, glm::vec3(1.0, 1.0f, 1.0f));
                    RenderText(shader, "Press right arrow for next level", 500.0f, 100.0f, 0.4f, glm::vec3(1.0, 1.0f, 1.0f));
                    glfwSwapBuffers(window);
                    glfwPollEvents();
                }
                if (flag_nex_lev == 1)
                {
                    level_comp = 0;
                    flag_nex_lev = 0;
                    starttime = glfwGetTime();
                    velocity = 0, transy = 0, k = 0, trans_coin = 0, trans_zap = 0, zap_osc = 0, flag_zap = 1;
                    lasttime = glfwGetTime(), velZap = 1.8, velTrans = 1.8;
                    int flag_vel = 0;
                    coin[1] = coin[6] = (double)(2 * (rand() % 10)) / (double)13 - 0.65;
                    coin[11] = coin[16] = coin[1] + 0.1;
                    zap[1] = zap[6] = (double)(1.1 * (double)(rand() % 10)) / (double)10 - 0.8;
                    zap[11] = zap[16] = zap[1] + 0.6;
                    float starttime = glfwGetTime();

                    while (!glfwWindowShouldClose(window) && flag_dead != 1)
                    {
                        if (glfwGetTime() - starttime > 20)
                        {
                            level_comp = 1;
                            break;
                        }
                        // Enable FB
                        //
                        //
                        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
                        //
                        //
                        //
                        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
                        glClear(GL_COLOR_BUFFER_BIT);
                        RenderText(shader, "Level: 3", 50.0f, 580.0f, 0.4f, glm::vec3(1.0, 1.0f, 1.0f));
                        float distance = 100 * (glfwGetTime() - starttime);
                        per += to_string((int)distance);
                        per += "m / 2000m";
                        RenderText(shader, per, 200.0f, 580.0f, 0.4f, glm::vec3(1.0, 1.0f, 1.0f));
                        per = "Distance Travelled: ";
                        coin_str += to_string(coin_ctr);
                        RenderText(shader, coin_str, 500.0f, 580.0f, 0.4f, glm::vec3(1.0, 1.0f, 1.0f));
                        coin_str = "Coins Collected: ";
                        shaderProgram.use();
                        if (glfwGetTime() - lasttime > 0.005)
                            flag_vel = 1;
                        if (zap_osc + zap[11] > 0.95)
                        {
                            flag_zap = -1;
                        }
                        if (zap_osc + zap[1] < -0.78)
                        {
                            flag_zap = 1;
                        }
                        if (trans_coin < -2.15)
                        {
                            trans_coin = 0;
                            coin[1] = coin[6] = (double)(2 * (rand() % 10)) / (double)13 - 0.65;
                            coin[11] = coin[16] = coin[1] + 0.1;
                        }
                        if (trans_zap < -2.4)
                        {
                            trans_zap = 0;
                            zap[1] = zap[6] = (double)(1.1 * (double)(rand() % 10)) / (double)10 - 0.8;
                            zap[11] = zap[16] = zap[1] + 0.6;
                            int random = rand() % 2;
                            if (random == 0)
                                flag_zap = 1;
                            else
                                flag_zap = -1;
                        }
                        if ((coin[11] > vertices[1] + transy && coin[1] < vertices[11] + transy) &&
                            (coin[5] + trans_coin > vertices[0] && coin[0] + trans_coin < vertices[5]))
                        {
                            trans_coin = 0;
                            coin[1] = coin[6] = (double)(2 * (rand() % 10)) / (double)13 - 0.65;
                            coin[11] = coin[16] = coin[1] + 0.1;
                            coin_ctr++;
                        }
                        if ((zap[11] + zap_osc > vertices[1] + transy && zap[1] + zap_osc < vertices[11] + transy) &&
                            (zap[5] + trans_zap > vertices[0] && zap[0] + trans_zap < vertices[5]))
                        {
                            flag_dead = 1;
                        }
                        processInput(window);
                        if (glfwGetTime() - lasttime > 0.005)
                        {
                            trans_coin -= velTrans * (glfwGetTime() - lasttime);
                            trans_zap -= velTrans * (glfwGetTime() - lasttime);
                            zap_osc += flag_zap * velZap * (glfwGetTime() - lasttime);
                        }
                        glBindVertexArray(VAO_1);
                        glBindBuffer(GL_ARRAY_BUFFER, VBO_1);
                        glm::mat4 trans = glm::mat4(1.0f);
                        trans = glm::translate(trans, glm::vec3(0.0, transy, 0.0));
                        glUniformMatrix4fv(glGetUniformLocation(shaderProgram.ID, "transform"), 1, GL_FALSE, glm::value_ptr(trans));

                        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

                        glActiveTexture(GL_TEXTURE2);
                        glBindTexture(GL_TEXTURE_2D, texture_floor);
                        glActiveTexture(GL_TEXTURE1);
                        glBindTexture(GL_TEXTURE_2D, texture);
                        glActiveTexture(GL_TEXTURE3);
                        glBindTexture(GL_TEXTURE_2D, texture_coin);
                        glActiveTexture(GL_TEXTURE4);
                        glBindTexture(GL_TEXTURE_2D, texture_zap);

                        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
                        shaderProgram.setBool("iszap", flag_fly);
                        glUniform1i(glGetUniformLocation(shaderProgram.ID, "ourTexture"), 1);
                        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void *)0);
                        shaderProgram.setBool("iszap", 0);

                        trans = glm::mat4(1.0f);
                        glUniformMatrix4fv(glGetUniformLocation(shaderProgram.ID, "transform"), 1, GL_FALSE, glm::value_ptr(trans));
                        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_DYNAMIC_DRAW);
                        glUniform1i(glGetUniformLocation(shaderProgram.ID, "ourTexture"), 2);
                        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void *)0);

                        trans = glm::translate(trans, glm::vec3(trans_coin, 0.0, 0.0));
                        glUniformMatrix4fv(glGetUniformLocation(shaderProgram.ID, "transform"), 1, GL_FALSE, glm::value_ptr(trans));
                        glBufferData(GL_ARRAY_BUFFER, sizeof(coin), coin, GL_DYNAMIC_DRAW);
                        glUniform1i(glGetUniformLocation(shaderProgram.ID, "ourTexture"), 3);
                        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void *)0);

                        trans = glm::mat4(1.0f);
                        shaderProgram.setBool("iszap", 1);
                        trans = glm::translate(trans, glm::vec3(trans_zap, zap_osc, 0.0));
                        glUniformMatrix4fv(glGetUniformLocation(shaderProgram.ID, "transform"), 1, GL_FALSE, glm::value_ptr(trans));
                        glBufferData(GL_ARRAY_BUFFER, sizeof(zap), zap, GL_DYNAMIC_DRAW);
                        glUniform1i(glGetUniformLocation(shaderProgram.ID, "ourTexture"), 4);
                        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void *)0);
                        shaderProgram.setBool("iszap", 0);
                        //
                        //
                        //
                        // Disable FB and Render Quad
                        //
                        //
                        //
                        //
                        //
                        glBindFramebuffer(GL_FRAMEBUFFER, 0);

                        bool horizontal = true, first_iteration = true;
                        int amount = 10;
                        shaderBlur.use();
                        for (unsigned int i = 0; i < amount; i++)
                        {
                            glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
                            shaderBlur.setInt("horizontal", horizontal);
                            glActiveTexture(GL_TEXTURE0);
                            glBindTexture(
                                GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : pingpongBuffers[!horizontal]);
                            shaderBlur.setInt("image", 0);
                            glBindVertexArray(VAO_fb);
                            glBindBuffer(GL_ARRAY_BUFFER, VBO_fb);
                            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_fb);
                            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                            horizontal = !horizontal;
                            if (first_iteration)
                                first_iteration = false;
                        }
                        glBindFramebuffer(GL_FRAMEBUFFER, 0);
                        glBindFramebuffer(GL_FRAMEBUFFER, 0);
                        glDisable(GL_DEPTH_TEST);
                        glClearColor(1, 1, 1, 1);
                        glClear(GL_COLOR_BUFFER_BIT);
                        fbShader.use();
                        glActiveTexture(GL_TEXTURE0);
                        glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
                        glActiveTexture(GL_TEXTURE1);
                        glBindTexture(GL_TEXTURE_2D, pingpongBuffers[!horizontal]);
                        fbShader.setInt("bloom", bloom);
                        fbShader.setFloat("exposure", exposure);
                        glBindVertexArray(VAO_fb);
                        glBindBuffer(GL_ARRAY_BUFFER, VBO_fb);
                        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_fb);
                        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                        //
                        //
                        //
                        // Done
                        //
                        //
                        //
                        //
                        //
                        glfwSwapBuffers(window);
                        glfwPollEvents();
                        if (glfwGetTime() - lasttime > 0.005)
                        {
                            vertices2[3] += velTrans * (glfwGetTime() - lasttime);
                            vertices2[8] += velTrans * (glfwGetTime() - lasttime);
                            vertices2[13] += velTrans * (glfwGetTime() - lasttime);
                            vertices2[18] += velTrans * (glfwGetTime() - lasttime);
                        }
                        if (flag_vel == 1)
                        {
                            lasttime = glfwGetTime();
                            flag_vel = 0;
                        }
                    }
                    if (level_comp == 1)
                    {
                        flag_win=1;
                        while (!glfwWindowShouldClose(window) && flag_restart == 0)
                        {
                            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
                            glClear(GL_COLOR_BUFFER_BIT);
                            processInput(window);
                            RenderText(shader, "You Won", 100.0f, 350.0f, 2.0f, glm::vec3(1.0, 1.0f, 1.0f));
                            RenderText(shader, "Press enter for restart", 500.0f, 100.0f, 0.4f, glm::vec3(1.0, 1.0f, 1.0f));
                            glfwSwapBuffers(window);
                            glfwPollEvents();
                        }
                    }
                }
            }
        }
    }
    if (flag_dead == 1)
    {
        while (!glfwWindowShouldClose(window) && flag_restart == 0)
        {
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            processInput(window);
            coin_str = "Coins Collected: ";
            coin_str += to_string(coin_ctr);
            RenderText(shader, "Game Over", 100.0f, 350.0f, 2.0f, glm::vec3(1.0, 1.0f, 1.0f));
            RenderText(shader, coin_str, 100.0f, 150.0f, 2.0f, glm::vec3(1.0, 1.0f, 1.0f));
            coin_str = "Coins Collected: ";
            RenderText(shader, "Press enter for restart", 500.0f, 100.0f, 0.4f, glm::vec3(1.0, 1.0f, 1.0f));
            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }
    if (flag_restart == 1)
    {
        flag_dead = 0;
        level_comp = 0;
        flag_nex_lev = 0;
        flag_restart = 0;
        flag_win=0;
        starttime = glfwGetTime();
        velocity = 0, transy = 0, k = 0, trans_coin = 0, trans_zap = 0, zap_osc = 0, flag_zap = 1;
        lasttime = glfwGetTime(), velZap = 1.0, velTrans = 1.0;
        int flag_vel = 0;
        coin_ctr=0;
        goto restart;
    }
    glfwTerminate();
    return 0;
}