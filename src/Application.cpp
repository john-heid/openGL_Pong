#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <array>
#include <algorithm>
#include <random>


#include "Renderer.h"

#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "Shader.h"
#include "Texture.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "ImGUI/imgui.h"
#include "ImGUI/imgui_impl_glfw.h"
#include "ImGUI/imgui_impl_opengl3.h"


static void cursorPositionCallback(GLFWwindow* window, double xPos, double yPos);
void cursorEnterCallback(GLFWwindow* window, int entered);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

bool AABB(float ax, float ay, float aw, float ah, float bx, float by, float bw, float bh);
void point_scored(int& player);
void check_winner(int& player1, int& player2);
void ball_reset();
void paddle_reset();

constexpr float PI = 3.14159265358979f;

//constexpr float resolution_x{ 960.0f };
//constexpr float resolution_y{ 540.0f };
//constexpr float resolution_x{ 1280.0f };
//constexpr float resolution_y{ 720.0f };
constexpr float resolution_x{ 1920.0f };
constexpr float resolution_y{ 1080.0f };
constexpr float center_x{ resolution_x / 2 };
constexpr float center_y{ resolution_y / 2 };
constexpr float scoreboard_offset_x{ 60.0f };
constexpr float paddle_boundary_offset_y{ 18.0f }; //  Half height of paddle
//constexpr glm::vec3 ball_spawn_location = glm::vec3( center_x, center_y, 0.0f);


bool player_one_up{ false };
bool player_one_down{ false };

bool player_two_up{ false };
bool player_two_down{ false };



double global_posX{};
double global_posY{};

constexpr float paddle_w = 20.0f;
constexpr float paddle_h = 100.0f;
float p1_x{ 0.0f };
float p1_y{ center_y - (paddle_h / 2.0f) };
float p2_x{ resolution_x - paddle_w };
float p2_y{ center_y - (paddle_h / 2.0f) };
constexpr float paddle_speed{ resolution_y * 0.0025f };
constexpr float paddle_offset_to_center_y{ paddle_h / 2.0f };

glm::vec2 ball_pos{ center_x, center_y }; // add offsets
glm::vec2 ball_vel{ 0.0f, 0.0f };
float ball_size = 20.0f;
float ball_speed_initial{ resolution_x * 0.002 };
float ball_speed_increment{ resolution_x * 0.0003 };
float ball_speed{ ball_speed_initial };

// Random starting angle for ball
std::mt19937 rng(std::random_device{}());
std::uniform_int_distribution<int> angle_dist(-30, 30);
std::uniform_int_distribution<int> dir_dist(0, 1);

int p1_score{ 0 };
int p2_score{ 0 };









int main()
{
    // glfw: initialize and configure
    // ------------------------------
    // Create window with graphics context
    //float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor()); // Valid on GLFW 3.3+ only
    //GLFWwindow* window = glfwCreateWindow((int)(1280 * main_scale), (int)(800 * main_scale), "Dear ImGui GLFW+OpenGL3 example", nullptr, nullptr);
    if (!glfwInit())
        return -1;

    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac

    //glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    //glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // glfw window creation
    // --------------------

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    
    //GLFWwindow* window = glfwCreateWindow(static_cast<int>(resolution_x), static_cast<int>(resolution_y), "Pong", NULL, NULL);
    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "Pong", monitor, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }                                                                        
    
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    
    
    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------




    std::cout << glGetString(GL_VERSION) << std::endl;
    {
        float background_positions[] {
            0.0f,       0.0f,       0.0f,   0.0f,
            1920.0f,    0.0f,       1.0f,   0.0f,
            1920.0f,    1080.0f,    1.0f,   1.0f,
            0.0f,       1080.0f,    0.0f,   1.0f,
        };
        float p1_positions[] = {
            0.0f,       0.0f, 0.0f, 0.0f, // 0
            paddle_w,   0.0f, 1.0f, 0.0f, // 1
            paddle_w,   paddle_h, 1.0f, 1.0f, // 2
            0.0f,       paddle_h, 0.0f, 1.0f, // 3
        };
        float ball_positions[] = {
            0.0f,       0.0f,   0.0f, 0.0f,
            ball_size,  0.0f,   1.0f, 0.0f,
            ball_size,  ball_size,  1.0f, 1.0f,
            0.0f,       ball_size,  0.0f, 1.0f,
        };

        float scoreboard_positions[] = {
            0.0f,   0.0f,   0.0f,   0.0f,
            48.0f,  0.0f,   1.0f,   0.0f,
            48.0f,  80.0f,  1.0f,   1.0f,
            0.0f,   80.0f,  0.0f,   1.0f,
        };

        unsigned int indices[] = {
            0, 1, 2,
            2, 3, 0,
        };

        GLCall(glEnable(GL_BLEND));
        GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
        
        // Background Setup
        VertexArray va_background;
        VertexBuffer vb_background(background_positions, 4 * 4 * sizeof(float));
        VertexBufferLayout layout_background;
        layout_background.Push<float>(2);
        layout_background.Push<float>(2);
        va_background.AddBuffer(vb_background, layout_background);

        // Paddle Setup
        VertexArray va;
        VertexBuffer vb(p1_positions, 4 * 4 * sizeof(float));
        VertexBufferLayout layout;
        layout.Push<float>(2);
        layout.Push<float>(2);
        va.AddBuffer(vb, layout);

        // Ball Setup
        VertexArray ball_va;
        VertexBuffer ball_vb(ball_positions, 4 * 4 * sizeof(float));
        VertexBufferLayout ball_layout;
        ball_layout.Push<float>(2);
        ball_layout.Push<float>(2);
        ball_va.AddBuffer(ball_vb, ball_layout);

        // Scoreboard Setup
        VertexArray scoreboard_va;
        VertexBuffer scoreboard_vb(scoreboard_positions, 4 * 4 * sizeof(float));
        VertexBufferLayout scoreboard_layout;
        scoreboard_layout.Push<float>(2);
        scoreboard_layout.Push<float>(2);
        scoreboard_va.AddBuffer(scoreboard_vb, scoreboard_layout);

        IndexBuffer ib(indices, 6);

        glm::mat4 proj = glm::ortho(0.0f, resolution_x, 0.0f, resolution_y, -1.0f, 1.0f);
        glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0));
        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0));

        // multiplication order matters. Massive headache when switched to model * view * proj to match MVP. Do not do that.
        glm::mat4 mvp = proj * view * model;

        Shader shader("res/shaders/Basic.shader");
        shader.Bind();
        shader.SetUniformMat4f("u_MVP", mvp);
        Shader color_shader("res/shaders/Color.Shader");
        color_shader.Bind();
        color_shader.SetUniform4f("u_Color", 1.0f, 1.0f, 1.0f, 1.0f);
        color_shader.SetUniformMat4f("u_MVP", mvp);
        
        Texture background_texture("res/textures/background.png");
        Texture paddle_texture("res/textures/pong_paddle.png");
        Texture ball_texture("res/textures/pong_ball.png");
        Texture score_zero("res/textures/score_zero.png");
        Texture score_one("res/textures/score_one.png");
        Texture score_two("res/textures/score_two.png");
        Texture score_three("res/textures/score_three.png");
        
        const float score_half_width = score_zero.GetWidth() / 2.0f;

        glm::vec3 scoreboard_location_p1{ center_x - scoreboard_offset_x - score_half_width, center_y, 0.0f };
        glm::vec3 scoreboard_location_p2{ center_x + scoreboard_offset_x - score_half_width, center_y, 0.0f };


        std::array<Texture*, 4> score_textures = { &score_zero, &score_one, &score_two, &score_three };


        va.Unbind();
        vb.Unbind();
        ib.Unbind();
        shader.Unbind();

        Renderer renderer{};




        // User Input
        glfwSetCursorPosCallback(window, cursorPositionCallback);
        glfwSetCursorEnterCallback(window, cursorEnterCallback);
        glfwSetMouseButtonCallback(window, mouseButtonCallback);
        glfwSetKeyCallback(window, keyCallback);



        
        ball_reset();

        // render loop
        // -----------
        while (!glfwWindowShouldClose(window))
        {
            // input
            // -----


            ball_pos += ball_vel;
            float ball_x = ball_pos.x;
            float ball_y = ball_pos.y;

            // Ceiling Bounds - Ball
            if (ball_pos.y <= 0.0f || ball_pos.y >= resolution_y - ball_size) {
                ball_vel.y *= -1.0f;
            }

            // Horizontal Bounds / Reset / Score - Ball
            if (ball_pos.x >= resolution_x - ball_size) {
                point_scored(p1_score);
                check_winner(p1_score, p2_score);
                paddle_reset();
                ball_reset();
            }
            // Where 0.0f is the left boundary
            if (ball_pos.x <= 0.0f) {
                point_scored(p2_score);
                check_winner(p1_score, p2_score);
                paddle_reset();
                ball_reset();
            }

            // Player 1 Collision
            if (AABB(ball_x, ball_y, ball_size, ball_size, p1_x, p1_y, paddle_w, paddle_h)) {
                ball_pos.x = p1_x + paddle_w;
                float paddle_center{ p1_y + paddle_offset_to_center_y };
                float ball_center{ ball_y + (ball_size / 2.0f) };
                float hit_pos = (ball_center - paddle_center) / (paddle_h / 2.0f);
                
                float max_bounce_angle = 65.0f * (PI / 180.0f);

                glm::vec2 dir = glm::vec2(
                    abs(cos(max_bounce_angle * hit_pos)),
                    sin(max_bounce_angle * hit_pos)
                );
                ball_speed += ball_speed_increment;
                ball_vel = glm::normalize(dir) * ball_speed;
            }
            // Player 2 Collision
            if (AABB(ball_x, ball_y, ball_size, ball_size, p2_x, p2_y, paddle_w, paddle_h)) {
                ball_pos.x = p2_x - ball_size;
                float paddle_center{ p2_y + paddle_offset_to_center_y };
                float ball_center{ ball_y + (ball_size / 2.0f) };
                float hit_pos = (ball_center - paddle_center) / (paddle_h / 2.0f);
                float max_bounce_angle = 65.0f * (PI / 180.0f);

                glm::vec2 dir = glm::vec2(
                    -abs(cos(max_bounce_angle * hit_pos)),
                    sin(max_bounce_angle * hit_pos)
                );
                ball_speed += ball_speed_increment;
                ball_vel = glm::normalize(dir) * ball_speed;
            }



            // render
            // ------
            renderer.Clear();

            /*ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();*/

            // Background
            glm::mat4 background_model = glm::mat4(1.0f);
            background_model = glm::translate(background_model, glm::vec3(0.0f, 0.0f, 0.0f));
            glm::mat4 mvpBackground = proj * view * background_model;
            background_texture.Bind();
            shader.Bind();
            shader.SetUniform1i("u_Texture", 0);
            shader.SetUniformMat4f("u_MVP", mvpBackground);
            renderer.Draw(va_background, ib, shader);

            // Player 1
            glm::mat4 player1_model = glm::mat4(1.0f);
            player1_model = glm::translate(player1_model, glm::vec3(p1_x, p1_y, 0.0f));
            glm::mat4 mvp1 = proj * view * player1_model;
            paddle_texture.Bind();
            shader.Bind();
            shader.SetUniform1i("u_Texture", 0);
            shader.SetUniformMat4f("u_MVP", mvp1);
            renderer.Draw(va, ib, shader);

            // Player 2
            glm::mat4 player2_model = glm::mat4(1.0f);
            player2_model = glm::translate(player2_model, glm::vec3(p2_x, p2_y, 0.0f));
            glm::mat4 mvp2 = proj * view * player2_model;
            paddle_texture.Bind();
            shader.Bind();
            shader.SetUniform1i("u_Texture", 0);
            shader.SetUniformMat4f("u_MVP", mvp2);
            renderer.Draw(va, ib, shader);
            

            // Scoreboard - Player 1
            glm::mat4 score_player1_model = glm::mat4(1.0f);
            score_player1_model = glm::translate(score_player1_model, scoreboard_location_p1);
            glm::mat4 score_player1_mvp = proj * view * score_player1_model;
            score_textures[p1_score]->Bind();
            color_shader.Bind();
            color_shader.SetUniform1i("u_UseTexture", 1);
            color_shader.SetUniform4f("u_Color", 1.0f, 1.0f, 1.0f, 1.0f);
            color_shader.SetUniform1i("u_Texture", 0);
            color_shader.SetUniformMat4f("u_MVP", score_player1_mvp);
            renderer.Draw(scoreboard_va, ib, color_shader);

            // Scoreboard - Player 2
            glm::mat4 score_player2_model = glm::mat4(1.0f);
            score_player2_model = glm::translate(score_player2_model, scoreboard_location_p2);
            glm::mat4 score_player2_mvp = proj * view * score_player2_model;
            score_textures[p2_score]->Bind();
            color_shader.Bind();
            color_shader.SetUniform1i("u_UseTexture", 1);
            color_shader.SetUniform4f("u_Color", 1.0f, 1.0f, 1.0f, 1.0f);
            color_shader.SetUniform1i("u_Texture", 0);
            color_shader.SetUniformMat4f("u_MVP", score_player2_mvp);
            renderer.Draw(scoreboard_va, ib, color_shader);

            // Ball
            glm::mat4 ball_model = glm::mat4(1.0f);
            ball_model = glm::translate(ball_model, glm::vec3(ball_x, ball_y, 0.0f));
            glm::mat4 ballmvp = proj * view * ball_model;
            color_shader.Bind();
            color_shader.SetUniform1i("u_UseTexture", 0);
            color_shader.SetUniform4f("u_Color", 1.0f, 0.0f, 0.0f, 1.0f);
            color_shader.SetUniformMat4f("u_MVP", ballmvp);
            renderer.Draw(ball_va, ib, color_shader);
            


            //{
            //    static float f = 0.0f;
            //    ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f

            //    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            //}


            //ImGui::Render();
            //ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
            // -------------------------------------------------------------------------------
            glfwSwapBuffers(window);
            // Poll for and process events


            if (player_one_up && player_one_down) {
                ;
            }
            else {
                if (player_one_up && p1_y + paddle_h < resolution_y) {
                    p1_y += paddle_speed;
                }
                
                if (player_one_down && p1_y > 0.0f) {
                    p1_y -= paddle_speed;
                }
            }
            p1_y = std::clamp(p1_y, 0.0f, resolution_y - paddle_h);
            if (player_two_up && player_two_down) {
                ;
            }
            else {
                if (player_two_up && p2_y + paddle_h < resolution_y) {
                    p2_y += paddle_speed;
                }

                if (player_two_down && p2_y > 0.0f) {
                    p2_y -= paddle_speed;
                }
            }
            p2_y = std::clamp(p2_y, 0.0f, resolution_y - paddle_h);

            glfwPollEvents();
        }

        // optional: de-allocate all resources once they've outlived their purpose
        // -----------------------------------------------------------------------
    }
    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    //ImGui_ImplOpenGL3_Shutdown();
    //ImGui_ImplGlfw_Shutdown();
    //ImGui::DestroyContext();

    glfwDestroyWindow(window);

    glfwTerminate();
    return 0;
}


static void cursorPositionCallback(GLFWwindow* window, double xPos, double yPos) 
{
    //std::cout << xPos << " : " << yPos << std::endl;
    global_posX = xPos;
    global_posY = -1 * yPos;
}

void cursorEnterCallback(GLFWwindow* window, int entered)
{
    /*if (entered) {
        std::cout << "Entered Window" << std::endl;
    }
    else {
        std::cout << "Left Window" << std::endl;
    }*/
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    //if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
    //    std::cout << "Right button pressed" << std::endl;
    //}
    //if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
    //    std::cout << "Right button released" << std::endl;
    //}
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // Menu
    // Exit/Close
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    // Player 1
    if (key == 87 && action == GLFW_PRESS) {
        player_one_up = true;
    }
    else if (key == 87 && action == GLFW_RELEASE) {
        player_one_up = false;
    }
    if (key == 83 && action == GLFW_PRESS) {
        player_one_down = true;
    }
    else if (key == 83 && action == GLFW_RELEASE) {
        player_one_down = false;
    }



    // Player 2
    if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
        player_two_up = true;
    }
    else if (key == GLFW_KEY_UP && action == GLFW_RELEASE) {
        player_two_up = false;
    }
    if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
        player_two_down = true;
    }
    else if (key == GLFW_KEY_DOWN && action == GLFW_RELEASE) {
        player_two_down = false;
    }



}

bool AABB(float ax, float ay, float aw, float ah, float bx, float by, float bw, float bh) {
    return ax < bx + bw &&
        ax + aw > bx &&
        ay < by + bh &&
        ay + ah > by;
}

void point_scored(int& player) {
    player += 1;
}

void check_winner(int& player1, int& player2) {
    if (player1 >= 3) {
        std::cout << "Player 1 wins";
        player1 = 0;
        player2 = 0;
        // reset scores or end game
    }
    if (player2 >= 3) {
        std::cout << "Player 2 wins";
        player1 = 0;
        player2 = 0;
        // reset scores or end game
    }
}

void ball_reset() {
    ball_pos = glm::vec2(center_x - (ball_size / 2.0f), center_y - (ball_size / 2.0f));
    float angle = angle_dist(rng) * (PI / 180.0f);
    glm::vec2 dir = glm::vec2(
        cos(angle) * (dir_dist(rng) ? 1 : -1),
        sin(angle)
    );
    ball_speed = ball_speed_initial;
    ball_vel = glm::normalize(dir) * ball_speed;
}

void paddle_reset() {
    p1_y = center_y - (paddle_h/2.0f);
    p2_y = center_y - (paddle_h/2.0f);
}