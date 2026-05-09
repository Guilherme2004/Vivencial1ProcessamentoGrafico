#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <random>
#include <iostream>

struct Vertex {
    glm::vec2 position;
    glm::vec3 color;
};

std::vector<Vertex> todosVertices;
bool bufferPrecisaAtualizar = false;

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<float> dist(0.0f, 1.0f);

const char* vertexShaderSource = R"glsl(
    #version 330 core
    layout (location = 0) in vec2 aPos;
    layout (location = 1) in vec3 aColor;

    out vec3 ourColor;
    uniform mat4 projection;

    void main() {
        gl_Position = projection * vec4(aPos, 0.0, 1.0);
        gl_PointSize = 5.0;
        ourColor = aColor;
    }
)glsl";

const char* fragmentShaderSource = R"glsl(
    #version 330 core
    out vec4 FragColor;
    in vec3 ourColor;

    void main() {
        FragColor = vec4(ourColor, 1.0f);
    }
)glsl";

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        Vertex novoVertice;
        novoVertice.position = glm::vec2((float)xpos, (float)ypos);
        novoVertice.color = glm::vec3(1.0f, 1.0f, 1.0f);
        
        todosVertices.push_back(novoVertice);

        if (todosVertices.size() % 3 == 0) {
            float r = dist(gen);
            float g = dist(gen);
            float b = dist(gen);
            
            size_t n = todosVertices.size();
            todosVertices[n-1].color = glm::vec3(r, g, b);
            todosVertices[n-2].color = glm::vec3(r, g, b);
            todosVertices[n-3].color = glm::vec3(r, g, b);
        }

        bufferPrecisaAtualizar = true;
    }
}

unsigned int compilarShader(unsigned int tipo, const char* fonte) {
    unsigned int shader = glCreateShader(tipo);
    glShaderSource(shader, 1, &fonte, NULL);
    glCompileShader(shader);
    
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    return shader;
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    int largura = 800;
    int altura = 600;
    GLFWwindow* window = glfwCreateWindow(largura, altura, "Atividade Vivencial - Guilherme Rosa, Fabier Brumimg da Silva, Mateus de Moule", NULL, NULL);
    if (window == NULL) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        return -1;
    }

    glEnable(GL_PROGRAM_POINT_SIZE);

    unsigned int vertexShader = compilarShader(GL_VERTEX_SHADER, vertexShaderSource);
    unsigned int fragmentShader = compilarShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
    glEnableVertexAttribArray(1);

    glm::mat4 projection = glm::ortho(0.0f, (float)largura, (float)altura, 0.0f, -1.0f, 1.0f);

    while (!glfwWindowShouldClose(window)) {
        glfwGetWindowSize(window, &largura, &altura);
        glViewport(0, 0, largura, altura);
        projection = glm::ortho(0.0f, (float)largura, (float)altura, 0.0f, -1.0f, 1.0f);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        if (bufferPrecisaAtualizar) {
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, todosVertices.size() * sizeof(Vertex), todosVertices.data(), GL_DYNAMIC_DRAW);
            bufferPrecisaAtualizar = false;
        }

        glUseProgram(shaderProgram);
        unsigned int projLoc = glGetUniformLocation(shaderProgram, "projection");
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        glBindVertexArray(VAO);

        int totalVertices = todosVertices.size();
        int verticesCompletos = (totalVertices / 3) * 3;
        int verticesPendentes = totalVertices % 3;

        if (verticesCompletos > 0) {
            glDrawArrays(GL_TRIANGLES, 0, verticesCompletos);
        }
        
        if (verticesPendentes > 0) {
            glDrawArrays(GL_POINTS, verticesCompletos, verticesPendentes);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}