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

struct Triangle { 
	vec3 position;
	vec3 color;
	GLuint VAO;
};	
// Protótipos das funções
int setupShader();
GLuint createTriangle(float x1, float y1, float x2, float y2, float x3, float y3);
Triangle setupStruct(float x1, float y1, float x2, float y2, float x3, float y3, float r, float g, float b);

const GLuint WIDTH = 800, HEIGHT = 600;

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
	// Inicialização da GLFW
	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Ativa a suavização de serrilhado (MSAA) com 8 amostras por pixel
	glfwWindowHint(GLFW_SAMPLES, 8);

	// Criação da janela GLFW
	GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Tarefav2 PG M2 ", nullptr, nullptr);
	if (!window){
		std::cerr << "Falha ao criar a janela GLFW" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// GLAD: carrega todos os ponteiros d funções da OpenGL
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
		std::cerr << "Falha ao inicializar GLAD" << std::endl;
		return -1;
	}

	// Obtendo as informações de versão
	const GLubyte *renderer = glGetString(GL_RENDERER); /* get renderer string */
	const GLubyte *version = glGetString(GL_VERSION);	/* version as a string */
	cout << "Renderer: " << renderer << endl;
	cout << "OpenGL version supported " << version << endl;

	// Definindo as dimensões da viewport com as mesmas dimensões da janela da aplicação
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	// Compilando e buildando o programa de shader
	GLuint shaderID = setupShader();

	//Armazenando as posições e cores dos triangulos que serão exibidos
	vector<Triangle> triangles;
	triangles.push_back(setupStruct(-0.5,0.0,		-0.25,0.0,		-0.5, -0.25, 	1.0, 0, 0));
	triangles.push_back(setupStruct(0.25,0.0,		-0.25,0.0,		 0.0, -0.25, 	0, 1.0, 0));
	triangles.push_back(setupStruct(-0.5, -0.5, 	-0.25, -0.25,	0.0, -0.5,		0, 0, 1.0));
	triangles.push_back(setupStruct(0.25, 0.0, 		0.5, 0.0,		0.5, -0.25,		1.0, 0, 1.0));
	triangles.push_back(setupStruct(0.0, -0.5, 		0.5, -0.5,		0.25, -0.25,	0.0,1.0,1.0));

	// Enviando a cor desejada (vec4) para o fragment shader
	GLint colorLoc = glGetUniformLocation(shaderID, "inputColor");

	glUseProgram(shaderID); // Reseta o estado do shader para evitar problemas futuros

	mat4 model;
	//argumentos passados pra projeção são a área da cena, de 0 a 800 a largura, 0 à 600 altura e -1 e 1 a profundidade
	mat4 projection = ortho(0.0, 800.0, 0.0, 600.0, -1.0, 1.0);  
	glUniformMatrix4fv(glGetUniformLocation(shaderID, "projection"), 1, GL_FALSE, value_ptr(projection));


	// Loop da aplicação 
	while (!glfwWindowShouldClose(window)){

		// Checa se houveram eventos de input (key pressed, mouse moved etc.) e chama as funções de callback correspondentes
		glfwPollEvents();

		model = mat4(1); //matriz identidade
		model = translate(model,vec3(400.0,300.0,0.0));
		model = scale(model,vec3(300.0, 300.0,1.0));

		// Limpa o buffer de cor
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // cor de fundo
		glClear(GL_COLOR_BUFFER_BIT);

		glLineWidth(10);
		glPointSize(20);

		glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, GL_FALSE, value_ptr(model));

		const int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
        if (state == GLFW_PRESS) {
            double mx, my;
            glfwGetCursorPos(window, &mx, &my);
			double dx = mx - WIDTH / 2;
			double dy = -my + HEIGHT / 2;
			
			Triangle triangle;
			triangle.position = vec3(dx, dy, 0);
			triangle.color = vec3(((float)rand()/(float)(RAND_MAX)), ((float)rand()/(float)(RAND_MAX)), ((float)rand()/(float)(RAND_MAX)));
			triangle.VAO = createTriangle((dx/500)-0.1, (dy/500)-0.1, (dx/500)+0.1, (dy/500)-0.1, (dx/500)+0.0, (dy/500)+0.1);
			
			triangles.push_back(triangle);
        } 

		for(int i = 0; i < triangles.size(); i++){
			glBindVertexArray(triangles[i].VAO);
			glUniform4f(colorLoc, triangles[i].color[0], triangles[i].color[1], triangles[i].color[2], 1.0f); // enviando cor para variável uniform inputColor
			glDrawArrays(GL_TRIANGLES, 0, 3);
			glBindBuffer(GL_ARRAY_BUFFER, triangles[i].VAO);
		}

		// Troca os buffers da tela
		glfwSwapBuffers(window);
	}
	glfwTerminate();
	return 0;
}


//  Compilar e "buildar" um programa de shader simples e único neste exemplo de código
//  A função retorna o identificador do programa de shader
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

Triangle setupStruct(float x0, float y0, float x1, float y1, float x2, float y2, float r, float g, float b){
	Triangle triangle;
	triangle.position = vec3(x0, y0, 0);
	triangle.color = vec3(r,g,b);
	triangle.VAO = createTriangle(x0, y0, x1, y1, x2, y2);
	return triangle;
}

// Criar os buffers que armazenam a geometria de um triângulo
// A função retorna o identificador do VAO
GLuint createTriangle(float x0, float y0, float x1, float y1, float x2, float y2){
	GLfloat vertices[] = {
		// x   y  z
		x0, y0, 0,
		x1, y1, 0,
		x2, y2, 0
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