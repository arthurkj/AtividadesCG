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


// Protótipo da função de callback de teclado
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

// Protótipos das funções
int setupShader();
GLuint loadTexture(string filePath);
int loadSimpleOBJ(std::string filePath, int &nVertices, std::string &texturePath);

// Dimensões da janela (pode ser alterado em tempo de execução)
const GLuint WIDTH = 1000, HEIGHT = 1000;

bool keyLightOn = true;
bool fillLightOn = true;
bool backLightOn = true;

// Código fonte do Vertex Shader (em GLSL): ainda hardcoded
const GLchar *vertexShaderSource = R"(
#version 400
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec2 texc;

uniform mat4 projection;
uniform mat4 model;

out vec2 texCoord;
out vec3 vNormal;
out vec4 fragPos; 
out vec4 vColor;
void main()
{
    gl_Position = projection * model * vec4(position.x, position.y, position.z, 1.0);
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
uniform float ka;
uniform float kd;
uniform float ks;
uniform float q;
out vec4 color;
in vec4 fragPos;
in vec3 vNormal;
in vec4 vColor;

void main()
{
	vec3 lightColor = vec3(1.0, 1.0, 1.0);
	vec4 objectColor = texture(texBuff, texCoord);

	// Coeficiente de luz ambiente (A luz ambiente é uma só para a cena toda)
	vec3 ambient = ka * lightColor;

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

struct Object3D {
    GLuint VAO;
    int nVertices;
	GLuint textureID;
    glm::vec3 position;
    glm::vec3 scale;
    glm::vec3 rotation;
};

float moveSpeed = 0.05f;
float scaleSpeed = 0.05f;
float rotSpeed = 0.05f;

std::vector<Object3D> objects;

int selectedObject = 0;

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
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Olá, Vivencial Dois -- Arthur Juchem!", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	// Fazendo o registro da função de callback para a janela GLFW
	glfwSetKeyCallback(window, key_callback);

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

	int nVerticesObj1 = 0;
    int nVerticesObj2 = 0;
	string pathTex1 = "";
	string pathTex2 = "";

	GLuint vaoOBJ1 = loadSimpleOBJ("../assets/Modelos3D/Suzanne.obj", nVerticesObj1, pathTex1);
	GLuint vaoOBJ2 = loadSimpleOBJ("../assets/Modelos3D/Suzanne.obj", nVerticesObj2, pathTex2);

	Object3D obj1;
    obj1.VAO = vaoOBJ1;
    obj1.nVertices = nVerticesObj1;
    obj1.textureID = loadTexture(pathTex1);
    obj1.position = glm::vec3(-0.6f, 0.0f, 0.0f);
    obj1.scale = glm::vec3(0.2f);
    obj1.rotation = glm::vec3(0.0f);
    objects.push_back(obj1);

	Object3D obj2;
    obj2.VAO = vaoOBJ2;
    obj2.nVertices = nVerticesObj2;
    obj2.textureID = loadTexture(pathTex2);
    obj2.position = glm::vec3(0.6f, 0.0f, -0.5f);
    obj2.scale = glm::vec3(0.2f);
    obj2.rotation = glm::vec3(0.0f);
    objects.push_back(obj2);

	glUseProgram(shaderID);

	glm::mat4 model = glm::mat4(1); //matriz identidade;
	GLint modelLoc = glGetUniformLocation(shaderID, "model");
	//
	model = glm::rotate(model, /*(GLfloat)glfwGetTime()*/glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glEnable(GL_DEPTH_TEST);

	float ka = 0.1f;  // Coeficiente de reflexão ambiente
    float kd = 0.7f;  // Coeficiente de reflexão difusa
    float ks = 0.5f;  // Coeficiente de reflexão especular
    float q = 10.0f;

	glm::vec3 camPos = glm::vec3(0.0f, 0.0f, 3.0f);

    glUniform1f(glGetUniformLocation(shaderID, "ka"), ka);
    glUniform1f(glGetUniformLocation(shaderID, "kd"), kd);
    glUniform1f(glGetUniformLocation(shaderID, "ks"), ks);
    glUniform1f(glGetUniformLocation(shaderID, "q"), q);
    glUniform3f(glGetUniformLocation(shaderID, "camPos"), camPos.x, camPos.y, camPos.z);
    
    glUniform1i(glGetUniformLocation(shaderID, "texBuff"), 0);

	glm::mat4 projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -3.0f, 3.0f);
    GLint projLoc = glGetUniformLocation(shaderID, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	// Loop da aplicação - "game loop"
	while (!glfwWindowShouldClose(window))
	{
		// Checa se houveram eventos de input (key pressed, mouse moved etc.) e chama as funções de callback correspondentes
		glfwPollEvents();

		// Limpa o buffer de cor
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f); //cor de fundo
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::vec3 objPos = glm::vec3(0, 0, 0);

		glm::vec3 keyPos = objPos + glm::vec3(2.0f, 2.0f, 2.0f);
		glUniform3f(glGetUniformLocation(shaderID, "lightPos[0]"), keyPos.x, keyPos.y, keyPos.z);
		glUniform1i(glGetUniformLocation(shaderID, "lightOn[0]"), keyLightOn);
		glUniform1f(glGetUniformLocation(shaderID, "kl[0]"), 0.09f); 
		glUniform1f(glGetUniformLocation(shaderID, "kq[0]"), 0.032f);

		glm::vec3 fillPos = objPos + glm::vec3(-2.0f, 1.0f, 2.0f);
		glUniform3f(glGetUniformLocation(shaderID, "lightPos[1]"), fillPos.x, fillPos.y, fillPos.z);
		glUniform1i(glGetUniformLocation(shaderID, "lightOn[1]"), fillLightOn);
		glUniform1f(glGetUniformLocation(shaderID, "kl[1]"), 0.35f); 
		glUniform1f(glGetUniformLocation(shaderID, "kq[1]"), 0.44f);

		glm::vec3 backPos = objPos + glm::vec3(0.0f, 2.0f, -3.0f);
		glUniform3f(glGetUniformLocation(shaderID, "lightPos[2]"), backPos.x, backPos.y, backPos.z);
		glUniform1i(glGetUniformLocation(shaderID, "lightOn[2]"), backLightOn);
		glUniform1f(glGetUniformLocation(shaderID, "kl[2]"), 0.14f); 
		glUniform1f(glGetUniformLocation(shaderID, "kq[2]"), 0.07f);

		glLineWidth(10);
		glPointSize(20);

		float angle = (GLfloat)glfwGetTime();

		for (int i = 0; i < objects.size(); i++) {
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

    if (key == GLFW_KEY_W && (action == GLFW_PRESS || action == GLFW_REPEAT)) 
        objects[selectedObject].position.y += moveSpeed;
    if (key == GLFW_KEY_S && (action == GLFW_PRESS || action == GLFW_REPEAT)) 
        objects[selectedObject].position.y -= moveSpeed;
    if (key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT)) 
        objects[selectedObject].position.x -= moveSpeed;
    if (key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT)) 
        objects[selectedObject].position.x += moveSpeed;
    if (key == GLFW_KEY_I && (action == GLFW_PRESS || action == GLFW_REPEAT)) 
        objects[selectedObject].position.z -= moveSpeed; 
    if (key == GLFW_KEY_J && (action == GLFW_PRESS || action == GLFW_REPEAT)) 
        objects[selectedObject].position.z += moveSpeed;

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