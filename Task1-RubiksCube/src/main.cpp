#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <Debugger.h>
#include <VertexBuffer.h>
#include <VertexBufferLayout.h>
#include <IndexBuffer.h>
#include <VertexArray.h>
#include <Shader.h>
#include <Texture.h>
#include <Camera.h>
#include <RubiksCube.h>

#include <iostream>
#include <vector>

/* Set to 1 to render a single cube with 6 colored faces (test mode); 0 for full Rubik's cube */
#define SINGLE_CUBE_TEST 0

/* Window size */
const unsigned int width = 800;
const unsigned int height = 800;
const float FOVdegree = 45.0f;
const float near = 0.1f;
const float far = 100.0f;

/* Full cube: 6 faces, 24 vertices. Layout: position(3), color(3), texCoord(2) = 8 floats per vertex.
   No extra attributes - keeps stride correct so cube geometry renders properly. */
float cubeVertices[] = {
    // Front face (z = 0.5)
    -0.5f, -0.5f,  0.5f,    1.0f, 0.0f, 0.0f,    0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,    0.0f, 1.0f, 0.0f,    1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,    0.0f, 0.0f, 1.0f,    1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,    1.0f, 1.0f, 0.0f,    0.0f, 1.0f,
    // Back face (z = -0.5)
    -0.5f, -0.5f, -0.5f,    1.0f, 0.0f, 1.0f,    1.0f, 0.0f,
     0.5f, -0.5f, -0.5f,    0.0f, 1.0f, 1.0f,    0.0f, 0.0f,
     0.5f,  0.5f, -0.5f,    1.0f, 1.0f, 1.0f,    0.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,    0.0f, 0.0f, 0.0f,    1.0f, 1.0f,
    // Top face (y = 0.5)
    -0.5f,  0.5f,  0.5f,    1.0f, 0.0f, 0.0f,    0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,    0.0f, 1.0f, 0.0f,    1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,    0.0f, 0.0f, 1.0f,    1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,    1.0f, 1.0f, 0.0f,    0.0f, 1.0f,
    // Bottom face (y = -0.5)
    -0.5f, -0.5f,  0.5f,    1.0f, 0.0f, 1.0f,    0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,    0.0f, 1.0f, 1.0f,    1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,    1.0f, 1.0f, 1.0f,    1.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,    0.0f, 0.0f, 0.0f,    0.0f, 0.0f,
    // Right face (x = 0.5)
     0.5f, -0.5f,  0.5f,    1.0f, 0.0f, 0.0f,    0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,    0.0f, 1.0f, 0.0f,    1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,    0.0f, 0.0f, 1.0f,    1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,    1.0f, 1.0f, 0.0f,    0.0f, 1.0f,
    // Left face (x = -0.5)
    -0.5f, -0.5f,  0.5f,    1.0f, 0.0f, 1.0f,    1.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,    0.0f, 1.0f, 1.0f,    0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,    1.0f, 1.0f, 1.0f,    0.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,    0.0f, 0.0f, 0.0f,    1.0f, 1.0f,
};

/* Indices for cube faces (each face is 2 triangles - OpenGL draws only triangles) */
unsigned int cubeIndices[] = {
    // Front (z = 0.5)
    0, 1, 2,  2, 3, 0,
    // Back (z = -0.5)
    4, 5, 6,  6, 7, 4,
    // Top / Up (y = 0.5)
    8, 9, 10,  10, 11, 8,
    // Bottom / Down (y = -0.5)
    12, 13, 14,  14, 15, 12,
    // Right (x = 0.5)
    16, 17, 18,  18, 19, 16,
    // Left (x = -0.5)
    20, 21, 22,  22, 23, 20
};

/* Same 6 distinct face colors for every cubie (Right, Left, Up, Down, Front, Back) */
const glm::vec4 RUBIK_RIGHT  (0.0f, 0.2f, 0.8f, 1.0f);   // Blue
const glm::vec4 RUBIK_LEFT   (0.0f, 0.7f, 0.2f, 1.0f);   // Green
const glm::vec4 RUBIK_UP     (0.95f, 0.95f, 0.95f, 1.0f); // White
const glm::vec4 RUBIK_DOWN   (0.9f, 0.85f, 0.0f, 1.0f);  // Yellow
const glm::vec4 RUBIK_FRONT  (0.85f, 0.15f, 0.15f, 1.0f); // Red
const glm::vec4 RUBIK_BACK   (0.95f, 0.5f, 0.0f, 1.0f);  // Orange

/* Global state */
RubiksCube g_RubiksCube;
Camera* g_Camera = nullptr;
bool g_PickingMode = false;
int g_SelectedCubie = -1;
float g_SelectedDepth = 0.0f;
float g_PendingPickX = -1.0f;
float g_PendingPickY = -1.0f;
glm::vec2 g_LastMousePos(0.0f, 0.0f);
glm::mat4 g_GlobalCubeRotation(1.0f);
float g_LastFrameTime = 0.0f;

// Keyboard callback
void KeyCallback(GLFWwindow* window, int key, int scanCode, int action, int mods)
{
    if (action != GLFW_PRESS)
        return;
    
    switch (key)
    {
        // Wall rotations (only if not currently rotating)
        case GLFW_KEY_R:
            if (!g_RubiksCube.IsRotating())
                g_RubiksCube.StartWallRotation(Wall::RIGHT, 
                    g_RubiksCube.GetAngleIncrement(), 
                    g_RubiksCube.GetDirectionMultiplier() > 0);
            break;
        case GLFW_KEY_L:
            if (!g_RubiksCube.IsRotating())
                g_RubiksCube.StartWallRotation(Wall::LEFT, 
                    g_RubiksCube.GetAngleIncrement(), 
                    g_RubiksCube.GetDirectionMultiplier() > 0);
            break;
        case GLFW_KEY_U:
            if (!g_RubiksCube.IsRotating())
                g_RubiksCube.StartWallRotation(Wall::UP, 
                    g_RubiksCube.GetAngleIncrement(), 
                    g_RubiksCube.GetDirectionMultiplier() > 0);
            break;
        case GLFW_KEY_D:
            if (!g_RubiksCube.IsRotating())
                g_RubiksCube.StartWallRotation(Wall::DOWN, 
                    g_RubiksCube.GetAngleIncrement(), 
                    g_RubiksCube.GetDirectionMultiplier() > 0);
            break;
        case GLFW_KEY_F:
            if (!g_RubiksCube.IsRotating())
                g_RubiksCube.StartWallRotation(Wall::FRONT, 
                    g_RubiksCube.GetAngleIncrement(), 
                    g_RubiksCube.GetDirectionMultiplier() > 0);
            break;
        case GLFW_KEY_B:
            if (!g_RubiksCube.IsRotating())
                g_RubiksCube.StartWallRotation(Wall::BACK, 
                    g_RubiksCube.GetAngleIncrement(), 
                    g_RubiksCube.GetDirectionMultiplier() > 0);
            break;
        
        // Toggle rotation direction
        case GLFW_KEY_SPACE:
            g_RubiksCube.SetDirectionMultiplier(-g_RubiksCube.GetDirectionMultiplier());
            std::cout << "Rotation direction: " 
                      << (g_RubiksCube.GetDirectionMultiplier() > 0 ? "Clockwise" : "Counter-clockwise") 
                      << std::endl;
            break;
        
        // Adjust rotation angle
        case GLFW_KEY_Z:
            {
                float currentAngle = g_RubiksCube.GetAngleIncrement();
                if (currentAngle > 90.0f)
                {
                    g_RubiksCube.SetAngleIncrement(currentAngle / 2.0f);
                    std::cout << "Rotation angle: " << g_RubiksCube.GetAngleIncrement() << " degrees" << std::endl;
                }
            }
            break;
        case GLFW_KEY_A:
            {
                float currentAngle = g_RubiksCube.GetAngleIncrement();
                if (currentAngle < 180.0f)
                {
                    g_RubiksCube.SetAngleIncrement(currentAngle * 2.0f);
                    std::cout << "Rotation angle: " << g_RubiksCube.GetAngleIncrement() << " degrees" << std::endl;
                }
            }
            break;
        
        // Toggle picking mode
        case GLFW_KEY_P:
            g_PickingMode = !g_PickingMode;
            std::cout << "Picking mode: " << (g_PickingMode ? "ON" : "OFF") << std::endl;
            break;
        
        // Arrow keys - rotate whole cube
        case GLFW_KEY_UP:
            g_GlobalCubeRotation = glm::rotate(g_GlobalCubeRotation, glm::radians(5.0f), glm::vec3(1, 0, 0));
            g_RubiksCube.SetWholeCubeTransform(g_GlobalCubeRotation);
            break;
        case GLFW_KEY_DOWN:
            g_GlobalCubeRotation = glm::rotate(g_GlobalCubeRotation, glm::radians(-5.0f), glm::vec3(1, 0, 0));
            g_RubiksCube.SetWholeCubeTransform(g_GlobalCubeRotation);
            break;
        case GLFW_KEY_LEFT:
            g_GlobalCubeRotation = glm::rotate(g_GlobalCubeRotation, glm::radians(-5.0f), glm::vec3(0, 1, 0));
            g_RubiksCube.SetWholeCubeTransform(g_GlobalCubeRotation);
            break;
        case GLFW_KEY_RIGHT:
            g_GlobalCubeRotation = glm::rotate(g_GlobalCubeRotation, glm::radians(5.0f), glm::vec3(0, 1, 0));
            g_RubiksCube.SetWholeCubeTransform(g_GlobalCubeRotation);
            break;
    }
}

void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (action == GLFW_PRESS)
    {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        g_LastMousePos = glm::vec2(xpos, ypos);
        
        // Handle picking: request pick at next frame (color picking pass in main loop)
        if (g_PickingMode && button == GLFW_MOUSE_BUTTON_LEFT)
        {
            g_PendingPickX = static_cast<float>(xpos);
            g_PendingPickY = static_cast<float>(ypos);
        }
    }
}

void CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
    glm::vec2 currentPos(xpos, ypos);
    glm::vec2 delta = g_LastMousePos - currentPos;
    
    // Left mouse button - rotate whole cube or selected cubie
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        if (g_SelectedCubie >= 0 && g_Camera)
        {
            // Rotate selected cubie according to camera view (camera right and up axes)
            Cubie& cubie = g_RubiksCube.GetCubie(g_SelectedCubie);
            glm::vec3 cameraRight = g_Camera->GetRight();
            glm::vec3 cameraUp = g_Camera->GetUp();
            glm::mat4 rotRight = glm::rotate(glm::mat4(1.0f), glm::radians(delta.x * 0.5f), cameraRight);
            glm::mat4 rotUp = glm::rotate(glm::mat4(1.0f), glm::radians(-delta.y * 0.5f), cameraUp);
            cubie.selectionTransform = rotUp * rotRight * cubie.selectionTransform;
        }
        else
        {
            // Rotate whole cube
            glm::mat4 rotX = glm::rotate(glm::mat4(1.0f), glm::radians(delta.y * 0.5f), glm::vec3(1, 0, 0));
            glm::mat4 rotY = glm::rotate(glm::mat4(1.0f), glm::radians(delta.x * 0.5f), glm::vec3(0, 1, 0));
            g_GlobalCubeRotation = rotY * rotX * g_GlobalCubeRotation;
            g_RubiksCube.SetWholeCubeTransform(g_GlobalCubeRotation);
        }
    }
    // Right mouse button - pan camera or translate selected cubie (Z-Buffer: cube stays under mouse)
    else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
    {
        if (g_SelectedCubie >= 0 && g_Camera)
        {
            // Z-Buffer translation: unproject mouse + depth so cube stays under cursor
            glm::mat4 view = g_Camera->GetViewMatrix();
            glm::mat4 proj = g_Camera->GetProjectionMatrix();
            glm::vec4 viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
            float winY = static_cast<float>(height) - g_LastMousePos.y;
            float curWinY = static_cast<float>(height) - currentPos.y;
            glm::vec3 prevWorld = glm::unProject(glm::vec3(g_LastMousePos.x, winY, g_SelectedDepth), view, proj, viewport);
            glm::vec3 currWorld = glm::unProject(glm::vec3(currentPos.x, curWinY, g_SelectedDepth), view, proj, viewport);
            glm::vec3 deltaWorld = currWorld - prevWorld;
            Cubie& cubie = g_RubiksCube.GetCubie(g_SelectedCubie);
            glm::mat4 trans = glm::translate(glm::mat4(1.0f), deltaWorld);
            cubie.selectionTransform = trans * cubie.selectionTransform;
        }
        else
        {
            // Pan camera
            if (g_Camera)
                g_Camera->Pan(-delta.x, delta.y);
        }
    }
    
    g_LastMousePos = currentPos;
}

void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (g_Camera)
        g_Camera->Zoom(yoffset);
}

int main(int argc, char* argv[])
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
    {
        return -1;
    }
    
    /* Set OpenGL to Version 3.3.0 */
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(width, height, "Rubik's Cube", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    /* Load GLAD so it configures OpenGL */
    gladLoadGL();

    /* Control frame rate */
    glfwSwapInterval(1);

    /* Print OpenGL version */
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

    /* Set scope for automatic cleanup */
    {
        /* Enable blending and depth testing; disable culling so all 6 cube faces are visible */
        GLCall(glEnable(GL_BLEND));
        GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
        GLCall(glEnable(GL_DEPTH_TEST));
        GLCall(glDisable(GL_CULL_FACE));

        /* Create VAO, VBO, IBO for cube geometry */
        VertexArray va;
        VertexBuffer vb(cubeVertices, sizeof(cubeVertices));
        IndexBuffer ib(cubeIndices, sizeof(cubeIndices) / sizeof(unsigned int));
        /* Six separate index buffers (one per face). IndexBuffer ctor takes SIZE IN BYTES. */
        unsigned int face0[] = { 0, 1, 2, 2, 3, 0 };
        unsigned int face1[] = { 4, 5, 6, 6, 7, 4 };
        unsigned int face2[] = { 8, 9, 10, 10, 11, 8 };
        unsigned int face3[] = { 12, 13, 14, 14, 15, 12 };
        unsigned int face4[] = { 16, 17, 18, 18, 19, 16 };
        unsigned int face5[] = { 20, 21, 22, 22, 23, 20 };
        const unsigned int faceIndexBytes = 6 * sizeof(unsigned int);
        IndexBuffer ibFace0(face0, faceIndexBytes), ibFace1(face1, faceIndexBytes), ibFace2(face2, faceIndexBytes),
                    ibFace3(face3, faceIndexBytes), ibFace4(face4, faceIndexBytes), ibFace5(face5, faceIndexBytes);
        IndexBuffer* ibFaces[6] = { &ibFace0, &ibFace1, &ibFace2, &ibFace3, &ibFace4, &ibFace5 };

        VertexBufferLayout layout;
        layout.Push<float>(3);  // positions
        layout.Push<float>(3);  // colors
        layout.Push<float>(2);  // texCoords
        va.AddBuffer(vb, layout);

        /* Create texture */
        Texture texture("res/textures/plane.png");
        texture.Bind();
         
        /* Create shaders */
        Shader shader("res/shaders/basic.shader");
        shader.Bind();

        /* Unbind to prevent accidental modification */
        va.Unbind();
        vb.Unbind();
        ib.Unbind();
        shader.Unbind();

        /* Create camera with perspective projection */
        Camera camera(width, height);
        camera.SetPerspective(FOVdegree, near, far);
        g_Camera = &camera;
        
        // Position camera to view the cube
        camera.Pan(0.0f, 0.0f);  // Start at origin
        camera.Zoom(-10.0f);      // Move back to see the cube
        
        /* Set up input callbacks */
        glfwSetKeyCallback(window, KeyCallback);
        glfwSetMouseButtonCallback(window, MouseButtonCallback);
        glfwSetCursorPosCallback(window, CursorPosCallback);
        glfwSetScrollCallback(window, ScrollCallback);

        std::cout << "\n=== CONTROLS ===" << std::endl;
        std::cout << "Wall Rotations: R, L, U, D, F, B" << std::endl;
        std::cout << "Toggle Direction: SPACE" << std::endl;
        std::cout << "Angle Control: Z (divide by 2), A (multiply by 2)" << std::endl;
        std::cout << "Whole Cube Rotation: Arrow Keys or Left Mouse Drag" << std::endl;
        std::cout << "Camera Pan: Right Mouse Drag" << std::endl;
        std::cout << "Camera Zoom: Scroll Wheel" << std::endl;
        std::cout << "Picking Mode: P (toggle)" << std::endl;
        std::cout << "================\n" << std::endl;

        /* Main loop */
        float lastTime = glfwGetTime();
        
        while (!glfwWindowShouldClose(window))
        {
            /* Calculate delta time */
            float currentTime = glfwGetTime();
            float deltaTime = currentTime - lastTime;
            lastTime = currentTime;
            
            /* Update Rubik's cube animations */
            g_RubiksCube.Update(deltaTime);

            glm::mat4 view = camera.GetViewMatrix();
            glm::mat4 proj = camera.GetProjectionMatrix();
            glm::mat4 wholeCubeTransform = g_RubiksCube.GetWholeCubeTransform();
            const std::vector<Cubie>& cubies = g_RubiksCube.GetCubies();

            /* Color picking pass: when a pick was requested, draw with unique colors and read pixel */
            if (g_PendingPickX >= 0.0f && g_PendingPickY >= 0.0f)
            {
                GLCall(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
                GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
                shader.Bind();
                shader.SetUniform1i("u_PickingMode", 1);
                shader.SetUniform1i("u_UseFaceColors", 0);
                va.Bind();
                ib.Bind();
                for (int i = 0; i < static_cast<int>(cubies.size()); i++)
                {
                    if (i == 13) continue;  /* center cubie not drawn, skip in picking too */
                    glm::mat4 model;
                    if (g_RubiksCube.IsRotating() && g_RubiksCube.IsCubieInCurrentRotation(i))
                    {
                        glm::vec3 faceCenter = RubiksCube::GetFaceCenter(g_RubiksCube.GetCurrentWall());
                        glm::vec3 axis = g_RubiksCube.GetCurrentRotationAxis();
                        float signedAngle = g_RubiksCube.GetCurrentRotationAngle() * static_cast<float>(g_RubiksCube.GetCurrentRotationSign());
                        glm::mat4 toFace = glm::translate(glm::mat4(1.0f), faceCenter);
                        glm::mat4 R = glm::rotate(glm::mat4(1.0f), glm::radians(signedAngle), axis);
                        glm::mat4 fromFace = glm::translate(glm::mat4(1.0f), -faceCenter);
                        glm::mat4 toBase = glm::translate(glm::mat4(1.0f), cubies[i].GetBasePosition());
                        model = wholeCubeTransform * toFace * R * fromFace * toBase * g_RubiksCube.GetRotatingCubieStartTransform(i) * cubies[i].selectionTransform;
                    }
                    else
                        model = wholeCubeTransform * cubies[i].GetModelMatrix();
                    glm::mat4 mvp = proj * view * model;
                    /* Encode cubie index as unique color (skip 13 so center is never picked) */
                    float r = (float)((i + 1) & 0xFF) / 255.0f;
                    float g = (float)(((i + 1) >> 8) & 0xFF) / 255.0f;
                    float b = (float)(((i + 1) >> 16) & 0xFF) / 255.0f;
                    glm::vec4 pickColor(r, g, b, 1.0f);
                    shader.SetUniformMat4f("u_MVP", mvp);
                    shader.SetUniform4f("u_Color", pickColor);
                    GLCall(glDrawElements(GL_TRIANGLES, ib.GetCount(), GL_UNSIGNED_INT, nullptr));
                }
                int px = (int)g_PendingPickX;
                int py = (int)(height - 1 - g_PendingPickY);
                px = (px < 0) ? 0 : (px >= (int)width ? (int)width - 1 : px);
                py = (py < 0) ? 0 : (py >= (int)height ? (int)height - 1 : py);
                unsigned char pixel[4];
                GLCall(glReadPixels(px, py, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel));
                float depth;
                GLCall(glReadPixels(px, py, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth));
                int index = (int)pixel[0] + ((int)pixel[1] << 8) + ((int)pixel[2] << 16);
                index -= 1;
                if (index >= 0 && index < (int)cubies.size())
                {
                    g_SelectedCubie = index;
                    g_SelectedDepth = depth;
                    std::cout << "Picked cubie " << index << " (depth " << depth << ")" << std::endl;
                }
                else
                {
                    g_SelectedCubie = -1;
                }
                g_PendingPickX = -1.0f;
                g_PendingPickY = -1.0f;
            }
            
            /* Clear screen */
            GLCall(glClearColor(0.1f, 0.1f, 0.15f, 1.0f));
            GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
            
            /* Render: 6 draw calls per cube, each face has its own IBO and one color */
            shader.Bind();
            shader.SetUniform1i("u_PickingMode", 0);
            shader.SetUniform1i("u_UseFaceColors", 1);
            shader.SetUniform1i("u_Texture", 0);
            va.Bind();
            const glm::vec4 faceColors[6] = { RUBIK_FRONT, RUBIK_BACK, RUBIK_UP, RUBIK_DOWN, RUBIK_RIGHT, RUBIK_LEFT };
            /* u_FaceAxis: 0 = x constant (right/left), 1 = y constant (top/bottom), 2 = z constant (front/back) */
            const int faceAxis[6] = { 2, 2, 1, 1, 0, 0 };
            shader.SetUniform1f("u_EdgeWidth", 0.06f);

#if SINGLE_CUBE_TEST
            glm::mat4 model = wholeCubeTransform;
            glm::mat4 mvp = proj * view * model;
            shader.SetUniformMat4f("u_MVP", mvp);
            shader.SetUniform1i("u_Highlight", 0);
            for (int f = 0; f < 6; f++)
            {
                shader.SetUniform4f("u_ColorFace", faceColors[f]);
                shader.SetUniform1i("u_FaceAxis", faceAxis[f]);
                ibFaces[f]->Bind();
                GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));
            }
#else
            const int centerCubieIndex = 13;
            for (int i = 0; i < static_cast<int>(cubies.size()); i++)
            {
                if (i == centerCubieIndex)
                    continue;
                glm::mat4 model;
                if (g_RubiksCube.IsRotating() && g_RubiksCube.IsCubieInCurrentRotation(i))
                {
                    glm::vec3 faceCenter = RubiksCube::GetFaceCenter(g_RubiksCube.GetCurrentWall());
                    glm::vec3 axis = g_RubiksCube.GetCurrentRotationAxis();
                    float signedAngle = g_RubiksCube.GetCurrentRotationAngle() * static_cast<float>(g_RubiksCube.GetCurrentRotationSign());
                    glm::mat4 toFace = glm::translate(glm::mat4(1.0f), faceCenter);
                    glm::mat4 R = glm::rotate(glm::mat4(1.0f), glm::radians(signedAngle), axis);
                    glm::mat4 fromFace = glm::translate(glm::mat4(1.0f), -faceCenter);
                    glm::mat4 toBase = glm::translate(glm::mat4(1.0f), cubies[i].GetBasePosition());
                    model = wholeCubeTransform * toFace * R * fromFace * toBase * g_RubiksCube.GetRotatingCubieStartTransform(i) * cubies[i].selectionTransform;
                }
                else
                    model = wholeCubeTransform * cubies[i].GetModelMatrix();
                glm::mat4 mvp = proj * view * model;
                shader.SetUniformMat4f("u_MVP", mvp);
                shader.SetUniform1i("u_Highlight", (i == g_SelectedCubie) ? 1 : 0);
                for (int f = 0; f < 6; f++)
                {
                    shader.SetUniform4f("u_ColorFace", faceColors[f]);
                    shader.SetUniform1i("u_FaceAxis", faceAxis[f]);
                    ibFaces[f]->Bind();
                    GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));
                }
            }
#endif

            /* Swap buffers */
            glfwSwapBuffers(window);

            /* Poll events */
            glfwPollEvents();
        }
    }

    glfwTerminate();
    return 0;
}
