
#ifndef INTERFACE_USUARIO_H
#define INTERFACE_USUARIO_H

#include <stdint.h>
#include <stdbool.h>

// ──────────────────────────────────────────────────────────────
// Definição dos pinos da BitDogLab
// ──────────────────────────────────────────────────────────────
#define BOTAO_A         5       // GPIO 5  - confirmar / acionar
#define BOTAO_B         6       // GPIO 6  - cancelar / voltar
#define JOYSTICK_X      26      // GP26    - ADC0 (eixo X)
#define JOYSTICK_Y      27      // GP27    - ADC1 (eixo Y)

// ──────────────────────────────────────────────────────────────
// Constantes de tempo
// ──────────────────────────────────────────────────────────────
#define DEBOUNCE_MS         200     // anti-repique dos botões
#define PRESSAO_LONGA_MS    3000    // limiar para clique longo do botão A
#define DELAY_JOYSTICK_MS   180     // atraso entre eventos sucessivos do joystick

// ──────────────────────────────────────────────────────────────
// Constantes do ADC
// ADC do RP2040: 12 bits -> 0 a 4095
// Centro aproximado do joystick: ~2048
// ──────────────────────────────────────────────────────────────
#define ADC_CENTRO      2048
#define ADC_DEAD_ZONE   600

// ──────────────────────────────────────────────────────────────
// Estados da interface / telas
// ──────────────────────────────────────────────────────────────
typedef enum {
    TELA_PRINCIPAL = 0,     // exibe nível e status da bomba
    TELA_CONFIGURACAO,      // ajuste de limites mínimo e máximo
    TELA_CONFIRMACAO        // mensagem temporária de feedback
} Tela;

// ──────────────────────────────────────────────────────────────
// Eventos gerados pela interface do usuário
// ──────────────────────────────────────────────────────────────
typedef enum {
    EVENTO_NENHUM = 0,
    EVENTO_BTN_A_CURTO,     // confirmar / avançar
    EVENTO_BTN_A_LONGO,     // alternar modo manual / automático
    EVENTO_BTN_B,           // cancelar / voltar
    EVENTO_JOY_CIMA,        // incrementar valor
    EVENTO_JOY_BAIXO        // decrementar valor
} EventoUI;

// ──────────────────────────────────────────────────────────────
// Variável global do estado atual da tela
// Definida em interface_usuario.c
// ──────────────────────────────────────────────────────────────
extern Tela tela_atual;

// ──────────────────────────────────────────────────────────────
// Protótipos públicos do módulo
// ──────────────────────────────────────────────────────────────

// Inicializa botões, ADC do joystick e interrupções
void interface_init(void);

// Processa os eventos pendentes da interface
// Retorna um evento por chamada, ou EVENTO_NENHUM
EventoUI interface_processar_eventos(void);

// Leitura bruta dos eixos do joystick
uint16_t joystick_ler_x(void);
uint16_t joystick_ler_y(void);

#endif // INTERFACE_USUARIO_H

