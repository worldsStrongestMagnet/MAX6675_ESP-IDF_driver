#include "max6675.h"
#include "esp_log.h"
#include "driver/gpio.h"

static const char *TAG = "MAX6675_test";

extern "C" void app_main(void){
    max6675_t device;
    max6675_cfg_t configs = {
        .host = SPI2_HOST,
        .miso_io_num = GPIO_NUM_13,
        .sclk_io_num = GPIO_NUM_12,
        .cs_io_num = GPIO_NUM_10,
        .clk_speed_hz = 4000000,
        .max_transfer_size = 0,
        .spi_flags = 0,
    };

    max6675_err_t err = max6675_init(&device, &configs);
    if(err == MAX6675_ERR_NO_DEVICE){
        ESP_LOGE(TAG, "MAX6675 not detected!");
        return;
    }
    else if(err != MAX6675_OK){
        ESP_LOGE(TAG, "failed to init MAX6675");
        return;
    }

    while(true){
        uint16_t raw_reading;
        if(max6675_read_raw(&device, &raw_reading) == MAX6675_OK){
            uint16_t t12;
            bool fault;
            max6675_err_t err = max6675_parse_raw(raw_reading, &t12, &fault);
            if(err == MAX6675_OK){
                float temperature = max8875_temp12_to_c(t12);
                ESP_LOGE(TAG, "Temp = %.2f celsius", temperature);
            }
            else if(err == MAX6675_ERR_OPEN_TC){
                ESP_LOGE(TAG, "Thermocouple open");
            } else{
                ESP_LOGE(TAG, "parse error: %d", err);
            }
        }
        else{
            ESP_LOGE(TAG, "SPI read failed");
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}