
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "interface_usuario.h"
#include "display_oled.h"
#include "buzzer.h"
#include "sensor_nivel.h"

// =================================================================
// Configurações do sistema
// =================================================================
#define PERIODO_LEITURA_SENSOR_MS   1000
#define LIMIAR_NIVEL_CRITICO        10

// =================================================================
// Variáveis do sistema
// =================================================================
static uint8_t     nivel_atual  = 0;          // nível real lido do sensor
static uint8_t     lim_min      = 20;         // limite mínimo configurável
static uint8_t     lim_max      = 90;         // limite máximo configurável
static CampoConfig campo_ativo  = CAMPO_MINIMO;
static bool        modo_manual  = false;

static SensorStatus status_sensor = SENSOR_OK;
static uint32_t ultimo_tempo_leitura_sensor = 0;

static bool alerta_sensor_emitido = false;
static bool alerta_nivel_critico_emitido = false;

// =================================================================
// Retorna o texto de status que será mostrado na tela principal
// =================================================================
static const char *obter_status_bomba(void) {
    if (status_sensor != SENSOR_OK) {
        return "ERRO SENSOR";
    }

    if (modo_manual) {
        return "MANUAL";
    }

    if (nivel_atual <= lim_min) {
        return "BAIXO";
    }

    if (nivel_atual >= lim_max) {
        return "CHEIO";
    }

    return "AGUARDANDO";
}

// =================================================================
// Atualiza a tela principal com nível e status atuais
// =================================================================
static void atualizar_tela_principal(void) {
    display_tela_principal(nivel_atual, obter_status_bomba());
}

// =================================================================
// Faz a leitura do sensor e trata alertas
// =================================================================
static void atualizar_sensor_nivel(void) {
    uint8_t nivel_lido = 0;

    status_sensor = sensor_nivel_ler_percentual(&nivel_lido);

    if (status_sensor == SENSOR_OK) {
        nivel_atual = nivel_lido;
        printf("Nivel atual: %d%%\n", nivel_atual);

        // Se voltou a ler corretamente, limpa trava de alerta de falha
        alerta_sensor_emitido = false;

        // Alerta de nível crítico abaixo de 10%
        if (nivel_atual < LIMIAR_NIVEL_CRITICO) {
            if (!alerta_nivel_critico_emitido) {
                buzzer_nivel_critico();
                alerta_nivel_critico_emitido = true;
            }
        } else {
            alerta_nivel_critico_emitido = false;
        }
    }
    else {
        printf("Erro na leitura do sensor: %d\n", status_sensor);

        // Emite alerta apenas uma vez enquanto o erro persistir
        if (!alerta_sensor_emitido) {
            buzzer_falha_sensor();
            alerta_sensor_emitido = true;
        }
    }
}

// =================================================================
// Programa principal
// =================================================================
int main() {
    stdio_init_all();

    if (cyw43_arch_init()) {
        printf("Erro ao inicializar cyw43\n");
        return -1;
    }

    interface_init();
    display_init();
    buzzer_init();
    sensor_nivel_init();

    // Feedback sonoro de inicialização
    buzzer_confirmacao();

    // Primeira leitura do sensor
    atualizar_sensor_nivel();

    // Exibe tela inicial
    atualizar_tela_principal();
    printf("Sistema iniciado. Usando nivel real do HC-SR04.\n");

    ultimo_tempo_leitura_sensor = to_ms_since_boot(get_absolute_time());

    while (true) {
        uint32_t agora_ms = to_ms_since_boot(get_absolute_time());

        // =========================================================
        // Leitura periódica do sensor
        // =========================================================
        if ((agora_ms - ultimo_tempo_leitura_sensor) >= PERIODO_LEITURA_SENSOR_MS) {
            ultimo_tempo_leitura_sensor = agora_ms;

            atualizar_sensor_nivel();

            if (tela_atual == TELA_PRINCIPAL) {
                atualizar_tela_principal();
            }
        }

        // =========================================================
        // Processamento dos eventos da interface
        // =========================================================
        EventoUI evento = interface_processar_eventos();

        switch (evento) {

            // =====================================================
            // Botão A curto
            // =====================================================
            case EVENTO_BTN_A_CURTO:
                printf("Botao A curto\n");

                if (tela_atual == TELA_PRINCIPAL) {
                    buzzer_confirmacao();

                    tela_atual = TELA_CONFIGURACAO;
                    display_tela_configuracao(lim_min, lim_max, campo_ativo);
                }
                else if (tela_atual == TELA_CONFIGURACAO) {
                    if (campo_ativo == CAMPO_MINIMO) {
                        // Alterna da edição do mínimo para o máximo
                        buzzer_confirmacao();

                        campo_ativo = CAMPO_MAXIMO;
                        display_tela_configuracao(lim_min, lim_max, campo_ativo);
                    }
                    else {
                        // Segundo OK: salva e volta
                        buzzer_confirmacao();

                        tela_atual = TELA_CONFIRMACAO;
                        display_tela_confirmacao("CONFIG. SALVA");
                        printf("Limites salvos: min=%d%% max=%d%%\n", lim_min, lim_max);

                        sleep_ms(2000);

                        campo_ativo = CAMPO_MINIMO;
                        tela_atual  = TELA_PRINCIPAL;
                        atualizar_tela_principal();
                    }
                }
                break;

            // =====================================================
            // Botão A longo
            // =====================================================
            case EVENTO_BTN_A_LONGO:
                modo_manual = !modo_manual;
                printf("Modo manual: %s\n", modo_manual ? "ATIVADO" : "DESATIVADO");

                buzzer_confirmacao();

                tela_atual = TELA_CONFIRMACAO;
                display_tela_confirmacao(modo_manual ? "MODO MANUAL" : "MODO AUTO");

                sleep_ms(2000);

                tela_atual = TELA_PRINCIPAL;
                atualizar_tela_principal();
                break;

            // =====================================================
            // Botão B
            // =====================================================
            case EVENTO_BTN_B:
                printf("Botao B - voltando para tela principal\n");

                buzzer_confirmacao();

                campo_ativo = CAMPO_MINIMO;
                tela_atual  = TELA_PRINCIPAL;
                atualizar_tela_principal();
                break;

            // =====================================================
            // Joystick para cima
            // =====================================================
            case EVENTO_JOY_CIMA:
                if (tela_atual == TELA_CONFIGURACAO) {
                    if (campo_ativo == CAMPO_MINIMO && lim_min < (lim_max - 5)) {
                        lim_min++;
                    }
                    else if (campo_ativo == CAMPO_MAXIMO && lim_max < 100) {
                        lim_max++;
                    }

                    printf("Limites: min=%d%% max=%d%%\n", lim_min, lim_max);
                    display_tela_configuracao(lim_min, lim_max, campo_ativo);
                }
                break;

            // =====================================================
            // Joystick para baixo
            // =====================================================
            case EVENTO_JOY_BAIXO:
                if (tela_atual == TELA_CONFIGURACAO) {
                    if (campo_ativo == CAMPO_MINIMO && lim_min > 0) {
                        lim_min--;
                    }
                    else if (campo_ativo == CAMPO_MAXIMO && lim_max > (lim_min + 5)) {
                        lim_max--;
                    }

                    printf("Limites: min=%d%% max=%d%%\n", lim_min, lim_max);
                    display_tela_configuracao(lim_min, lim_max, campo_ativo);
                }
                break;

            default:
                break;
        }

        sleep_ms(10);
    }

    return 0;
}

