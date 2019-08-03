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
 * File        nt3h.h
 * Created by  Sean Farrelly
 * Version     1.0
 * 
 */

/*! @file nt3h.h
 * @brief Driver for NT3H2111 NFC device.
 */

#ifndef _NT3H_H_
#define _NT3H_H_

/*! CPP guard */
#ifdef __cplusplus
extern "C" {
#endif

#include "nt3h_defs.h"
#include <stdint.h>


/* ==== Initialisation & Deinitialisation ==== */
nt3h_status nt3h_init(struct nt3h_dev *dev);
nt3h_status nt3h_deinit(struct nt3h_dev *dev);
/* =========================================== */

/* ========= Memory IO Functions ========== */
/* Read & Write bytes to EEPROM and SRAM memory of NFC device */
nt3h_status nt3h_write_memory(struct nt3h_dev *dev, uint16_t mem_addr, uint16_t byte_offset, uint8_t *data, size_t len);
nt3h_status nt3h_read_memory(struct nt3h_dev *dev, uint16_t mem_addr, uint16_t byte_offset, uint8_t *data, size_t len);
nt3h_status nt3h_erase_memory(struct nt3h_dev *dev, uint16_t mem_addr, uint16_t byte_offset, size_t len);
/* Read & Write to Session Registers of NFC device */
nt3h_status nt3h_read_register(struct nt3h_dev *dev, uint8_t mem_addr, uint8_t reg_addr, uint8_t *data);
nt3h_status nt3h_write_register(struct nt3h_dev *dev, uint8_t mem_addr, uint8_t reg_addr, uint8_t mask, uint8_t data);
/* Read & Write to Configuration Registers of NFC device */
nt3h_status nt3h_read_config(struct nt3h_dev *dev, uint8_t mem_addr, uint8_t reg_addr, uint8_t *data);
nt3h_status nt3h_write_config(struct nt3h_dev *dev, uint8_t mem_addr, uint8_t reg_addr, uint8_t mask, uint8_t data);
/* ========================================== */

/* Helpful getters and setters for particular fields in NFC memory */
nt3h_status nt3h_read_field_present(struct nt3h_dev *dev, uint8_t *data);
nt3h_status nt3h_write_addr(struct nt3h_dev *dev, uint8_t addr); /* Write 'Addr' (I2C Address) field */
nt3h_status nt3h_read_cc(struct nt3h_dev *dev);    /* Read 'Capability Container' field */
nt3h_status nt3h_write_cc(struct nt3h_dev *dev, NFC_CCTypeDef *cc);     /* Write 'Capability Container' field */
/* ========================================== */


nt3h_status nt3h_IRQHandler(void);

nt3h_status nt3h_check(struct nt3h_dev *dev);

// /* Helpful toString log print functions */
// #if DEBUG_NFC == 1
// void NFC_PrintMemory(NFC_HandleTypeDef* nfc, uint16_t blockAddress, uint16_t noOfBlocks);
// void NFC_PrintDeviceProperties(NFC_HandleTypeDef *nfc);
// void NFC_PrintSessionRegisters(NFC_HandleTypeDef *hnfc);
// void NFC_PrintConfigRegisters(NFC_HandleTypeDef *hnfc);
// #endif
// /* ========================================== */


#ifdef __cplusplus
}
#endif /* End of CPP guard */
#endif /* NT3H_H_ */
/** @}*/