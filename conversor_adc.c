#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"

const int BLUE_LED = 12;  // Pino do LED Azul
const int RED_LED = 13;   // Pino do LED Vermelho
#define VRX 27            // Pino do eixo X do joystick
#define VRY 26            // Pino do eixo Y do joystick
#define SW 22             // Pino do botão do joystick
#define ADC_CHANNEL_0 0   // Canal ADC para o eixo X
#define ADC_CHANNEL_1 1   // Canal ADC para o eixo Y

const float DIVIDER_PWM = 16.0;  // Divisor do clock para o PWM
const uint16_t PERIOD = 4096;    // Período do PWM (12 bits)
const uint16_t CENTER_VALUE = 2048; // Valor central do joystick
const uint16_t DEAD_ZONE = 200;  // Zona morta ao redor do centro

uint slice_led_b, slice_led_r;   // Slices de PWM para os LEDs

// Função de configuração geral
void setup()
{
    stdio_init_all();              // Inicializa a comunicação serial
    setup_joystick();              // Configura o joystick
    setup_pwm_led(BLUE_LED, &slice_led_b); // Configura PWM para o LED Azul
    setup_pwm_led(RED_LED, &slice_led_r);  // Configura PWM para o LED Vermelho
}

// Configuração do joystick
void setup_joystick()
{
    adc_init();                    // Inicializa o ADC
    adc_gpio_init(VRX);            // Configura o pino do eixo X
    adc_gpio_init(VRY);            // Configura o pino do eixo Y
    adc_gpio_init(SW);             // Configura o pino do botão
    gpio_set_dir(SW, GPIO_IN);     // Define o pino do botão como entrada
    gpio_pull_up(SW);              // Habilita o pull-up no pino do botão
}

// Configuração do PWM para um LED
void setup_pwm_led(uint led, uint *slice)
{
    gpio_set_function(led, GPIO_FUNC_PWM); // Configura o pino como PWM
    *slice = pwm_gpio_to_slice_num(led);   // Obtém o slice do PWM
    pwm_set_clkdiv(*slice, DIVIDER_PWM);   // Define o divisor de clock
    pwm_set_wrap(*slice, PERIOD);          // Define o período do PWM
    pwm_set_enabled(*slice, true);         // Habilita o PWM
}

// Função para ler os valores dos eixos do joystick
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

// Função para mapear o valor do joystick para o brilho do LED
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

int main()
{
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

        sleep_ms(100); // Pequeno delay para a próxima leitura
    }
}