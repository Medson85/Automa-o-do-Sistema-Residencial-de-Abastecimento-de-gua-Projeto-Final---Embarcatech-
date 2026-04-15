
#ifndef DISPLAY_OLED_H
#define DISPLAY_OLED_H

#include <stdint.h>
#include <stdbool.h>

// ──────────────────────────────────────────────────────────────
// Pinagem I2C da BitDogLab
// OLED SSD1306 ligado ao barramento I2C1
// SDA = GPIO 14
// SCL = GPIO 15
// ──────────────────────────────────────────────────────────────
#define DISPLAY_SDA     14
#define DISPLAY_SCL     15

// ──────────────────────────────────────────────────────────────
// Campo em edição na tela de configuração
// ──────────────────────────────────────────────────────────────
typedef enum {
    CAMPO_MINIMO = 0,
    CAMPO_MAXIMO
} CampoConfig;

// ──────────────────────────────────────────────────────────────
// Protótipos das funções públicas do módulo
// ──────────────────────────────────────────────────────────────

// Inicializa o display OLED e o barramento I2C associado
void display_init(void);

// Tela principal:
// exibe o nível em percentual + barra gráfica + status da bomba
void display_tela_principal(uint8_t nivel_pct, const char *status_bomba);

// Tela de configuração:
// exibe os limites mínimo e máximo, destacando o campo ativo
void display_tela_configuracao(uint8_t lim_min, uint8_t lim_max, CampoConfig campo_ativo);

// Tela de confirmação:
// exibe uma mensagem curta de feedback ao usuário
void display_tela_confirmacao(const char *mensagem);

#endif // DISPLAY_OLED_H