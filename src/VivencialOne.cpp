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


// Protótipo da função de callback de teclado
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

// Protótipos das funções
int setupShader();
int loadSimpleOBJ(std::string filePath, int &nVertices, glm::vec3 color);

// Dimensões da janela (pode ser alterado em tempo de execução)
const GLuint WIDTH = 1000, HEIGHT = 1000;

// Código fonte do Vertex Shader (em GLSL): ainda hardcoded
const GLchar* vertexShaderSource = "#version 410\n"
"layout (location = 0) in vec3 position;\n"
"layout (location = 1) in vec3 color;\n"
"uniform mat4 model;\n"
"out vec4 finalColor;\n"
"void main()\n"
"{\n"
"gl_Position = model * vec4(position, 1.0);\n"
"finalColor = vec4(color, 1.0);\n"
"}\0";

//Códifo fonte do Fragment Shader (em GLSL): ainda hardcoded
const GLchar* fragmentShaderSource = "#version 410\n"
"in vec4 finalColor;\n"
"out vec4 color;\n"
"uniform int isWireframe;\n" 
"void main()\n"
"{\n"
"    if (isWireframe == 1) {\n"
"        color = vec4(0.0, 0.0, 0.0, 1.0);\n" 
"    } else {\n"
"        color = finalColor;\n"
"    }\n"
"}\n\0";

bool rotateX=false, rotateY=false, rotateZ=false;

struct Object3D {
    GLuint VAO;
    int nVertices;
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
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Olá, Vivencial 3D -- Arthur Juchem!", nullptr, nullptr);
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

	GLuint vaoOBJ1 = loadSimpleOBJ("../assets/Modelos3D/Suzanne.obj", nVerticesObj1, glm::vec3(1.0f, 0.0f, 0.0f));
	GLuint vaoOBJ2 = loadSimpleOBJ("../assets/Modelos3D/Suzanne.obj", nVerticesObj2, glm::vec3(1.0f, 1.0f, 0.0f));

	Object3D obj1;
    obj1.VAO = vaoOBJ1;
    obj1.nVertices = nVerticesObj1;
    obj1.position = glm::vec3(-0.6f, 0.0f, 0.0f);
    obj1.scale = glm::vec3(0.2f);
    obj1.rotation = glm::vec3(0.5f);
    objects.push_back(obj1);

	Object3D obj2;
    obj2.VAO = vaoOBJ2;
    obj2.nVertices = nVerticesObj2;
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

	GLint wireframeLoc = glGetUniformLocation(shaderID, "isWireframe");

	glEnable(GL_DEPTH_TEST);


	// Loop da aplicação - "game loop"
	while (!glfwWindowShouldClose(window))
	{
		// Checa se houveram eventos de input (key pressed, mouse moved etc.) e chama as funções de callback correspondentes
		glfwPollEvents();

		// Limpa o buffer de cor
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f); //cor de fundo
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glLineWidth(10);
		glPointSize(20);

		float angle = (GLfloat)glfwGetTime();

		for (int i = 0; i < objects.size(); i++) {
            glBindVertexArray(objects[i].VAO);
            
            glm::mat4 model = glm::mat4(1.0f); 
            
            model = glm::translate(model, objects[i].position);
            
            model = glm::rotate(model, objects[i].rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::rotate(model, objects[i].rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::rotate(model, objects[i].rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::scale(model, objects[i].scale);

			glUniform1i(wireframeLoc, 0);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glDrawArrays(GL_TRIANGLES, 0, objects[i].nVertices);
            
			glUniform1i(wireframeLoc, 1);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glLineWidth(2.0f);

			glEnable(GL_POLYGON_OFFSET_LINE);
            glPolygonOffset(-1.0f, -1.0f);
			glDrawArrays(GL_TRIANGLES, 0, objects[i].nVertices);
			glDisable(GL_POLYGON_OFFSET_LINE);

            glBindVertexArray(0);
        }

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
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