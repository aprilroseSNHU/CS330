///////////////////////////////////////////////////////////////////////////////
// viewmanager.h
// ============
// Manage the viewing of 3D objects within the viewport
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//  Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

#include "ViewManager.h"

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>    

// Declaration of the global variables and defines
namespace
{
    // Variables for window width and height
    const int WINDOW_WIDTH = 1000;
    const int WINDOW_HEIGHT = 800;
    const char* g_ViewName = "view";
    const char* g_ProjectionName = "projection";

    // Camera object used for viewing and interacting with the 3D scene
    Camera* g_pCamera = nullptr;

    // These variables are used for mouse movement processing
    float gLastX = WINDOW_WIDTH / 2.0f;
    float gLastY = WINDOW_HEIGHT / 2.0f;
    bool gFirstMouse = true;

    // Time between current frame and last frame
    float gDeltaTime = 0.0f;
    float gLastFrame = 0.0f;

    // If orthographic projection is on, this value will be true
    bool bOrthographicProjection = false;

    // Camera speed
    float m_CameraSpeed = 1.0f;
}

/***********************************************************
 *  ViewManager()
 *
 *  The constructor for the class
 ***********************************************************/
ViewManager::ViewManager(ShaderManager* pShaderManager)
{
    // Initialize the member variables
    m_pShaderManager = pShaderManager;
    m_pWindow = nullptr;
    g_pCamera = new Camera();
    // Default camera view parameters
    g_pCamera->Position = glm::vec3(0.5f, 5.5f, 10.0f);
    g_pCamera->Front = glm::vec3(0.0f, -0.5f, -2.0f);
    g_pCamera->Up = glm::vec3(0.0f, 1.0f, 0.0f);
    g_pCamera->Zoom = 80;

    // Camera speed
    m_CameraSpeed = 1.0f;
}

/***********************************************************
 *  ~ViewManager()
 *
 *  The destructor for the class
 ***********************************************************/
ViewManager::~ViewManager()
{
    // Free up allocated memory
    m_pShaderManager = nullptr;
    m_pWindow = nullptr;
    if (g_pCamera != nullptr)
    {
        delete g_pCamera;
        g_pCamera = nullptr;
    }
}

/***********************************************************
 *  CreateDisplayWindow()
 *
 *  This method is used to create the main display window.
 ***********************************************************/
GLFWwindow* ViewManager::CreateDisplayWindow(const char* windowTitle)
{
    GLFWwindow* window = nullptr;

    // Try to create the displayed OpenGL window
    window = glfwCreateWindow(
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        windowTitle,
        nullptr, nullptr);
    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return nullptr;
    }
    glfwMakeContextCurrent(window);

    // This callback is used to receive mouse moving events
    glfwSetCursorPosCallback(window, &ViewManager::Mouse_Position_Callback);
    glfwSetScrollCallback(window, &ViewManager::Scroll);

    // Enable blending for supporting transparent rendering
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_pWindow = window;

    return window;
}

/***********************************************************
 *  Mouse_Position_Callback()
 *
 *  This method is automatically called from GLFW whenever
 *  the mouse is moved within the active GLFW display window.
 ***********************************************************/
void ViewManager::Mouse_Position_Callback(GLFWwindow* window, double xMousePos, double yMousePos)
{
    // Increase or decrease movement speed for scroll
    xMousePos *= m_CameraSpeed;
    yMousePos *= m_CameraSpeed;

    // When the first mouse move event is received, this needs to be recorded so that
    // all subsequent mouse moves can correctly calculate the X position offset and Y
    // position offset for proper operation
    if (gFirstMouse)
    {
        gLastX = xMousePos;
        gLastY = yMousePos;
        gFirstMouse = false;
    }

    // Calculate the X offset and Y offset values for moving the 3D camera accordingly
    float xOffset = xMousePos - gLastX;
    float yOffset = gLastY - yMousePos; // Reversed since y-coordinates go from bottom to top

    // Set the current positions into the last position variables
    gLastX = xMousePos;
    gLastY = yMousePos;

    // Move the 3D camera according to the calculated offsets
    g_pCamera->ProcessMouseMovement(xOffset, yOffset);
}

/***********************************************************
 *  ProcessKeyboardEvents()
 *
 *  This method is called to process any keyboard events
 *  that may be waiting in the event queue.
 ***********************************************************/
void ViewManager::ProcessKeyboardEvents()
{
    // Close the window if the escape key has been pressed
    if (glfwGetKey(m_pWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(m_pWindow, true);
    }

    // If the camera object is null, then exit this method
    if (g_pCamera == nullptr)
    {
        return;
    }

    // Process camera movement
    float velocity = gDeltaTime * m_CameraSpeed;
    if (glfwGetKey(m_pWindow, GLFW_KEY_W) == GLFW_PRESS)
    {
        g_pCamera->ProcessKeyboard(FORWARD, velocity);
    }
    if (glfwGetKey(m_pWindow, GLFW_KEY_S) == GLFW_PRESS)
    {
        g_pCamera->ProcessKeyboard(BACKWARD, velocity);
    }
    if (glfwGetKey(m_pWindow, GLFW_KEY_A) == GLFW_PRESS)
    {
        g_pCamera->ProcessKeyboard(LEFT, velocity);
    }
    if (glfwGetKey(m_pWindow, GLFW_KEY_D) == GLFW_PRESS)
    {
        g_pCamera->ProcessKeyboard(RIGHT, velocity);
    }
    if (glfwGetKey(m_pWindow, GLFW_KEY_Q) == GLFW_PRESS)
    {
        g_pCamera->ProcessKeyboard(UP, velocity);
    }
    if (glfwGetKey(m_pWindow, GLFW_KEY_E) == GLFW_PRESS)
    {
        g_pCamera->ProcessKeyboard(DOWN, velocity);
    }

    // fro 2D
    if (glfwGetKey(m_pWindow, GLFW_KEY_1) == GLFW_PRESS)
    {
        std::cout << "Pressed 1" << std::endl;

        bOrthographicProjection = true;
    }
    // for 3D
    if (glfwGetKey(m_pWindow, GLFW_KEY_2) == GLFW_PRESS)
    {
        bOrthographicProjection = false;
    }
}

/***********************************************************
 *  Scroll()
 *
 *  This method is called when the mouse scroll wheel is used.
 ***********************************************************/
void ViewManager::Scroll(GLFWwindow* window, double xoffset, double yoffset)
{
    m_CameraSpeed += static_cast<float>(yoffset) * 0.1f;
    m_CameraSpeed = fmaxf(0.1f, m_CameraSpeed);
}

/***********************************************************
 *  PrepareSceneView()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene
 *  rendering
 ***********************************************************/
void ViewManager::PrepareSceneView()
{
    glm::mat4 view;
    glm::mat4 projection;

    // Per-frame timing
    float currentFrame = glfwGetTime();
    gDeltaTime = currentFrame - gLastFrame;
    gLastFrame = currentFrame;

    // Process any keyboard events that may be waiting in the event queue
    ProcessKeyboardEvents();

    // Get the current view matrix from the camera
    view = g_pCamera->GetViewMatrix();

    // Define the current projection matrix
    projection = glm::perspective(glm::radians(g_pCamera->Zoom), 
                                  (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 
                                  0.1f, 100.0f);

    // If the shader manager object is valid
    if (m_pShaderManager != nullptr)
    {
        // Set the view matrix into the shader for proper rendering
        m_pShaderManager->setMat4Value(g_ViewName, view);
        // Set the projection matrix into the shader for proper rendering
        m_pShaderManager->setMat4Value(g_ProjectionName, projection);
        // Set the view position of the camera into the shader for proper rendering
        m_pShaderManager->setVec3Value("viewPosition", g_pCamera->Position);
    }
}
