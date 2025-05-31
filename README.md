# PGCCHIB - Processamento Gráfico
Repositório criado usando como base o repositório exemplo da disciplina de **Processamento Gráfico**.

## Tarefa M2 - Create Triangle `CreateTriangle.cpp`
Crie uma função:
`GLuint createTriangle(float x0, float y0, float x1, float y1, float x2, float y2);`
Essa função deve criar um triângulo com as coordenadas dos vértices v0, v1 e v2 passadas por parâmetro, retornando seu identificador de VAO.

Instancie 5 triângulos na tela utilizando a função criada anteriormente.
Crie uma estrutura (struct ou classe) chamada Triangle que armazene: A posição do triângulo (x, y); A cor do triângulo (componentes RGB).
Utilizando a função criada anteriormente, gere um único VAO para um triângulo padrão com os seguintes vértices:  v0(-0.1, -0.1),   v1(0.1, -0.1),  v2(0.0, 0.1)
Usando um array, vector ou list de estruturas Triangle e o VAO criado, o programa deverá criar novos triângulos posicionados a partir do clique do mouse na tela. A cor de cada triângulo deve variar, sorteando-se valores para as componentes RGB da cor.

## Tarefa M3 - Jogo das Cores `JogosDasCores.cpp`

Você deve implementar um jogo que avalia uma escolha de cor do usuário e verifica quantos retângulos com cores similares podem ser removidos de uma grade de retângulos. Você deve executar os cálculos de similaridade de cores conforme o material visto no módulo. A cada tentativa os retângulos “acertados” pelo usuário devem ser removidos e um pontuação adequada deve ser associada. A cada nova tentativa o jogo deve diminuir a pontuação. Ao final o jogo deve indicar a pontuação total. 

## Tarefa Vivencial 1 - `CreateTriangleVertices.cpp`

1) Ao clicar na tela, você agora estará criando apenas 1 vértice
2) A cada 3 vértices criados, você criará um triângulo
3) Para cada novo triângulo criado, você deve usar uma cor nova.

## Tarefa M4 - Mapeamento de Texturas `HelloTexture.cpp`

1) Desenhe uma cena composta por vários retângulos texturizados (sprites) com diferentes
texturas
2) Crie uma classe Sprite, contendo como atributos:
  - O identificador do buffer de Geometria (VAO), que deverá ser um quadrilátero centrado na origem do sistema cartesiano, com as dimensões 1x1;
  - O identificador da textura do sprite;
  - Variáveis para armazenar a posição, escala e rotação, que alimentarão a matriz de transformações do objeto sprite (matriz de modelo);
  - Uma referência ao shader (o ID do shader ou ponteiro para objeto da classe que lê e gerencia os shaders). Dessa forma, você poderá encapsular os métodos de atualizar e desenhar o sprite dentro da classe.

## Tarefa Vivencial 2 - `ParallaxScrolling.cpp`

1) Você escolherá um dos sprites como sendo o “personagem” do jogador. Este personagem deverá se
mover para os lados ou para cima e para baixo utilizando o teclado.
2) O fundo da cena será um cenário construído em camadas. Inicialmente, o cenário terá todas as camadas posicionadas igualmente.
3) Ao mover o personagem, as camadas deverão deslocar-se junto com ele. As camadas mais próximas do personagem (exemplo: chão, árvores...) deverão se deslocar mais rápido (offset maior), enquanto as camadas mais distantes do personagem (exemplo: montanha, nuvens) irão se deslocar mais
lentamente
