#include <iostream>
#include <string>
#include <assert.h>
#include <cmath>
#include <thread>
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

#include <fstream>

//SHADERS
const GLchar *vertexShaderSource = R"(
#version 400
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texc;
out vec2 tex_coord;
uniform mat4 model;
uniform mat4 projection;
void main()
{
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

void main()
{
	color = texture(tex_buff,tex_coord + offsetTex);
}
)";

//VARIÁVEIS PERSONAGEM
struct Player {
	GLuint VAO;
	GLuint texID;
	vec3 position;
	vec3 dimensions; //tamanho do frame
	float ds, dt;
	int iAnimation, iFrame;
	int nAnimations, nFrames;
	bool isWalking = false;
    int tileMapLine = 1;
    int tileMapColumn = 1;
};

Player personagem;

int setupVAOSprite(int nAnimations, int nFrames, float &ds, float &dt);

bool move(int iAnimation, int toTileMapLine, int toTileMapColumn);

//VARIÁVEIS TILEMAP
int TILEMAP_WIDTH;
int TILEMAP_HEIGHT;

int t_width;
int t_height;

struct Tile {
    GLuint VAO;
    GLuint texID;
    int iTile;
    vec3 position;
    vec3 dimensions; // tamanho do losango 2:1
    float ds, dt;
};

int setupVAOTile(int nTiles, float &ds, float &dt);

vector<Tile> tileset;

vector<int> VALID_TILES;

void desenharMapa(GLuint shaderID);

void readConfigFile(string filename);

float tile_inicial_x; // centro do eixo x - o valor da metade da largura para centralizar o tilemap na janela
float tile_inicial_y; // divisão da altura da janela pela quantidade de linhas + metade do valor da altura para centralizar o tilemap também no eixo y

vector<int> map;

//VARIÁVEIS ITENS
struct Collectible {
	GLuint VAO;
	GLuint texID;
	vec3 position;
	vec3 dimensions;
	float ds, dt;
    int tileMapLine;
    int tileMapColumn;
};

vector<Collectible> collectibles;

int itens_coletados = 0;
int total_itens = 0;

void desenharItens(GLuint shaderID);



void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);

int setupShader();

int loadTexture(string filePath, int &width, int &height);

const GLuint WIDTH = 1000, HEIGHT = 800;


int main(){

	glfwInit();
	glfwWindowHint(GLFW_SAMPLES, 8);
	GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Jogo coleta - GrauB", nullptr, nullptr);
	if (!window){
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, key_callback);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
		return -1;
	}

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	//CARREGANDO SHADER
	GLuint shaderID = setupShader();
	glUseProgram(shaderID);

	readConfigFile("../src/tilemap.txt");

	tile_inicial_x = 400 - t_height; // centro do eixo x - o valor da metade da largura para centralizar o tilemap na janela
	tile_inicial_y = (600 / TILEMAP_HEIGHT) + t_height/2; // divisão da altura da janela pela quantidade de linhas + metade do valor da altura para centralizar o tilemap também no eixo y
	
	//CARREGANDO PERSONAGEM
	int imgWidth, imgHeight;
	personagem.nAnimations = 4;
	personagem.nFrames = 10;
	personagem.VAO = setupVAOSprite(personagem.nAnimations,personagem.nFrames,personagem.ds,personagem.dt);
	personagem.position = vec3(0, 0, 1.0);
	personagem.dimensions = vec3(35, 35, 1.0);
	GLuint personagemID = loadTexture("../assets/sprites/sprite_full.png",imgWidth,imgHeight);
	personagem.texID = personagemID;
	personagem.iAnimation = 1;
	personagem.iFrame = 0;

	//ATIVANDO TEXTURA
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(shaderID, "tex_buff"), 0);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_ALWAYS);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//MATRIZ DE PROJEÇÃO
	mat4 projection = ortho(0.0, 800.0, 0.0, 600.0, -1.0, 1.0);
	glUniformMatrix4fv(glGetUniformLocation(shaderID, "projection"), 1, GL_FALSE, value_ptr(projection));

	double lastTime = 0.0;
	double deltaT = 0.0;
	double currTime = glfwGetTime();
	double FPS = 12.0;

	bool venceu = false;

	while (!glfwWindowShouldClose(window))	{
		
		glfwPollEvents();

		glClearColor(0.54f, 0.77f, 0.98f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glLineWidth(10);
		glPointSize(20);

		if (!venceu && itens_coletados == total_itens && map[personagem.tileMapLine-1 + ((personagem.tileMapColumn-1)*TILEMAP_HEIGHT)] == 6 ) {
            venceu = true;
            std::cout << "Voce venceu! Todos os itens coletados." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(2));
            glfwSetWindowShouldClose(window, GL_TRUE);
        }

        desenharMapa(shaderID);
		desenharItens(shaderID);

		// Desenho do personagem
        personagem.position.x = tile_inicial_x + t_height + (personagem.tileMapColumn - personagem.tileMapLine) * t_height;
        personagem.position.y = tile_inicial_y + (personagem.tileMapColumn + personagem.tileMapLine) * (t_height/2);
        mat4 model = mat4(1);
		model = mat4(1);
		model = translate(model, personagem.position);
		model = rotate(model, radians(0.0f), vec3(0.0, 0.0, 1.0));
		model = scale(model,personagem.dimensions);
		glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, GL_FALSE, value_ptr(model));

		currTime = glfwGetTime();
		deltaT = currTime - lastTime;
		vec2 offsetTex;
		
		if (deltaT >= 1.0/FPS){
			if(personagem.isWalking) {
				personagem.iFrame = (personagem.iFrame + 1) % personagem.nFrames; // incremento "circular"
			}
			lastTime = currTime;
		}
		
		offsetTex.s = personagem.iFrame * personagem.ds;
		offsetTex.t = (personagem.iAnimation) * personagem.dt;
		glUniform2f(glGetUniformLocation(shaderID, "offsetTex"),offsetTex.s, offsetTex.t);

		glBindVertexArray(personagem.VAO);
		glBindTexture(GL_TEXTURE_2D, personagem.texID);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		glfwSwapBuffers(window);
	}
		
	glfwTerminate();
	return 0;
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode){

	personagem.isWalking = true;

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS){
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
	if (action != GLFW_PRESS && action != GLFW_REPEAT){
		personagem.isWalking = false;
	}
	
	if (key == GLFW_KEY_LEFT && action == GLFW_PRESS){
		move(2, personagem.tileMapLine, personagem.tileMapColumn-1);
    
	} else if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS){
		move(4, personagem.tileMapLine, personagem.tileMapColumn+1);
    
	} else if (key == GLFW_KEY_UP && action == GLFW_PRESS){
		move(3, personagem.tileMapLine+1, personagem.tileMapColumn);
		
	} else if (key == GLFW_KEY_DOWN && action == GLFW_PRESS){
		move(1, personagem.tileMapLine-1, personagem.tileMapColumn);
    
	} else if (key == GLFW_KEY_A && action == GLFW_PRESS){
		move(2, personagem.tileMapLine+1, personagem.tileMapColumn-1);
    
	} else if (key == GLFW_KEY_D && action == GLFW_PRESS){
		move(4, personagem.tileMapLine-1, personagem.tileMapColumn+1);
    
	} else if (key == GLFW_KEY_W && action == GLFW_PRESS){
		move(3, personagem.tileMapLine+1, personagem.tileMapColumn+1);
    
	} else if (key == GLFW_KEY_S && action == GLFW_PRESS){
		move(1, personagem.tileMapLine-1, personagem.tileMapColumn-1);
    } 
}

bool move(int iAnimation, int toTileMapLine, int toTileMapColumn){

	for(int i = 0; i < VALID_TILES.size(); i++){
		if(VALID_TILES[i] == map[toTileMapLine-1 + ((toTileMapColumn-1)*TILEMAP_HEIGHT)]){
			personagem.iAnimation = iAnimation;

			//NÃO DEIXA O PERSONAGEM SAIR DO MAPA
			personagem.tileMapLine = fmax(1, fmin(toTileMapLine, TILEMAP_WIDTH));
			personagem.tileMapColumn = fmax(1, fmin(toTileMapColumn, TILEMAP_HEIGHT));

			//MUDA TILE DE PEDRA (2) PARA LAVA (3)
			if(map[toTileMapLine-1 + ((toTileMapColumn-1)*TILEMAP_HEIGHT)] == 2) map[toTileMapLine-1 + ((toTileMapColumn-1)*TILEMAP_HEIGHT)] = 3;

			//VERIFICA COLETA DE ITENS
			for (int i = 0; i < collectibles.size(); i++) {
				if (personagem.tileMapLine == collectibles[i].tileMapLine && personagem.tileMapColumn == collectibles[i].tileMapColumn) {
					collectibles.erase(collectibles.begin() + i); // Remove o item coletado da lista
					itens_coletados++;
					if(itens_coletados == total_itens)
						std::cout << "Agora vá até o tile objetivo para vencer!" << std::endl;
			
					std::cout << "Coletou um item! Total: " << itens_coletados << std::endl;
					break; // Importante para evitar acessar como índice inválido
				}
			}

			return true;
		}
	}
	return false;
}

void readConfigFile(string filename) {
	ifstream file(filename);

	//NOME DO ARQUIVO DE TEXTURA
	string tilemapFile;
	file >> tilemapFile;
	string texture = "../assets/tilesets/" + tilemapFile;
    int imgWidth, imgHeight;
    GLuint texID = loadTexture(texture, imgWidth, imgHeight);

	//NUMERO DE TILES NO ARQUIVO
	string num_tiles;
	file >> num_tiles;   

	//DIMENSOES TILE
	string tile_height, tile_width;
	file >> tile_height;    
	file >> tile_width;
	
	t_height = stoi(tile_height);
	t_width = stoi(tile_width);

    for (int i = 0; i < stoi(num_tiles); i++){
        Tile tile;
        tile.dimensions = vec3(stoi(tile_width), stoi(tile_height), 1.0);
        tile.iTile = i;
        tile.texID = texID;
        tile.VAO = setupVAOTile( stoi(num_tiles), tile.ds, tile.dt);
        tileset.push_back(tile);
    }

	//TAMANHO MATRIZ DO MAP
	string tilemap_h, tilemap_w;
	file >> tilemap_h;    
	file >> tilemap_w;   
	TILEMAP_HEIGHT = stoi(tilemap_h);
	TILEMAP_WIDTH = stoi(tilemap_w);

	//CRIA O TILE MAP
	string type;
	for(int i = 0 ; i < TILEMAP_HEIGHT*TILEMAP_WIDTH; i ++){
		file >> type;
		map.push_back(stoi(type));
	} 

	//ADICIONA TILES 'CAMINHÁVEIS' PRA CONSTANTE 'VALID_TILES'
	string currentEl;
	do {
		file >> currentEl;
		if(stoi(currentEl) == -1) break;
		VALID_TILES.push_back(stoi(currentEl));
	} while (true);


	//CRIA OS COLETÁVEIS
	Collectible collectibleEl;
	string textureCollectible;
	file >> textureCollectible;
	collectibleEl.texID = loadTexture("../assets/" + textureCollectible, imgWidth, imgHeight);
	collectibleEl.VAO = setupVAOSprite(1, 1, collectibleEl.ds, collectibleEl.dt);

	string posX, posY;
	while(file >> posX){ 
		file >> posY;
		collectibleEl.tileMapLine = stoi(posX);
		collectibleEl.tileMapColumn = stoi(posY);
		collectibles.push_back(collectibleEl);
		total_itens++;
	} 

	file.close();
}

int setupVAOSprite(int nAnimations, int nFrames, float &ds, float &dt){

	ds = 1.0 / (float) nFrames;
	dt = 1.0 / (float) nAnimations;
	
	GLfloat vertices[] = {
		// x   y    z    s     t
		-0.5,  0.5, 0.0, 0.0, dt, //V0
		-0.5, -0.5, 0.0, 0.0, 0.0, //V1
		 0.5,  0.5, 0.0, ds, dt, //V2
		 0.5, -0.5, 0.0, ds, 0.0  //V3
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

int setupVAOTile(int nTiles, float &ds, float &dt){
    ds = 1.0 / (float)nTiles;
    dt = 1.0;

    float th = 1.0, tw = 1.0;

    GLfloat vertices[] = {
        // x   y    z    s     t
        0.0, th / 2.0f, 0.0, 0.0, dt / 2.0f, // A
        tw / 2.0f, th, 0.0, ds / 2.0f, dt,   // B
        tw / 2.0f, 0.0, 0.0, ds / 2.0f, 0.0, // D
        tw, th / 2.0f, 0.0, ds, dt / 2.0f    // C
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

void desenharMapa(GLuint shaderID){
    for (int i = 0; i < TILEMAP_HEIGHT; i++){
        for (int j = 0; j < TILEMAP_WIDTH; j++){
			Tile curr_tile = tileset[map[i + j * TILEMAP_HEIGHT]];
			
            float x = tile_inicial_x + (j - i) * curr_tile.dimensions.x/2.0;
            float y = tile_inicial_y + (i + j) * curr_tile.dimensions.y/2.0;
			
			mat4 model = mat4(1);
            model = translate(model, vec3(x, y, 0.0));
            model = scale(model, curr_tile.dimensions);
            glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, GL_FALSE, value_ptr(model));

            vec2 offsetTex;
            offsetTex.s = curr_tile.iTile * curr_tile.ds;
            offsetTex.t = 0.0;
            glUniform2f(glGetUniformLocation(shaderID, "offsetTex"), offsetTex.s, offsetTex.t);
            glBindVertexArray(curr_tile.VAO);              
            glBindTexture(GL_TEXTURE_2D, curr_tile.texID); 
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }
    }
}

void desenharItens(GLuint shaderID){
    for (int i = 0; i < collectibles.size(); i++){
			Collectible curr_collectible = collectibles[i];
			
			float x = tile_inicial_x + t_height + (collectibles[i].tileMapColumn - collectibles[i].tileMapLine) * t_height;
			float y = tile_inicial_y + (collectibles[i].tileMapColumn + collectibles[i].tileMapLine) * (t_height/2);

			mat4 model = mat4(1);
			model = translate(model,vec3(x,y,0.0));
            model = scale(model, vec3(20,20,0.0));
            glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, GL_FALSE, value_ptr(model));

            vec2 offsetTex;
            offsetTex.s = 1.0;
            offsetTex.t = 1.0;
            glUniform2f(glGetUniformLocation(shaderID, "offsetTex"), offsetTex.s, offsetTex.t);
            glBindVertexArray(curr_collectible.VAO);              
            glBindTexture(GL_TEXTURE_2D, curr_collectible.texID); 
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
}

int loadTexture(string filePath, int &width, int &height){
	GLuint texID;

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
		}
		else{
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

int setupShader(){
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	GLint success;
	GLchar infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
				  << infoLog << std::endl;
	}
	// Fragment shader
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
				  << infoLog << std::endl;
	}

    GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
				  << infoLog << std::endl;
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return shaderProgram;
}