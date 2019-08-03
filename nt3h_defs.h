/*
 * MIT License
 * 
 * Copyright (c) 2019 Sean Farrelly
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 * File        nt3h_defs.h
 * Created by  Sean Farrelly
 * Version     1.0
 * 
 */

/*! @file nt3h_defs.h
 * @brief Definitions file for NT3H2111 NFC device driver.
 */

#ifndef _NT3H_DEFS_H_
#define _NT3H_DEFS_H_

/*! CPP guard */
#ifdef __cplusplus
extern "C" {
#endif

/*!
 * @brief Include header files
 */
#include <stdint.h>

#define NT3H_DEFAULT_I2C_ADDRESS        0x40


/*!
 * @brief NT3H API status codes
 */
typedef enum {
    NT3H_OK,
    NT3H_E_NULL_PTR,
    NT3H_E_DEV_NOT_FOUND,
    NT3H_E_INVALID_ARGS,
    NT3H_E_BUSY,
} nt3h_status;

/*!
 * @brief Type declarations
 */
typedef void (*nt3h_com_mem_func_ptr_t)(uint8_t dev_id, uint8_t *data, uint32_t len);
typedef void (*nt3h_com_mem_func_ptr_t)(uint8_t dev_id, uint8_t addr, uint8_t *data, uint32_t len);
typedef void (*nt3h_delay_ms_func_ptr_t)(uint32_t period);

typedef struct {
    uint8_t MagicNumber;
    uint8_t Version;
    uint8_t MLEN;
    uint8_t AccessControl;
} NFC_CCTypeDef;

typedef enum {
    NT3H_IO_NORMAL,
    NT3H_IO_MEMORY
} nt3h_com_type;

struct nt3h_dev {

    /* Device ID */
    uint16_t dev_id;

    /*! Perhaps just use enum to differentiate between mem and normal */

    /* User defined I2C write function pointer */
    nt3h_com_func_ptr_t mem_write;

    /* User defined I2C read function pointer */
    nt3h_com_func_ptr_t mem_read;

    /* User defined I2C write function pointer */
    nt3h_com_func_ptr_t write;

    /* User defined I2C read function pointer */
    nt3h_com_func_ptr_t read;

    /* User defined delay ms function pointer */
    nt3h_delay_ms_func_ptr_t delay_ms;

    //uint32_t            Password;   /* Password for memory protection */
    //volatile NFC_CCTypeDef       CC;         /* Capability Container */

};

#ifdef __cplusplus
}
#endif /* End of CPP guard */
#endif /* NT3H_DEFS_H_ */
/** @}*/