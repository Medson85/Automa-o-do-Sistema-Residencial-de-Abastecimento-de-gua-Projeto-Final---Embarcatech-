
#ifndef BUZZER_H
#define BUZZER_H

#include <stdint.h>
#include <stdbool.h>

// ──────────────────────────────────────────────────────────────
// Pinos dos buzzers na BitDogLab
// ──────────────────────────────────────────────────────────────
#define BUZZER_A    10
#define BUZZER_B    21

// ──────────────────────────────────────────────────────────────
// Protótipos públicos
// ──────────────────────────────────────────────────────────────

// Inicializa os buzzers
void buzzer_init(void);

// Liga um tom com frequência definida
void buzzer_tocar(uint32_t frequencia_hz);

// Desliga o buzzer
void buzzer_parar(void);

// Emite um bip único
void buzzer_bip(uint32_t frequencia_hz, uint32_t duracao_ms);

// Padrões do projeto
void buzzer_confirmacao(void);      // 1 bip curto
void buzzer_bomba_ligada(void);     // 2 bipes curtos
void buzzer_bomba_desligada(void);  // 3 bipes curtos
void buzzer_nivel_critico(void);    // 1 bip longo
void buzzer_falha_sensor(void);     // 2 bipes longos

#endif // BUZZER_H

