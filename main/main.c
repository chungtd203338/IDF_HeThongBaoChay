#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "dht11.h"
#include "driver/i2c.h"
#include "lcd_i2c.h"
#include "driver/adc.h"
#include "driver/uart.h"
#include "string.h"
#include "driver/gpio.h"
#define flameSensor 5
#define buzzer 19
#define MQ2_ADC 32
#define MQ2 14
#define DHT11PIN 5
#define UART_NUM UART_NUM_2
#define TXD_PIN (GPIO_NUM_17)
#define RXD_PIN (GPIO_NUM_16)
#define BUF_SIZE (1024)
static const adc_channel_t channel = ADC1_CHANNEL_4;
void lcd1602_write_char(lcd1602_t *lcd, char character) {
    lcd1602_send_data(lcd, (uint8_t)character);
}

// Hàm ghi một chuỗi lên màn hình LCD
void lcd1602_write_string(lcd1602_t *lcd, const char *str) {
    while (*str) {
        lcd1602_write_char(lcd, *str);
        str++;
    }
}
void lcd1602_write_float(lcd1602_t *lcd, float num, int decimalPlaces) {
    char buffer[20];
    snprintf(buffer, sizeof(buffer), "%.*f", decimalPlaces, num);
    lcd1602_write_string(lcd, buffer);
}

// Hàm ghi một số nguyên lên màn hình LCD
void lcd1602_write_int(lcd1602_t *lcd, int num) {
    char buffer[20];
    snprintf(buffer, sizeof(buffer), "%d", num);
    lcd1602_write_string(lcd, buffer);
}
void init_uart() {
    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

    uart_param_config(UART_NUM, &uart_config);
    uart_set_pin(UART_NUM, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_NUM, BUF_SIZE * 2, 0, 0, NULL, 0);
}
void send_at_command(const char *command) {
    const int len = strlen(command);
    uart_write_bytes(UART_NUM, command, len);
    //uart_write_bytes(UART_NUM, "\r\n", 2);
}

void app_main(void)
{   
    init_uart();
    lcd1602_i2c_init(21, 22, 0);

    lcd1602_t lcdtmp = {0};
    lcdtmp.i2caddr = 0x27;
    lcdtmp.backlight = 1;
    
    lcd1602_dcb_set(&lcdtmp, 1, 0, 0);

    lcd1602_init(&lcdtmp);
     lcd1602_set_pos(&lcdtmp, 0, 0);

    // Ghi một chuỗi lên màn hình
    lcd1602_write_string(&lcdtmp, "Hello LCD!");

    lcd1602_set_pos(&lcdtmp, 1, 0);

    // Ghi một số nguyên lên màn hình
    lcd1602_write_int(&lcdtmp, 123);

    printf("ESP32 DHT11 TEST:%s,%s!\r\n", __DATE__, __TIME__);
    //   vTaskDelay(5000 / portTICK_PERIOD_MS);
    //   send_at_command("ATD+8497021042;");
    float humidity, temperature;
    
    uint32_t gas=0;
    uint8_t flame=1;
    uint8_t dataGas=0;
    adc1_config_width(ADC_WIDTH_BIT_12);    // Độ rộng bit 12
    adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_2_5 ); 
    while (1)
    {   gas=adc1_get_raw(channel);
        flame=gpio_get_level(flameSensor);
        printf("%d\n",flame);
        printf("%d\n",gas);

        dht_read_data(DHT11PIN, &humidity, &temperature);
        printf("Temp=%f, Humi=%f\r\n", temperature, humidity);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        lcd1602_clear(&lcdtmp);
        lcd1602_set_pos(&lcdtmp, 0, 0);
        lcd1602_write_string(&lcdtmp, "Nhiet do:");
        lcd1602_set_pos(&lcdtmp, 0, 10);
        lcd1602_write_float(&lcdtmp, temperature,0);
        lcd1602_write_string(&lcdtmp, " C");

        lcd1602_set_pos(&lcdtmp, 1, 0);
        lcd1602_write_string(&lcdtmp, "Do am :");
        lcd1602_set_pos(&lcdtmp, 1, 10);
        lcd1602_write_int(&lcdtmp, humidity);
        //lcd1602_write_float(&lcdtmp, gas,0);
        lcd1602_write_string(&lcdtmp, " %");
        if(flame ==0){
            printf("có cháy");
            gpio_set_level(buzzer, 1);
            lcd1602_clear(&lcdtmp);
            lcd1602_set_pos(&lcdtmp, 0, 0);
            lcd1602_write_string(&lcdtmp, "WARNING!!");
            lcd1602_set_pos(&lcdtmp, 1, 0);
            lcd1602_write_string(&lcdtmp, "BURNING");
            delay(1000);
        }
        else {
            printf("không cháy");
            gpio_set_level(buzzer, 0);
        }
        if(gas >2800){
            dataGas=1;
            printf("có khí gas");
            gpio_set_level(buzzer, 1);
            lcd1602_clear(&lcdtmp);
            lcd1602_set_pos(&lcdtmp, 0, 0);
            lcd1602_write_string(&lcdtmp, "WARNING!!");
            lcd1602_set_pos(&lcdtmp, 1, 0);
            lcd1602_write_string(&lcdtmp, "GAS");
            delay(1000);
        }
        else{
            dataGas=0;
            gpio_set_level(buzzer, 0);
        } 
    }
}
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "freertos/event_groups.h"
// #include "esp_wifi.h"
// #include "esp_log.h"
// #include "esp_event_loop.h"
// #include "esp_smartconfig.h"
// #include "esp_system.h"
// #include "nvs_flash.h"
// #include "esp_spi_flash.h"

// #include "user_smartconfig.h"
// #include "hal_led.h"


// void app_main()
// {
	
//     esp_chip_info_t chip_info;
//     esp_chip_info( &chip_info );
	
// 	printf("========================================================\r\n");
// 	printf("[WIFI SMARTCONFIG] Example : Smartconfig WeChat! \r\n");
// 	printf("[WIFI SMARTCONFIG] FLASH SIZE: %d MB (%s) \r\n", spi_flash_get_chip_size() / (1024 * 1024),
// 	                                                         (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");
// 	printf("[WIFI SMARTCONFIG] Free memory: %d bytes \r\n", esp_get_free_heap_size());
	
// 	ledGpioInit();
	
// 	user_smartconfig_init();
// }
