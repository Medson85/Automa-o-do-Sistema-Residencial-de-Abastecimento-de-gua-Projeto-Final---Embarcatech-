
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "buzzer.h"

// ──────────────────────────────────────────────────────────────
// Configuração interna
// ──────────────────────────────────────────────────────────────
// Clock base do RP2040: 125 MHz
// Vamos usar divisor fixo e calcular o wrap.
#define PWM_DIVISOR         125.0f

// Frequências típicas agradáveis para alerta
#define TOM_PADRAO_HZ       2000
#define TOM_ALERTA_HZ       1500

// ──────────────────────────────────────────────────────────────
// Variáveis internas
// ──────────────────────────────────────────────────────────────
static uint slice_buzzer_a;
static uint slice_buzzer_b;

// ──────────────────────────────────────────────────────────────
// Helper interno: configura a frequência do PWM
// ──────────────────────────────────────────────────────────────
static void buzzer_configurar_frequencia(uint gpio, uint slice, uint32_t frequencia_hz) {
    // Fórmula aproximada:
    // freq_pwm = 125000000 / (divisor * (wrap + 1))
    uint32_t wrap = (uint32_t)(125000000.0f / (PWM_DIVISOR * frequencia_hz)) - 1;

    pwm_set_clkdiv(slice, PWM_DIVISOR);
    pwm_set_wrap(slice, wrap);

    // 50% duty cycle
    pwm_set_gpio_level(gpio, wrap / 2);
}

// ──────────────────────────────────────────────────────────────
// Inicialização
// ──────────────────────────────────────────────────────────────
void buzzer_init(void) {
    gpio_set_function(BUZZER_A, GPIO_FUNC_PWM);
    gpio_set_function(BUZZER_B, GPIO_FUNC_PWM);

    slice_buzzer_a = pwm_gpio_to_slice_num(BUZZER_A);
    slice_buzzer_b = pwm_gpio_to_slice_num(BUZZER_B);

    pwm_set_enabled(slice_buzzer_a, false);
    pwm_set_enabled(slice_buzzer_b, false);

    printf("Buzzers inicializados\n");
}

// ──────────────────────────────────────────────────────────────
// Liga o tom nos dois buzzers
// ──────────────────────────────────────────────────────────────
void buzzer_tocar(uint32_t frequencia_hz) {
    buzzer_configurar_frequencia(BUZZER_A, slice_buzzer_a, frequencia_hz);
    buzzer_configurar_frequencia(BUZZER_B, slice_buzzer_b, frequencia_hz);

    pwm_set_enabled(slice_buzzer_a, true);
    pwm_set_enabled(slice_buzzer_b, true);
}

// ──────────────────────────────────────────────────────────────
// Desliga os dois buzzers
// ──────────────────────────────────────────────────────────────
void buzzer_parar(void) {
    pwm_set_enabled(slice_buzzer_a, false);
    pwm_set_enabled(slice_buzzer_b, false);
}

// ──────────────────────────────────────────────────────────────
// Bip simples
// ──────────────────────────────────────────────────────────────
void buzzer_bip(uint32_t frequencia_hz, uint32_t duracao_ms) {
    buzzer_tocar(frequencia_hz);
    sleep_ms(duracao_ms);
    buzzer_parar();
}

// ──────────────────────────────────────────────────────────────
// 1 bip curto = confirmação
// ──────────────────────────────────────────────────────────────
void buzzer_confirmacao(void) {
    buzzer_bip(TOM_PADRAO_HZ, 100);
}

// ──────────────────────────────────────────────────────────────
// 2 bipes curtos = bomba ligada
// ──────────────────────────────────────────────────────────────
void buzzer_bomba_ligada(void) {
    buzzer_bip(TOM_PADRAO_HZ, 100);
    sleep_ms(100);
    buzzer_bip(TOM_PADRAO_HZ, 100);
}

// ──────────────────────────────────────────────────────────────
// 3 bipes curtos = bomba desligada
// ──────────────────────────────────────────────────────────────
void buzzer_bomba_desligada(void) {
    buzzer_bip(TOM_PADRAO_HZ, 100);
    sleep_ms(100);
    buzzer_bip(TOM_PADRAO_HZ, 100);
    sleep_ms(100);
    buzzer_bip(TOM_PADRAO_HZ, 100);
}

// ──────────────────────────────────────────────────────────────
// 1 bip longo = nível crítico
// ──────────────────────────────────────────────────────────────
void buzzer_nivel_critico(void) {
    buzzer_bip(TOM_ALERTA_HZ, 800);
}

// ──────────────────────────────────────────────────────────────
// 2 bipes longos = falha de leitura do sensor
// ──────────────────────────────────────────────────────────────
void buzzer_falha_sensor(void) {
    buzzer_bip(TOM_ALERTA_HZ, 500);
    sleep_ms(150);
    buzzer_bip(TOM_ALERTA_HZ, 500);
}

