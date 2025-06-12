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
	float offset;
	GLuint TexID;
	GLuint VAO;
};

Sprite player;
vector<Sprite> meteors;
vector<Sprite> background;
GLuint createSprite();

int moveX;
int moveY;

bool checkCollision(Sprite object1, Sprite object2);
int setupShader();
int loadTexture(string filePath);

const GLuint WIN_WIDTH = 800, WIN_HEIGHT = 600;

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
const GLchar* fragmentShaderSource = "#version 400\n"
"uniform vec4 inputColor;\n"
"out vec4 color;\n"
"void main()\n"
"{\n"
"color = inputColor;\n"
"}\n\0";

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

	/****************************************************************/
	/*CRIANDO SPRITES*/
	Sprite sprite;
	sprite.VAO = createSprite();
	float spriteDimension;


	//SPRITES BACKGROUND (4 para fazer infinito)
/* 	sprite.dimensions = vec3(WIN_HEIGHT,WIN_WIDTH,1.0);
	sprite.rotation = 90.0f;
	sprite.offset = 0.2f;
	sprite.TexID = loadTexture("../assets/background.png");
	sprite.position = vec3(WIN_WIDTH/2, WIN_HEIGHT/2, 0.0);
	background.push_back(sprite); */

	//SPRITES METEORS
	vector<string> images = {"../assets/Meteor_01.png", "../assets/Meteor_04.png", "../assets/Meteor_10.png", "../assets/Meteor_07.png","../assets/Meteor_06.png"};
	for(int i = 0; i < 8; i++){
		spriteDimension = rand() % 70 + 70;
		sprite.position = vec3( rand() % WIN_WIDTH , rand() % WIN_HEIGHT, 0.0);
		sprite.dimensions = vec3( spriteDimension , spriteDimension ,1.0);
		sprite.rotation = 0.0f;
		sprite.offset = 1 + (rand() % 80)/14;
		sprite.TexID = loadTexture(images[rand() % images.size()]);
		meteors.push_back(sprite); 
	}

	//SPRITE PLAYER
	sprite.position = vec3(WIN_WIDTH/2-200, WIN_HEIGHT/2-200, 0.0);
	sprite.dimensions = vec3(100,100,1.0);
	sprite.rotation = 25.0f;
	sprite.offset = 1.0f;
	sprite.TexID = loadTexture("../assets/ship.png");
	player = sprite;


	/****************************************************************/

	glUseProgram(shaderID);

	GLuint colorLoc = glGetUniformLocation(shaderID, "inputColor");

	glActiveTexture(GL_TEXTURE0); // Ativando o primeiro buffer de textura do OpenGL

	glUniform1i(glGetUniformLocation(shaderID, "tex_buff"), 0); // Criando a variável uniform pra mandar a textura pro shader

	glEnable(GL_DEPTH_TEST); // Habilita o teste de profundidade
	glDepthFunc(GL_ALWAYS); // Testa a cada ciclo

	glEnable(GL_BLEND); //Habilita a transparência -- canal alpha
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //Seta função de transparência

	mat4 projection = ortho(0.0, 800.0, 600.0, 0.0, -1.0, 1.0);
	glUniformMatrix4fv(glGetUniformLocation(shaderID, "projection"), 1, GL_FALSE, value_ptr(projection));

	vec3 positionMoved;
	mat4 model;

	float rotationGoal;

	while(!glfwWindowShouldClose(window)){

		glfwPollEvents();

		glClearColor(0,0,0,1);
		glClear(GL_COLOR_BUFFER_BIT);

		glLineWidth(10);
		glPointSize(20);

		moveX = 0;
		moveY = 0;

				const int stateUP = glfwGetKey(window, GLFW_KEY_UP);
		if(stateUP == GLFW_PRESS) {
			moveY = -5;
			//rotationGoal =  0.0;
		} 
		const int stateRIGHT = glfwGetKey(window, GLFW_KEY_RIGHT); 
		if(stateRIGHT == GLFW_PRESS) {
			moveX = +5;	
			//rotationGoal =  90.0;
		}
		const int stateDOWN = glfwGetKey(window, GLFW_KEY_DOWN);
		if(stateDOWN == GLFW_PRESS) {
			moveY = +5;
			//rotationGoal =  180.0;
		}
		const int stateLEFT = glfwGetKey(window, GLFW_KEY_LEFT);
		if(stateLEFT == GLFW_PRESS) {
			moveX = -5;	
			//rotationGoal = 270.0;
		}


		//Se qualquer tecla está pressionada faz a rotação correspondente
		if(stateUP == GLFW_PRESS || stateDOWN == GLFW_PRESS || stateRIGHT == GLFW_PRESS || stateLEFT == GLFW_PRESS){

			if(moveX > 0){
				rotationGoal = 90;
				if(moveY < 0) {
					rotationGoal -= 45;
				} else if(moveY > 0) {
					rotationGoal += 45;
				}
			} else if (moveX < 0){
				rotationGoal = 270;
				if(moveY < 0) {
					rotationGoal += 45;
				} else if(moveY > 0) {
					rotationGoal -= 45;
				}
			} else if(moveY > 0){
				rotationGoal = 0;
			} else if (moveY < 0) {
				rotationGoal = 180;
			}
			rotationGoal += 0;

			if(player.rotation != rotationGoal){
				//faz a rotação ao contrário se a rota mais próxima for passando pelo angulo 0
				if(player.rotation - rotationGoal > 180 || player.rotation - rotationGoal < -180){

					if(player.rotation - rotationGoal <= 0){
						player.rotation -= 5;

						if(player.rotation > rotationGoal) player.rotation = rotationGoal;
					
					} else if(player.rotation - rotationGoal > 0){
						player.rotation += 5;
						if(player.rotation < rotationGoal) player.rotation = rotationGoal;
					}

					//Se passou de 360 reseta pra 0 e se desceu abaixo de 0 reseta pra 360
					if(player.rotation > 360) { 	player.rotation = 0;  }
					else if(player.rotation < 0){ 	player.rotation = 360;	}

				//Faz a rotação normal, sem necessitar checar pelo 360
				} else {
					if(player.rotation - rotationGoal >= 0){
						if(player.rotation > rotationGoal) player.rotation -= 5;
						
					} else if(player.rotation - rotationGoal <= 0){
						if(player.rotation < rotationGoal) player.rotation += 5;
					}
				}
			}
		}

			
		//TRANSFORMAÇÕES E DRAW NAVE PLAYER
/* 		model = mat4(1);
		model = translate(model, player.position);
		model = rotate(model, radians(player.rotation), vec3(0.0, 0.0, 1.0));
		model = scale(model, player.dimensions); */

		glBindVertexArray(player.VAO);
				
		glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, GL_FALSE, value_ptr(model));
		glUniform4f(colorLoc, 255, 255, 255, 1.0f);
		glDrawArrays(GL_TRIANGLE_STRIP, 0 , 4);

		//TRANSFORMAÇÕES E DRAW METEOROS
		for(int i = 0; i < meteors.size(); i++){
			//MOVIMENTA OS METEOROS PARA DIREÇÃO CONTRARIA DO JOGADOR, CADA METEORO TEM SEU OFFSET, SE MOVIMENTANDO A VELOCIDADES DIFERENTES
			meteors[i].position = vec3(
				(meteors[i].position[0] - (moveX*meteors[i].offset)), 
				(meteors[i].position[1] - (moveY*meteors[i].offset)),
				meteors[i].position[2]);
			//QUANDO O METEORO SAÍ DO ESPAÇO DA TELA SUA POSIÇÃO É ENVIADA PARA O OUTRO EXTREMO
			if(meteors[i].position[0] < 0) meteors[i].position[0] = WIN_WIDTH;
			if(meteors[i].position[0] > WIN_WIDTH) meteors[i].position[0] = 0;
			if(meteors[i].position[1] < 0) meteors[i].position[1] = WIN_HEIGHT;
			if(meteors[i].position[1] > WIN_HEIGHT) meteors[i].position[1] = 0;
			//TRANSFORMAÇÕES
			model = mat4(1);
			model = translate(model, meteors[i].position);
			//model = rotate(model, radians(meteors[i].rotation), vec3(0.0, 0.0, 1.0));
			model = scale(model, meteors[i].dimensions);
			//DRAW METEOROS
			glBindVertexArray(meteors[i].VAO);
			glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, GL_FALSE, value_ptr(model));
			glUniform4f(colorLoc, 255, 0, 0, 1.0f);	
			//glBindTexture(GL_TEXTURE_2D, meteors[i].TexID); // Conectando ao buffer de textura
			glDrawArrays(GL_TRIANGLE_STRIP, 0 , 4);
		} 

		//TRANSFORMAÇÕES E DRAW NAVE PLAYER
		model = mat4(1);
		model = translate(model, player.position);
		//model = rotate(model, radians(player.rotation), vec3(0.0, 0.0, 1.0));
		model = scale(model, player.dimensions);
		glBindVertexArray(player.VAO);
		glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, GL_FALSE, value_ptr(model));
		glUniform4f(colorLoc, 255, 255, 255, 1.0f);
//		glBindTexture(GL_TEXTURE_2D, player.TexID); // Conectando ao buffer de textura
		glDrawArrays(GL_TRIANGLE_STRIP, 0 , 4);

		for(int i = 0; i < meteors.size(); i++){
			if(checkCollision(player, meteors[i]))
				printf("player colidiu :(");
		} 

		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}

float sen, cose;
int count = 0;

bool checkCollision(Sprite object1, Sprite object2){
	vector<vec3> p;
	vector<vec3> e;

	sen = sin(object1.rotation);
	cose = cos(object1.rotation);
	
	//ponto 0,0
	p.push_back(vec3(object1.position[0],object1.position[1], 0.0));
	//ponto 1,0
	p.push_back(vec3(
		 (object1.position[0]+object1.dimensions[0])*cose - object1.position[1]*sen,
		 object1.position[1]*sen + object1.position[1]*cose,
		0.0));
	//ponto 0,1
	p.push_back(vec3(
		object1.position[0]*cose - (object1.position[1]+object1.dimensions[1])*sen,
		(object1.position[1]+object1.dimensions[1])*sen + (object1.position[1]+object1.dimensions[1])*cose,
		0.0));
	//ponto 1,1
	p.push_back(vec3(
		(object1.position[0]+object1.dimensions[0])*cose - (object1.position[1]+object1.dimensions[1])*sen,
		(object1.position[1]+object1.dimensions[1])*sen + (object1.position[1]+object1.dimensions[1])*cose,
		0.0));

	//ARESTAS
	sen = sin(object2.rotation);
	cose = cos(object2.rotation);
	//aresta 0,0 - 1,0
	e.push_back(vec3(object2.position[0],object2.position[1], 0.0));
	e.push_back(vec3(
		 (object2.position[0]+object2.dimensions[0])*cose - object2.position[1]*sen,
		 object2.position[1]*sen + object2.position[1]*cose,
		0.0));
	//aresta 0,0 - 0,1
	e.push_back(vec3(object2.position[0],object2.position[1], 0.0));
	e.push_back(vec3(
		 (object2.position[0]+object2.dimensions[0])*cose - object2.position[1]*sen,
		 object2.position[1]*sen + object2.position[1]*cose,
		0.0));
	//aresta 1,0 - 1,1
	e.push_back(vec3(
		object2.position[0]*cose - (object2.position[1]+object2.dimensions[1])*sen,
		(object2.position[1]+object2.dimensions[1])*sen + (object2.position[1]+object2.dimensions[1])*cose,
		0.0));
	e.push_back(vec3(
		(object2.position[0]+object2.dimensions[0])*cose - (object2.position[1]+object2.dimensions[1])*sen,
		(object2.position[1]+object2.dimensions[1])*sen + (object2.position[1]+object2.dimensions[1])*cose,
		0.0));
	//aresta 0,1 - 1,1
		e.push_back(vec3(
		object2.position[0]*cose - (object2.position[1]+object2.dimensions[1])*sen,
		(object2.position[1]+object2.dimensions[1])*sen + (object2.position[1]+object2.dimensions[1])*cose,
		0.0));
	e.push_back(vec3(
		(object2.position[0]+object2.dimensions[0])*cose - (object2.position[1]+object2.dimensions[1])*sen,
		(object2.position[1]+object2.dimensions[1])*sen + (object2.position[1]+object2.dimensions[1])*cose,
		0.0));

	//verificar todos os 4 pontos do object1 com todas as arrestas do object2
	for(int i = 0; i < p.size(); i++){
		// Px > Jx - ((Jy-Py)*(Jx-Ix) / (Jy - Iy))
		// P = p[i]; 	Px = p[i][0]; 	Py = p[i][1]
		// J = e[j];	Jx = e[j][0];	Jy = e[j][1]
		// I = e[j+1];	Ix = e[j+1][0];	Iy = e[j+1][1]
		// p[i][0] > e[i][0] - ((e[i][1]-p[i][1])*(e[i][0]-e[i+1][0]) / (e[i][1] - e[i+1][1]))

		for(int j = 0; j < e.size(); j+=2){
			count = 0;

			if(p[i][0] > e[j][0] - ((e[j][1]-p[i][1])*(e[j][0]-e[j+1][0]) / (e[j][1] - e[j+1][1]))){
				count++;
				//printf("colisao %d", count);
				//printf("\nContador ponto %d - aresta %d", i, j);
				//printf("\n%f, %f", p[i][0],p[i][1]);

			}
			//printf("\nPonto %d - count %d", i, count);
			if(count%2==1){
				printf("\nColidiu");
				return true;
			}
		}
	}
	return false;
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