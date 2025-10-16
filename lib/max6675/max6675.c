#include "max6675.h"
#include "driver/spi_master.h"
#include "esp_err.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "MAX6675";

max6675_err_t max6675_init(max6675_t *out_device, const max6675_cfg_t *cfg){

    if(!out_device || !cfg){
        return MAX6675_ERR_INVALID_ARG;
    }

    spi_bus_config_t buscfg = {
        .miso_io_num = cfg -> miso_io_num,
        .mosi_io_num = -1,
        .sclk_io_num = cfg -> sclk_io_num,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = cfg -> max_transfer_size > 0 ? cfg -> max_transfer_size : sizeof(uint16_t),
    };

    esp_err_t err = spi_bus_initialize(cfg -> host, &buscfg, SPI_DMA_CH_AUTO);

    if(err != ESP_OK && err != ESP_ERR_INVALID_STATE){
        ESP_LOGE(TAG, "spi_bus_initialize failed: %s", esp_err_to_name(err));
        return (max6675_err_t) err;
    }

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = cfg -> clk_speed_hz,
        .mode = 0,
        .spics_io_num = cfg -> cs_io_num,
        .queue_size = 1,
        .flags = cfg -> spi_flags,
    };

    spi_device_handle_t spi_dev = NULL;

    err = spi_bus_add_device(cfg -> host, &devcfg, &spi_dev);
    if(err != ESP_OK){
        ESP_LOGE(TAG, "spi_bus_add_device failed: %s", esp_err_to_name(err));
        return (max6675_err_t) err;
    }

    out_device -> spi_dev = spi_dev;

    uint16_t test_raw = 0xFFFF;
    err = max6675_read_raw(out_device, &test_raw);
    if(err != ESP_OK){
        ESP_LOGE(TAG, "MAX6675 read failed during init: %s", esp_err_to_name(err));
        return MAX6675_ERR_NO_DEVICE;
    }

    if(test_raw == 0xFFFF || test_raw == 0x0000){
        ESP_LOGE(TAG, "No valid response from MAX6675. Check wiring or power.");
        return MAX6675_ERR_NO_DEVICE;
    }

    ESP_LOGI(TAG, "MAX6675 initialized: CS = %d, CLK = %d, MISO = %d", cfg -> cs_io_num, cfg-> sclk_io_num, cfg -> miso_io_num);

    return MAX6675_OK;
}

max6675_err_t max6675_deinit(max6675_t *dev){
    if(dev == NULL){
        return MAX6675_ERR_INVALID_ARG;
    }

    esp_err_t err = spi_bus_remove_device(dev -> spi_dev);

    if(err != ESP_OK){
        ESP_LOGE(TAG, "spi_bus_remove_device failed: %s", esp_err_to_name(err));
        return (max6675_err_t) err;
    }

    return MAX6675_OK;
}

max6675_err_t max6675_read_raw(max6675_t *dev, uint16_t *raw_out){
    if(!dev || !raw_out){
        return MAX6675_ERR_INVALID_ARG;
    }

    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length = 16;
    t.rxlength = 16;
    t.flags = SPI_TRANS_USE_RXDATA;
    t.tx_buffer = NULL;

    esp_err_t err = spi_device_transmit(dev -> spi_dev, &t);
     if(err != ESP_OK){
        ESP_LOGE(TAG, "spi_device_transmit failed: %s", esp_err_to_name(err));
        return (max6675_err_t) err;
     }

     uint16_t raw = *((uint16_t *) t.rx_data);
     *raw_out = raw;
     return MAX6675_OK;
}
