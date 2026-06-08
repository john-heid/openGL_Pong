#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <array>

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
void check_winner(int player1, int player2);


bool player_one_up{ false };
bool player_one_down{ false };

bool player_two_up{ false };
bool player_two_down{ false };

float p1_x{ 0.0f };
float p1_y{ 200.0f };

float p2_x{ 940.0f };
float p2_y{ 200.0f };

float global_posX{};
float global_posY{};

float paddle_w = 20.0f;
float paddle_h = 38.0f;
float paddle_speed{ 5.0f };

float ball_x = 475.0f;
float ball_y = 265.0f;
float ball_size = 10.0f;
float ball_vel_x = 2.0f;
float ball_vel_y = 2.0f;

int p1_score{ 0 };
int p2_score{ 0 };







int main()
{
    // glfw: initialize and configure
    // ------------------------------
    // Create window with graphics context
    float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor()); // Valid on GLFW 3.3+ only
    GLFWwindow* window = glfwCreateWindow((int)(1280 * main_scale), (int)(800 * main_scale), "Dear ImGui GLFW+OpenGL3 example", nullptr, nullptr);
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
    window = glfwCreateWindow(960, 540, "Hello World", NULL, NULL);
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
        float p1_positions[] = {
            0.0f,   0.0f, 0.0f, 0.0f, // 0
            20.0f,  0.0f, 1.0f, 0.0f, // 1
            20.0f, 38.0f, 1.0f, 1.0f, // 2
            0.0f,  38.0f, 0.0f, 1.0f, // 3
        };
        float ball_positions[] = {
            0.0f,   0.0f,   0.0f, 0.0f,
            10.0f,  0.0f,   1.0f, 0.0f,
            10.0f,  10.0f,  1.0f, 1.0f,
            0.0f,   10.0f,  0.0f, 1.0f,
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

        glm::mat4 proj = glm::ortho(0.0f, 960.0f, 0.0f, 540.0f, -1.0f, 1.0f);
        glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0));
        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0));

        // multiplication order matters. Massive headache when switched to model * view * proj to match MVP. Do not do that.
        glm::mat4 mvp = proj * view * model;

        Shader shader("res/shaders/Basic.shader");
        shader.Bind();
        shader.SetUniform4f("u_Color", 1.0f, 1.0f, 1.8f, 1.0f);
        shader.SetUniformMat4f("u_MVP", mvp);

        Texture paddle_texture("res/textures/redcar_icon.png");
        Texture ball_texture("res/textures/pong_ball.png");
        Texture score_zero("res/textures/score_zero.png");
        Texture score_one("res/textures/score_one.png");
        Texture score_two("res/textures/score_two.png");
        Texture score_three("res/textures/score_three.png");
        std::cout << "score_zero width: " << score_zero.GetWidth() << std::endl;



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



        


        // render loop
        // -----------
        while (!glfwWindowShouldClose(window))
        {
            // input
            // -----
            ball_x += ball_vel_x;
            ball_y += ball_vel_y;

            // Ceiling Bounds
            if (ball_y <= 0.0f || ball_y >= 540.0f - 10.0f) {
                ball_vel_y *= -1.0f;
            }

            // Horizontal Bounds / Reset / Score
            if (ball_x >= 960.0f - 10.0f) {
                point_scored(p1_score);
                check_winner(p1_score, p2_score);
                ball_x = 475.0f;
                ball_y = 265.0f;
            }
            if (ball_x <= 0.0f) {
                point_scored(p2_score);
                check_winner(p1_score, p2_score);
                ball_x = 475.0f;
                ball_y = 265.0f;
            }

            // Player 1 Collision
            if (AABB(ball_x, ball_y, ball_size, ball_size, p1_x, p1_y, paddle_w, paddle_h)) {
                ball_vel_x *= -1.0f;
                std::cout << "Collision!!!!!Player 1";
            }
            // Player 2 Collision
            if (AABB(ball_x, ball_y, ball_size, ball_size, p2_x, p2_y, paddle_w, paddle_h)) {
                ball_vel_x *= -1.0f;
                std::cout << "Collision!!!!Player 2";
            }



            // render
            // ------
            renderer.Clear();
            shader.Bind();

            /*ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();*/

            // Player 1
            glm::mat4 player1_model = glm::mat4(1.0f);
            player1_model = glm::translate(player1_model, glm::vec3(0.0f, p1_y, 0.0f));
            glm::mat4 mvp1 = proj * view * player1_model;
            paddle_texture.Bind();
            shader.SetUniform1i("u_Texture", 0);
            shader.SetUniformMat4f("u_MVP", mvp1);
            renderer.Draw(va, ib, shader);

            // Player 2
            glm::mat4 player2_model = glm::mat4(1.0f);
            player2_model = glm::translate(player2_model, glm::vec3(p2_x, p2_y, 0.0f));
            glm::mat4 mvp2 = proj * view * player2_model;
            paddle_texture.Bind();
            shader.SetUniform1i("u_Texture", 0);
            shader.SetUniformMat4f("u_MVP", mvp2);
            renderer.Draw(va, ib, shader);
            
            // Ball
            glm::mat4 ball_model = glm::mat4(1.0f);
            ball_model = glm::translate(ball_model, glm::vec3(ball_x, ball_y, 0.0f));
            glm::mat4 ballmvp = proj * view * ball_model;
            ball_texture.Bind();
            shader.SetUniform4f("u_Color", 1.0f, 1.0f, 1.0f, 1.0f);

            shader.SetUniform1i("u_Texture", 0);
            shader.SetUniformMat4f("u_MVP", ballmvp);
            renderer.Draw(ball_va, ib, shader);

            // Scoreboard - Player 1
            glm::mat4 score_player1_model = glm::mat4(1.0f);
            score_player1_model = glm::translate(score_player1_model, glm::vec3(380.0f, 300.0f, 0.0f));
            glm::mat4 score_player1_mvp = proj * view * score_player1_model;
            score_textures[p1_score]->Bind();
            shader.SetUniform4f("u_Color", 1.0f, 1.0f, 1.0f, 1.0f);
            shader.SetUniform1i("u_Texture", 0);
            shader.SetUniformMat4f("u_MVP", score_player1_mvp);
            renderer.Draw(scoreboard_va, ib, shader);

            // Scoreboard - Player 2
            glm::mat4 score_player2_model = glm::mat4(1.0f);
            score_player2_model = glm::translate(score_player2_model, glm::vec3(440.0f, 300.0f, 0.0f));
            glm::mat4 score_player2_mvp = proj * view * score_player2_model;
            score_textures[p2_score]->Bind();
            shader.SetUniform4f("u_Color", 1.0f, 1.0f, 1.0f, 1.0f);
            shader.SetUniform1i("u_Texture", 0);
            shader.SetUniformMat4f("u_MVP", score_player2_mvp);
            renderer.Draw(scoreboard_va, ib, shader);





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
                if (player_one_up) {
                    p1_y += paddle_speed;
                }
                if (player_one_down) {
                    p1_y -= paddle_speed;
                }
            }
            if (player_two_up && player_two_down) {
                ;
            }
            else {
                if (player_two_up) {
                    p2_y += paddle_speed;
                }
                if (player_two_down) {
                    p2_y -= paddle_speed;
                }
            }


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

void check_winner(int player1, int player2) {
    if (player1 >= 3) {
        std::cout << "Player 1 wins";
    }
    if (player2 >= 3) {
        std::cout << "Player 2 wins";
    }
}