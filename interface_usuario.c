
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "interface_usuario.h"

// ──────────────────────────────────────────────────────────────
// Estado atual da interface
// ──────────────────────────────────────────────────────────────
Tela tela_atual = TELA_PRINCIPAL;

// ──────────────────────────────────────────────────────────────
// Flags setadas pela ISR e consumidas no loop principal
// ──────────────────────────────────────────────────────────────
static volatile bool flag_btn_a = false;
static volatile bool flag_btn_b = false;

// Instante do último acionamento válido
// usado para debounce e para medir pressão longa
static volatile uint32_t tempo_press_a = 0;
static volatile uint32_t tempo_press_b = 0;

// ──────────────────────────────────────────────────────────────
// Callback de interrupção dos botões
// BitDogLab: usamos pull-up interno
// Em repouso = HIGH
// Pressionado = LOW
// Portanto, pressionar gera borda de descida
// ──────────────────────────────────────────────────────────────
static void callback_botoes(uint gpio, uint32_t eventos) {
    uint32_t agora = to_ms_since_boot(get_absolute_time());

    // Botão A
    if (gpio == BOTAO_A) {
        if ((agora - tempo_press_a) > DEBOUNCE_MS) {
            flag_btn_a = true;
            tempo_press_a = agora;
        }
    }

    // Botão B
    if (gpio == BOTAO_B) {
        if ((agora - tempo_press_b) > DEBOUNCE_MS) {
            flag_btn_b = true;
            tempo_press_b = agora;
        }
    }
}

// ──────────────────────────────────────────────────────────────
// Inicialização da interface do usuário
// ──────────────────────────────────────────────────────────────
void interface_init(void) {
    // Botão A
    gpio_init(BOTAO_A);
    gpio_set_dir(BOTAO_A, GPIO_IN);
    gpio_pull_up(BOTAO_A);

    // Botão B
    gpio_init(BOTAO_B);
    gpio_set_dir(BOTAO_B, GPIO_IN);
    gpio_pull_up(BOTAO_B);

    // Interrupção na borda de descida = botão pressionado
    gpio_set_irq_enabled_with_callback(
        BOTAO_A,
        GPIO_IRQ_EDGE_FALL,
        true,
        &callback_botoes
    );

    gpio_set_irq_enabled(
        BOTAO_B,
        GPIO_IRQ_EDGE_FALL,
        true
    );

    // ADC do joystick
    adc_init();
    adc_gpio_init(JOYSTICK_X);   // GP26
    adc_gpio_init(JOYSTICK_Y);   // GP27

    printf("Interface usuario inicializada\n");
}

// ──────────────────────────────────────────────────────────────
// Leitura do joystick
// ADC 12 bits: 0..4095
// Centro aproximado: 2048
// ──────────────────────────────────────────────────────────────
uint16_t joystick_ler_x(void) {
    adc_select_input(0);   // ADC0 = GP26
    return adc_read();
}

uint16_t joystick_ler_y(void) {
    adc_select_input(1);   // ADC1 = GP27
    return adc_read();
}

// ──────────────────────────────────────────────────────────────
// Processamento dos eventos
// Prioridade:
// 1) Botão A longo
// 2) Botão A curto
// 3) Botão B
// 4) Joystick
// ──────────────────────────────────────────────────────────────
EventoUI interface_processar_eventos(void) {
    uint32_t agora = to_ms_since_boot(get_absolute_time());

    // ----------------------------------------------------------
    // Botão A: pressão longa
    // flag setada pela ISR
    // botão ainda pressionado -> nível LOW
    // ----------------------------------------------------------
    if (flag_btn_a && !gpio_get(BOTAO_A)) {
        if ((agora - tempo_press_a) >= PRESSAO_LONGA_MS) {
            flag_btn_a = false;
            printf("Botao A: pressao longa detectada\n");
            return EVENTO_BTN_A_LONGO;
        }
    }

    // ----------------------------------------------------------
    // Botão A: pressão curta
    // flag setada pela ISR
    // botão já foi solto -> nível HIGH
    // ----------------------------------------------------------
    if (flag_btn_a && gpio_get(BOTAO_A)) {
        flag_btn_a = false;
        printf("Botao A: pressao curta detectada\n");
        return EVENTO_BTN_A_CURTO;
    }

    // ----------------------------------------------------------
    // Botão B: simples
    // como só existe um evento para B, basta retornar
    // quando a flag for observada
    // ----------------------------------------------------------
    if (flag_btn_b) {
        flag_btn_b = false;
        printf("Botao B: pressionado\n");
        return EVENTO_BTN_B;
    }

    // ----------------------------------------------------------
    // Joystick: só na tela de configuração
    // ----------------------------------------------------------
    if (tela_atual == TELA_CONFIGURACAO) {
        uint16_t joy_y = joystick_ler_y();

        // Dependendo da montagem do joystick, o sentido pode ficar invertido.
        // Se perceber que cima/baixo ficou trocado, basta inverter estas duas comparações.

        if (joy_y > (ADC_CENTRO + ADC_DEAD_ZONE)) {
            sleep_ms(DELAY_JOYSTICK_MS);
            return EVENTO_JOY_CIMA;
        }

        if (joy_y < (ADC_CENTRO - ADC_DEAD_ZONE)) {
            sleep_ms(DELAY_JOYSTICK_MS);
            return EVENTO_JOY_BAIXO;
        }
    }

    return EVENTO_NENHUM;
}

