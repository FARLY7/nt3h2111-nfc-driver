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

#ifndef _NT3H2111_H_
#define _NT3H2111_H_

/*! CPP guard */
#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>
#include "stm32l0xx_hal.h"

/* Allow NFC Debug log statements or not */
#define DEBUG_NFC 1 

typedef enum {
    NFC_OK           = 0U,
    NFC_BUSY         = 1U,
    NFC_ERROR        = 2U,
    NFC_INVALID_ARGS = 3U,
} NFC_StatusTypeDef;

typedef enum {
    NFC_STATE_RESET = 0x00U,
    NFC_STATE_READY,
    NFC_STATE_BUSY,
    NFC_STATE_TIMEOUT,
    NFC_STATE_ERROR
} NFC_State;

// typedef struct {
//     I2C_HandleTypeDef   *hi2c;      /* HAL I2C Handler */
//     uint16_t            DevAddress; /* I2C Address of NFC slave */
// } NFC_InitTypeDef;

typedef struct {
    uint8_t MagicNumber;
    uint8_t Version;
    uint8_t MLEN;
    uint8_t AccessControl;
} NFC_CCTypeDef;

typedef struct {

    //NFC_InitTypeDef     Init;

    I2C_HandleTypeDef   *hi2c;      /* HAL I2C Handler */

    uint16_t            DevAddress; /* I2C Address of NFC slave */

    uint32_t            Password;   /* Password for memory protection */

    volatile NFC_CCTypeDef       CC;         /* Capability Container */

    // size_t              MemorySize; /* User memory size according to Capability Container MLEN */

    // size_t              MemoryUsed; /* No of bytes used from user memory */

    volatile NFC_State      State;      /* Current state of NFC driver */

    //void (* Callback)(void);        /* Perhaps add functionality for callbacks later */

} NFC_HandleTypeDef;

NFC_HandleTypeDef hnfc;


/* ==== Initialisation & Deinitialisation ==== */
NFC_StatusTypeDef NFC_Init(NFC_HandleTypeDef *hnfc);
NFC_StatusTypeDef NFC_DeInit(NFC_HandleTypeDef *hnfc);
/* =========================================== */

/* ========= Memory IO Functions ========== */
/* Read & Write bytes to EEPROM and SRAM memory of NFC device */
NFC_StatusTypeDef NFC_WriteMemory(NFC_HandleTypeDef *hnfc, uint16_t address, uint16_t byteOffset, uint8_t bytes[], size_t size, uint32_t timeout);
NFC_StatusTypeDef NFC_ReadMemory(NFC_HandleTypeDef *hnfc, uint16_t address, uint16_t byteOffset, uint8_t bytes[], size_t size, uint32_t timeout);
NFC_StatusTypeDef NFC_EraseMemory(NFC_HandleTypeDef *hnfc, uint16_t address, uint16_t byteOffset, size_t size, uint32_t timeout);
/* Read & Write to Session Registers of NFC device */
NFC_StatusTypeDef NFC_ReadRegister(NFC_HandleTypeDef *hnfc, uint8_t memAddress, uint8_t regAddress, uint8_t *regData, uint32_t timeout);
NFC_StatusTypeDef NFC_WriteRegister(NFC_HandleTypeDef *hnfc, uint8_t memAddress, uint8_t regAddress, uint8_t mask, uint8_t regData, uint32_t timeout);
/* Read & Write to Configuration Registers of NFC device */
NFC_StatusTypeDef NFC_ReadConfig(NFC_HandleTypeDef *hnfc, uint8_t memAddress, uint8_t regAddress, uint8_t *regData, uint32_t timeout);
NFC_StatusTypeDef NFC_WriteConfig(NFC_HandleTypeDef *hnfc, uint8_t memAddress, uint8_t regAddress, uint8_t mask, uint8_t regData, uint32_t timeout);
/* ========================================== */

// /* ============== TLV Functions ============= */
// HAL_StatusTypeDef NFC_ParseTLV(LinkedListTLV *tlv);
// /* ========================================== *

// /* =========== == NDEF Function ============= */
// HAL_StatusTypeDef NFC_NDEF_ParseMessage(uint8_t *buf, size_t len, )
// /* ========================================== */

/* Helpful getters and setters for particular fields in NFC memory */
NFC_StatusTypeDef NFC_ReadFieldPresent(NFC_HandleTypeDef *hnfc, uint8_t *isFieldPresent);
NFC_StatusTypeDef NFC_WriteAddr(NFC_HandleTypeDef *hnfc, uint8_t addr); /* Write 'Addr' (I2C Address) field */
NFC_StatusTypeDef NFC_ReadCC(NFC_HandleTypeDef *hnfc);    /* Read 'Capability Container' field */
NFC_StatusTypeDef NFC_WriteCC(NFC_HandleTypeDef *hnfc, NFC_CCTypeDef *cc);     /* Write 'Capability Container' field */
/* ========================================== */

/* Helpful toString log print functions */
#if DEBUG_NFC == 1
void NFC_PrintMemory(NFC_HandleTypeDef* nfc, uint16_t blockAddress, uint16_t noOfBlocks);
void NFC_PrintDeviceProperties(NFC_HandleTypeDef *nfc);
void NFC_PrintSessionRegisters(NFC_HandleTypeDef *hnfc);
void NFC_PrintConfigRegisters(NFC_HandleTypeDef *hnfc);
#endif
/* ========================================== */


#ifdef __cplusplus
}
#endif /* End of CPP guard */
#endif /* NT3H_H_ */
/** @}*/