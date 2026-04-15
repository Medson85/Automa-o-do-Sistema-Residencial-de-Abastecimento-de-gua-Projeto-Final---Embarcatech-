#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "ssd1306.h"
#include "display_oled.h"

// ──────────────────────────────────────────────────────────────
// Buffer e área de renderização
// A biblioteca da BitDogLab trabalha com um buffer bruto de bytes.
// "frame_area" define a região do display que será atualizada.
// Aqui usamos a tela inteira.
// ──────────────────────────────────────────────────────────────
static uint8_t buf[ssd1306_buffer_length];

static struct render_area frame_area = {
    .start_column = 0,
    .end_column   = ssd1306_width - 1,
    .start_page   = 0,
    .end_page     = ssd1306_n_pages - 1
};

// ──────────────────────────────────────────────────────────────
// Helpers internos
// ──────────────────────────────────────────────────────────────

// Limpa todo o buffer local
static void limpar_buffer(void) {
    memset(buf, 0, ssd1306_buffer_length);
}

// Envia o buffer atual ao display
static void atualizar_display(void) {
    render_on_display(buf, &frame_area);
}

// Desenha um pixel diretamente no buffer
static void desenhar_pixel(int x, int y) {
    if (x < 0 || x >= ssd1306_width) return;
    if (y < 0 || y >= (ssd1306_n_pages * 8)) return;

    int index = x + (y / 8) * ssd1306_width;
    buf[index] |= (1u << (y % 8));
}

// Desenha uma linha horizontal
static void desenhar_linha_h(int x0, int x1, int y) {
    if (x1 < x0) {
        int temp = x0;
        x0 = x1;
        x1 = temp;
    }

    for (int x = x0; x <= x1; x++) {
        desenhar_pixel(x, y);
    }
}

// Desenha contorno de retângulo
static void desenhar_retangulo(int x, int y, int largura, int altura) {
    if (largura <= 0 || altura <= 0) return;

    // Linhas horizontais
    for (int i = x; i < x + largura; i++) {
        desenhar_pixel(i, y);
        desenhar_pixel(i, y + altura - 1);
    }

    // Linhas verticais
    for (int j = y; j < y + altura; j++) {
        desenhar_pixel(x, j);
        desenhar_pixel(x + largura - 1, j);
    }
}

// Preenche retângulo sólido
static void preencher_retangulo(int x, int y, int largura, int altura) {
    if (largura <= 0 || altura <= 0) return;

    for (int j = y; j < y + altura; j++) {
        for (int i = x; i < x + largura; i++) {
            desenhar_pixel(i, j);
        }
    }
}

// ──────────────────────────────────────────────────────────────
// Inicialização do display
// ──────────────────────────────────────────────────────────────
void display_init(void) {
    // Inicializa o barramento I2C1 com a frequência definida pela biblioteca
    i2c_init(i2c1, ssd1306_i2c_clock * 1000);

    // Configura os pinos da BitDogLab para I2C
    gpio_set_function(DISPLAY_SDA, GPIO_FUNC_I2C);
    gpio_set_function(DISPLAY_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(DISPLAY_SDA);
    gpio_pull_up(DISPLAY_SCL);

    // Inicializa o OLED SSD1306
    ssd1306_init();

    // Calcula o tamanho da área de renderização
    calculate_render_area_buffer_length(&frame_area);

    // Limpa o display na inicialização
    limpar_buffer();
    atualizar_display();

    printf("Display OLED inicializado\n");
}

// ──────────────────────────────────────────────────────────────
// Tela principal
// Layout:
// y= 0  -> "CAIXA D'AGUA"
// y=10  -> linha separadora
// y=16  -> "Nivel: xx%"
// y=27  -> barra gráfica
// y=44  -> status da bomba
// ──────────────────────────────────────────────────────────────
void display_tela_principal(uint8_t nivel_pct, const char *status_bomba) {
    char linha[32];

    if (nivel_pct > 100) {
        nivel_pct = 100;
    }

    limpar_buffer();

    // Título
    ssd1306_draw_string(buf, 14, 0, "CAIXA D'AGUA");

    // Linha separadora
    desenhar_linha_h(0, 127, 10);

    // Texto do nível
    snprintf(linha, sizeof(linha), "Nivel: %d%%", nivel_pct);
    ssd1306_draw_string(buf, 0, 16, linha);

    // Barra gráfica
    // Contorno
    desenhar_retangulo(12, 27, 104, 10);

    // Preenchimento interno proporcional (máx. 102 px)
    uint8_t preenchimento = (uint8_t)((nivel_pct * 102) / 100);
    if (preenchimento > 0) {
        preencher_retangulo(13, 28, preenchimento, 8);
    }

    // Status
    ssd1306_draw_string(buf, 0, 44, (char *)status_bomba);

    atualizar_display();
}

// ──────────────────────────────────────────────────────────────
// Tela de configuração
// Layout:
// y= 0  -> "CONFIGURACAO"
// y=10  -> linha separadora
// y=16  -> "Min: xx%"
// y=32  -> "Max: xx%"
// y=52  -> "A:OK B:Volta"
//
// Nesta versão, o campo ativo é indicado por ">".
// Isso é mais simples e robusto com a biblioteca da BitDogLab.
// ──────────────────────────────────────────────────────────────
void display_tela_configuracao(uint8_t lim_min, uint8_t lim_max, CampoConfig campo_ativo) {
    char linha_min[20];
    char linha_max[20];

    snprintf(linha_min, sizeof(linha_min), "Min: %d%%", lim_min);
    snprintf(linha_max, sizeof(linha_max), "Max: %d%%", lim_max);

    limpar_buffer();

    // Título
    ssd1306_draw_string(buf, 14, 0, "CONFIGURACAO");
    desenhar_linha_h(0, 127, 10);

    // Campo mínimo
    if (campo_ativo == CAMPO_MINIMO) {
        ssd1306_draw_string(buf, 0, 16, ">");
        ssd1306_draw_string(buf, 10, 16, linha_min);
        ssd1306_draw_string(buf, 10, 32, linha_max);
    } else {
        ssd1306_draw_string(buf, 10, 16, linha_min);
        ssd1306_draw_string(buf, 0, 32, ">");
        ssd1306_draw_string(buf, 10, 32, linha_max);
    }

    // Rodapé
    ssd1306_draw_string(buf, 0, 52, "A:OK B:Volta");

    atualizar_display();
}

// ──────────────────────────────────────────────────────────────
// Tela de confirmação
// Layout:
// y= 6  -> retângulo decorativo
// y=10  -> "OK!"
// y=36  -> mensagem
// ──────────────────────────────────────────────────────────────
void display_tela_confirmacao(const char *mensagem) {
    limpar_buffer();

    // Caixa decorativa
    desenhar_retangulo(48, 6, 32, 18);
    ssd1306_draw_string(buf, 52, 10, "OK!");

    // Mensagem
    ssd1306_draw_string(buf, 0, 36, (char *)mensagem);

    atualizar_display();
}
