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
	float offset; 	//multiplicar offset*moveX quando calculando a posição; 1 se distancia padrão, 0.8 -> mais longe, se move mais lentamente
	GLuint TexID;
	GLuint VAO;
};

Sprite player, player2;
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
	GLFWwindow *window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, "Teste Colisao 1", nullptr, nullptr);

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

	//SPRITE PLAYER
	sprite.position = vec3(300, 200, 0.0);
	sprite.dimensions = vec3(100,100,1.0);
	sprite.rotation = 0.0f;
	sprite.offset = 1.0f;
	sprite.TexID = loadTexture("../assets/ship.png");
	player = sprite;

	
	//SPRITE PLAYER
	sprite.position = vec3(200, 200, 0.0);
	sprite.dimensions = vec3(100, 100,1.0);
	sprite.rotation = 45.0f;
	sprite.offset = 1.0f;
	sprite.TexID = loadTexture("../assets/ship.png");
	player2 = sprite;

	checkCollision(player,player2);



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


		const int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
		if(state == GLFW_PRESS){
            double mx, my;
            glfwGetCursorPos(window, &mx, &my);

			printf("\n(%d, %d)", (int)mx, (int)my);

		}
			
		//TRANSFORMAÇÕES E DRAW NAVE PLAYER
		model = mat4(1);
		model = translate(model, player.position);
		model = rotate(model, radians(player.rotation), vec3(0.0, 0.0, 1.0));
		model = scale(model, player.dimensions);

		glBindVertexArray(player.VAO);
				
		glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, GL_FALSE, value_ptr(model));
		glUniform4f(colorLoc, 255, 255, 255, 1.0f);
		glDrawArrays(GL_TRIANGLE_STRIP, 0 , 4);

		model = mat4(1);
		model = translate(model, player2.position);
		model = rotate(model, radians(player2.rotation), vec3(0.0, 0.0, 1.0));
		model = scale(model, player2.dimensions);

		glBindVertexArray(player2.VAO);
		glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, GL_FALSE, value_ptr(model));
		glUniform4f(colorLoc, 255, 255, 255, 1.0f);
		glDrawArrays(GL_TRIANGLE_STRIP, 0 , 4);

		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}


bool checkCollision(Sprite ship, Sprite object2){

	float top_1 = ship.position[1];
	float bottom_1 = ship.position[1] + ship.dimensions[1];
	float left_1 = ship.position[0];
	float right_1 = ship.position[0] + ship.dimensions[1];

	float sen = sin(object2.rotation);
	float cose = cos(object2.rotation);

	//https://math.stackexchange.com/questions/2790488/how-to-find-the-dimensions-new-points-after-rotating-a-shape

	float xmin = object2.position[0];
	float ymin = object2.position[1];

	float xmax = object2.position[0]+object2.dimensions[0];
	float ymax = object2.position[1]+object2.dimensions[1];

	float x0 = (xmin + xmax) / 2; 
	float y0 = (ymin + ymax) / 2;

	float x1 = x0 + (xmin - x0)*cose - (ymin - y0)*sen;
	float y1 = y0 + (xmin - x0)*sen + (ymin - y0)*cose;

	float x2 = x0 + (xmax - x0)*cose - (ymin - y0)*sen;
	float y2 = y0 + (xmax - x0)*sen + (ymin - y0)*cose;

	float x3 = x0 + (xmin - x0)*cose - (ymax - y0)*sen;
	float y3 = y0 + (xmin - x0)*sen + (ymax - y0)*cose;

	float x4 = x0 + (xmax - x0)*cose - (ymax - y0)*sen;
	float y4 = y0 + (xmax - x0)*sen + (ymax - y0)*cose;

	float left_2 = fmin(fmin(x1, x2), fmin(x3, x4));
	float top_2 = fmin(fmin(y1, y2), fmin(y3, y4));
	float right_2 = fmax(fmax(x1, x2), fmax(x3, x4));
	float bottom_2 = fmax(fmax(y1, y2), fmax(y3, y4));

	if(bottom_1 < top_2){
		return false;
	}

	if(bottom_2 < top_1){
		return false;
	}

	if(right_1 < left_2){
		return false;
	}

	if(right_2 < left_1){
		return false;
	}

	printf("COLISAO");
	return true;
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