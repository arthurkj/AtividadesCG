/*
 *  Codificado por Rossana Baptista Queiroz
 *  para as disciplinas de Processamento Gráfico/Computação Gráfica - Unisinos
 *  Versão inicial: 07/04/2017
 *  Última atualização: 14/03/2025
 *
 *  Este arquivo contém a função `loadSimpleOBJ`, responsável por carregar arquivos
 *  no formato Wavefront .OBJ e armazenar seus vértices em um VAO para renderização
 *  com OpenGL.
 *
 *  Forma de uso (carregamento de um .obj)
 *  -----------------
 *  ...
 *  int nVertices;
 *  GLuint objVAO = loadSimpleOBJ("../Modelos3D/Cube.obj", nVertices);
 *  ...
 *
 *  Chamada de desenho (Polígono Preenchido - GL_TRIANGLES), no loop do programa:
 *  ----------------------------------------------------------
 *  ...
 *  glBindVertexArray(objVAO);
 *  glDrawArrays(GL_TRIANGLES, 0, nVertices);
 *
 */

 // Cabeçalhos necessários (para esta função), acrescentar ao seu código 
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
 
 
using namespace std;

// GLAD
#include <glad/glad.h>
 
// GLFW
#include <GLFW/glfw3.h>
 
//GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct Mesh 
{
    GLuint VAO; 

};

int loadSimpleOBJ(string filePATH, int &nVertices, string &texturePath)
{
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> texCoords;
    std::vector<glm::vec3> normals;
    std::vector<GLfloat> vBuffer;

    std::ifstream arqEntrada(filePATH.c_str());
    if (!arqEntrada.is_open()) 
    {
        std::cerr << "Erro ao tentar ler o arquivo " << filePATH << std::endl;
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
            string mtlFileName;
            ssline >> mtlFileName;
            string basePath = filePATH.substr(0, filePATH.find_last_of('/') + 1);
            
            std::ifstream arqMTL(basePath + mtlFileName);
            if (arqMTL.is_open()) 
            {
                string linhaMTL;
                while (std::getline(arqMTL, linhaMTL)) 
                {
                    std::istringstream ssMTL(linhaMTL);
                    string wordMTL;
                    ssMTL >> wordMTL;
                    if (wordMTL == "map_Kd") 
                    {
                        string nomeImagem;
                        ssMTL >> nomeImagem;
                        texturePath = basePath + nomeImagem; 
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
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    nVertices = vBuffer.size() / 5;

    return VAO;
}