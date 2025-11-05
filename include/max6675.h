#ifndef _MAX6675_H_
#define _MAX6675_H_

#include "driver/spi_master.h"
#include "esp_err.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Error/fault codes specific to MAX6675 operations */
typedef enum {
    MAX6675_OK = 0,
    MAX6675_ERR_INVALID_ARG = ESP_ERR_INVALID_ARG,
    MAX6675_ERR_SPI = -1,       /**< generic SPI error / transaction failure */
    MAX6675_ERR_OPEN_TC = -2,   /**< thermocouple open / disconnected (fault bit) */
    MAX6675_ERR_NO_DEVICE = -3, /** No valid response from MAX6675 chip - check power/SPI connections **/
} max6675_err_t;

/** Opaque handle for a MAX6675-connected device */
typedef struct {
    spi_device_handle_t spi_dev;  /**< SPI device handle (as attached to bus) */
} max6675_t;

/** Configuration parameters for initialization */
typedef struct {
    spi_host_device_t host;     /**< SPI bus (e.g. SPI2_HOST, SPI3_HOST) */
    int miso_io_num;            /**< GPIO number for MISO (SO) */
    int sclk_io_num;            /**< GPIO number for SCK */
    int cs_io_num;              /**< Chip‐Select GPIO (active low) */
    int clk_speed_hz;           /**< SPI clock speed in Hz (≤ ~4.3 MHz per datasheet) */
    int max_transfer_size;      /**< Maximum transfer buffer size (bytes) or 0 for default */
    /** optional SPI flags, e.g. SPI_DEVICE_NO_DUMMY, etc. */
    int spi_flags;
} max6675_cfg_t;

/**
 * @brief Initialize the MAX6675 interface (attach to SPI bus)
 *
 * This will (if needed) initialize the SPI bus and attach the MAX6675 as a device.
 *
 * @param[out] out_dev Pointer to a `max6675_t` structure to populate.
 * @param[in] cfg Configuration parameters for SPI and pins.
 * @return MAX6675_OK (or ESP_OK) on success, or other error code.
 */
max6675_err_t max6675_init(max6675_t *out_dev, const max6675_cfg_t *cfg);

/**
 * @brief Deinitialize / free the MAX6675 device
 *
 * This will remove the SPI device. It does *not* deinitialize the SPI bus
 * (you’ll need to handle that if you manage it externally).
 *
 * @param dev Previously initialized handle
 * @return MAX6675_OK on success, or error code
 */
max6675_err_t max6675_deinit(max6675_t *dev);

/**
 * @brief Read the raw 16-bit data from the MAX6675
 *
 * The raw 16 bits include temperature bits and fault bit (bit 2). No scaling or shifting is done.
 *
 * @param dev The MAX6675 device handle
 * @param[out] raw_out The raw 16-bit data
 * @return MAX6675_OK or an SPI error code
 */
max6675_err_t max6675_read_raw(max6675_t *dev, uint16_t *raw_out);

static inline max6675_err_t max6675_parse_raw(uint16_t raw, uint16_t *temp12, bool *fault){
    if (temp12 == NULL || fault == NULL){
        return MAX6675_ERR_INVALID_ARG;
    }

    uint16_t swapped = (uint16_t) __builtin_bswap16(raw);

    *fault = ((swapped & (1u << 2)) != 0);
    
    if(*fault){
        return MAX6675_ERR_OPEN_TC;
    }

    *temp12 = (swapped >> 3) & 0x0FFF;

    return MAX6675_OK;
}

static inline float max6675_temp12_to_c(uint16_t temp12){
    return (float) temp12 * 0.25f;
}

#ifdef __cplusplus
}
#endif

#endif // _MAX6675_H_
