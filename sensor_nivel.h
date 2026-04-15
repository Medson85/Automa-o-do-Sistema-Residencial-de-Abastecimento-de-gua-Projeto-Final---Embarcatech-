#ifndef SENSOR_NIVEL_H
#define SENSOR_NIVEL_H

#include <stdint.h>
#include <stdbool.h>

// ──────────────────────────────────────────────────────────────
// Pinos do HC-SR04
// Ajuste conforme sua montagem
// ──────────────────────────────────────────────────────────────
#define HC_SR04_TRIG    18
#define HC_SR04_ECHO    19

// ──────────────────────────────────────────────────────────────
// Tempos de timeout
// ──────────────────────────────────────────────────────────────
#define HC_SR04_TIMEOUT_US      30000   // 30 ms
#define HC_SR04_TRIGGER_US      10      // pulso mínimo de trigger

// ──────────────────────────────────────────────────────────────
// Calibração do reservatório
//
// distancia_vazio_cm:
// distância entre sensor e água quando a caixa está vazia
//
// distancia_cheio_cm:
// distância entre sensor e água quando a caixa está cheia
//
// Exemplo:
// vazio = 40 cm
// cheio = 5 cm
// ──────────────────────────────────────────────────────────────
#define DISTANCIA_VAZIO_CM      118.0f
#define DISTANCIA_CHEIO_CM      60.0f

// ──────────────────────────────────────────────────────────────
// Status de leitura
// ──────────────────────────────────────────────────────────────
typedef enum {
    SENSOR_OK = 0,
    SENSOR_ERRO_TIMEOUT_SUBIDA,
    SENSOR_ERRO_TIMEOUT_DESCIDA,
    SENSOR_ERRO_PARAMETRO
} SensorStatus;

// ──────────────────────────────────────────────────────────────
// Protótipos públicos
// ──────────────────────────────────────────────────────────────

// Inicializa os pinos do HC-SR04
void sensor_nivel_init(void);

// Lê a distância em cm
SensorStatus sensor_nivel_ler_distancia(float *distancia_cm);

// Converte distância em percentual
SensorStatus sensor_nivel_ler_percentual(uint8_t *nivel_pct);

// Converte uma distância qualquer em percentual
SensorStatus sensor_nivel_converter_para_percentual(float distancia_cm, uint8_t *nivel_pct);

#endif // SENSOR_NIVEL_H