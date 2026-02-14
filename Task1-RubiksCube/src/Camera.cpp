#include <Camera.h>

void Camera::SetOrthographic(float near, float far)
{
    m_Near = near;
    m_Far = far;

    // Rest Projection and View matrices
    m_Projection = glm::ortho(m_Left, m_Right, m_Bottom, m_Top, near, far);
    m_View = glm::lookAt(m_Position, m_Position + m_Orientation, m_Up);
}

void Camera::SetPerspective(float fov, float near, float far)
{
    m_FOV = fov;
    m_Near = near;
    m_Far = far;
    
    float aspect = (float)m_Width / (float)m_Height;
    m_Projection = glm::perspective(glm::radians(fov), aspect, near, far);
    m_View = glm::lookAt(m_Position, m_Position + m_Orientation, m_Up);
}

void Camera::RotateView(float deltaX, float deltaY)
{
    // Rotate camera orientation based on mouse movement
    // This is a simple implementation - rotate around world axes
    glm::vec3 right = glm::normalize(glm::cross(m_Orientation, m_Up));
    
    // Horizontal rotation (around Y axis)
    float rotY = deltaX * 0.005f;
    m_Orientation = glm::rotate(m_Orientation, rotY, glm::vec3(0, 1, 0));
    
    // Vertical rotation (around right vector)
    float rotX = deltaY * 0.005f;
    m_Orientation = glm::rotate(m_Orientation, rotX, right);
    
    m_Orientation = glm::normalize(m_Orientation);
    m_View = glm::lookAt(m_Position, m_Position + m_Orientation, m_Up);
}

void Camera::Pan(float deltaX, float deltaY)
{
    // Move camera up/down/left/right
    glm::vec3 right = glm::normalize(glm::cross(m_Orientation, m_Up));
    
    float panSpeed = 0.01f;
    m_Position += right * deltaX * panSpeed;
    m_Position += m_Up * deltaY * panSpeed;
    
    m_View = glm::lookAt(m_Position, m_Position + m_Orientation, m_Up);
}

void Camera::Zoom(float delta)
{
    // Move camera forward/backward along view direction
    float zoomSpeed = 0.5f;
    m_Position += m_Orientation * delta * zoomSpeed;
    
    m_View = glm::lookAt(m_Position, m_Position + m_Orientation, m_Up);
}

void Camera::EnableInputs(GLFWwindow* window)
{
    // Set camera as the user pointer for the window
    glfwSetWindowUserPointer(window, this);
    
    // Note: Callback functions are now defined in main.cpp
    // to allow integration with RubiksCube
}
