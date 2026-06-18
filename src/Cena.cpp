#include "Cena.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// Função para carregar um modelo OBJ simples, retornando o VAO e preenchendo o número de vértices, caminho da textura e propriedades de material
int loadSimpleOBJ(std::string filePath, int &nVertices, std::string &texturePath, glm::vec3 &ka, glm::vec3 &kd, glm::vec3 &ks)
{
    ka = glm::vec3(0.1f);
    kd = glm::vec3(0.7f);
    ks = glm::vec3(0.5f);
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> texCoords;
    std::vector<glm::vec3> normals;
    std::vector<GLfloat> vBuffer;

    std::ifstream arqEntrada(filePath.c_str());
    if (!arqEntrada.is_open()) 
    {
        std::cerr << "Erro ao tentar ler o arquivo " << filePath << std::endl;
        return -1;
    }

    std::string line;
    while (std::getline(arqEntrada, line)) 
    {
        std::istringstream ssline(line);
        std::string word;
        ssline >> word;

        if (word == "mtllib") 
        {
            std::string mtlFileName;
            ssline >> mtlFileName;
            std::string basePath = filePath.substr(0, filePath.find_last_of('/') + 1);
            
            std::ifstream arqMTL(basePath + mtlFileName);
            if (arqMTL.is_open()) 
            {
                std::string linhaMTL;
                while (std::getline(arqMTL, linhaMTL)) 
                {
                    std::istringstream ssMTL(linhaMTL);
                    std::string wordMTL;
                    ssMTL >> wordMTL;
                    if (wordMTL == "map_Kd") 
                    {
                        std::string nomeImagem;
                        ssMTL >> nomeImagem;
                        texturePath = basePath + nomeImagem; 
                    } else if (wordMTL == "Ka") {
                        ssMTL >> ka.r >> ka.g >> ka.b;
                    } else if (wordMTL == "Kd") {
                        ssMTL >> kd.r >> kd.g >> kd.b;
                    } else if (wordMTL == "Ks") {
                        ssMTL >> ks.r >> ks.g >> ks.b;
                    }
                }
                arqMTL.close();
            }
        }
        else if (word == "v") 
        {
            glm::vec3 vertice;
            ssline >> vertice.x >> vertice.y >> vertice.z;
            vertices.push_back(vertice);
        } 
        else if (word == "vt") 
        {
            glm::vec2 vt;
            ssline >> vt.s >> vt.t;
            texCoords.push_back(vt);
        } 
        else if (word == "vn") 
        {
            glm::vec3 normal;
            ssline >> normal.x >> normal.y >> normal.z;
            normals.push_back(normal);
        } 
        else if (word == "f")
        {
            while (ssline >> word) 
            {
                int vi = 0, ti = 0, ni = 0;
                std::istringstream ss(word);
                std::string index;

                if (std::getline(ss, index, '/')) vi = !index.empty() ? std::stoi(index) - 1 : 0;
                if (std::getline(ss, index, '/')) ti = !index.empty() ? std::stoi(index) - 1 : 0;
                if (std::getline(ss, index)) ni = !index.empty() ? std::stoi(index) - 1 : 0;

                vBuffer.push_back(vertices[vi].x);
                vBuffer.push_back(vertices[vi].y);
                vBuffer.push_back(vertices[vi].z);
                
                vBuffer.push_back(normals[ni].x);
                vBuffer.push_back(normals[ni].y);
                vBuffer.push_back(normals[ni].z);

                vBuffer.push_back(texCoords[ti].s);
                vBuffer.push_back(texCoords[ti].t);
            }
        }
    }

    arqEntrada.close();

    GLuint VBO, VAO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vBuffer.size() * sizeof(GLfloat), vBuffer.data(), GL_STATIC_DRAW);
    
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    
    // Posição
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    // Normais
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    
    // Coordenadas de textura
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    nVertices = vBuffer.size() / 8; // x, y, z, nx, ny, nz, s, t (valores atualmente armazenados por vértice)

    return VAO;
}

// Função para carregar uma textura
GLuint loadTexture(std::string filePath) {
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

// Função para carregar a cena a partir de um arquivo txt.
// Preenche os objetos, luzes e parâmetros da câmera
void loadScene(std::string filePath, Camera &camera, std::vector<Object3D> &objects, std::vector<Light> &lights, float &fov, float &zNear, float &zFar) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Erro: Nao foi possivel abrir o arquivo de cena " << filePath << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string type;
        ss >> type;

        if (type == "CAMERA") {
            float px, py, pz, yaw, pitch;
            ss >> px >> py >> pz >> yaw >> pitch >> fov >> zNear >> zFar;
            camera.Position = glm::vec3(px, py, pz);
            camera.Yaw = yaw;
            camera.Pitch = pitch;
            camera.updateCameraVectors();
        }
        else if (type == "OBJECT") {
            std::string objPath;
            float px, py, pz, scale, rot;
            ss >> objPath >> px >> py >> pz >> scale >> rot;

            int nVertices = 0;
            std::string texturePath = "";
            glm::vec3 ka, kd, ks;
            
            GLuint vao = loadSimpleOBJ(objPath, nVertices, texturePath, ka, kd, ks);
            
            Object3D obj;
            obj.VAO = vao;
            obj.nVertices = nVertices;
            obj.textureID = loadTexture(texturePath);
            obj.position = glm::vec3(px, py, pz);
            obj.scale = glm::vec3(scale);
            // A rotação é aplicada apenas no eixo Y, permitindo que um objeto gire em torno de si mesmo
            obj.rotation = glm::vec3(0.0f, rot, 0.0f);
            obj.ka = ka; 
            obj.kd = kd; 
            obj.ks = ks;
            obj.tParam = 0.0f;
            obj.isMoving = false;
            
            objects.push_back(obj);
        } else if (type == "PATH") {
            if (!objects.empty()) {
                float px, py, pz;
                ss >> px >> py >> pz;
                objects.back().pathPoints.push_back(glm::vec3(px, py, pz));
                
                // O cálculo da curva de Bézier exige pelo menos 4 pontos
                if (objects.back().pathPoints.size() >= 4) {
                    objects.back().isMoving = true;
                }
            }
        } else if (type == "LIGHT") {
            Light l;
            ss >> l.position.x >> l.position.y >> l.position.z >> l.kl >> l.kq;
            lights.push_back(l);
        }
    }
    file.close();
    std::cout << "Cena carregada com sucesso! " << objects.size() << " objetos instanciados." << std::endl;
}