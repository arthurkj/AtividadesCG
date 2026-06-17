/* Hello Triangle - código adaptado de https://learnopengl.com/#!Getting-started/Hello-Triangle
 *
 * Adaptado por Rossana Baptista Queiroz
 * para as disciplinas de Processamento Gráfico/Computação Gráfica - Unisinos
 * Versão inicial: 7/4/2017
 * Última atualização em 07/03/2025
 */

#include <iostream>
#include <string>
#include <assert.h>

using namespace std;

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

//GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Cena.h"

// Protótipo da função de callback de teclado
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

void mouse_callback(GLFWwindow* window, double xpos, double ypos);

// Protótipos das funções
int setupShader();
GLuint loadTexture(string filePath);
void loadScene(std::string filePath, Camera &camera, std::vector<Object3D> &objects);

// Dimensões da janela (pode ser alterado em tempo de execução)
const GLuint WIDTH = 1000, HEIGHT = 1000;

bool keyLightOn = true;
bool fillLightOn = true;
bool backLightOn = true;
float q = 10.0f;

// Código fonte do Vertex Shader (em GLSL): ainda hardcoded
const GLchar *vertexShaderSource = R"(
#version 400
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec2 texc;

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;

out vec2 texCoord;
out vec3 vNormal;
out vec4 fragPos; 
out vec4 vColor;
void main()
{
    gl_Position = projection * view * model * vec4(position.x, position.y, position.z, 1.0);
    fragPos = model * vec4(position.x, position.y, position.z, 1.0);
    texCoord = texc;
    vNormal = mat3(transpose(inverse(model))) * normal;
    vColor = vec4(color,1.0);
})";

//Códifo fonte do Fragment Shader (em GLSL): ainda hardcoded
const GLchar *fragmentShaderSource = R"(
#version 400
in vec2 texCoord;
uniform sampler2D texBuff;

// Nossas 3 luzes (Arrays no lugar de uma variável única)
uniform vec3 lightPos[3];
uniform bool lightOn[3]; // Controle para ligar/desligar
uniform float kl[3];     // Fator Linear da atenuação
uniform float kq[3];     // Fator Quadrático da atenuação

uniform vec3 camPos;
uniform vec3 ka;
uniform vec3 kd;
uniform vec3 ks;
uniform float q;
out vec4 color;
in vec4 fragPos;
in vec3 vNormal;
in vec4 vColor;

void main()
{
	vec3 lightColor = vec3(0.8, 0.8, 0.8);
	vec4 objectColor = texture(texBuff, texCoord);

	// Coeficiente de luz ambiente (A luz ambiente é uma só para a cena toda)
	vec3 ambient = ka * 0.3 * lightColor;

    // Acumuladores para as 3 luzes
    vec3 diffuseTotal = vec3(0.0);
    vec3 specularTotal = vec3(0.0);

	vec3 N = normalize(vNormal);
	vec3 V = normalize(camPos - vec3(fragPos));

    // Laço passando pelas 3 fontes de luz
    for(int i = 0; i < 3; i++) {
        if(lightOn[i]) {
            // 1. Calcula a distância (d) e a atenuação
            float d = length(lightPos[i] - vec3(fragPos));
            // Assumimos Kc = 1.0 para evitar divisão por zero
            float att = 1.0 / (1.0 + kl[i] * d + kq[i] * (d * d));

            // Direção da luz atual
	        vec3 L = normalize(lightPos[i] - vec3(fragPos));
	        
            // 2. Coeficiente de reflexão difusa (com atenuação)
            float diff = max(dot(N, L), 0.0);
	        vec3 diffuse = kd * diff * lightColor * att; 
            diffuseTotal += diffuse;

	        // 3. Coeficiente de reflexão especular (com atenuação)
	        vec3 R = normalize(reflect(-L, N));
	        float spec = max(dot(R, V), 0.0);
	        spec = pow(spec, q);
	        vec3 specular = ks * spec * lightColor * att;
            specularTotal += specular;
        }
    }

    // Combina tudo
	vec3 result = (ambient + diffuseTotal) * vec3(objectColor) + specularTotal;
	color = vec4(result, 1.0);
})";

bool rotateX=false, rotateY=false, rotateZ=false;

float moveSpeed = 0.05f;
float scaleSpeed = 0.05f;
float rotSpeed = 0.05f;

std::vector<Object3D> objects;
std::vector<Light> sceneLights;
float fov = 45.0f, zNear = 0.1f, zFar = 100.0f;

int selectedObject = 0;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

float deltaTime = 0.0f; 
float lastFrame = 0.0f; 
bool firstMouse = true;
float lastX = WIDTH / 2.0f;
float lastY = HEIGHT / 2.0f;

// Função MAIN
int main()
{
	// Inicialização da GLFW
	glfwInit();

	//Muita atenção aqui: alguns ambientes não aceitam essas configurações
	//Você deve adaptar para a versão do OpenGL suportada por sua placa
	//Sugestão: comente essas linhas de código para desobrir a versão e
	//depois atualize (por exemplo: 4.5 com 4 e 5)
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//Essencial para computadores da Apple
	#ifdef __APPLE__
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	#endif

	// Criação da janela GLFW
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Olá, Grau B -- Arthur Kist Juchem!", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	// Fazendo o registro da função de callback para a janela GLFW
	glfwSetKeyCallback(window, key_callback);
	
	glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// GLAD: carrega todos os ponteiros d funções da OpenGL
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;

	}

	// Obtendo as informações de versão
	const GLubyte* renderer = glGetString(GL_RENDERER); /* get renderer string */
	const GLubyte* version = glGetString(GL_VERSION); /* version as a string */
	cout << "Renderer: " << renderer << endl;
	cout << "OpenGL version supported " << version << endl;

	// Definindo as dimensões da viewport com as mesmas dimensões da janela da aplicação
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);


	// Compilando e buildando o programa de shader
	GLuint shaderID = setupShader();

    loadScene("../assets/cena.txt", camera, objects, sceneLights, fov, zNear, zFar);

	glUseProgram(shaderID);

	glm::mat4 model = glm::mat4(1); //matriz identidade;
	GLint modelLoc = glGetUniformLocation(shaderID, "model");
	//
	model = glm::rotate(model, /*(GLfloat)glfwGetTime()*/glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glEnable(GL_DEPTH_TEST);

	glm::vec3 camPos = glm::vec3(0.0f, 0.0f, 3.0f);

    glUniform1f(glGetUniformLocation(shaderID, "q"), q);
    glUniform3f(glGetUniformLocation(shaderID, "camPos"), camPos.x, camPos.y, camPos.z);
    
    glUniform1i(glGetUniformLocation(shaderID, "texBuff"), 0);

	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
    GLint projLoc = glGetUniformLocation(shaderID, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	// Loop da aplicação - "game loop"
	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.ProcessKeyboard(0, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.ProcessKeyboard(1, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.ProcessKeyboard(2, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.ProcessKeyboard(3, deltaTime);

		glm::mat4 view = camera.GetViewMatrix();
        glUniformMatrix4fv(glGetUniformLocation(shaderID, "view"), 1, GL_FALSE, glm::value_ptr(view));
        
        glUniform3f(glGetUniformLocation(shaderID, "camPos"), camera.Position.x, camera.Position.y, camera.Position.z);
        glUniform1f(glGetUniformLocation(shaderID, "q"), q);

		// Checa se houveram eventos de input (key pressed, mouse moved etc.) e chama as funções de callback correspondentes
		glfwPollEvents();

		// Limpa o buffer de cor
		glClearColor(0.0f, 0.3f, 0.8f, 1.0f); //cor de fundo
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::vec3 objPos = glm::vec3(0, 0, 0);

		// Verifica se o scene.txt carregou as 3 luzes para não quebrar a memória
        if (sceneLights.size() >= 3) {
            
            // LUZ 0: Key Light
            glUniform3f(glGetUniformLocation(shaderID, "lightPos[0]"), sceneLights[0].position.x, sceneLights[0].position.y, sceneLights[0].position.z);
            glUniform1i(glGetUniformLocation(shaderID, "lightOn[0]"), keyLightOn);
            glUniform1f(glGetUniformLocation(shaderID, "kl[0]"), sceneLights[0].kl);
            glUniform1f(glGetUniformLocation(shaderID, "kq[0]"), sceneLights[0].kq);

            // LUZ 1: Fill Light
            glUniform3f(glGetUniformLocation(shaderID, "lightPos[1]"), sceneLights[1].position.x, sceneLights[1].position.y, sceneLights[1].position.z);
            glUniform1i(glGetUniformLocation(shaderID, "lightOn[1]"), fillLightOn);
            glUniform1f(glGetUniformLocation(shaderID, "kl[1]"), sceneLights[1].kl);
            glUniform1f(glGetUniformLocation(shaderID, "kq[1]"), sceneLights[1].kq);

            // LUZ 2: Back Light
            glUniform3f(glGetUniformLocation(shaderID, "lightPos[2]"), sceneLights[2].position.x, sceneLights[2].position.y, sceneLights[2].position.z);
            glUniform1i(glGetUniformLocation(shaderID, "lightOn[2]"), backLightOn);
            glUniform1f(glGetUniformLocation(shaderID, "kl[2]"), sceneLights[2].kl);
            glUniform1f(glGetUniformLocation(shaderID, "kq[2]"), sceneLights[2].kq);
            
        } else {
            std::cout << "Aviso: O scene.txt precisa ter exatamente 3 linhas do tipo LIGHT." << std::endl;
        }

		glLineWidth(10);
		glPointSize(20);

		float angle = (GLfloat)glfwGetTime();

		for (int i = 0; i < objects.size(); i++) {
            if (objects[i].isMoving && objects[i].pathPoints.size() >= 4) {
                
                int N = objects[i].pathPoints.size();
                int idx = objects[i].currentTarget;
                
                // Pegamos 4 pontos consecutivos. O uso do % N garante o ciclo infinito!
                glm::vec3 p0 = objects[i].pathPoints[(idx) % N];
                glm::vec3 p1 = objects[i].pathPoints[(idx + 1) % N];
                glm::vec3 p2 = objects[i].pathPoints[(idx + 2) % N];
                glm::vec3 p3 = objects[i].pathPoints[(idx + 3) % N];

                // Avançamos o parâmetro t baseado no tempo (velocidade da curva)
                objects[i].tParam += deltaTime * 0.5f; // Altere o 0.5f para deixar mais rápido ou mais lento

                if (objects[i].tParam > 1.0f) {
                    objects[i].tParam = 0.0f; // Reseta o t
                    // Avança 3 índices para o próximo segmento da curva de Bézier
                    objects[i].currentTarget = (objects[i].currentTarget + 3) % N; 
                }

                // Fórmula Polinomial Cúbica de Bézier
                float t = objects[i].tParam;
                float u = 1.0f - t;
                
                // P(t) = (1-t)^3 * P0 + 3t(1-t)^2 * P1 + 3t^2(1-t) * P2 + t^3 * P3
                glm::vec3 bezierPos = (u * u * u) * p0 + 
                                      (3.0f * u * u * t) * p1 + 
                                      (3.0f * u * t * t) * p2 + 
                                      (t * t * t) * p3;

                objects[i].position = bezierPos;
            }

            glBindVertexArray(objects[i].VAO);
            
			glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, objects[i].textureID);

            glm::mat4 model = glm::mat4(1.0f); 
            
            model = glm::translate(model, objects[i].position);
            
            model = glm::rotate(model, objects[i].rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::rotate(model, objects[i].rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::rotate(model, objects[i].rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::scale(model, objects[i].scale);

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

            glUniform3f(glGetUniformLocation(shaderID, "ka"), objects[i].ka.r, objects[i].ka.g, objects[i].ka.b);
            glUniform3f(glGetUniformLocation(shaderID, "kd"), objects[i].kd.r, objects[i].kd.g, objects[i].kd.b);
            glUniform3f(glGetUniformLocation(shaderID, "ks"), objects[i].ks.r, objects[i].ks.g, objects[i].ks.b);

            glDrawArrays(GL_TRIANGLES, 0, objects[i].nVertices);
            
            glBindVertexArray(0);
        }
		// Troca os buffers da tela
		glfwSwapBuffers(window);
	}
	// Finaliza a execução da GLFW, limpando os recursos alocados por ela
	glfwTerminate();
	return 0;
}

// Função de callback de teclado - só pode ter uma instância (deve ser estática se
// estiver dentro de uma classe) - É chamada sempre que uma tecla for pressionada
// ou solta via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if (key == GLFW_KEY_TAB && action == GLFW_PRESS) {
        selectedObject = (selectedObject + 1) % objects.size();
    }

	if (key == GLFW_KEY_1 && action == GLFW_PRESS)
		keyLightOn = !keyLightOn;

	if (key == GLFW_KEY_2 && action == GLFW_PRESS)
		fillLightOn = !fillLightOn;

	if (key == GLFW_KEY_3 && action == GLFW_PRESS)
		backLightOn = !backLightOn;

    if (key == GLFW_KEY_X && (action == GLFW_PRESS || action == GLFW_REPEAT)) 
        objects[selectedObject].rotation.x += rotSpeed;
    if (key == GLFW_KEY_Y && (action == GLFW_PRESS || action == GLFW_REPEAT)) 
        objects[selectedObject].rotation.y += rotSpeed;
    if (key == GLFW_KEY_Z && (action == GLFW_PRESS || action == GLFW_REPEAT)) 
        objects[selectedObject].rotation.z += rotSpeed;

    if (key == GLFW_KEY_LEFT_BRACKET && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        objects[selectedObject].scale -= glm::vec3(scaleSpeed);
        if (objects[selectedObject].scale.x < 0.1f) objects[selectedObject].scale = glm::vec3(0.1f);
    }
    if (key == GLFW_KEY_RIGHT_BRACKET && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        objects[selectedObject].scale += glm::vec3(scaleSpeed);
    }
    if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        objects[selectedObject].pathPoints.push_back(objects[selectedObject].position);
    }
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        objects[selectedObject].isMoving = !objects[selectedObject].isMoving;
    }
    if (key == GLFW_KEY_UP && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        objects[selectedObject].position.z -= moveSpeed;
    }
    if (key == GLFW_KEY_DOWN && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        objects[selectedObject].position.z += moveSpeed;
    }
    if (key == GLFW_KEY_LEFT && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        objects[selectedObject].position.x -= moveSpeed;
    }
    if (key == GLFW_KEY_RIGHT && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        objects[selectedObject].position.x += moveSpeed;
    }
    if (key == GLFW_KEY_PAGE_UP && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        objects[selectedObject].position.y += moveSpeed;
    }
    if (key == GLFW_KEY_PAGE_DOWN && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        objects[selectedObject].position.y -= moveSpeed;
    }
    if (key == GLFW_KEY_EQUAL && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        q += 2.0f;
        cout << "Brilho Especular (q) aumentado para: " << q << endl;
    }
    if (key == GLFW_KEY_MINUS && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        q -= 2.0f;
        if (q < 1.0f) q = 1.0f; 
        cout << "Brilho Especular (q) diminuido para: " << q << endl;
    }
}

//Esta função está basntante hardcoded - objetivo é compilar e "buildar" um programa de
// shader simples e único neste exemplo de código
// O código fonte do vertex e fragment shader está nos arrays vertexShaderSource e
// fragmentShader source no iniçio deste arquivo
// A função retorna o identificador do programa de shader
int setupShader()
{
	// Vertex shader
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	// Checando erros de compilação (exibição via log no terminal)
	GLint success;
	GLchar infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	// Fragment shader
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	// Checando erros de compilação (exibição via log no terminal)
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	// Linkando os shaders e criando o identificador do programa de shader
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	// Checando por erros de linkagem
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return shaderProgram;
}

GLuint loadTexture(string filePath) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true); 
    unsigned char *data = stbi_load(filePath.c_str(), &width, &height, &nrChannels, 0);
    
    if (data) {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "Falha ao carregar textura: " << filePath << std::endl;
    }
    stbi_image_free(data);
    return textureID;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if(firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    
    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

