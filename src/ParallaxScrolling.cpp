#include <iostream>
#include <string>
#include <assert.h>
#include <vector>

using namespace std;

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// STB_IMAGE
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using namespace glm;

#include <cmath>

#include <ctime>

struct Sprite {
	vec3 position;
	vec3 dimensions;
	float rotation;
	float offset; 	//multiplicar offset*moveX quando calculando a posição
					//1 se distancia padrão, 0.8 -> mais longe, se move mais lentamente
	GLuint TexID;
	GLuint VAO;
};

Sprite player;
vector<Sprite> meteors;
vector<Sprite> background;

int moveX;
int moveY;

int setupShader();

int loadTexture(string filePath);

const GLuint WIN_WIDTH = 800, WIN_HEIGHT = 600;

GLuint createSprite();

const GLuint Sprite_WIDTH = 100, Sprite_HEIGHT = 100;

// Código fonte do Vertex Shader (em GLSL): ainda hardcoded
const GLchar *vertexShaderSource = R"(
 #version 400
 layout (location = 0) in vec3 position;
 layout (location = 1) in vec3 color;
 layout (location = 2) in vec2 texc;
 uniform mat4 projection;
 uniform mat4 model;
 out vec3 vColor;
 out vec2 tex_coord;
 void main()
 {
	gl_Position = projection * model * vec4(position.x, position.y, position.z, 1.0);
	vColor = color;
	tex_coord = texc;
 }
)";

// Código fonte do Fragment Shader (em GLSL): ainda hardcoded
const GLchar *fragmentShaderSource = R"(
 #version 400
 in vec3 vColor;
 in vec2 tex_coord;
 out vec4 color;
 uniform sampler2D tex_buff;
 void main()
 {
	 color = texture(tex_buff,tex_coord);//vec4(vColor,1.0);
 }
)";

int main(){

	srand(time(0));

	glfwInit();

	glfwWindowHint(GLFW_SAMPLES, 8);

	GLFWwindow *window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, "Jogo das cores M3", nullptr, nullptr);

	if(!window){
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);	

	if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
		return -1;
	}

	glViewport(0,0,WIN_WIDTH, WIN_HEIGHT);

	GLuint shaderID = setupShader();

	Sprite Sprite;
	Sprite.VAO = createSprite();
	
	//SPRITES BACKGROUND (4 para fazer infinito)

	Sprite.dimensions = vec3(WIN_HEIGHT,WIN_WIDTH,1.0);
	Sprite.rotation = 90.0f;
	Sprite.offset = 0.2f;
	Sprite.TexID = loadTexture("../assets/background.png");
	Sprite.position = vec3(WIN_WIDTH/2, WIN_HEIGHT/2, 0.0);
	background.push_back(Sprite);
	//GRID 3X3, quando o player chega em uma nova porção 
	for(int i = 0; i < 9; i++){
		spriteDimension = rand() % 70 + 70;
		Sprite.dimensions = vec3( spriteDimension , spriteDimension ,1.0);
		Sprite.position = vec3( rand() % WIN_WIDTH , rand() % WIN_HEIGHT, 0.0);
		Sprite.rotation = rand() % 360;
		Sprite.offset = 0.4f + (rand() % 1);
		Sprite.TexID = loadTexture(images[rand() % images.size()]);

		meteors.push_back(Sprite);
	}

	//SPRITES METEORS
	vector<string> images = {"../assets/Meteor_01.png", "../assets/Meteor_04.png", "../assets/Meteor_10.png"};
	int spriteDimension;

	for(int i = 0; i < 8; i++){
		spriteDimension = rand() % 70 + 70;
		Sprite.dimensions = vec3( spriteDimension , spriteDimension ,1.0);
		Sprite.position = vec3( rand() % WIN_WIDTH , rand() % WIN_HEIGHT, 0.0);
		Sprite.rotation = rand() % 360;
		Sprite.offset = 0.4f + (rand() % 1);
		Sprite.TexID = loadTexture(images[rand() % images.size()]);

		meteors.push_back(Sprite);
	}

	//SPRITE PLAYER
	Sprite.position = vec3(WIN_WIDTH-60, WIN_HEIGHT-60, 0.0);
	Sprite.dimensions = vec3(360,360,1.0);
	Sprite.rotation = 45.0f;
	Sprite.offset = 1.0f;
	Sprite.TexID = loadTexture("../assets/ship.png");
	player = Sprite;

	glUseProgram(shaderID);

	glActiveTexture(GL_TEXTURE0); // Ativando o primeiro buffer de textura do OpenGL

	glUniform1i(glGetUniformLocation(shaderID, "tex_buff"), 0); // Criando a variável uniform pra mandar a textura pro shader

	mat4 projection = ortho(0.0, 800.0, 600.0, 0.0, -1.0, 1.0);
	glUniformMatrix4fv(glGetUniformLocation(shaderID, "projection"), 1, GL_FALSE, value_ptr(projection));

	while(!glfwWindowShouldClose(window)){

		glfwPollEvents();

		glClearColor(0,0,0,1);
		glClear(GL_COLOR_BUFFER_BIT);

		glLineWidth(10);
		glPointSize(20);

		moveX = 0;
		moveY = 0;

		const int stateD = glfwGetKey(window, GLFW_KEY_D); //EXEMPLO ISSO GLFW_KEY_E
		if(state == GLFW_PRESS){
			printf("KEY_D pressed")
			moveX = +5;
		}
		const int stateA = glfwGetKey(window, GLFW_KEY_A); //TESTE SE FUNCIONA ASSIM PRO D	
		if(state == GLFW_PRESS){
			printf("KEY_A pressed")
			moveX = -5;
		}
		const int stateD = glfwGetKey(window, GLFW_KEY_W); //TESTE SE FUNCIONA ASSIM PRO D	
		if(state == GLFW_PRESS){
			printf("KEY_W pressed")
			moveY = +5;
		}
		const int stateD = glfwGetKey(window, GLFW_KEY_W); //TESTE SE FUNCIONA ASSIM PRO D	
		if(state == GLFW_PRESS){
			printf("KEY_W pressed")
			moveY = -5;
		}

		for(int i = 0; i < meteors.size(); i++){
				
			/* Os valores devem ser invertidos a não ser no caso do "player",
			ou seja, ao aperta D o translate do jogador vai ser +moveX
			enquanto dos outros meteors será -moveY */
			mat4 model = mat4(1);
			model = translate(model, vec3(
				(meteors[i].position[0] * (moveX*meteors[i].offset)) % WIN_WIDTH, 
				(meteors[i].position[1] * (moveY*meteors[i].offset)) % WIN_HEIGHT,
				meteors[i].position[2]));
			model = rotate(model, radians(meteors[i].rotation), vec3(0.0, 0.0, 1.0));
			model = scale(model, meteors[i].dimensions);

			glBindVertexArray(meteors[i].VAO);
			
			glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, GL_FALSE, value_ptr(model));
			
			glBindTexture(GL_TEXTURE_2D, meteors[i].TexID); // Conectando ao buffer de textura

			glDrawArrays(GL_TRIANGLE_STRIP, 0 , 4);
		} 

		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}

int setupShader(){
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

GLuint createSprite(){
	GLfloat vertices[] = {
		// x   y     z    r   g    b    s     t
		-0.5, -0.5, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,// v0
		-0.5,  0.5, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0,// v2
		0.5,  -0.5, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0,// v1
		0.5,  0.5,  0.0, 0.0, 0.0, 0.0, 1.0, 1.0 // v3
	};

	GLuint VBO, VAO;
	// Geração do identificador do VBO
	glGenBuffers(1, &VBO);
	// Faz a conexão (vincula) do buffer como um buffer de array
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// Envia os dados do array de floats para o buffer da OpenGl
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	// Geração do identificador do VAO (Vertex Array Object)
	glGenVertexArrays(1, &VAO);
	// Vincula (bind) o VAO primeiro, e em seguida  conecta e seta o(s) buffer(s) de vértices e os ponteiros para os atributos
	glBindVertexArray(VAO);
	// Para cada atributo do vertice, criamos um "AttribPointer" (ponteiro para o atributo), indicando:
	//  Localização no shader * (a localização dos atributos devem ser correspondentes no layout especificado no vertex shader)
	//  Numero de valores que o atributo tem (por ex, 3 coordenadas xyz)
	//  Tipo do dado
	//  Se está normalizado (entre zero e um)
	//  Tamanho em bytes
	//  Deslocamento a partir do byte zero
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid *)0);
	glEnableVertexAttribArray(0); 

	// Ponteiro pro atributo 1 - Cor - componentes r,g e b
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid *)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	// Ponteiro pro atributo 2 - Coordenada de textura - coordenadas s,t
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid *)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	// Observe que isso é permitido, a chamada para glVertexAttribPointer registrou o VBO como o objeto de buffer de vértice
	// atualmente vinculado - para que depois possamos desvincular com segurança
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Desvincula o VAO (é uma boa prática desvincular qualquer buffer ou array para evitar bugs medonhos)
	glBindVertexArray(0);

	return VAO;
}

int loadTexture(string filePath){
	GLuint texID;

	// Gera o identificador da textura na memória
	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width, height, nrChannels;

	unsigned char *data = stbi_load(filePath.c_str(), &width, &height, &nrChannels, 0);

	if (data)
	{
		if (nrChannels == 3) // jpg, bmp
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		}
		else // png
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}

	stbi_image_free(data);

	glBindTexture(GL_TEXTURE_2D, 0);

	return texID;
}