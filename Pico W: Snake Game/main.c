#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

// ================= CONFIG =================
#define LARGURA 20
#define ALTURA 10
#define VELOCIDADE_MS 250

#define MAX_COBRA 100

// ================= ESTADO =================
int cobra_x[MAX_COBRA], cobra_y[MAX_COBRA];
int tamanho;
int fruta_x, fruta_y;
char direcao;
bool jogo_rodando;

// Framebuffer (altura fixa!)
char tela[ALTURA + 6][LARGURA + 3];

// ================= UTIL ===================
void limpar_terminal() {
    printf("\033[H\033[2J");
}

// ================= JOGO ===================
void reset_jogo() {
    tamanho = 3;
    direcao = 'd';
    jogo_rodando = true;

    int cx = LARGURA / 2;
    int cy = ALTURA / 2;

    for (int i = 0; i < tamanho; i++) {
        cobra_x[i] = cx - i;
        cobra_y[i] = cy;
    }

    fruta_x = rand() % LARGURA;
    fruta_y = rand() % ALTURA;
}

// ================= FRAMEBUFFER =============
void limpar_buffer() {
    for (int y = 0; y < ALTURA + 6; y++) {
        for (int x = 0; x < LARGURA + 3; x++) {
            tela[y][x] = ' ';
        }
        tela[y][LARGURA + 2] = '\0';
    }
}

void desenhar_no_buffer() {
    limpar_buffer();

    int linha = 0;

    // Título
    snprintf(tela[linha++], LARGURA + 3,
             "SNAKE PICO W          ");

    // Borda superior
    for (int i = 0; i < LARGURA + 2; i++)
        tela[linha][i] = '#';
    linha++;

    // Área de jogo
    for (int y = 0; y < ALTURA; y++) {
        tela[linha][0] = '#';
        tela[linha][LARGURA + 1] = '#';

        for (int x = 0; x < LARGURA; x++) {
            char c = ' ';

            if (x == fruta_x && y == fruta_y) {
                c = '@';
            }

            for (int i = 0; i < tamanho; i++) {
                if (cobra_x[i] == x && cobra_y[i] == y) {
                    c = (i == 0) ? 'O' : 'o';
                    break;
                }
            }

            tela[linha][x + 1] = c;
        }
        linha++;
    }

    // Borda inferior
    for (int i = 0; i < LARGURA + 2; i++)
        tela[linha][i] = '#';
    linha++;

    // Score
    snprintf(tela[linha++], LARGURA + 3,
             "Score: %d            ", (tamanho - 3) * 10);

    // Game over
    if (!jogo_rodando) {
        snprintf(tela[linha++], LARGURA + 3,
                 "GAME OVER - F reset  ");
    }
}

// ================= LOGICA =================
void atualizar() {
    if (!jogo_rodando) return;

    for (int i = tamanho - 1; i > 0; i--) {
        cobra_x[i] = cobra_x[i - 1];
        cobra_y[i] = cobra_y[i - 1];
    }

    if (direcao == 'w') cobra_y[0]--;
    if (direcao == 's') cobra_y[0]++;
    if (direcao == 'a') cobra_x[0]--;
    if (direcao == 'd') cobra_x[0]++;

    if (cobra_x[0] == fruta_x && cobra_y[0] == fruta_y) {
        tamanho++;
        fruta_x = rand() % LARGURA;
        fruta_y = rand() % ALTURA;
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    } else {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
    }

    if (cobra_x[0] < 0 || cobra_x[0] >= LARGURA ||
        cobra_y[0] < 0 || cobra_y[0] >= ALTURA) {
        jogo_rodando = false;
    }

    for (int i = 1; i < tamanho; i++) {
        if (cobra_x[0] == cobra_x[i] &&
            cobra_y[0] == cobra_y[i]) {
            jogo_rodando = false;
        }
    }
}

// ================= MAIN ===================
int main() {
    stdio_init_all();
    if (cyw43_arch_init()) return -1;

    reset_jogo();

    while (true) {
        int c, ultima = -1;

        while ((c = getchar_timeout_us(0)) != PICO_ERROR_TIMEOUT) {
            ultima = c;
        }

        if (ultima != -1) {
            char e = (char)ultima;

            if (e == 'f' || e == 'F') reset_jogo();

            if (jogo_rodando) {
                if ((e == 'w' && direcao != 's') ||
                    (e == 's' && direcao != 'w') ||
                    (e == 'a' && direcao != 'd') ||
                    (e == 'd' && direcao != 'a')) {
                    direcao = e;
                }
            }
        }

        atualizar();
        desenhar_no_buffer();

        limpar_terminal();
        for (int i = 0; i < ALTURA + 6; i++) {
            printf("%s\n", tela[i]);
        }

        fflush(stdout);
        sleep_ms(VELOCIDADE_MS);
    }
}
