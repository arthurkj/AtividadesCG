# Olá, Grau B — Renderizador de Cenas 3D com Iluminação e Animação

Este projeto consiste em um renderizador de cenas tridimensionais desenvolvido em C++ e OpenGL (4.1+) para a disciplina de Computação Gráfica na Unisinos. O sistema é capaz de carregar dinamicamente uma descrição de cenário a partir de um arquivo de texto, renderizar múltiplos objetos 3D (.obj) com texturas e materiais, simular um modelo de iluminação múltipla (Phong) com atenuação física e movimentar elementos por meio de curvas compostas de Bézier.


## Setup
O projeto é composto por três arquivos principais que se encontram na pasta `src`:
* Cena.h - Header criado para definir códigos que são utilizados em múltiplos arquivos desse projeto.
* Cena.cpp - Arquivo responsável pelo carregamento da cena, seus objetos e texturas.
* GrauB.cpp - Arquivo responsável pelo gerenciamento da pipeline gráfico.

Além destes, na pasta `assets` se encontram dependências visuais do projeto:
* cena.txt - Arquivo de texto que é utilizado para configuração da cena. Nele, são definidas as configurações da camêra, bem como posicionamento e comportamento dos objetos.

Na pasta `assets/Modelos3D/GrauB`, se encontram todos os arquivos `.obj` e `mtl` dos modelos utilizado no projeto. As texturas desses modelos podem ser encontradas na pasta `assets/Modelos3D/GrauB/textures`.

---

As configurações presentes nesse repositório são compatíveis com o sistema operacional MacOS.

## Controles

### Navegação
* Mouse: direcionamento da câmera
* WASD: movimentação na cena

### Manipulação e seleção de objetos
* TAB: altera o objeto selecionado na cena
* Setas: movimenta o objeto selecionado na cena
* X, Y, Z: rotaciona do objeto selecionado
* [, ]: diminui/aumenta a escala do objeto selecionado
* P: marca ponto da rota do objeto selecionado
* Space: inicia/interrompe movimentação do objeto selecionado (precisa de pelo menos 4 pontos)

### Iluminação
* 1: liga/desliga luz principal
* 2: liga/desliga luz de preenchimento
* 3: liga/desliga contraluz
* -: aumenta o valor do expoente de brilho especular (torna o reflexo menor e mais concentrado)
* +: diminui o valor do expoente de brilho especular (expande o reflexo)

## Configuração da cena
A configuração da cena é totalmente parametrizável através do arquivo `cena.txt`. Cada linha representa um elemento a ser adicionado na cena, o tipo desse elemento sendo representado por uma palavra-chave e em seguida os seus parâmetros. Segue abaixo a relação das palavra-chaves, suas descrições e seus parâmetros.

1. CAMERA
* Descrição: configura as propriedades de posicionamento inicial da câmera e visualização em perspectiva (frustrum).
* Parâmetros: `CAMERA px py pz yaw pitch fov zNear zFar`
    * `px py pz`: posição inicial da câmera
    * `yaw`: ângulo de rotação horizontal inicial
    * `pitch`: ângulo de rotação vertical inicial
    * `fov`: campo de visão
    * `zNear`: distância do plano de corte próximo
    * `zFar`: distância do plano de corte afastado

2. LIGHT
* Descrição: instancia uma fonte de luz pontual (Point Light) no cenário. O projeto exige a declaração de exatamente 3 fontes de luz, sendo elas luz principal, luz de preenchimento e contraluz, respectivamente.
* Parâmetros: `LIGHT px py pz kl kq`
    * `px py pz`: posição do centro emissor
    * `kl`: fator de atenuação linear
    * `kq`: fator de atenuação quadrática

3. OBJECT
* Descrição: instancia um objeto na cena através de modelo 3D.
* Parâmetros: `OBJECT caminho_do_arquivo px py pz escala rotacao`
    * `caminho_do_arquivo`: caminho relativo para o arquivo .obj
    * `px py pz`: posição inicial do objeto
    * `escala`: tamanho do objeto
    * `rotação`: ângulo de rotação inicial em torno do eixo Y

4. PATH
* Descrição: injeta um ponto para a rota do último objeto declarado.
* Parâmetros: `PATH px py pz`
    * `px py pz`: coordenadas do ponto

## Assets
A cena inicialmente desenvolvida nesse projeto tem possui a temática de futebol.

* `field.obj`
    * Descrição: objeto utilizado como base da cena, fazendo referência ao campo de futebol.
    * Fonte: https://sketchfab.com/3d-models/football-field-9752185eebef49d78b3ae529ea2235cf
* `banco.obj`
    * Descrição: modelo dos bancos de reservas à beira do campo.
    * Fonte: https://sketchfab.com/3d-models/locker-room-bench-free-model-0e34168cb7f043e6a4a90b1b02ece321
* `trave.obj`
    * Descrição: modelo das traves do campo.
    * Fonte: https://sketchfab.com/3d-models/low-poly-football-field-with-ball-628703e94efc4d71b4244bbba1b4dfa4
    * OBS: esse modelo foi recortado na ferramenta blender para selecionar apenas a trave.
* `jogador_corpo.obj`
    * Descrição: modelo do corpo das jogadoras.
    * Fonte: https://sketchfab.com/3d-models/soccer-6441f016652e4a7ab9216e9419efb4d1
    * OBS: esse modelo foi recortado na ferramenta blender para selecionar apenas o corpo da jogadora.
* `jogador_corpo-azul.obj`
    * Descrição: modelo do corpo das jogadoras.
    * Fonte: https://sketchfab.com/3d-models/soccer-6441f016652e4a7ab9216e9419efb4d1
    * OBS: esse modelo foi recortado na ferramenta blender para selecionar apenas o corpo da jogadora. Além disso, a textura foi alterada mudando a cor do uniforme.
* `jogador_cabeca.obj`
    * Descrição: modelo da cabeça das jogadoras.
    * Fonte: https://sketchfab.com/3d-models/soccer-6441f016652e4a7ab9216e9419efb4d1
    * OBS: esse modelo foi recortado na ferramenta blender para selecionar apenas a cabeça da jogadora. 
* `jogador_cabelo.obj`
    * Descrição: modelo do cabelo das jogadoras.
    * Fonte: https://sketchfab.com/3d-models/soccer-6441f016652e4a7ab9216e9419efb4d1
    * OBS: esse modelo foi recortado na ferramenta blender para selecionar apenas o cabelo da jogadora. 