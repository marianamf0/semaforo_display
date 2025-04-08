#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "inc/ssd1306.h"
#include "pico/binary_info.h"

#define I2C_PORT i2c0
#define I2C_SDA 14
#define I2C_SCL 15
#define LED_R_PIN 13
#define LED_G_PIN 11
#define LED_B_PIN 12
#define BTN_A_PIN 5

int button_state = 0;

void green_signal(){
    gpio_put(LED_R_PIN, 0);
    gpio_put(LED_G_PIN, 1);
    gpio_put(LED_B_PIN, 0);   
}

void yellow_signal(){
    gpio_put(LED_R_PIN, 1);
    gpio_put(LED_G_PIN, 1);
    gpio_put(LED_B_PIN, 0);
}

void red_signal(){
    gpio_put(LED_R_PIN, 1);
    gpio_put(LED_G_PIN, 0);
    gpio_put(LED_B_PIN, 0);
}

void clear_display(struct render_area *area){
    uint8_t ssd[ssd1306_buffer_length];
    memset(ssd, 0, ssd1306_buffer_length);
    render_on_display(ssd, area);
}

void message_display(struct render_area *area, char *text[], size_t text_count){
    clear_display(area);
    uint8_t ssd[ssd1306_buffer_length];
    int total_text_height = 8 * text_count;
    int y_start = (ssd1306_height - total_text_height) / 2;

    int y = y_start;
    for (size_t i = 0; i < text_count; i++) {
        int text_length = strlen(text[i]);
        int x_start = (ssd1306_width - (text_length * 8)) / 2;
        ssd1306_draw_string(ssd, x_start, y, text[i]);
        y += 15;
    }

    render_on_display(ssd, area);
}

int wait_with_read(int time){
    for(int i = 0; i < time; i = i+100){
        button_state = !gpio_get(BTN_A_PIN);
        if(button_state == 1){
            return 1;
        }
        sleep_ms(100);
    }
    return 0;
}


int main()
{
    stdio_init_all();

    // Inicialização do i2c
    i2c_init(i2c1, ssd1306_i2c_clock * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Processo de inicialização completo do OLED SSD1306
    ssd1306_init();

    struct render_area frame_area = {
        start_column : 0,
        end_column : ssd1306_width - 1,
        start_page : 0,
        end_page : ssd1306_n_pages - 1
    };

    calculate_render_area_buffer_length(&frame_area);
    clear_display(&frame_area);

    // INICIANDO LEDS
    gpio_init(LED_R_PIN);
    gpio_set_dir(LED_R_PIN, GPIO_OUT);
    gpio_init(LED_G_PIN);
    gpio_set_dir(LED_G_PIN, GPIO_OUT);
    gpio_init(LED_B_PIN);
    gpio_set_dir(LED_B_PIN, GPIO_OUT);

    // INICIANDO BOTÄO
    gpio_init(BTN_A_PIN);
    gpio_set_dir(BTN_A_PIN, GPIO_IN);
    gpio_pull_up(BTN_A_PIN);

restart:
    char *text_green[] = {"SINAL ABERTO", "ATRAVESSAR", "COM CUIDADO"};
    char *text_red[] = {"SINAL FECHADO", "AGUARDE", " "};
    char *text_yellow[] = {"SINAL DE", "ATENCAO", "PREPARE-SE"};

    
    while (true) {
        
        int time_green = button_state ? 10000: 8000;  

        green_signal();
        message_display(&frame_area, text_green, count_of(text_green));
        sleep_ms(time_green);

        red_signal();
        message_display(&frame_area, text_red, count_of(text_red));
        button_state = wait_with_read(8000); 

        int time_yellow = button_state ? 5000: 3000;

        yellow_signal();
        message_display(&frame_area, text_yellow, count_of(text_yellow));
        sleep_ms(time_yellow);

}
return 0;
}
