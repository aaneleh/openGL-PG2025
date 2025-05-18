# PGCCHIB - Processamento Gráfico
Repositório criado usando como base o repositório exemplo da disciplina de **Processamento Gráfico**.

## Tarefa M2 - Create Triangle `CreateTriangle.cpp`
Crie uma função:
`GLuint createTriangle(float x0, float y0, float x1, float y1, float x2, float y2);`
Essa função deve criar um triângulo com as coordenadas dos vértices v0, v1 e v2 passadas por parâmetro, retornando seu identificador de VAO.

Instancie 5 triângulos na tela utilizando a função criada anteriormente.

Crie uma estrutura (struct ou classe) chamada Triangle que armazene:
A posição do triângulo (x, y);
A cor do triângulo (componentes RGB).
Utilizando a função criada anteriormente, gere um único VAO para um triângulo padrão com os seguintes vértices:  v0(-0.1, -0.1),   v1(0.1, -0.1),  v2(0.0, 0.1)

Usando um array, vector ou list de estruturas Triangle e o VAO criado, o programa deverá criar novos triângulos posicionados a partir do clique do mouse na tela. A cor de cada triângulo deve variar, sorteando-se valores para as componentes RGB da cor.

## Tarefa M3 - Jogo das Cores `JogosDasCores.cpp`

Você deve implementar um jogo que avalia uma escolha de cor do usuário e verifica quantos retângulos com cores similares podem ser removidos de uma grade de retângulos. Você deve executar os cálculos de similaridade de cores conforme o material visto no módulo. A cada tentativa os retângulos “acertados” pelo usuário devem ser removidos e um pontuação adequada deve ser associada. A cada nova tentativa o jogo deve diminuir a pontuação. Ao final o jogo deve indicar a pontuação total. 

## Tarefa Vivencial 1 - `CreateTriangleVertices`

1) Ao clicar na tela, você agora estará criando apenas 1 vértice
2) A cada 3 vértices criados, você criará um triângulo
3) Para cada novo triângulo criado, você deve usar uma cor nova.