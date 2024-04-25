#include <stdio.h>
#include <stdlib.h>
#include <string.h> // Para usar strings
#include <time.h>

#ifdef WIN32
#include <windows.h> // inclui apenas no Windows
#include "gl/glut.h"
#endif

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

// SOIL é a biblioteca para leitura das imagens
#include "include/SOIL.h"

// Um pixel RGBpixel (24 bits)
typedef struct
{
    unsigned char r, g, b;
} RGBpixel;

// Uma imagem RGBpixel
typedef struct
{
    int width, height;
    RGBpixel *pixels;
} Img;

// Protótipos
void load(char *name, Img *pic);
void valida();
int cmp(const void *elem1, const void *elem2);

// Funções da interface gráfica e OpenGL
void init();
void draw();
void keyboard(unsigned char key, int x, int y);

// Largura e altura da janela
int width, height;

// Identificadores de textura
GLuint tex[3];

// As 3 imagens
Img pic[3];

// Imagem selecionada (0,1,2)
int sel;

// Enums para facilitar o acesso às imagens
#define ORIGEM 0
#define DESEJ 1
#define SAIDA 2

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("alchemy [origem] [destino]\n");
        printf("Origem eh a fonte das cores, destino eh a imagem desejada\n");
        exit(1);
    }
    glutInit(&argc, argv);

    // Define do modo de operacao da GLUT
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

    // pic[0] -> imagem com as cores
    // pic[1] -> imagem desejada
    // pic[2] -> resultado do algoritmo

    // Carrega as duas imagens
    load(argv[1], &pic[ORIGEM]);
    load(argv[2], &pic[DESEJ]);

    // A largura e altura da janela são calculadas de acordo com a maior
    // dimensão de cada imagem
    width = pic[ORIGEM].width > pic[DESEJ].width ? pic[ORIGEM].width : pic[DESEJ].width;
    height = pic[ORIGEM].height > pic[DESEJ].height ? pic[ORIGEM].height : pic[DESEJ].height;

    // A largura e altura da imagem de saída são iguais às da imagem desejada (1)
    pic[SAIDA].width = pic[DESEJ].width;
    pic[SAIDA].height = pic[DESEJ].height;
    pic[SAIDA].pixels = malloc(pic[DESEJ].width * pic[DESEJ].height * 3); // W x H x 3 bytes (RGBpixel)

    // Especifica o tamanho inicial em pixels da janela GLUT
    glutInitWindowSize(width, height);

    // Cria a janela passando como argumento o titulo da mesma
    glutCreateWindow("Quebra-Cabeca digital");

    // Registra a funcao callback de redesenho da janela de visualizacao
    glutDisplayFunc(draw);

    // Registra a funcao callback para tratamento das teclas ASCII
    glutKeyboardFunc(keyboard);

    // Cria texturas em memória a partir dos pixels das imagens
    tex[ORIGEM] = SOIL_create_OGL_texture((unsigned char *)pic[ORIGEM].pixels, pic[ORIGEM].width, pic[ORIGEM].height, SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);
    tex[DESEJ] = SOIL_create_OGL_texture((unsigned char *)pic[DESEJ].pixels, pic[DESEJ].width, pic[DESEJ].height, SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);

    // Exibe as dimensões na tela, para conferência
    printf("Origem   : %s %d x %d\n", argv[1], pic[ORIGEM].width, pic[ORIGEM].height);
    printf("Desejada : %s %d x %d\n", argv[2], pic[DESEJ].width, pic[DESEJ].height);
    sel = ORIGEM; // pic1

    // Define a janela de visualizacao 2D
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(0.0, width, height, 0.0);
    glMatrixMode(GL_MODELVIEW);

    srand(time(0)); // Inicializa gerador aleatório (se for usar random)

    printf("Processando...\n");

    #pragma region NOSSO_CODIGO

    int tam = pic[DESEJ].width * pic[DESEJ].height;

    // Quicksort nos pixels de ORIGEM

    // Cria uma copia da imagem de origem
    Img sorted;
    sorted.width = pic[ORIGEM].width;
    sorted.height = pic[ORIGEM].height;
    sorted.pixels = malloc(tam * sizeof(RGBpixel));
    memcpy(sorted.pixels, pic[ORIGEM].pixels, sizeof(RGBpixel) * tam);

    // Realiza quicksort em sorted.pixels
    // qsort(sorted.pixels, tam, sizeof(RGBpixel), &cmp);

    // Determina SAIDA.pixels como sorted.pixels (debug para determinar se o qsort funciona)
    // memcpy(pic[SAIDA].pixels, sorted.pixels, sizeof(RGBpixel) * tam);
    
    // Montagem de SAIDA com base em sorted
    printf("tamanho img: %d\n", tam);

    RGBpixel *ptra = pic[DESEJ].pixels, *ptrb = sorted.pixels;
    unsigned char ared, agreen, ablue; // pixels de DESEJ
    unsigned char bred, bgreen, bblue; // pixels de sorted
    RGBpixel a;
    RGBpixel b;
    RGBpixel lembra;
    RGBpixel comparaPixel;
    RGBpixel montagem[tam];
    //int compara[3];
    //int *c;
    int diff;
    // percorre pic[DESEJ]
    for (int i = 0; i < tam; i++, *(ptra++)) {
        
        ptrb = &sorted.pixels;
        ared = (*ptra).r; agreen = (*ptra).g; ablue = (*ptra).b;
        a = *ptra;

        // percorre sorted
        for (int j = 0; j < tam; j++, *(ptrb++)) {
            b = *ptrb;
            /*comparaPixel.r = abs(ared - bred);
            comparaPixel.g = abs(agreen - bgreen);
            comparaPixel.b = abs(ablue - bblue);*/
            if (j == 0 || diff > abs(cmp2(ptrb, ptra))) {
                lembra = b;
                diff = abs(cmp2(ptrb, ptra));
            }

            if (diff == 0) break;
            // aqui tem um negócio engraçado
            // se tu deixar o printf fora do if, o código funciona normal
            // mas demora uma eternidade pra completar por causa da quantidade de printf
            if (j % 100 == 0) {
                printf("tam = %d | i = %d | j = %d | cmp2 = %d | diff = %d\n", tam, i, j, abs(cmp2(ptrb, ptra)), diff);
            }

            // ^ o código só funciona se as coisas aqui debaixo também estiverem comentadas

            if (j == 85) { // por algum motivo ele quebra sempre no 85
                printf("ta no 85\n");
            }
            else if (j == 86) {
                printf("passou do 85\n");
            }
            /*if (j == 84 || j == 85) {
                printf("  A | B\n");
                printf("r %c | %c\n", ptra->r, ptrb->r);
                printf("g %c | %c\n", ptra->g, ptrb->g);
                printf("b %c | %c\n", ptra->b, ptrb->b);
            }*/
            /*if ((comparaPixel.r < lembra.r && comparaPixel.g < lembra.g && comparaPixel.b < lembra.b)||
                j == 0) {
                    lembra = comparaPixel;
                }*/
            
           //debug
           /*if (j % 1000 == 0) {
               printf("contador j: %d tam: %d\n", j, pic[DESEJ].height*pic[DESEJ].width);
           }
           */
        }
        printf("\nloop i = %d completo | lembra: r = %c : g = %c : b = %c\n", i, lembra.r, lembra.g, lembra.b);
        if (i == tam / 10) printf("10%%\n");
        else if (i == tam / 4) printf("25%%\n");
        else if (i == tam / 2) printf("50%%\n");
        else if (i == 3 * tam / 4) printf("75%%\n");
        montagem[i] = lembra;
        //debug
        //printf("contador i: %d, tam: %d\n", i, pic[DESEJ].height * pic[DESEJ].width);
    }
    memcpy(pic[SAIDA].pixels, montagem, sizeof(RGBpixel) * tam);
    printf("100%%\n");

    #pragma endregion NOSSO_CODIGO

    // NÃO ALTERAR A PARTIR DAQUI!

    // Cria textura para a imagem de saída
    tex[SAIDA] = SOIL_create_OGL_texture((unsigned char *)pic[SAIDA].pixels, pic[SAIDA].width, pic[SAIDA].height, SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);
    // Grava imagem de saída em out.bmp, para conferência
    SOIL_save_image("out.bmp", SOIL_SAVE_TYPE_BMP, pic[SAIDA].width, pic[SAIDA].height, 3, (const unsigned char *)pic[SAIDA].pixels);

    // Entra no loop de eventos, não retorna
    glutMainLoop();
}

// Carrega uma imagem para a struct Img
void load(char *name, Img *pic)
{
    int chan;
    pic->pixels = (RGBpixel *)SOIL_load_image(name, &pic->width, &pic->height, &chan, SOIL_LOAD_RGB);
    if (!pic->pixels)
    {
        printf("SOIL loading error: '%s'\n", SOIL_last_result());
        exit(1);
    }
    printf("Load: %d x %d x %d\n", pic->width, pic->height, chan);
}

// Verifica se o algoritmo foi aplicado corretamente:
// Ordena os pixels da imagem origem e de saída por R, G e B;
// depois compara uma com a outra: devem ser iguais
void valida()
{
    int ok = 1;
    int size = pic[ORIGEM].width * pic[ORIGEM].height;
    // Aloca memória para os dois arrays
    RGBpixel *aux1 = malloc(size * 3);
    RGBpixel *aux2 = malloc(size * 3);
    // Copia os pixels originais
    memcpy(aux1, pic[ORIGEM].pixels, size * 3);
    memcpy(aux2, pic[SAIDA].pixels, size * 3);
    // Mostra primeiros 8 pixels de ambas as imagens
    // antes de ordenar (teste)
    for (int i = 0; i < 8; i++)
        printf("[%d %d %d] ", aux1[i].r, aux1[i].g, aux1[i].b);
    printf("\n");
    for (int i = 0; i < 8; i++)
        printf("[%d %d %d] ", aux2[i].r, aux2[i].g, aux2[i].b);
    printf("\n");
    printf("Validando...\n");
    // Ordena ambos os arrays
    qsort(aux1, size, sizeof(RGBpixel), cmp);
    qsort(aux2, size, sizeof(RGBpixel), cmp);
    // Mostra primeiros 8 pixels de ambas as imagens
    // depois de ordenar
    for (int i = 0; i < 8; i++)
        printf("[%d %d %d] ", aux1[i].r, aux1[i].g, aux1[i].b);
    printf("\n");
    for (int i = 0; i < 8; i++)
        printf("[%d %d %d] ", aux2[i].r, aux2[i].g, aux2[i].b);
    printf("\n");
    for (int i = 0; i < size; i++)
    {
        if (aux1[i].r != aux2[i].r ||
            aux1[i].g != aux2[i].g ||
            aux1[i].b != aux2[i].b)
        {
            // Se pelo menos um dos pixels for diferente, o algoritmo foi aplicado incorretamente
            printf("*** INVÁLIDO na posição %d ***: %02X %02X %02X -> %02X %02X %02X\n",
                   i, aux1[i].r, aux1[i].g, aux1[i].b, aux2[i].r, aux2[i].g, aux2[i].b);
            ok = 0;
            break;
        }
    }
    // Libera memória dos arrays ordenados
    free(aux1);
    free(aux2);
    if (ok)
        printf(">>>> TRANSFORMAÇÃO VÁLIDA <<<<<\n");
}

// Funcao de comparacao para qsort: ordena por R, G, B (desempate nessa ordem)
int cmp(const void *elem1, const void *elem2)
{
    RGBpixel *ptr1 = (RGBpixel *)elem1;
    RGBpixel *ptr2 = (RGBpixel *)elem2;
    unsigned char r1 = ptr1->r;
    unsigned char r2 = ptr2->r;
    unsigned char g1 = ptr1->g;
    unsigned char g2 = ptr2->g;
    unsigned char b1 = ptr1->b;
    unsigned char b2 = ptr2->b;
    int r = 0;
    if (r1 < r2)
        r = -1;
    else if (r1 > r2)
        r = 1;
    else if (g1 < g2)
        r = -1;
    else if (g1 > g2)
        r = 1;
    else if (b1 < b2)
        r = -1;
    else if (b1 > b2)
        r = 1;
    return r;
}

int cmp2(const void *elem1, const void *elem2)
{
    RGBpixel *ptr1 = (RGBpixel *)elem1;
    RGBpixel *ptr2 = (RGBpixel *)elem2;
    unsigned char r1 = ptr1->r;
    unsigned char r2 = ptr2->r;
    unsigned char g1 = ptr1->g;
    unsigned char g2 = ptr2->g;
    unsigned char b1 = ptr1->b;
    unsigned char b2 = ptr2->b;
    int r = 0;
    r += r1 - r2;
    r += g1 - g2;
    r += b1 - b2;
    return r;
}

//
// Funções de callback da OpenGL
//
// SÓ ALTERE SE VOCÊ TIVER ABSOLUTA CERTEZA DO QUE ESTÁ FAZENDO!
//

// Gerencia eventos de teclado
void keyboard(unsigned char key, int x, int y)
{
    if (key == 27)
    {
        // ESC: libera memória e finaliza
        free(pic[0].pixels);
        free(pic[1].pixels);
        free(pic[2].pixels);
        exit(1);
    }
    if (key >= '1' && key <= '3')
        // 1-3: seleciona a imagem correspondente (origem, desejada e saída)
        sel = key - '1';
    // V para validar a solução
    if (key == 'v')
        valida();
    glutPostRedisplay();
}

// Callback de redesenho da tela
void draw()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Preto
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Para outras cores, veja exemplos em /etc/X11/rgb.txt

    glColor3ub(255, 255, 255); // branco

    // Ativa a textura corresponde à imagem desejada
    glBindTexture(GL_TEXTURE_2D, tex[sel]);
    // E desenha um retângulo que ocupa toda a tela
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);

    glTexCoord2f(0, 0);
    glVertex2f(0, 0);

    glTexCoord2f(1, 0);
    glVertex2f(pic[sel].width, 0);

    glTexCoord2f(1, 1);
    glVertex2f(pic[sel].width, pic[sel].height);

    glTexCoord2f(0, 1);
    glVertex2f(0, pic[sel].height);

    glEnd();
    glDisable(GL_TEXTURE_2D);

    // Exibe a imagem
    glutSwapBuffers();
}
