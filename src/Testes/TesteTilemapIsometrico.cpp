/*******DEPENDENCIAS*****/
#include <iostream>
#include <string>
#include <assert.h>
#include <cmath>
#include <vector>
using namespace std;
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
using namespace glm;
#include <cmath>


/****SHADERS****/
const GLchar *vertexShaderSource = R"(
 #version 400
 layout (location = 0) in vec3 position;
 layout (location = 1) in vec2 texc;
 out vec2 tex_coord;
 uniform mat4 model;
 uniform mat4 projection;
 void main() {
	tex_coord = vec2(texc.s, 1.0 - texc.t);
	gl_Position = projection * model * vec4(position, 1.0);
 }
)";

const GLchar *fragmentShaderSource = R"(
 #version 400
 in vec2 tex_coord;
 out vec4 color;
 uniform sampler2D tex_buff;
 uniform vec2 offsetTex;

 void main() {
	 color = texture(tex_buff,tex_coord + offsetTex);
 }
)";


/*******VARIÁVEIS E FUNÇÕES*****/
struct Tile{
    vec3 position;
    GLuint VAO;
    GLuint texture;
    int type;
    int maxTypes;
    float ds;
    float dt;
};

vector<Tile> tiles;

int tilemap[6][8] = {
    {2, 2, 2, 2, 0, 0, 0, 0},
    {2, 2, 2, 2, 2, 0, 0, 0},
    {2, 2, 2, 2, 2, 2, 2, 2},
    {2, 2, 2, 2, 4, 4, 4, 4},
    {2, 4, 4, 4, 4, 4, 4, 4},
    {4, 4, 4, 1, 1, 4, 4, 4}
};

GLuint createVAOTile(int type, float &ds, float &dt);

GLuint setupShader();

int loadTexture(string filePath, int &width, int &height);


/*****MAIN*****/
int main(){

    /****INICIALIZAÇÃO GLFW, GLAD E JANELA***/
    glfwInit();
	glfwWindowHint(GLFW_SAMPLES, 4);
	GLFWwindow *window = glfwCreateWindow(800, 600, "Isometrico teste 2", nullptr, nullptr);
	if (!window){
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
		return -1;
	}

   	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);


    /***SHADER***/
	GLuint shaderID = setupShader();
    glUseProgram(shaderID);


    /***CRIANDO 1 STRUCT TILE SIMPLES E INSERANDO NO VETOR 'TILES'***/
/*     
    Tile water;
    water.position = vec3(400, 300, 0.0);
    water.type = 4;
    water.maxTypes = 5;
    water.VAO = createVAOTile(water.maxTypes, water.ds, water.dt); 
    int imgWidth, imgHeight;
    water.texture = loadTexture("../assets/TileSet.png", imgWidth, imgHeight);

    tiles.push_back(water); */
    

    Tile tile;
    tile.maxTypes = 5;
    tile.VAO = createVAOTile(tile.maxTypes, tile.ds, tile.dt);
    int imgWidth, imgHeight;
    tile.texture = loadTexture("../assets/TileSet.png", imgWidth, imgHeight);

    int x, y;

    int tamanho = 50;

    for(int i = 0 ; i < 6; i ++){
        for(int j = 0 ; j < 8; j ++){
            x = (tamanho*j);
            y = (tamanho*i);
            tile.position = vec3(
                            x-y,
                            (x+y)/2,
                            0.0);
            tile.type = tilemap[i][j];
            tiles.push_back(tile);
        }   
    }

    /****CONFIGURAÇÕES DE TEXTURA****/
	glActiveTexture(GL_TEXTURE0); // Ativando o primeiro buffer de textura do OpenGL
	glUniform1i(glGetUniformLocation(shaderID, "tex_buff"), 0); // Criando a variável uniform pra mandar a textura pro shader
	glEnable(GL_DEPTH_TEST); // Habilita o teste de profundidade
	glDepthFunc(GL_ALWAYS); // Testa a cada ciclo
	glEnable(GL_BLEND); //Habilita a transparência -- canal alpha
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //Seta função de transparência


    /****PROJEÇÃO****/
	mat4 projection = ortho(0.0, 800.0, 600.0, 0.0, -1.0, 1.0);
	glUniformMatrix4fv(glGetUniformLocation(shaderID, "projection"), 1, GL_FALSE, value_ptr(projection));


    /*****LOOP DA APLICAÇÃO***/
    while(!glfwWindowShouldClose(window)){

        /***RESET***/
		glfwPollEvents();
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // cor de fundo
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glLineWidth(10);
		glPointSize(20);

        /*DESENHA TODOS TILES NO VETOR 'TILES' */
        for(int i = 0; i < tiles.size(); i++){   
            mat4 model = mat4(1);
            model = translate(model, vec3(350, 100, 0.0));
            model = translate(model,tiles[i].position);
            model = scale(model,vec3(tamanho,tamanho,1.0));
            glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, GL_FALSE, value_ptr(model));
            glUniform2f(glGetUniformLocation(shaderID, "offsetTex"), tiles[i].type * tiles[i].ds, 1.0); 
            glBindVertexArray(tiles[i].VAO); // Conectando ao buffer de geometria
            glBindTexture(GL_TEXTURE_2D, tiles[i].texture); // Conectando ao buffer de textura
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }

		glfwSwapBuffers(window);
    }

	glfwTerminate();
	return 0;
}




/***RETORNA VAO DE UM QUADRADO PADRÃO COM SUPORTE APENAS PARA TEXTURA***/
GLuint createVAOTile(int type, float &ds, float &dt){

	ds = 1.0 / (float) type;
    dt = 1.0;

    GLfloat vertices[] = {
		// x    y    z    s   t
		-1,  0,  0.0, 0.0, dt,
		0, -0.5, 0.0, 0.0, 0.0,
		0,  0.5, 0.0, ds, dt, 
	    1, 0,    0.0, ds, 0.0  
	};

	GLuint VBO, VAO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid *)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid *)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	return VAO;
}


GLuint setupShader(){
	// Vertex shader
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	// Checando erros de compilação (exibição via log no terminal)
	GLint success;
	GLchar infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success){
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
				  << infoLog << std::endl;
	}
	// Fragment shader
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	// Checando erros de compilação (exibição via log no terminal)
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success){
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
				  << infoLog << std::endl;
	}
	// Linkando os shaders e criando o identificador do programa de shader
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	// Checando por erros de linkagem
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success){
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
				  << infoLog << std::endl;
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return shaderProgram;
}


int loadTexture(string filePath, int &width, int &height){
	GLuint texID;
	// Gera o identificador da textura na memória
	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	int nrChannels;

	unsigned char *data = stbi_load(filePath.c_str(), &width, &height, &nrChannels, 0);

	if (data){
		if (nrChannels == 3){
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		}else{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}
		glGenerateMipmap(GL_TEXTURE_2D);
	}else{
		std::cout << "Failed to load texture" << std::endl;
	}

	stbi_image_free(data);

	glBindTexture(GL_TEXTURE_2D, 0);

	return texID;
}