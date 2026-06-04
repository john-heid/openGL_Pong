#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

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



bool player_one_up{ false };
bool player_one_down{ false };

bool player_two_up{ false };
bool player_two_down{ false };

float player_one_position_y{ 0.0f };
float player_two_position_y{ 0.0f };

float global_posX{};
float global_posY{};

float paddle_speed{ 10.0f };







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
            100.0f, 100.0f, 0.0f, 0.0f, // 0
            200.0f, 100.0f, 1.0f, 0.0f, // 1
            200.0f, 200.0f, 1.0f, 1.0f, // 2
            100.0f, 200.0f, 0.0f, 1.0f, // 3
        };

        unsigned int indices[] = {
            0, 1, 2,
            2, 3, 0,
        };

        GLCall(glEnable(GL_BLEND));
        GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
        
        VertexArray va;
        VertexBuffer vb(p1_positions, 4 * 4 * sizeof(float));

        VertexBufferLayout layout;
        layout.Push<float>(2);
        layout.Push<float>(2);
        va.AddBuffer(vb, layout);

        IndexBuffer ib(indices, 6);

        glm::mat4 proj = glm::ortho(0.0f, 960.0f, 0.0f, 540.0f, -1.0f, 1.0f);
        glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(-100, 0, 0));
        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(200, 200, 0));

        // multiplication order matters. Massive headache when switched to model * view * proj to match MVP. Do not do that.
        glm::mat4 mvp = proj * view * model;

        Shader shader("res/shaders/Basic.shader");
        shader.Bind();
        shader.SetUniform4f("u_Color", 0.8f, 0.3f, 0.8f, 1.0f);
        shader.SetUniformMat4f("u_MVP", mvp);

        Texture texture("res/textures/ChernoLogo.png");
        texture.Bind();
        shader.SetUniform1i("u_Texture", 0);

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

            // render
            // ------
            renderer.Clear();

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            // 1. Reset to identity every frame so movement doesn't accumulate exponentially
            glm::mat4 currentModel = glm::mat4(1.0f);

            // 2. Translate from the origin using your base position + dynamic input offsets
            currentModel = glm::translate(currentModel, glm::vec3(200.0f, 200.0f + player_one_position_y, 0.0f));

            // 3. Update the MVP using the fresh model matrix
            mvp = proj * view * currentModel;

            shader.Bind();
            shader.SetUniform4f("u_Color", 1.0f, 0.3f, 0.8f, 1.0f);
            shader.SetUniformMat4f("u_MVP", mvp);

            renderer.Draw(va, ib, shader);


            {
                static float f = 0.0f;
                ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f

                ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            }


            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


            // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
            // -------------------------------------------------------------------------------
            glfwSwapBuffers(window);
            // Poll for and process events


            if (player_one_up && player_one_down) {
                ;
            }
            else {
                if (player_one_up) {
                    player_one_position_y += paddle_speed;
                }
                if (player_one_down) {
                    player_one_position_y -= paddle_speed;
                }
            }


            glfwPollEvents();
        }

        // optional: de-allocate all resources once they've outlived their purpose
        // -----------------------------------------------------------------------
    }
    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

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
    if (entered) {
        std::cout << "Entered Window" << std::endl;
    }
    else {
        std::cout << "Left Window" << std::endl;
    }
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        std::cout << "Right button pressed" << std::endl;
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
        std::cout << "Right button released" << std::endl;
    }
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
    if (key == 87 && action == GLFW_PRESS) {
        player_two_up = true;
    }
    else if (key == 87 && action == GLFW_RELEASE) {
        player_two_up = false;
    }
    if (key == 83 && action == GLFW_PRESS) {
        player_two_down = true;
    }
    else if (key == 83 && action == GLFW_RELEASE) {
        player_two_down = false;
    }
}