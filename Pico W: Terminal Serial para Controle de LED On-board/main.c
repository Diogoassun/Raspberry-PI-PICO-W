#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

int main() {
    // Inicializa stdio (USB serial)
    stdio_init_all();

    // Aguarda o USB estabilizar (importante!)
    sleep_ms(2000);

    printf("Inicializando Pico W...\n");

    // Inicializa o chip Wi-Fi (necessário para o LED)
    if (cyw43_arch_init()) {
        printf("Erro ao inicializar CYW43\n");
        while (true) {
            sleep_ms(1000);
        }
    }

    printf("Pronto! Digite ON ou OFF e pressione Enter\n");

    char buffer[16];
    int idx = 0;

    while (true) {
        // ESSENCIAL: mantém o Wi-Fi ativo
        cyw43_arch_poll();

        // Lê caractere do serial (não bloqueante)
        int c = getchar_timeout_us(0);

        if (c != PICO_ERROR_TIMEOUT) {
            if (c == '\n' || c == '\r') {
                buffer[idx] = '\0';

                if (strcmp(buffer, "ON") == 0) {
                    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
                    printf("LED LIGADO\n");
                }
                else if (strcmp(buffer, "OFF") == 0) {
                    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
                    printf("LED DESLIGADO\n");
                }
                else {
                    printf("Comando desconhecido: %s\n", buffer);
                }

                // Limpa o buffer para o próximo comando
                idx = 0;
            }
            else if (idx < (int)sizeof(buffer) - 1) {
                buffer[idx++] = (char)c;
            }
        }

        sleep_ms(10);
    }
}
