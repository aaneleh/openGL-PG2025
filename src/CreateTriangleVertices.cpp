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

struct Triangle { 
	vec3 position;
	vec3 color;
	GLuint VAO;
};	

//Recebe os 3 vertices, cria um VAO e sorteia a cor
Triangle setupTriangle(float x1, float y1, float x2, float y2, float x3, float y3);

//Armazena os 3 últimos pontos clicacos
vector<vec3> vertices;

//Armazena o último vertice para não considerar dois cliques seguidos no mesmo lugar
vec3 lastVertice = vec3(-1.0,-1.0,-1.0);

//Armazena todos os triangulos criados
vector<Triangle> triangles;

const GLuint WIN_WIDTH = 800, WIN_HEIGHT = 600;

int setupShader();


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

		const int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
		if(state == GLFW_PRESS){
            double mx, my;
            glfwGetCursorPos(window, &mx, &my);

            if(vertices.size() == 0 && lastVertice[0]==-1.0){

                printf("\n(%d, %d)", (int)mx, (int)my);

                lastVertice = vec3(mx, my, 1.0);
                vertices.push_back(lastVertice);

            } else if(mx != lastVertice[0] && my != lastVertice[1]){

                printf("\n(%d, %d)", (int)mx, (int)my);
                lastVertice = vec3(mx, my, 1.0);
                vertices.push_back(lastVertice);

                if(vertices.size() == 3){

                    triangles.push_back(setupTriangle(vertices[0][0],vertices[0][1],vertices[1][0],vertices[1][1],vertices[2][0],vertices[2][1]));
                    vertices.pop_back();
                    vertices.pop_back();
                    vertices.pop_back();

                }
            }  
        }

		for(int i = 0; i < triangles.size(); i++){
            
            mat4 model = mat4(1);
            model = translate(model, triangles[i].position);
            //model = translate(model, vec3(100,100,0));
            
            model = scale(model, vec3(1.0,1.0,1.0));
		    
            glBindVertexArray(triangles[i].VAO);

            glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, GL_FALSE, value_ptr(model));
            glUniform4f(colorLoc, triangles[i].color[0], triangles[i].color[1], triangles[i].color[2], 1.0f);
            glDrawArrays(GL_TRIANGLE_STRIP, 0 ,6);
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


Triangle setupTriangle(float x0, float y0, float x1, float y1, float x2, float y2){

	Triangle triangle;
    triangle.position = vec3((x0+x1+x2)/3, (y0+y1+y2)/3, 0);
	triangle.color = vec3(((float)rand()/(float)(RAND_MAX)), ((float)rand()/(float)(RAND_MAX)), ((float)rand()/(float)(RAND_MAX)));

    GLfloat vertices[] = {
		// x   y  z
		x0, y0, 0,
		x1, y1, 0,
		x2, y2, 0
	}; 

	GLuint VBO, VAO;

    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glGenVertexArrays(1, &VAO);

    glBindVertexArray(VAO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid *)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);


	triangle.VAO = VAO;

	glBindVertexArray(0);

	return triangle;
}