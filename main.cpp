// Autor: Bosa Boskovic
 
#include <iostream>
#include <fstream>
#include <sstream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

//GLM biblioteke
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Util.h"
#include "cooking.h"
#include "model.h"
#include "AssemblyLogic.h"
#include "ketchup.h"
#include "splash.h"
#include "senf.h"
#include "Name.h"

enum AppState { MENU, COOKING, ASSEMBLY, FINISHED };
AppState currentState = MENU;

float buttonLeft = -0.2f, buttonRight = 0.2f;
float buttonBottom = -0.1f, buttonTop = 0.1f;

float stoveTopY = -0.5f + (0.1f * 2.5f); // ≈ -0.25
float pattyHalfHeight = 0.05f;
float pattyRestY = stoveTopY + pattyHalfHeight;

bool useTex = false;
bool transparent = false;
// Parametri kamere
glm::vec3 cameraPos = glm::vec3(0.0f, 0.5f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float yaw = -90.0f; // Rotacija levo-desno
float pitch = 0.0f;  // Rotacija gore-dole
float lastX = 500, lastY = 500; // Centar ekrana
bool firstMouse = true;

// Broj segmenata za krug (što više, to je valjak "okrugliji")
const int STRIDE = 10; // X, Y, Z, R, G, B, A, Nx, Ny, Nz (bez tekstura za sada)
const int segments = 30;

glm::vec3 bottomBunPos = glm::vec3(0.0f, 0.2f, 0.0f);

glm::vec3 lightPos = glm::vec3(2.0f, 3.0f, 2.0f);
glm::vec3 lightColor = glm::vec3(1.0f, 0.8f, 0.9f); // blago roze
float lightIntensity = 1.2f;

bool lightEnabled = true;
bool depthTestEnabled = true;  // Default stanje
bool cullFaceEnabled = false; // Default stanje

// Pozicija pljeskavice (WASD)
glm::vec3 pattyPos = glm::vec3(0.0f, 0.0f, 0.0f);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    // Odmah zatvara prozor na pritisak ESC
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    if (key == GLFW_KEY_G && action == GLFW_PRESS) {
        useTex = !useTex;
    }
    if (key == GLFW_KEY_T && action == GLFW_PRESS) {
        transparent = !transparent;
    }
    if (key == GLFW_KEY_L && action == GLFW_PRESS)
    {
        lightEnabled = !lightEnabled;
    }
    if (action == GLFW_PRESS) {
        switch (key) {
        case GLFW_KEY_1:
            depthTestEnabled = true;
            break;
        case GLFW_KEY_2:
            depthTestEnabled = false;
            break;
        case GLFW_KEY_3:
            cullFaceEnabled = true;
            break;
        case GLFW_KEY_4:
            cullFaceEnabled = false;
            break;
        }
    }
}

unsigned int preprocessTexture(const char* filepath) {
    unsigned int texture = loadImageToTexture(filepath); // Učitavanje teksture
    glBindTexture(GL_TEXTURE_2D, texture); // Vezujemo se za teksturu kako bismo je podesili

    // Generisanje mipmapa - predefinisani različiti formati za lakše skaliranje po potrebi (npr. da postoji 32 x 32 verzija slike, ali i 16 x 16, 256 x 256...)
    glGenerateMipmap(GL_TEXTURE_2D);

    // Podešavanje strategija za wrap-ovanje - šta da radi kada se dimenzije teksture i poligona ne poklapaju
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // S - tekseli po x-osi
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // T - tekseli po y-osi

    // Podešavanje algoritma za smanjivanje i povećavanje rezolucije: nearest - bira najbliži piksel, linear - usrednjava okolne piksele
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    return texture;
}
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // Obrnuto jer y ide od dole na gore
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (currentState == MENU && button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        int width, height;
        glfwGetWindowSize(window, &width, &height);

        // Konverzija koordinata miša (0 do Width) u NDC (-1 do 1)
        float mouseX = (2.0f * xpos) / width - 1.0f;
        float mouseY = 1.0f - (2.0f * ypos) / height;

        // Provera da li je klik unutar kvadrata
        if (mouseX >= buttonLeft && mouseX <= buttonRight && mouseY >= buttonBottom && mouseY <= buttonTop) {
            currentState = COOKING;
            // Kada pređemo u igru, sakrijemo kursor za FPS kontrolu
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
}

void updateCamera(GLuint shader, GLuint viewLoc) {
    glm::mat4 view = glm::lookAt(
        cameraPos,
        cameraPos + cameraFront,
        cameraUp
    );
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
}


float pattyVertices[1500];

void generatePatty() {
    float radius = 0.2f;
    float height = 0.05f;
    int idx = 0;
    //roza
    float r = 1.0f, g = 0.7f, b = 0.75f, a = 1.0f;

    // 1. OMOTAČ (Triangle Strip)
    for (int i = 0; i <= segments; i++) {
        float angle = (float)i / segments * 2.0f * 3.14159f;
        float x = cos(angle) * radius;
        float z = sin(angle) * radius;

        // Gornja tačka
        pattyVertices[idx++] = x; pattyVertices[idx++] = height; pattyVertices[idx++] = z; // Pos
        pattyVertices[idx++] = r; pattyVertices[idx++] = g; pattyVertices[idx++] = b; pattyVertices[idx++] = a; // Color
        pattyVertices[idx++] = x; pattyVertices[idx++] = 0.0f; pattyVertices[idx++] = z; // Normal (gleda ka vani)

        // Donja tačka
        pattyVertices[idx++] = x; pattyVertices[idx++] = -height; pattyVertices[idx++] = z; // Pos
        pattyVertices[idx++] = r; pattyVertices[idx++] = g; pattyVertices[idx++] = b; pattyVertices[idx++] = a; // Color
        pattyVertices[idx++] = x; pattyVertices[idx++] = 0.0f; pattyVertices[idx++] = z; // Normal
    }

    // 2. GORNJI POKLOPAC (Triangle Fan)
    pattyVertices[idx++] = 0.0f; pattyVertices[idx++] = height; pattyVertices[idx++] = 0.0f; // Centar
    pattyVertices[idx++] = r; pattyVertices[idx++] = g; pattyVertices[idx++] = b; pattyVertices[idx++] = a;
    pattyVertices[idx++] = 0.0f; pattyVertices[idx++] = 1.0f; pattyVertices[idx++] = 0.0f; // Normal gore

    for (int i = 0; i <= segments; i++) {
        float angle = (float)i / segments * 2.0f * 3.14159f;
        pattyVertices[idx++] = cos(angle) * radius;
        pattyVertices[idx++] = height;
        pattyVertices[idx++] = sin(angle) * radius;
        pattyVertices[idx++] = r; pattyVertices[idx++] = g; pattyVertices[idx++] = b; pattyVertices[idx++] = a;
        pattyVertices[idx++] = 0.0f; pattyVertices[idx++] = 1.0f; pattyVertices[idx++] = 0.0f;
    }

    // 3. DONJI POKLOPAC (Triangle Fan)
    pattyVertices[idx++] = 0.0f; pattyVertices[idx++] = -height; pattyVertices[idx++] = 0.0f; // Centar
    pattyVertices[idx++] = r; pattyVertices[idx++] = g; pattyVertices[idx++] = b; pattyVertices[idx++] = a;
    pattyVertices[idx++] = 0.0f; pattyVertices[idx++] = -1.0f; pattyVertices[idx++] = 0.0f; // Normal dole

    for (int i = 0; i <= segments; i++) {
        float angle = (float)i / segments * 2.0f * 3.14159f;
        pattyVertices[idx++] = cos(angle) * radius;
        pattyVertices[idx++] = -height;
        pattyVertices[idx++] = sin(angle) * radius;
        pattyVertices[idx++] = r; pattyVertices[idx++] = g; pattyVertices[idx++] = b; pattyVertices[idx++] = a;
        pattyVertices[idx++] = 0.0f; pattyVertices[idx++] = -1.0f; pattyVertices[idx++] = 0.0f;
    }
}

int main(void)
{
    if (!glfwInit())
    {
        std::cout<<"GLFW Biblioteka se nije ucitala! :(\n";
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window;
    const char wTitle[] = "Brza hrana";
    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);

    unsigned int wWidth = mode->width;
    unsigned int wHeight = mode->height;

    window = glfwCreateWindow(wWidth, wHeight, wTitle, primaryMonitor, NULL);
    glfwSetKeyCallback(window, keyCallback);
    
    if (window == NULL)
    {
        std::cout << "Prozor nije napravljen! :(\n";
        glfwTerminate();
        return 2;
    }
    
    glfwMakeContextCurrent(window);

    glfwSetCursorPosCallback(window, mouse_callback); // Povezuje funkciju za miš
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Zaključava miš u centar
    
    if (glewInit() != GLEW_OK)
    {
        std::cout << "GLEW nije mogao da se ucita! :'(\n";
        return 3;
    }
    glfwSwapInterval(0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++ PROMJENLJIVE I BAFERI +++++++++++++++++++++++++++++++++++++++++++++++++
    
    unsigned int unifiedShader = createShader("basic.vert", "basic.frag");
    glUseProgram(unifiedShader);
    glUniform1i(glGetUniformLocation(unifiedShader, "uTex"), 0);
    unsigned int ignoreLightLoc = glGetUniformLocation(unifiedShader, "ignoreLight");
    unsigned int useVertexColorLoc = glGetUniformLocation(unifiedShader, "useVertexColor");

    unsigned int dice[6] = { };
    for (int i = 0; i < 6; ++i) {
        std::string path = "res/dice" + std::to_string(i + 1) + ".png";
        dice[i] = preprocessTexture(path.c_str());
    }
    GLuint ringlaTex = preprocessTexture("res/ringlaa.jpg");
    GLuint splashTexture = preprocessTexture("res/flekaKecapa1.png");
    GLuint senfTexture = preprocessTexture("res/senfMrlja1.png");
    GLuint krajTexture = preprocessTexture("res/kraj.png");

    float vertices[] =
    {
        //X    Y    Z      R    G    B    A          S   T      Nx Ny Nz
          // Prednja strana - CRNA
           0.1, 0.1, 0.1,   0.0, 0.0, 0.0, 1.0,      0,  0,    0, 0, 1,
          -0.1, 0.1, 0.1,   0.0, 0.0, 0.0, 1.0,      1,  0,    0, 0, 1,
          -0.1,-0.1, 0.1,   0.0, 0.0, 0.0, 1.0,      1,  1,    0, 0, 1,
           0.1,-0.1, 0.1,   0.0, 0.0, 0.0, 1.0,      0,  1,    0, 0, 1,

           // Leva strana - CRNA
           -0.1, 0.1, 0.1,   0.0, 0.0, 0.0, 1.0,      0,  0,    -1, 0, 0,
           -0.1, 0.1,-0.1,   0.0, 0.0, 0.0, 1.0,      1,  0,    -1, 0, 0,
           -0.1,-0.1,-0.1,   0.0, 0.0, 0.0, 1.0,      1,  1,    -1, 0, 0,
           -0.1,-0.1, 0.1,   0.0, 0.0, 0.0, 1.0,      0,  1,    -1, 0, 0,

           // Donja strana - CRNA
            0.1,-0.1, 0.1,   0.0, 0.0, 0.0, 1.0,      0,  0,    0, -1, 0,
           -0.1,-0.1, 0.1,   0.0, 0.0, 0.0, 1.0,      1,  0,    0, -1, 0,
           -0.1,-0.1,-0.1,   0.0, 0.0, 0.0, 1.0,      1,  1,    0, -1, 0,
            0.1,-0.1,-0.1,   0.0, 0.0, 0.0, 1.0,      0,  1,    0, -1, 0,

            // Gornja strana - BIJELA
             0.1, 0.1, 0.1,   1.0, 1.0, 1.0, 1.0,      0,  0,    0, 1, 0,
             0.1, 0.1,-0.1,   1.0, 1.0, 1.0, 1.0,      1,  0,    0, 1, 0,
            -0.1, 0.1,-0.1,   1.0, 1.0, 1.0, 1.0,      1,  1,    0, 1, 0,
            -0.1, 0.1, 0.1,   1.0, 1.0, 1.0, 1.0,      0,  1,    0, 1, 0,

            // Desna strana - CRNA
             0.1, 0.1, 0.1,   0.0, 0.0, 0.0, 1.0,      0,  0,    1, 0, 0,
             0.1,-0.1, 0.1,   0.0, 0.0, 0.0, 1.0,      1,  0,    1, 0, 0,
             0.1,-0.1,-0.1,   0.0, 0.0, 0.0, 1.0,      1,  1,    1, 0, 0,
             0.1, 0.1,-0.1,   0.0, 0.0, 0.0, 1.0,      0,  1,    1, 0, 0,

             // Zadnja strana - CRNA
              0.1, 0.1,-0.1,   0.0, 0.0, 0.0, 1.0,      0,  0,    0, 0, -1,
              0.1,-0.1,-0.1,   0.0, 0.0, 0.0, 1.0,      1,  0,    0, 0, -1,
             -0.1,-0.1,-0.1,   0.0, 0.0, 0.0, 1.0,      1,  1,    0, 0, -1,
             -0.1, 0.1,-0.1,   0.0, 0.0, 0.0, 1.0,      0,  1,    0, 0, -1,
    };

    float barVertices[] = {
        // Pozicije (X, Y) - NDC koordinate (-1 do 1)
        -0.5f,  0.9f,  // Top left
        -0.5f,  0.85f, // Bottom left
         0.5f,  0.85f, // Bottom right
         0.5f,  0.9f   // Top right
    };
    // 1. Dugme u centru ekrana za MENU
    float buttonVertices[] = {
    -0.1f,  0.05f,  // Top-left
    -0.1f, -0.05f,  // Bottom-left
     0.1f, -0.05f,  // Bottom-right
     0.1f,  0.05f   // Top-right
    };

    float imageVertices[] = {
    -0.5f,  0.5f, 0.0f, 0.0f, 1.0f,
    -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
     0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
     0.5f,  0.5f, 0.0f, 1.0f, 1.0f
    };




    unsigned int stride = (3 + 4 + 2 + 3) * sizeof(float); 
    
    unsigned int imageVAO, imageVBO;
    glGenVertexArrays(1, &imageVAO);
    glGenBuffers(1, &imageVBO);
    glBindVertexArray(imageVAO);
    glBindBuffer(GL_ARRAY_BUFFER, imageVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(imageVertices), imageVertices, GL_STATIC_DRAW);

    // POSITION (vec3)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // TEX COORDS (vec2)
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);


    glBindVertexArray(0);


    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    unsigned int VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(7 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void*)(10 * sizeof(float)));
    glEnableVertexAttribArray(3);

    unsigned int barVAO, barVBO;
    glGenVertexArrays(1, &barVAO);
    glGenBuffers(1, &barVBO);
    glBindVertexArray(barVAO);
    glBindBuffer(GL_ARRAY_BUFFER, barVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(barVertices), barVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    unsigned int buttonVAO, buttonVBO;
    glGenVertexArrays(1, &buttonVAO);
    glGenBuffers(1, &buttonVBO);
    glBindVertexArray(buttonVAO);
    glBindBuffer(GL_ARRAY_BUFFER, buttonVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(buttonVertices), buttonVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    generatePatty();
    unsigned int pattyVAO, pattyVBO;
    glGenVertexArrays(1, &pattyVAO);
    glGenBuffers(1, &pattyVBO);

    glBindVertexArray(pattyVAO);
    glBindBuffer(GL_ARRAY_BUFFER, pattyVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pattyVertices), pattyVertices, GL_STATIC_DRAW);

    // Layout: Pos(3), Color(4), Normals(3) -> Stride = 10 * float
    unsigned int pStride = 10 * sizeof(float);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, pStride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, pStride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, pStride, (void*)(7 * sizeof(float)));
    glEnableVertexAttribArray(3);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Setup kečapa
    int ketchupVertexCount = getKetchupVertexCount(segments);
    float* ketchupVertices = new float[ketchupVertexCount * 10]; // 10 floatova po vertexu
    generateKetchup(ketchupVertices, segments);
    unsigned int ketchupVAO = setupKetchupBuffers(ketchupVertices, ketchupVertexCount);
    delete[] ketchupVertices; // Oslobodi memoriju posle upload-a na GPU
    
    //setup senfa
    int senfVertexCount = getSenfVertexCount(segments);
    float* senfVertices = new float[senfVertexCount * 10];
    generateSenf(senfVertices, segments);
    unsigned int senfVAO = setupSenfBuffers(senfVertices, senfVertexCount);
    delete[] senfVertices;

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++            UNIFORME            +++++++++++++++++++++++++++++++++++++++++++++++++

    glm::mat4 model = glm::mat4(1.0f); //Matrica transformacija - mat4(1.0f) generise jedinicnu matricu
    unsigned int modelLoc = glGetUniformLocation(unifiedShader, "uM");
    
    glm::mat4 pattyModel = glm::mat4(1.0f);
    pattyModel = glm::translate(pattyModel, pattyPos); // Koristi našu pattyPos varijablu
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(pattyModel));

    glm::mat4 view; //Matrica pogleda (kamere)
    view = glm::lookAt(glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // lookAt(Gdje je kamera, u sta kamera gleda, jedinicni vektor pozitivne Y ose svijeta  - ovo rotira kameru)
    unsigned int viewLoc = glGetUniformLocation(unifiedShader, "uV");
    
    
    glm::mat4 projectionP = glm::perspective(glm::radians(90.0f), (float)wWidth / (float)wHeight, 0.1f, 100.0f); //Matrica perspektivne projekcije (FOV, Aspect Ratio, prednja ravan, zadnja ravan)
    glm::mat4 projectionO = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 100.0f); //Matrica ortogonalne projekcije (Lijeva, desna, donja, gornja, prednja i zadnja ravan)
    unsigned int projectionLoc = glGetUniformLocation(unifiedShader, "uP");

    CookingState pattyCook;
    unsigned int colorModLoc = glGetUniformLocation(unifiedShader, "uColorMod");

    bool mustardActive = false;
    glm::vec4 mustardColor = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f); // Žuta
    bool ketchupPlaced = false;

    // Dobijanje lokacija uniforma za svetlo
    unsigned int lightPosLoc = glGetUniformLocation(unifiedShader, "lightPos");
    unsigned int lightColorLoc = glGetUniformLocation(unifiedShader, "lightColor");
    unsigned int lightIntensityLoc = glGetUniformLocation(unifiedShader, "lightIntensity");
    unsigned int lightEnabledLoc = glGetUniformLocation(unifiedShader, "lightEnabled");
    unsigned int viewPosLoc = glGetUniformLocation(unifiedShader, "viewPos");

    // Pošalji inicijalne vrednosti
    glUseProgram(unifiedShader);
    glUniform3fv(lightPosLoc, 1, glm::value_ptr(lightPos));
    glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
    glUniform1f(lightIntensityLoc, lightIntensity);
    glUniform1i(lightEnabledLoc, lightEnabled ? 1 : 0);
    glUniform3fv(viewPosLoc, 1, glm::value_ptr(cameraPos));


    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++ RENDER LOOP - PETLJA ZA CRTANJE +++++++++++++++++++++++++++++++++++++++++++++++++
    
    
    

    glUseProgram(unifiedShader); //Slanje default vrijednosti uniformi
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model)); //(Adresa matrice, broj matrica koje saljemo, da li treba da se transponuju, pokazivac do matrica)
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projectionP));
    glBindVertexArray(VAO);


    glClearColor(0.5, 0.5, 0.5, 1.0);
    glCullFace(GL_BACK);//Biranje lica koje ce se eliminisati (tek nakon sto ukljucimo Face Culling)

    glm::vec3 position = glm::vec3(0.0f, -0.5f, 0.0f); // Početna pozicija u donjem delu ekrana
    float scaleFactor = 2.5f; // Povećano

    unsigned int useTexLoc = glGetUniformLocation(unifiedShader, "useTex");
    unsigned int transLoc = glGetUniformLocation(unifiedShader, "transparent");
    glEnable(GL_DEPTH_TEST);

    GLuint dugmeTexture = loadImageToTexture("res/porucivanje1.png");
    
    Model plate("objekti/cadnav.com_model/Model_D0901A18/plate.obj");
    float tableThickness = 0.1f;
    float tableTopY = -0.6f;

    glm::vec3 plateCenterWorld(
        0.0f,
        tableTopY + tableThickness / 2.0f + 0.01f,
        0.0f
    );


    AssemblyLogic assembly(plateCenterWorld);

    if (plate.meshes.size() == 0) {
        std::cout << "GRESKA: Model nije ucitan ili nema mesh-eva!\n";
    }
    else {
        std::cout << "Broj mesh-eva u modelu: " << plate.meshes.size() << std::endl;
    }

    Model bottomBun("objekti/donjaZemicka/source/donjaZemicka.obj");
    if (bottomBun.meshes.size() == 0) {
        std::cout << "GRESKA: Donja zemicka nije ucitana!\n";
    }
    else {
        std::cout << "Donja zemicka OK, mesh-eva: "
            << bottomBun.meshes.size() << std::endl;
    }
    Model splashModel("objekti/source/SPLASH.obj");
    if (splashModel.meshes.size() == 0) {
        std::cout << "GRESKA: Splash model nije ucitan!\n";
    }
    else {
        std::cout << "Splash model OK, mesh-eva: " << splashModel.meshes.size() << std::endl;
    }
    Model cucumber("objekti/krastavac/source/cucumber LP gotov.obj");
    if (cucumber.meshes.size() == 0) {
        std::cout << "GRESKA: Krastavac nije ucitan!\n";
    }
    else {
        std::cout << "Krastavac OK, mesh-eva: " << cucumber.meshes.size() << std::endl;
    }
    Model onion("objekti/luk/luk.obj");  // Promeni "luk.obj" ako se fajl zove drugačije
    if (onion.meshes.size() == 0) {
        std::cout << "GRESKA: Luk nije ucitan!\n";
    }
    else {
        std::cout << "Luk OK, mesh-eva: " << onion.meshes.size() << std::endl;
    }
    Model tomato("objekti/paradajz/paradajz.obj");
    if (tomato.meshes.size() == 0) {
        std::cout << "GRESKA: Paradajz nije ucitan!\n";
    }
    else {
        std::cout << "Paradajz OK, mesh-eva: " << tomato.meshes.size() << std::endl;
    }
    Model cheese("objekti/sir/source/model.obj");

    if (cheese.meshes.size() == 0) {
        std::cout << "GRESKA: Sir nije ucitan!\n";
    }
    else {
        std::cout << "Sir OK, mesh-eva: "
            << cheese.meshes.size() << std::endl;
    }

    Model lettuce("objekti/salata/LettuceRomaine_v1_L2.123c7e3ed02e-aea2-4c2a-9844-0f0bfd2b87b0/10188_LettuceRomaine_v1-L2.obj");

    if (lettuce.meshes.size() == 0) {
        std::cout << "GRESKA: Sir nije ucitan!\n";
    }
    else {
        std::cout << "Sir OK, mesh-eva: "
            << lettuce.meshes.size() << std::endl;
    }


    std::cout << "========== TEST SPLASH MODEL ==========\n";
    for (int i = 0; i < splashModel.meshes.size(); i++) {
        std::cout << "Mesh " << i << ": vertices=" << splashModel.meshes[i].vertices.size()
            << ", indices=" << splashModel.meshes[i].indices.size() << "\n";
    }
    std::cout << "========================================\n";
    SplashManager splashManager;

    static float lastFrame = 0.0f;

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    Name nameOverlayBosa("res/potpis1.png");

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        float startTime = glfwGetTime();

        if (pattyPos.y <= -0.15f && abs(pattyPos.x) < 0.3f && abs(pattyPos.z) < 0.3f) {
            pattyCook.update(deltaTime);
        }
        
        if (currentState == COOKING || currentState == ASSEMBLY) {
            // 1. INPUT ZA KAMERU (STRELICE)
            float camSpeed = 0.05f;
            if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)    cameraPos += camSpeed * cameraFront;
            if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)  cameraPos -= camSpeed * cameraFront;
            if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)  cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * camSpeed;
            if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * camSpeed;
        }
        float pattySpeed = 0.02f;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) pattyPos.z -= pattySpeed;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) pattyPos.z += pattySpeed;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) pattyPos.x -= pattySpeed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) pattyPos.x += pattySpeed;
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) pattyPos.y += pattySpeed;
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) pattyPos.y -= pattySpeed;
        
        if (depthTestEnabled)
            glEnable(GL_DEPTH_TEST);
        else
            glDisable(GL_DEPTH_TEST);

        if (cullFaceEnabled)
            glEnable(GL_CULL_FACE);
        else
            glDisable(GL_CULL_FACE);

        if (currentState == ASSEMBLY) {
            if (!assembly.isCurrentLocked()) {
                static bool kPressed = false;
                bool splashAdded = false;
                if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS && !kPressed) {
                    kPressed = true;
                    IngredientType cur = assembly.getCurrentType();

                    if (cur == KETCHUP || cur == MUSTARD) {
                        glm::vec3 bottlePos = assembly.getCurrentPosition();
                        glm::vec3 stackTop = assembly.getStackTopPosition();

                        SauceType currentSauce = (cur == KETCHUP) ? SAUCE_KETCHUP : SAUCE_MUSTARD;
                        
                        float bottleTipY = bottlePos.y - 0.115f;
                        float burgerTopY = stackTop.y;
                        float horizontalDist = glm::length(glm::vec2(bottlePos.x - stackTop.x, bottlePos.z - stackTop.z));
                        float verticalDist = abs(bottleTipY - burgerTopY);

                        
                        bool hitBurger = splashManager.addSplash(bottlePos, stackTop, tableTopY, currentSauce);

                        
                        if (cur == KETCHUP) {
                            std::cout << (hitBurger ? "✅ Pogodak! " : "❌ Promasaj! ") << "Donesi senf!\n";
                            assembly.advanceToMustard();
                        }
                        else if (cur == MUSTARD) {
                            std::cout << (hitBurger ? "✅ Pogodak! " : "❌ Promasaj! ") << "Idemo dalje!\n";
                            assembly.skipToNextIngredient();

                            std::cout << "🔎 Nakon skipToNextIngredient:\n";
                            std::cout << "   Trenutni sastojak: " << assembly.getCurrentType() << "\n";
                            std::cout << "   hideCurrentBottle: " << assembly.shouldHideCurrentBottle() << "\n";
                            glm::vec3 pos = assembly.getCurrentPosition();
                            std::cout << "   Pozicija: (" << pos.x << ", " << pos.y << ", " << pos.z << ")\n";
                        }
                    }
                }

                if (glfwGetKey(window, GLFW_KEY_K) == GLFW_RELEASE) {
                    kPressed = false;
                }

                // Kontrole za pomeranje
                glm::vec3& pos = assembly.getCurrentPosition();
                float speed = 0.02f;

                if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) pos.z -= speed;
                if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) pos.z += speed;
                if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) pos.x -= speed;
                if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) pos.x += speed;
                if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) pos.y += speed;
                if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) pos.y -= speed;

                assembly.clampCurrentAboveStack();
                static int frameCounter = 0;
                if (frameCounter++ % 60 == 0 && assembly.getCurrentType() == PICKLES) {
                    glm::vec3 target = assembly.getStackTopPosition();
                    target.y += 0.05f; // Isti offset kao u updateCurrentPosition()

                    float distXZ = glm::length(glm::vec2(pos.x - target.x, pos.z - target.z));
                    float distY = abs(pos.y - target.y);

                    std::cout << "🎯 Krastavac distanca:\n";
                    std::cout << "   XZ: " << distXZ << " (limit: 0.15)\n";
                    std::cout << "   Y:  " << distY << " (limit: 0.3)\n";
                    std::cout << "   Cilj: (" << target.x << ", " << target.y << ", " << target.z << ")\n";
                }
                
                    assembly.updateCurrentPosition();
                
            }
        }

        float pushDownLimit = pattyRestY - 0.03f; // koliko sme malo da "utone"

        if (pattyPos.y < pattyRestY) {
            if (pattyPos.y < pushDownLimit) {
                pattyPos.y = pushDownLimit;
            }
            else {
                pattyPos.y += (pattyRestY - pattyPos.y) * 0.2f;
            }
        }


        float time = (float)glfwGetTime();
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, GL_TRUE);
        }

        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
        {
            glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projectionP));
        }
        if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
        {
            glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projectionO));
        }
        glClearColor(0.5, 0.5, 0.5, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Osvjezavamo i Z bafer i bafer boje

        if (currentState == MENU) {
            glUseProgram(unifiedShader);
            glUniform1i(ignoreLightLoc, 1);

            glm::mat4 ortho = glm::ortho(-1.f, 1.f, -1.f, 1.f, -1.f, 1.f);
            glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(ortho));
            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.f)));

            // Dugme (plavo)
            glm::mat4 buttonModel = glm::mat4(1.f);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(buttonModel));

            glUniform1i(useTexLoc, 0);                     // ISKLJUČENA tekstura
            glUniform4f(colorModLoc, 1.0f, 0.71f, 0.76f, 1.0f);
            

            glBindVertexArray(buttonVAO);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

            glBindVertexArray(0);
        }


        else if (currentState == COOKING) {

            glUseProgram(unifiedShader);
            glUniform1i(ignoreLightLoc, 1);
            updateCamera(unifiedShader, viewLoc);
            glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projectionP));

            glBindVertexArray(VAO);
            glUniform1i(useTexLoc, 0);
            glUniform1i(useVertexColorLoc, 0);
            glUniform4f(colorModLoc, 0.0f, 0.0f, 0.0f, 1.0f);

            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(0.0f, -0.5f, 0.0f));
            model = glm::scale(model, glm::vec3(2.5f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
           

            glUniform1i(useTexLoc, 0);
            glUniform4f(colorModLoc, 0.0f, 0.0f, 0.0f, 1.0f);

            // crtamo SVE OSIM GORNJE STRANE
            for (int i = 0; i < 6; ++i) {
                if (i == 3) continue; // index 3 = GORNJA STRANA
                glDrawArrays(GL_TRIANGLE_FAN, i * 4, 4);
            }
            // --- GORNJA STRANA SA RINGLOM ---
            glUniform1i(useTexLoc, 1);
            glUniform4f(colorModLoc, 1.0f, 1.0f, 1.0f, 1.0f);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, ringlaTex);

            // gornja strana = i = 3
            glDrawArrays(GL_TRIANGLE_FAN, 3 * 4, 4);

            // Matrice za nogice (pomeramo ih u odnosu na centar rerne)
            glm::vec3 legPositions[] = {
                glm::vec3(-0.08f, -0.15f,  0.08f), // Napred levo
                glm::vec3(0.08f, -0.15f,  0.08f), // Napred desno
                glm::vec3(-0.08f, -0.15f, -0.08f), // Pozadi levo
                glm::vec3(0.08f, -0.15f, -0.08f)  // Pozadi desno
            };
            // --- ISKLJUČI TEKSTURU ZA NOGE ---
            glUniform1i(useTexLoc, 0);
            glUniform4f(colorModLoc, 0.0f, 0.0f, 0.0f, 1.0f);

            for (int i = 0; i < 4; i++) {
                glm::mat4 legModel = model;
                legModel = glm::translate(legModel, legPositions[i]);
                legModel = glm::scale(legModel, glm::vec3(0.1f, 0.5f, 0.1f));
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(legModel));

                for (int j = 0; j < 6; j++) {
                    glDrawArrays(GL_TRIANGLE_FAN, j * 4, 4);
                }
            }



            glBindVertexArray(pattyVAO);
            //crtanje pljeskavice
            glm::mat4 pModel = glm::mat4(1.0f);
            pModel = glm::translate(pModel, pattyPos);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(pModel));

            glm::vec3 c = pattyCook.getCurrentColor();
            glUniform4f(colorModLoc, c.r, c.g, c.b, 1.0f);
            glUniform1i(useTexLoc, 0); // Pljeskavica nema teksturu
            // Omotač
            glDrawArrays(GL_TRIANGLE_STRIP, 0, (segments + 1) * 2);
            // Gornji poklopac (Centar + segmenti + 1)
            glDrawArrays(GL_TRIANGLE_FAN, (segments + 1) * 2, segments + 2);
            // Donji poklopac
            glDrawArrays(GL_TRIANGLE_FAN, (segments + 1) * 2 + (segments + 2), segments + 2);

            glUniform4f(colorModLoc, 1.0f, 1.0f, 1.0f, 1.0f);
            glUniform1i(useTexLoc, useTex); // Vrati teksturu ako je uključena na 'G'

            // Boja pljeskavice iz CookingState-a
            glUniform4f(colorModLoc, c.r, c.g, c.b, 1.0f);
            glUniform1i(useTexLoc, 0); // Isključi teksturu za meso

            glDrawArrays(GL_TRIANGLE_STRIP, 0, (segments + 1) * 2);
            glDrawArrays(GL_TRIANGLE_FAN, (segments + 1) * 2, segments + 2);
            glDrawArrays(GL_TRIANGLE_FAN, (segments + 1) * 2 + (segments + 2), segments + 2);

            glUniform4f(colorModLoc, 1.0f, 1.0f, 1.0f, 1.0f);
            glUniform1i(useTexLoc, useTex);

            glUniform1i(ignoreLightLoc, 1);

            // --- CRTANJE LOADING BARA (2D Overlay) ---
            glDisable(GL_DEPTH_TEST); 
            glUseProgram(unifiedShader);

            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
            glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
            glUniform1i(useTexLoc, 0);

            glUniform4f(colorModLoc, 1.0f, 1.0f, 1.0f, 1.0f);

            glBindVertexArray(barVAO);

            float progress = pattyCook.progress; // Vrednost od 0.0 do 1.0

            glm::mat4 barModel = glm::mat4(1.0f);
            barModel = glm::translate(barModel, glm::vec3(-0.5f * (1.0f - progress), 0.0f, 0.0f));
            barModel = glm::scale(barModel, glm::vec3(progress, 1.0f, 1.0f));

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(barModel));
            glUniform4f(colorModLoc, 0.0f, 1.0f, 0.0f, 1.0f); // Zelena boja
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

            glEnable(GL_DEPTH_TEST); // Obavezno vrati Depth Test za sledeći frejm
            glEnable(GL_DEPTH_TEST); // Obavezno vrati Depth Test za sledeći frejm

            if (pattyCook.progress >= 1.0f) {
                currentState = ASSEMBLY;
            }
        }
        else if (currentState == ASSEMBLY) {
            glUseProgram(unifiedShader);
            glUniform1i(ignoreLightLoc, 0); // PRIMENI svetlo

            // Kamera se pomera mišem i strelicama (kao u COOKING)
            updateCamera(unifiedShader, viewLoc);
            glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projectionP));
            // Update uniforma svetla svaki frame
            glUseProgram(unifiedShader);
            glUniform3fv(lightPosLoc, 1, glm::value_ptr(lightPos));
            glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
            glUniform1f(lightIntensityLoc, lightIntensity);
            glUniform1i(lightEnabledLoc, lightEnabled ? 1 : 0);
            glUniform3fv(viewPosLoc, 1, glm::value_ptr(cameraPos));

            glBindVertexArray(VAO);
            glUniform1i(useTexLoc, 0);
            glUniform1i(useVertexColorLoc, 0);
            glUniform4f(colorModLoc, 0.6f, 0.4f, 0.2f, 1.0f); // braon sto
            
            float tableThickness = 0.1f;
            float tableWidth = 8.0f;
            float tableDepth = 2.0f;
            float tableTopY = -0.6f;

            glm::mat4 tableModel = glm::mat4(1.0f);
            tableModel = glm::translate(tableModel, glm::vec3(0.0f, tableTopY, 0.0f));
            glm::mat4 tableTopModel = tableModel;
            tableTopModel = glm::scale(
                tableTopModel,
                glm::vec3(tableWidth, tableThickness, tableDepth)
            );
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(tableTopModel));
            // CRTAMO CELO TELO STOLA (6 strana)
            for (int i = 0; i < 6; i++) {
                glDrawArrays(GL_TRIANGLE_FAN, i * 4, 4);
            }

            
            float legHeight = 1.2f;
            float legSize = 0.15f;

            float offsetX = (tableWidth / 2.0f) - 0.2f;
            float offsetZ = (tableDepth / 2.0f) - 0.2f;

            float legY = -(tableThickness / 2.0f + legHeight / 2.0f);

            glm::vec3 legLocalPos[] = {
                {-0.75f, -0.1f,  -0.15f},
                { -0.75f, -0.1f,  0.15f},
                {0.75f, -0.1f, -0.15f},
                { 0.75f, -0.1f, 0.15f}
            };

            for (int i = 0; i < 4; i++) {
                glm::mat4 legModel = tableModel;
                legModel = glm::translate(legModel, legLocalPos[i]);
                legModel = glm::scale(legModel, glm::vec3(legSize, legHeight, legSize));

                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(legModel));

                for (int j = 0; j < 6; j++)
                    glDrawArrays(GL_TRIANGLE_FAN, j * 4, 4);
            } 
            
            glUseProgram(unifiedShader); // koristi isti shader koji si koristila za kocke/pljeskavicu

            glEnable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE); // isključi culling, jer .obj možda ima "pogrešne" normale

            // Model matrica tanjira (centar + skala)
            glm::mat4 plateModel = tableModel;

            // tačno NA vrh ploče
            plateModel = glm::translate(
                plateModel,
                glm::vec3(0.0f, tableThickness / 2.0f + 0.01f, 0.0f)
            );

            // fina skala
            plateModel = glm::scale(plateModel, glm::vec3(0.0025f));

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(plateModel));
            glUniform1i(useTexLoc, 0); // Isključi teksturu
            glUniform1i(useVertexColorLoc, 0);
            glUniform4f(colorModLoc, 1.0f, 1.0f, 1.0f, 1.0f); // Bela boja

            glDisable(GL_CULL_FACE);
            glEnable(GL_DEPTH_TEST);
            plate.Draw();
            // ================= CRTANJE MRLJA =================
            for (const auto& splash : splashManager.getSplashes()) {
                if (splash.type == SPLASH_3D) {
                    glEnable(GL_DEPTH_TEST);
                    glDisable(GL_CULL_FACE);
                    glUseProgram(unifiedShader);
                    updateCamera(unifiedShader, viewLoc);
                    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projectionP));

                    glm::mat4 splashTransform = glm::mat4(1.0f);
                    glm::vec3 fixedPos = splash.position;
                    fixedPos.y += 0.22f;   // 🔥 PODIGNI IZNAD PLJESKAVICE

                    splashTransform = glm::translate(splashTransform, fixedPos);
                    splashTransform = glm::scale(splashTransform, glm::vec3(0.01f, 0.003f, 0.01f));

                    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(splashTransform));

                    // BOJA ZAVISI OD TIPA SOSA
                    if (splash.sauce == SAUCE_KETCHUP) {
                        glUniform4f(colorModLoc, 1.0f, 0.0f, 0.0f, 1.0f);  // Crvena
                    }
                    else {
                        glUniform4f(colorModLoc, 1.0f, 1.0f, 0.0f, 1.0f);  // Žuta
                    }

                    glUniform1i(useTexLoc, 0);
                    splashModel.Draw();
                }
                else if (splash.type == SPLASH_2D) {
                    glm::mat4 splashTransform = glm::mat4(1.0f);

                    glm::vec3 raisedPos = splash.position;
                    raisedPos.y += 0.03f;

                    splashTransform = glm::translate(splashTransform, raisedPos);
                    splashTransform = glm::rotate(splashTransform, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
                    splashTransform = glm::scale(splashTransform, glm::vec3(0.3f, 0.3f, 1.0f));

                    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(splashTransform));
                    glUniform4f(colorModLoc, 1.0f, 1.0f, 1.0f, 1.0f);
                    glUniform1i(useTexLoc, 1);

                    glActiveTexture(GL_TEXTURE0);

                    // TEKSTURA ZAVISI OD TIPA SOSA
                    if (splash.sauce == SAUCE_KETCHUP) {
                        glBindTexture(GL_TEXTURE_2D, splashTexture);
                    }
                    else {
                        glBindTexture(GL_TEXTURE_2D, senfTexture);
                    }

                    glBindVertexArray(imageVAO);
                    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
                    glBindVertexArray(0);
                }
            }
            // ================= POSTAVLJENI SASTOJCI =================
            for (const auto& ing : assembly.getPlaced()) {
                if (ing.type == KETCHUP || ing.type == MUSTARD) {
                    continue;
                }
                glm::mat4 m = glm::mat4(1.0f);
                m = glm::translate(m, ing.position);

                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
                glUniform1i(useTexLoc, 0);

                if (ing.type == BOTTOM_BUN) {
                    m = glm::scale(m, glm::vec3(0.004f));
                    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
                    glUniform4f(colorModLoc, 0.72f, 0.52f, 0.32f, 1.0f);
                    bottomBun.Draw();
                }
                else if (ing.type == PATTY) {
                    // POVEĆAJ pljeskavicu 1.5x
                    m = glm::scale(m, glm::vec3(0.9f));
                    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));

                    glm::vec3 c = pattyCook.getCurrentColor();
                    glUniform4f(colorModLoc, c.r, c.g, c.b, 1.0f);

                    glBindVertexArray(pattyVAO);
                    glDrawArrays(GL_TRIANGLE_STRIP, 0, (segments + 1) * 2);
                    glDrawArrays(GL_TRIANGLE_FAN, (segments + 1) * 2, segments + 2);
                    glDrawArrays(GL_TRIANGLE_FAN, (segments + 1) * 2 + (segments + 2), segments + 2);
                }
                else if (ing.type == PICKLES) {
                    m = glm::translate(m, glm::vec3(0.0f, 0.03f, 0.0f)); // 🔼 malo gore
                    m = glm::scale(m, glm::vec3(3.0f));
                    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
                    glUniform4f(colorModLoc, 0.0f, 0.8f, 0.0f, 1.0f); // Zelena boja
                    cucumber.Draw();
                }
                else if (ing.type == ONION) {
                    glm::mat4 m = glm::mat4(1.0f);
                    m = glm::translate(m, ing.position);

                    m = glm::scale(m, glm::vec3(0.12f));  

                    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
                    glUniform1i(useTexLoc, 0);
                    glUniform1i(useVertexColorLoc, 0);
                    glUniform4f(colorModLoc, 0.9f, 0.85f, 0.7f, 1.0f); // Bež/svetlo smeđa

                    onion.Draw();
                }
                else if (ing.type == LETTUCE) {
                    m = glm::translate(m, glm::vec3(0.0f, 0.01f, 0.0f));

                    // Okreni salatu na bok
                    m = glm::rotate(m, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

                    m = glm::scale(m, glm::vec3(0.005f));

                    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));

                    glUniform1i(useTexLoc, 0);
                    glUniform1i(useVertexColorLoc, 0);
                    glUniform4f(colorModLoc, 0.0f, 1.0f, 0.0f, 1.0f);

                    lettuce.Draw();
                }
                else if (ing.type == CHEESE)
                {
                    glm::mat4 m = glm::mat4(1.0f);
                    m = glm::translate(m, ing.position);
                    m = glm::scale(m, glm::vec3(0.025f));

                    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
                    glUniform1i(useTexLoc, 0);

                    glUniform4f(colorModLoc, 1.0f, 0.85f, 0.1f, 1.0f);

                    cheese.Draw();
                }



                else if (ing.type == TOMATO) {
                    glm::mat4 m = glm::mat4(1.0f);
                    m = glm::translate(m, ing.position);

                    m = glm::scale(m, glm::vec3(0.025f));

                    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
                    glUniform1i(useTexLoc, 0);
                    glUniform1i(useVertexColorLoc, 0);
                    glUniform4f(colorModLoc, 1.0f, 0.2f, 0.2f, 1.0f); // Crvena (paradajz)

                    tomato.Draw();
                }
                else if (ing.type == TOP_BUN) {
                    m = glm::scale(m, glm::vec3(0.004f));
                    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
                    glUniform4f(colorModLoc, 0.85f, 0.65f, 0.4f, 1.0f); // Svetlija braon (gornja zemička)
                    bottomBun.Draw(); // Koristi isti model kao donja zemička
                }
            }
            for (const auto& ing : assembly.getPlaced()) {
                if (ing.type == TOP_BUN) {
                    currentState = FINISHED;
                }
            }

            // ================= AKTIVNI SASTOJAK =================
            IngredientType cur = assembly.getCurrentType();
            glm::vec3 pos = assembly.getCurrentPosition();
            
            bool shouldSkipDrawing = false;
            if (cur == KETCHUP || cur == MUSTARD) {
                for (const auto& s : splashManager.getSplashes()) {
                    if (cur == KETCHUP && s.sauce == SAUCE_KETCHUP && s.type == SPLASH_3D) {
                        shouldSkipDrawing = true;
                        break;
                    }
                    if (cur == MUSTARD && s.sauce == SAUCE_MUSTARD && s.type == SPLASH_3D) {
                        shouldSkipDrawing = true;
                        break;
                    }
                }
            }
            if (!assembly.shouldHideCurrentBottle() && !shouldSkipDrawing) {
                glDisable(GL_CULL_FACE);

                if ((cur == KETCHUP || cur == MUSTARD) ) {
                    glm::mat4 bottleModel = glm::mat4(1.0f);
                    bottleModel = glm::translate(bottleModel, pos);
                    // Rotacija 180 stepeni oko X ose (da vrh ide na dole)
                    bottleModel = glm::rotate(bottleModel, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
                    bottleModel = glm::scale(bottleModel, glm::vec3(1.5f));

                    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(bottleModel));
                    glUniform4f(colorModLoc, 1.0f, 1.0f, 1.0f, 1.0f); // Bela boja kao osnova
                    glUniform1i(useTexLoc, 0); // Isključi teksture
                    glUniform1i(useVertexColorLoc, 1); // UKLJUČI ŽUTU BOJU IZ SENF.CPP

                    if (cur == MUSTARD) {
                        glBindVertexArray(senfVAO);
                        drawSenf(segments); // Poziva tvoju funkciju iz senf.cpp
                    }
                    else if (cur == KETCHUP) {
                        glBindVertexArray(ketchupVAO);
                        drawKetchup(segments);
                    }
                    glUniform1i(useVertexColorLoc, 0); // Vrati na nulu za ostale
                }
                

                else if (cur == BOTTOM_BUN) {
                glm::mat4 m = glm::mat4(1.0f);
                m = glm::translate(m, pos);

                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
                glUniform1i(useTexLoc, 0);
                glUniform1i(useVertexColorLoc, 0);

                    m = glm::scale(m, glm::vec3(0.004f));
                    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
                    glUniform4f(colorModLoc, 0.72f, 0.52f, 0.32f, 1.0f);
                    bottomBun.Draw();
                }
                else if (cur == PATTY) {
                    glm::mat4 m = glm::mat4(1.0f);
                    m = glm::translate(m, pos);

                    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
                    glUniform1i(useTexLoc, 0);
                    glUniform1i(useVertexColorLoc, 0);

                    m = glm::scale(m, glm::vec3(0.9f));
                    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));

                    glm::vec3 c = pattyCook.getCurrentColor();
                    glUniform4f(colorModLoc, c.r, c.g, c.b, 1.0f);

                    glBindVertexArray(pattyVAO);
                    glDrawArrays(GL_TRIANGLE_STRIP, 0, (segments + 1) * 2);
                    glDrawArrays(GL_TRIANGLE_FAN, (segments + 1) * 2, segments + 2);
                    glDrawArrays(GL_TRIANGLE_FAN, (segments + 1) * 2 + (segments + 2), segments + 2);
                }
                else if (cur == PICKLES) {
                    std::cout << "🥒 CRTAM KRASTAVAC na poziciji: (" << pos.x << ", " << pos.y << ", " << pos.z << ")\n";

                    glm::mat4 m = glm::mat4(1.0f);
                    m = glm::translate(m, pos);

                    m = glm::scale(m, glm::vec3(3.0f));  // BILO: 0.004f → SADA: 0.4f

                    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
                    glUniform1i(useTexLoc, 0);
                    glUniform1i(useVertexColorLoc, 0);
                    glUniform4f(colorModLoc, 0.0f, 0.8f, 0.0f, 1.0f); // Zelena boja

                    cucumber.Draw();

                    std::cout << "   ✅ cucumber.Draw() POZVAN!\n";
                }
                else if (cur == ONION) {
                    glm::mat4 m = glm::mat4(1.0f);
                    m = glm::translate(m, pos);

                     m = glm::scale(m, glm::vec3(0.12f));

                    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
                    glUniform1i(useTexLoc, 0);
                    glUniform1i(useVertexColorLoc, 0);
                    glUniform4f(colorModLoc, 0.9f, 0.85f, 0.7f, 1.0f);

                    onion.Draw();
                }
                else if (cur == LETTUCE) {
                    glm::mat4 m = glm::mat4(1.0f);

                    glm::vec3 pos = assembly.getCurrentPosition();
                    m = glm::translate(m, pos);

                    m = glm::translate(m, glm::vec3(0.0f, 0.01f, 0.0f)); // tweak po potrebi

                    // Okreni na bok (npr. rotacija oko X ili Z ose)
                    m = glm::rotate(m, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

                    m = glm::scale(m, glm::vec3(0.005f));

                    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));

                    glUniform1i(useTexLoc, 0);
                    glUniform1i(useVertexColorLoc, 0);
                    glUniform4f(colorModLoc, 0.0f, 1.0f, 0.0f, 1.0f);

                    lettuce.Draw();
                }
                else if (cur == CHEESE)
                {
                    glm::mat4 m = glm::mat4(1.0f);
                    m = glm::translate(m, pos);

                    m = glm::scale(m, glm::vec3(0.025f));

                    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));

                    glUniform1i(useTexLoc, 0);
                    glUniform1i(useVertexColorLoc, 0);

                    glUniform4f(colorModLoc, 1.0f, 0.85f, 0.1f, 1.0f);

                    cheese.Draw();   
                }

                else if (cur == TOMATO) {
                    glm::mat4 m = glm::mat4(1.0f);
                    m = glm::translate(m, pos);

                    m = glm::scale(m, glm::vec3(0.025f));

                    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
                    glUniform1i(useTexLoc, 0);
                    glUniform1i(useVertexColorLoc, 0);
                    glUniform4f(colorModLoc, 1.0f, 0.2f, 0.2f, 1.0f); // Crvena (paradajz)

                    tomato.Draw(); 
                }
                else if (cur == TOP_BUN) {
                    glm::mat4 m = glm::mat4(1.0f);
                    m = glm::translate(m, pos);
                    m = glm::scale(m, glm::vec3(0.004f));
                    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
                    glUniform1i(useTexLoc, 0);
                    glUniform1i(useVertexColorLoc, 0);
                    glUniform4f(colorModLoc, 0.85f, 0.65f, 0.4f, 1.0f); // Svetlija braon (gornja zemička)
                    bottomBun.Draw();
                }
                
            }
            glUniform1i(useTexLoc, 0);   

        }
        else if (currentState == FINISHED)
        {
            glDisable(GL_DEPTH_TEST);
            glUseProgram(unifiedShader);
            glUniform1i(ignoreLightLoc, 1);

            glm::mat4 ortho = glm::ortho(-1.f, 1.f, -1.f, 1.f, -1.f, 1.f);

            glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(ortho));
            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.f)));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.f)));

            glUniform1i(useTexLoc, 1);
            glUniform4f(colorModLoc, 1.f, 1.f, 1.f, 1.f);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, krajTexture);

            glBindVertexArray(imageVAO);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

            glBindVertexArray(0);
        }

            while (glfwGetTime() - startTime < 1.0 / 75.0) {}
        
            nameOverlayBosa.draw(0.5f);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
    glDeleteProgram(unifiedShader);

    glfwTerminate();
    return 0;
}
