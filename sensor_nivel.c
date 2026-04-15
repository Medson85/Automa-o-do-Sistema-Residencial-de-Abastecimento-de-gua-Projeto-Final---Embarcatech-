#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "sensor_nivel.h"

// ──────────────────────────────────────────────────────────────
// Inicialização
// ──────────────────────────────────────────────────────────────
void sensor_nivel_init(void) {
    // TRIG como saída
    gpio_init(HC_SR04_TRIG);
    gpio_set_dir(HC_SR04_TRIG, GPIO_OUT);
    gpio_put(HC_SR04_TRIG, 0);

    // ECHO como entrada
    gpio_init(HC_SR04_ECHO);
    gpio_set_dir(HC_SR04_ECHO, GPIO_IN);

    printf("Sensor de nivel HC-SR04 inicializado\n");
}

// ──────────────────────────────────────────────────────────────
// Lê distância em cm
//
// Funcionamento:
// 1. Gera pulso de 10 us em TRIG
// 2. Espera ECHO subir
// 3. Mede quanto tempo ECHO fica em nível alto
// 4. Converte tempo em distância
//
// Fórmula prática:
// distancia_cm = tempo_us / 58.0
//
// Isso já considera ida e volta do som no ar.
// ──────────────────────────────────────────────────────────────
SensorStatus sensor_nivel_ler_distancia(float *distancia_cm) {
    if (distancia_cm == NULL) {
        return SENSOR_ERRO_PARAMETRO;
    }

    uint32_t tempo_inicio;
    uint32_t tempo_echo_subida;
    uint32_t tempo_echo_descida;
    uint32_t duracao_echo_us;

    // Garante TRIG em nível baixo antes do pulso
    gpio_put(HC_SR04_TRIG, 0);
    sleep_us(2);

    // Pulso de trigger
    gpio_put(HC_SR04_TRIG, 1);
    sleep_us(HC_SR04_TRIGGER_US);
    gpio_put(HC_SR04_TRIG, 0);

    // Espera subida do ECHO
    tempo_inicio = time_us_32();
    while (gpio_get(HC_SR04_ECHO) == 0) {
        if ((time_us_32() - tempo_inicio) > HC_SR04_TIMEOUT_US) {
            return SENSOR_ERRO_TIMEOUT_SUBIDA;
        }
    }

    tempo_echo_subida = time_us_32();

    // Espera descida do ECHO
    while (gpio_get(HC_SR04_ECHO) == 1) {
        if ((time_us_32() - tempo_echo_subida) > HC_SR04_TIMEOUT_US) {
            return SENSOR_ERRO_TIMEOUT_DESCIDA;
        }
    }

    tempo_echo_descida = time_us_32();

    // Duração do pulso de eco em microssegundos
    duracao_echo_us = tempo_echo_descida - tempo_echo_subida;

    // Conversão para cm
    *distancia_cm = (float)duracao_echo_us / 58.0f;

    return SENSOR_OK;
}

// ──────────────────────────────────────────────────────────────
// Converte distância em percentual
//
// Lógica:
// - quando distância = DISTANCIA_VAZIO_CM  => 0%
// - quando distância = DISTANCIA_CHEIO_CM  => 100%
//
// Como no reservatório cheio a água fica mais perto do sensor,
// a distância diminui conforme o nível sobe.
// ──────────────────────────────────────────────────────────────
SensorStatus sensor_nivel_converter_para_percentual(float distancia_cm, uint8_t *nivel_pct) {
    float percentual;
    float faixa_util;

    if (nivel_pct == NULL) {
        return SENSOR_ERRO_PARAMETRO;
    }

    faixa_util = DISTANCIA_VAZIO_CM - DISTANCIA_CHEIO_CM;

    if (faixa_util <= 0.0f) {
        return SENSOR_ERRO_PARAMETRO;
    }

    percentual = ((DISTANCIA_VAZIO_CM - distancia_cm) / faixa_util) * 100.0f;

    // Saturação do valor
    if (percentual < 0.0f) {
        percentual = 0.0f;
    }

    if (percentual > 100.0f) {
        percentual = 100.0f;
    }

    *nivel_pct = (uint8_t)(percentual + 0.5f);

    return SENSOR_OK;
}

// ──────────────────────────────────────────────────────────────
// Faz a leitura completa do nível em percentual
// ──────────────────────────────────────────────────────────────
SensorStatus sensor_nivel_ler_percentual(uint8_t *nivel_pct) {
    float distancia_cm;
    SensorStatus status;

    if (nivel_pct == NULL) {
        return SENSOR_ERRO_PARAMETRO;
    }

    status = sensor_nivel_ler_distancia(&distancia_cm);
    if (status != SENSOR_OK) {
        return status;
    }

    return sensor_nivel_converter_para_percentual(distancia_cm, nivel_pct);
}