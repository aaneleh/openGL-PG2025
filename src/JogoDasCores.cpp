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

using namespace glm;

#include <cmath>

#include <ctime>

struct Quad {
	vec3 position;
	vec3 dimensions;
	vec3 color;
};

int setupShader();

const GLuint WIN_WIDTH = 800, WIN_HEIGHT = 600;

GLuint createQuad();

const GLuint QUAD_WIDTH = 100, QUAD_HEIGHT = 100;

bool endGame = false;

int highScore = 0;
int score = 0;
int basePoints = 100;


// Código fonte do Vertex Shader (em GLSL)
const GLchar* vertexShaderSource = "#version 400\n"
"layout (location = 0) in vec3 position;\n"
"uniform mat4 projection;\n"
"uniform mat4 model;\n"
"void main()\n"
"{\n"
//...pode ter mais linhas de código aqui!
"gl_Position = projection * model * vec4(position.x, position.y, position.z, 1.0);\n"
"}\0";

//Código fonte do Fragment Shader (em GLSL)
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
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
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


	Quad grid[WIN_WIDTH/QUAD_WIDTH][WIN_HEIGHT/QUAD_HEIGHT];

	for(int i = 0; i < WIN_WIDTH/QUAD_WIDTH; i++){
		for(int j = 0 ; j < WIN_HEIGHT/QUAD_HEIGHT; j++){
			Quad quad;
			quad.position = vec3(i*QUAD_WIDTH + (QUAD_WIDTH/2),j*QUAD_HEIGHT + (QUAD_HEIGHT/2),0.0);
			quad.dimensions = vec3(QUAD_WIDTH,QUAD_HEIGHT,1.0);
			quad.color = vec3(((float)rand()/(float)(RAND_MAX)), ((float)rand()/(float)(RAND_MAX)), ((float)rand()/(float)(RAND_MAX)));
			grid[i][j] = quad;
		}
	}


	GLuint VAO = createQuad();

	glUseProgram(shaderID);

	GLuint colorLoc = glGetUniformLocation(shaderID, "inputColor");

	mat4 projection = ortho(0.0, 800.0, 600.0, 0.0, -1.0, 1.0);
	glUniformMatrix4fv(glGetUniformLocation(shaderID, "projection"), 1, GL_FALSE, value_ptr(projection));


	while(!glfwWindowShouldClose(window)){

		glfwPollEvents();

		glClearColor(0,0,0,1);
		glClear(GL_COLOR_BUFFER_BIT);

		glLineWidth(10);
		glPointSize(20);

		glBindVertexArray(VAO);

		const int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
		if(state == GLFW_PRESS){

			//SE O JOGO ACABOU GERA NOVOS QUADRADOS E ATUALIZA A PONTUAÇÃO
			if(endGame){
				if(score > highScore) highScore = score;
				score = 0;
				endGame = false;

				for(int i = 0; i < WIN_WIDTH/QUAD_WIDTH; i++){
					for(int j = 0 ; j < WIN_HEIGHT/QUAD_HEIGHT; j++){
						Quad quad;
						quad.position = vec3(i*QUAD_WIDTH + (QUAD_WIDTH/2),j*QUAD_HEIGHT + (QUAD_HEIGHT/2),0.0);
						quad.dimensions = vec3(QUAD_WIDTH,QUAD_HEIGHT,1.0);
						quad.color = vec3(((float)rand()/(float)(RAND_MAX)), ((float)rand()/(float)(RAND_MAX)), ((float)rand()/(float)(RAND_MAX)));
						grid[i][j] = quad;
					}
				}
			//VERIFICA CLICK, APAGA QUADRADOS E CALCULA A PONTUAÇÃO
			} else {
				double mx, my;
				glfwGetCursorPos(window, &mx, &my);

				//SE CLICAR EM UM QUADRADO APAGADO NÃO REGISTRA O CLICK
				if(grid[(int)(mx/QUAD_WIDTH)][(int)(my/QUAD_HEIGHT)].position[0] != -100){
					//PEGA A COR DO QUADRADO CLICADO
					vec3 colorRemoved = grid[(int)(mx/QUAD_WIDTH)][(int)(my/QUAD_HEIGHT)].color;

					int count = 0;
					int multiply = 0;
					for(int i = 0; i < WIN_WIDTH/QUAD_WIDTH; i++){
						for(int j = 0 ; j < WIN_HEIGHT/QUAD_HEIGHT; j++){
							//SE JÁ ESTÁ EXCLUÍDO, AUMENTA O CONTADOR
							if(grid[i][j].position[0] == -100){
								count++;
							//SE A COR É SIMILAR (0.3) APAGA O QUADRADO, AUMENTA PONTUÇÃO E O CONTADOR
							} else if( sqrt( pow(colorRemoved[0] - grid[i][j].color[0], 2) + pow(colorRemoved[1] - grid[i][j].color[1], 2) + pow(colorRemoved[2] - grid[i][j].color[2], 2)) < 0.3){
								grid[i][j].position = vec3(-100,-100,0);
								count++;
								multiply++;
								score += basePoints*multiply;
							}

						}
					}

					//SE CONTADOR = QUANTIDADE DE QUADRADOS FINALIZA O JOGO MOSTRANDO A PONTUAÇÃO
					if(count == (WIN_WIDTH/QUAD_WIDTH)*(WIN_HEIGHT/QUAD_HEIGHT)){
						printf("\n\nFim de jogo\n");

						if(highScore > 0 && score > highScore) printf("Novo record!\n");
						
						printf("Pontuacao final - %d\n", score);

						if(highScore > score) printf("Seu record e %d\n", highScore);
						
						printf("\n\n");

						endGame = true;
					}
				}
				
			}
		}

		//DESENHA OS QUADRADOS
		for(int i = 0; i < WIN_WIDTH/QUAD_WIDTH; i++){
			for(int j = 0 ; j < WIN_HEIGHT/QUAD_HEIGHT; j++){
				mat4 model = mat4(1);
				model = translate(model, grid[i][j].position);
				model = scale(model, grid[i][j].dimensions);
				
				glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, GL_FALSE, value_ptr(model));
				glUniform4f(colorLoc, grid[i][j].color[0], grid[i][j].color[1], grid[i][j].color[2], 1.0f);
				glDrawArrays(GL_TRIANGLE_STRIP, 0 ,6);
			}
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


GLuint createQuad(){

	GLfloat vertices[] = {
		-0.5, -0.5, 0,
		-0.5, 0.5, 0,
		0.5, -0.5, 0,
		0.5, 0.5, 0
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
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid *)0);
	glEnableVertexAttribArray(0);

	// Observe que isso é permitido, a chamada para glVertexAttribPointer registrou o VBO como o objeto de buffer de vértice
	// atualmente vinculado - para que depois possamos desvincular com segurança
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Desvincula o VAO (é uma boa prática desvincular qualquer buffer ou array para evitar bugs medonhos)
	glBindVertexArray(0);

	return VAO;
}