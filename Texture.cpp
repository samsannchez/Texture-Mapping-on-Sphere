
using namespace std;
#include <iostream>

#define _USE_MATH_DEFINES
#include <cmath>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

const float radius = 1.0f;
static float angleX = 0.0f;
static float angleY = 0.0f;

// timing
float deltaTime = 0.0f; // time between current frame and last frame
float lastFrame = 0.0f;

typedef struct {
    vector <float> positions;
    vector <float> normals;
    vector <float> texCoords;
    vector <unsigned int> indices;
} SphereData;

SphereData genSphereData(float radius, int rows, int cols) {
    float PI = float(M_PI);
    SphereData sp;

    for (int i = 0; i <= rows; i++) {
        float t = float(i) / float(rows);
        float theta = PI - t * PI;
        float y = float(cos(theta));
        float sinAngle = float(sin(theta));
        for (int j = 0; j <= cols; j++) {
            float s = float(j) / float(cols);
            float phi = 2. * s * PI;
            float x = -float(cos(phi)) * sinAngle;
            float z = float(sin(phi)) * sinAngle;
            sp.positions.push_back(radius * x); sp.positions.push_back(radius * y); sp.positions.push_back(radius * z);
            sp.normals.push_back(x); sp.normals.push_back(y); sp.normals.push_back(z);
            sp.texCoords.push_back(s); sp.texCoords.push_back(t);
        }
    }
    // calculate triangle indices
    for (int row = 1; row <= rows; row++) {
        for (int col = 0; col < cols; col++) {
            int index1 = (row - 1) * (cols + 1) + col,
                index2 = row * (cols + 1) + col,
                index3 = row * (cols + 1) + (col + 1);
            sp.indices.push_back(index1);
            sp.indices.push_back(index2);
            sp.indices.push_back(index3);

            index2 = index3;
            index3 = (row - 1) * (cols + 1) + (col + 1);
            sp.indices.push_back(index1);
            sp.indices.push_back(index2);
            sp.indices.push_back(index3);
        }
    }
    return sp;
}
 unsigned int  createArrayObject(unsigned int& nElements)
{
    SphereData sp = genSphereData(radius, 16, 32);
    nElements = sp.indices.size();

    unsigned int sphereVAO;
    unsigned int positionVBO, sphereEBO, normalVBO, texVBO;

    glGenVertexArrays(1, &sphereVAO);
    glBindVertexArray(sphereVAO);

    glGenBuffers(1, &positionVBO);
    glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
    glBufferData(GL_ARRAY_BUFFER, sp.positions.size() * sizeof(float), sp.positions.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &normalVBO);
    glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
    glBufferData(GL_ARRAY_BUFFER, sp.normals.size() * sizeof(float), sp.normals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &texVBO);
    glBindBuffer(GL_ARRAY_BUFFER, texVBO);
    glBufferData(GL_ARRAY_BUFFER, sp.texCoords.size() * sizeof(float), sp.texCoords.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(2);


    glGenBuffers(1, &sphereEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sp.indices.size() * sizeof(unsigned int), sp.indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);

    return sphereVAO;
}


unsigned int getTexture(const char *textureFileName){
    unsigned int texture;
    // texture 1
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // load image, create texture and generate mipmaps
    int width, height, nChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load("/Users/samsannchez/Desktop/Fall2022/COP5725/OpenGL/OpenGL/Texture/earth.jpeg", &width, &height, &nChannels, 0);
    if (data)
    {
        GLenum format = 0;
        if (nChannels == 1)
            format = GL_RED;
        else if (nChannels == 3)
            format = GL_RGB;
        else if (nChannels == 4)
            format = GL_RGBA;

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        std::cout << textureFileName << " Loaded successfully. Its size is: " << width << "x" << height << " and the number of channels are " << nChannels << std::endl;
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    return texture;
}

const char* vsShaderSource = R"(#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

uniform mat4 view;
uniform mat4 projection;

out vec2 fragUV;
out vec3 fragNormal;

void main () {
    gl_Position =  projection * view * vec4(position, 1.0);
    fragNormal = normalize(normal);
    fragUV = texCoord;
}
)";
const char* fsShaderSource = R"(#version 330 core

in vec2  fragUV;
in vec3 fragNormal;

uniform sampler2D tex;

out vec4 outColor;

void main() {
    vec3 N = normalize(fragNormal);
    outColor = vec4(texture(tex,fragUV));
}
)";
//    outColor = vec4(abs(N)*texture(tex, fragUV).rgb,1.0);


unsigned int createShader(const char* shaderSource, unsigned int shaderType, const char* msg) {
    unsigned int shaderObj = glCreateShader(shaderType);
    int length = (int)strlen(shaderSource);
    glShaderSource(shaderObj, 1, &shaderSource, &length);
    glCompileShader(shaderObj);

    // compile vertex shader and error check
    int success;
    glGetShaderiv(shaderObj, GL_COMPILE_STATUS, &success);

    if (success == GL_FALSE) {
        const int maxSize = 1024;
        char message[maxSize];
        glGetShaderInfoLog(shaderObj, maxSize, NULL, message);
        std::cout << "Failed to compile: " << msg << std::endl;
        std::cout << message << std::endl;
        glDeleteShader(shaderObj);
    }
    else {
        std::cout << msg << " Successfully complied."<<std::endl;
    }

    return shaderObj;
}
unsigned int createProgram(const char *vsShaderSource, const char* fsShaderSource) {
    unsigned int vertexShaderObj = createShader(vsShaderSource, GL_VERTEX_SHADER, "Sphere Vertex Shader");
    unsigned int fragmentShaderObj = createShader(fsShaderSource, GL_FRAGMENT_SHADER, "Sphere Fragment Shader");
    
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShaderObj);
    glAttachShader(shaderProgram, fragmentShaderObj);
    glLinkProgram(shaderProgram);

    // delete vertex and fragment shaders
    glDeleteShader(vertexShaderObj);
    glDeleteShader(fragmentShaderObj);

    int success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);

    // error check
    if (success == GL_FALSE) {
        const int messageSize=1024;
        char message[messageSize];
        glGetProgramInfoLog(shaderProgram,messageSize, NULL, message);
        std::cout << "Failed to link shaders:" << std::endl;
        std::cout << message << std::endl;
    }
    else {
        std::cout << "Shader program Successfully Created." << std::endl;
    }

    // validate program
    glValidateProgram(shaderProgram);

    return shaderProgram;
}



// process input: check for escape key being pressed
void key_callback(GLFWwindow* window, int key, int keycode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);

    float cameraSpeed = static_cast<float>(50 * deltaTime);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        angleY += cameraSpeed;
        if (angleY > 89.0f) // avoid singularity condition
            angleY = 89.0f;
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        angleY -=  cameraSpeed;
        if (angleY < -89.0f) // avoid singularity condition
            angleY = -89.0f;
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        angleX += cameraSpeed;
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        angleX -= cameraSpeed;
    }

}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

glm::mat4 getViewMatrix() {
    glm::mat4 rot = glm::mat4(1.0);
    // rotate sphere on X and Y axis
    rot = glm::rotate(rot, glm::radians(angleX), glm::vec3(0.0f, 1.0f, 0.0f));
    rot = glm::rotate(rot, glm::radians(angleY), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::vec4 viewVector = rot*glm::vec4(0, 0, radius*5,0.);
    return glm::lookAt(glm::vec3(viewVector.x, viewVector.y, viewVector.z), glm::vec3(0,0,0), glm::vec3(0,1,0));
}
/// returns the created window
GLFWwindow* initializeGL(int SCR_WIDTH, int SCR_HEIGHT)
{
    // initialize glfw, use opengl 3.3 plus core-profile
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    // create a window object and do a null check
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Textures Sphere", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return NULL;
    }

    // set the opengl context for the window to be the current context
    glfwMakeContextCurrent(window);

    // set the callback function for resizing the viewport when window is resized
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glfwSetKeyCallback(window, key_callback);

    // initialize GLEW
    if (glewInit() != GLEW_OK)
    {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return NULL;
    }

    return window;
}
void render(unsigned int sphereProgram, unsigned int VAO, glm::mat4 view, glm::mat4 projection, int nElements, unsigned int texture) {
    glUseProgram(sphereProgram);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(glGetUniformLocation(sphereProgram, "tex"), 0);
    glUniformMatrix4fv(glGetUniformLocation(sphereProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(sphereProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
 
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, nElements, GL_UNSIGNED_INT, (void*)0);

    glUseProgram(0);
}
int main(void) {
  
    int width = 640, height = 640;
    GLFWwindow* window = initializeGL(width, height);
    
    
    glClearColor(0.1f, 0.0f, 0.1f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    unsigned int sphereShaderProgram = createProgram(vsShaderSource, fsShaderSource);

    unsigned int numIndices;
    
    unsigned int sphereVAO = createArrayObject(numIndices);
    
    // load and create a texture
    unsigned int texture = getTexture("/Users/samsannchez/Desktop/Fall2022/COP5725/OpenGL/OpenGL/Texture/earth.jpeg");

    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 10.0f);
        
    // render loop
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
    
        processInput(window);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // also clear the depth buffer now!
        glm::mat4 view = getViewMatrix();

        render(sphereShaderProgram, sphereVAO, view, projection, numIndices, texture);
        
        glfwSwapBuffers(window);
        
        // poll for events
        glfwPollEvents();
    }
    
    // optional: de-allocate all resources once they've outlived their purpose:
    glDeleteVertexArrays(1, &sphereVAO);
    glDeleteProgram(sphereShaderProgram);
    
    // glfw: destroy window and terminate
    glfwDestroyWindow(window);
    glfwTerminate();
    
    exit(EXIT_SUCCESS);
}
