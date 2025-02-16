#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/i2c.h"
#include "inc/ssd1306.h"
#include "inc/font.h"

const int BUT_A = 5;     // Pino do botão A
const int GREEN_LED = 11; // Pino do LED Verde
const int BLUE_LED = 12;  // Pino do LED Azul
const int RED_LED = 13;   // Pino do LED Vermelho
#define VRX 27            // Pino do eixo X do joystick
#define VRY 26            // Pino do eixo Y do joystick
#define SW 22             // Pino do botão do joystick
#define ADC_CHANNEL_0 0   // Canal ADC para o eixo X
#define ADC_CHANNEL_1 1   // Canal ADC para o eixo Y

#define I2C_PORT i2c1
#define I2C_ADDR 0x3C
#define I2C_SDA 14
#define I2C_SCL 15

ssd1306_t ssd;

const float DIVIDER_PWM = 16.0;  // Divisor do clock para o PWM
const uint16_t PERIOD = 4096;    // Período do PWM (12 bits)
const uint16_t CENTER_VALUE = 2048; // Valor central do joystick
const uint16_t DEAD_ZONE = 200;  // Zona morta ao redor do centro

uint slice_led_b, slice_led_r;   // Slices de PWM para os LEDs

bool pwm_enabled = true; // Flag para verificar se o PWM está habilitado
bool double_border = false; // Flag para verificar se a borda dupla está habilitada

// Função de configuração geral
void setup();
// Configuração do joystick
void setup_joystick();
// Configuração do PWM para um LED
void setup_pwm_led(uint led, uint *slice);
// Função para ler os valores dos eixos do joystick
void joystick_read_axis(uint16_t *vrx_value, uint16_t *vry_value);
// Função para mapear o valor do joystick para o brilho do LED
uint16_t map_joystick_to_brightness(uint16_t joystick_value);
static void irq_handler(uint gpio, uint32_t events);
void i2c_setup();
void draw_square(uint8_t x, uint8_t y);
void move_square(uint8_t *x, uint8_t *y, uint16_t vrx_value, uint16_t vry_value);
void draw_border();
void toggle_border();

int main()
{
    uint8_t square_x = 64, square_y = 32;
    uint16_t vrx_value, vry_value;
    setup();

    while (true)
    {
        joystick_read_axis(&vrx_value, &vry_value); // Lê os valores do joystick

        // Ajusta o brilho do LED Azul (eixo Y)
        uint16_t blue_brightness = map_joystick_to_brightness(vry_value);
        pwm_set_gpio_level(BLUE_LED, blue_brightness);

        // Ajusta o brilho do LED Vermelho (eixo X)
        uint16_t red_brightness = map_joystick_to_brightness(vrx_value);
        pwm_set_gpio_level(RED_LED, red_brightness);

        move_square(&square_x, &square_y, vrx_value, vry_value);

        sleep_ms(100); // Pequeno delay para a próxima leitura
    }
}

// Função de configuração geral
void setup()
{
    stdio_init_all();              // Inicializa a comunicação serial
    setup_joystick();              // Configura o joystick
    i2c_setup();                   // Configura o display OLED
    setup_pwm_led(BLUE_LED, &slice_led_b); // Configura PWM para o LED Azul
    setup_pwm_led(RED_LED, &slice_led_r);  // Configura PWM para o LED Vermelho
    gpio_init(GREEN_LED);          // Inicializa o pino do LED Verde
    gpio_set_dir(GREEN_LED, GPIO_OUT); // Define o pino do LED Verde como saída
    gpio_init(BUT_A);              // Inicializa o pino do botão A
    gpio_set_dir(BUT_A, GPIO_IN);  // Define o pino do botão A como entrada
    gpio_pull_up(BUT_A);           // Habilita o pull-up no pino do botão A
    gpio_set_irq_enabled_with_callback(BUT_A, GPIO_IRQ_EDGE_FALL, true, &irq_handler); // Habilita a interrupção no botão A
}

void setup_joystick()
{
    adc_init();                    // Inicializa o ADC
    adc_gpio_init(VRX);            // Configura o pino do eixo X
    adc_gpio_init(VRY);            // Configura o pino do eixo Y
    gpio_set_dir(SW, GPIO_IN);     // Define o pino do botão como entrada
    gpio_pull_up(SW);              // Habilita o pull-up no pino do botão
    gpio_set_irq_enabled_with_callback(SW, GPIO_IRQ_EDGE_FALL, true, &irq_handler); // Habilita a interrupção no botão A
}

void setup_pwm_led(uint led, uint *slice)
{
    gpio_set_function(led, GPIO_FUNC_PWM); // Configura o pino como PWM
    *slice = pwm_gpio_to_slice_num(led);   // Obtém o slice do PWM
    pwm_set_clkdiv(*slice, DIVIDER_PWM);   // Define o divisor de clock
    pwm_set_wrap(*slice, PERIOD);          // Define o período do PWM
    pwm_set_enabled(*slice, true);         // Habilita o PWM
}

void joystick_read_axis(uint16_t *vrx_value, uint16_t *vry_value)
{
    // Leitura do eixo X
    adc_select_input(ADC_CHANNEL_1); // Seleciona o canal do eixo X
    sleep_us(2);                     // Pequeno delay para estabilidade
    *vrx_value = adc_read();         // Lê o valor do eixo X (0-4095)

    // Leitura do eixo Y
    adc_select_input(ADC_CHANNEL_0); // Seleciona o canal do eixo Y
    sleep_us(2);                     // Pequeno delay para estabilidade
    *vry_value = adc_read();         // Lê o valor do eixo Y (0-4095)

    printf("X: %d, Y: %d\n", *vrx_value, *vry_value); // Exibe os valores
}

uint16_t map_joystick_to_brightness(uint16_t joystick_value)
{
    // Verifica se o valor está dentro da zona morta
    if (joystick_value >= (CENTER_VALUE - DEAD_ZONE) && joystick_value <= (CENTER_VALUE + DEAD_ZONE))
    {
        return 0; // Se estiver na zona morta, retorna 0 (LED desligado)
    }

    // Calcula a diferença absoluta em relação ao centro (2048)
    int16_t diff = (int16_t)joystick_value - CENTER_VALUE;
    if (diff < 0) diff = -diff; // Valor absoluto

    // Subtrai a zona morta da diferença para começar o brilho a partir do fim da zona morta
    diff -= DEAD_ZONE;

    // Mapeia a diferença para o intervalo de 0 a 4095
    if (diff > (CENTER_VALUE - DEAD_ZONE)) diff = (CENTER_VALUE - DEAD_ZONE); // Limita ao máximo
    return (uint16_t)(diff * 2); // Multiplica por 2 para usar todo o range
}

static void irq_handler(uint gpio, uint32_t events) {
    static volatile uint32_t last_time_a = 0; // Tempo da última pressão do botão A
    volatile uint32_t current_time_a = to_ms_since_boot(get_absolute_time()); // Tempo atual da pressão do botão A
    static volatile uint32_t last_time_j = 0; // Tempo da última pressão do botão do joystick
    volatile uint32_t current_time_j = to_ms_since_boot(get_absolute_time()); // Tempo atual da pressão do botão do joystick

    // Verifica se o botão pressionado foi o A
    if (gpio == BUT_A) {
        if (current_time_a - last_time_a < 200) return; // Verifica se o tempo de debounce foi atingido
        last_time_a = current_time_a;

        pwm_enabled = !pwm_enabled;
        // Desativar o PWM dos LEDs
        pwm_set_enabled(slice_led_b, pwm_enabled);
        pwm_set_enabled(slice_led_r, pwm_enabled);
        return;
    }

    // Verifica se o botão pressionado foi o do joystick
    if (gpio == SW) {
        if (current_time_j - last_time_j < 200) return; // Verifica se o tempo de debounce foi atingido
        last_time_j = current_time_j;

        if (pwm_enabled) return; // Se o PWM estiver habilitado, não faz nada
        gpio_put(GREEN_LED, !gpio_get(GREEN_LED)); // Inverte o estado do LED Verde

        toggle_border(); // Alterna entre borda simples e dupla
    }
}

void i2c_setup() {
    // Inicialização do I2C em 400 kHz
    i2c_init(I2C_PORT, 400 * 1000);

    // Configuração dos pinos SDA e SCL
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    ssd1306_init(&ssd, WIDTH, HEIGHT, false, I2C_ADDR, I2C_PORT); // Inicializa o display OLED
    ssd1306_config(&ssd); // Configura o display OLED
    ssd1306_fill(&ssd, false); // Limpa o display OLED
    ssd1306_send_data(&ssd); // Envia os dados para o display OLED
}

void draw_square(uint8_t x, uint8_t y) {
    ssd1306_fill(&ssd, false);       // Limpa a tela
    draw_border();                   // Redesenha a borda antes de desenhar o quadrado
    ssd1306_draw_square(&ssd, x, y); // Desenha o quadrado na posição atualizada
    ssd1306_send_data(&ssd);         // Envia os dados para o display
}

// valores máximo e mínimo de vrx e vry [0, 4095]
// valores máximo e mínimo de x e y [0, 128] e [0, 64]
void move_square(uint8_t *x, uint8_t *y, uint16_t vrx_value, uint16_t vry_value) {
    uint8_t speed = 8;
    
    if (vrx_value < CENTER_VALUE - DEAD_ZONE || vrx_value > CENTER_VALUE + DEAD_ZONE) {
        if (vrx_value < CENTER_VALUE) {
            if (*x > 1) *x -= speed;  // Garante que não ultrapasse a borda esquerda
        } else {
            if (*x < WIDTH - 9) *x += speed; // Garante que não ultrapasse a borda direita
        }
    }

    if (vry_value < CENTER_VALUE - DEAD_ZONE || vry_value > CENTER_VALUE + DEAD_ZONE) {
        if (vry_value < CENTER_VALUE) {
            if (*y < HEIGHT - 9) *y += speed; // Garante que não ultrapasse a borda inferior
        } else {
            if (*y > 1) *y -= speed; // Garante que não ultrapasse a borda superior
        }
    }

    draw_square(*x, *y); // Atualiza a tela com a nova posição do quadrado
}

// Desenhara a borda do display
void draw_border() {
    ssd1306_rect(&ssd, 0, 0, WIDTH, HEIGHT, true, false);

    if (double_border) {
        ssd1306_rect(&ssd, 1, 1, WIDTH - 2, HEIGHT - 2, true, false);
    }

    ssd1306_send_data(&ssd);
}

void toggle_border() {
    double_border = !double_border; // Alterna entre borda simples e dupla
}