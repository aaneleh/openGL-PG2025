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

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using namespace glm;

#include <cmath>
#include <ctime>

/*******SHADERS*******/
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


struct Sprite {
    vec3 position;
    vec3 dimensions;
    float rotation;
    vec3 velocity;
    GLuint texture;
    GLuint VAO;
};

Sprite ship;
vector<Sprite> meteors;
Sprite background;

Sprite createRandomMeteor();
bool checkCollision(Sprite object1, Sprite object2);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);
GLuint createVAO();
GLuint setupShader();
GLuint loadTexture(string filePath);

const GLuint WIDTH = 1000, HEIGHT = 800;
int NUM_METEORS = 20;


int main(){
	srand(time(0));
	glfwInit();
	glfwWindowHint(GLFW_SAMPLES, 8);
	GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Jogo nave", nullptr, nullptr);

	if(!window){
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);	
	glfwSetKeyCallback(window, key_callback);
	if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
		return -1;
	}

	glViewport(0,0,WIDTH, HEIGHT);

	GLuint shaderID = setupShader();

	/****************************************************************/
	/*CRIANDO SPRITES*/    
	background.dimensions = vec3(HEIGHT,WIDTH,1.0);
	background.position = vec3(WIDTH/2, HEIGHT/2, 0.0);
	background.rotation = 90.0f;
	background.texture = loadTexture("../assets/background.png");
	background.VAO = createVAO();


 	//SPRITES METEORS
	vector<string> images = {"../assets/Meteor_01.png", "../assets/Meteor_04.png", "../assets/Meteor_10.png", "../assets/Meteor_07.png","../assets/Meteor_06.png"};
    Sprite sprite;
	float spriteDimension;
	for(int i = 0; i < NUM_METEORS; i++){
		meteors.push_back(createRandomMeteor()); 
	} 

	//SPRITE PLAYER
	ship.dimensions = vec3(100,100,1.0);
	ship.position = vec3(WIDTH/2-ship.dimensions[0], HEIGHT/2-ship.dimensions[1], 0.0);
	ship.rotation = 90.0f;
	ship.texture = loadTexture("../assets/ship.png");
	ship.VAO = createVAO();

	/****************************************************************/

	glUseProgram(shaderID);

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

		//TRANSFORMAÇÕES E DRAW BACKGROUND
		model = mat4(1);
		model = translate(model, background.position);
		model = rotate(model, radians(background.rotation), vec3(0.0, 0.0, 1.0));
		model = scale(model, background.dimensions);
		glBindVertexArray(background.VAO);
		glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, GL_FALSE, value_ptr(model));
		glBindTexture(GL_TEXTURE_2D, background.texture); 
		glDrawArrays(GL_TRIANGLE_STRIP, 0 , 4);

		//TRANSFORMAÇÕES E DRAW METEOROS
        while(meteors.size() < NUM_METEORS){
            meteors.push_back(createRandomMeteor());
        }
		for(int i = 0; i < meteors.size(); i++){
            checkCollision(meteors[i], ship);

            meteors[i].position = vec3(
                meteors[i].position[0]+meteors[i].velocity[0]-ship.velocity[0],
                meteors[i].position[1]+meteors[i].velocity[1]-ship.velocity[1],
                0.0);
            if(meteors[i].position[0] < 0 || meteors[i].position[1] > HEIGHT+meteors[i].dimensions[1]/2 || meteors[i].position[1] < 0-meteors[i].dimensions[1]/2) {
                meteors.erase(meteors.begin() + i); // Remove o item coletado da lista
            }
			//TRANSFORMAÇÕES
			model = mat4(1);
			model = translate(model, meteors[i].position);
			model = rotate(model, radians(meteors[i].rotation), vec3(0.0, 0.0, 1.0));
			model = scale(model, meteors[i].dimensions);
			//DRAW METEOROS
			glBindVertexArray(meteors[i].VAO);
			glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, GL_FALSE, value_ptr(model));
			glBindTexture(GL_TEXTURE_2D, meteors[i].texture);
			glDrawArrays(GL_TRIANGLE_STRIP, 0 , 4);
		} 
			
		//TRANSFORMAÇÕES E DRAW NAVE PLAYER
		model = mat4(1);
		model = translate(model, ship.position);
		model = rotate(model, radians(ship.rotation), vec3(0.0, 0.0, 1.0));
		model = scale(model, ship.dimensions);
		glBindVertexArray(ship.VAO);
		glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, GL_FALSE, value_ptr(model));
		glBindTexture(GL_TEXTURE_2D, ship.texture); // Conectando ao buffer de textura
		glDrawArrays(GL_TRIANGLE_STRIP, 0 , 4);

		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode){

	float moveX = 0;
	float moveY = 0;

    if(key == GLFW_KEY_UP && !(action == GLFW_RELEASE)) {
        moveY = -5;
    } else if(key == GLFW_KEY_DOWN && !(action == GLFW_RELEASE)) {
        moveY = +5;
    }

    if(key == GLFW_KEY_RIGHT && !(action == GLFW_RELEASE)) {
        moveX = +5;	
    } else if(key == GLFW_KEY_LEFT && !(action == GLFW_RELEASE)) {
        moveX = -5;	
    }

    ship.velocity = vec3(moveX, moveY, 0.0);

}


bool checkCollision(Sprite ship, Sprite object2){

	float top_1 = ship.position[1]+30;
	float bottom_1 = ship.position[1] + ship.dimensions[1] - 30;
	float left_1 = ship.position[0] + 30;
	float right_1 = ship.position[0] + ship.dimensions[1] - 30;

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

Sprite createRandomMeteor(){
    vector<string> images = {"../assets/Meteor_01.png", "../assets/Meteor_04.png", "../assets/Meteor_10.png", "../assets/Meteor_07.png","../assets/Meteor_06.png"};
    Sprite meteor;
	float meteorDimension;
    meteorDimension = rand() % 70 + 70;
    meteor.dimensions = vec3( meteorDimension , meteorDimension ,1.0);
    meteor.rotation = 0.0;
    //meteor.rotation = rand() % 360;

    meteor.texture = loadTexture(images[rand() % images.size()]);
    meteor.VAO = createVAO();
    meteor.velocity = vec3(-(rand() % 7), (rand() % 7)-(rand() % 7),0.0);
    //meteor.position = vec3( rand() % WIDTH , rand() % HEIGHT, 0.0);
    meteor.position = vec3(WIDTH, HEIGHT/2, 0.0);
    return meteor;
}


GLuint setupShader(){
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

    GLint success;
	GLchar infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success){
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
				  << infoLog << std::endl;
	}

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success){
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
				  << infoLog << std::endl;
	}

	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

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

GLuint createVAO(){
	GLfloat vertices[] = {
		// x   y     z    r   g    b    s     t
		-0.5, -0.5, 0.0, 1.0, 1.0, 1.0, 0.0, 0.0,// v0
		-0.5,  0.5, 0.0, 1.0, 1.0, 1.0, 1.0, 0.0,// v2
		0.5,  -0.5, 0.0, 1.0, 1.0, 1.0, 0.0, 1.0,// v1
		0.5,  0.5,  0.0, 1.0, 1.0, 1.0, 1.0, 1.0 // v3
	};

	GLuint VBO, VAO;

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	
    glGenVertexArrays(1, &VAO);

    glBindVertexArray(VAO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid *)0);
	glEnableVertexAttribArray(0); 

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid *)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid *)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	return VAO;
}

GLuint loadTexture(string filePath){
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

	if (data) {
		if (nrChannels == 3) {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		} else {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}
		glGenerateMipmap(GL_TEXTURE_2D);
	} else {
		std::cout << "Failed to load texture" << std::endl;
	}

	stbi_image_free(data);

	glBindTexture(GL_TEXTURE_2D, 0);

	return texID;
}