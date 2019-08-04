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
 * File        nt3h.c
 * Created by  Sean Farrelly
 * Version     1.0
 * 
 */

/*! @file nt3h.c
 * @brief Driver for NT3H2111 NFC device.
 */
#include <stdio.h>
#include <string.h>
#include "nt3h.h"

#define NFC_I2C_MEM_BLOCK_SIZE 16                       /* Size of NFC memory block in bytes, from I2C perspective */
#define NFC_I2C_MEM_ADDRESS_SIZE (I2C_MEMADD_SIZE_8BIT) /* Size of NFC I2C memory address (8-Bits, or 16-bits) */
#define NFC_SRAM_ADDRESS 0xF8                           /* Size of NFC SRAM Region */
#define NFC_SRAM_LENGTH 64                              /* Bytes */
#define NFC_MEMORY_ERASE_VALUE 0x00U
#define NFC_DEFAULT_TIMEOUT 1000 /* Default timeout value used in blocking mode (ms) */

/* Capability Container field masks */
#define NFC_CC_MAGIC_NUMBER 0xFF000000U
#define NFC_CC_VER_ACCESS 0x00FF0000U
#define NFC_CC_MLEN 0x0000FF00U
#define NFC_CC_ADDITIONAL 0x000000FFU

/* Block of data as defined by NFC_I2C_MEM_BLOCK_SIZE */
struct data_block
{
    uint8_t data[NFC_I2C_MEM_BLOCK_SIZE];
};

static nt3h_status null_ptr_check(struct nt3h_dev *dev);
static nt3h_status read_blocks(struct nt3h_dev *dev, uint16_t mem_addr, struct data_block *block, uint16_t block_cnt);
static nt3h_status write_blocks(struct nt3h_dev *dev, uint16_t mem_addr, struct data_block *block, uint16_t block_cnt);

/**
  * @brief  Initialise NFC device. Will check if device is alive over I2C
  * @param  *hnfc Pointer to NFC handler 
  * @retval HAL status
  */
nt3h_status nt3h_init(struct nt3h_dev *dev)
{
    nt3h_status rslt;

    /* Check for null pointer in device structure */
    if ((rslt = null_ptr_check(dev)) != NT3H_OK)
        return rslt;

    /* Check if device is responding */
    if ((rslt = nt3h_check(dev)) != NT3H_OK)
        return rslt;

    /* Read compatibility container from device */
    if ((rslt = nt3h_read_cc(dev)) != NT3H_OK)
        return rslt;

    if (IS_CC_EMPTY(cc))
    {
        //     (hnfc->CC.MagicNumber == 0 && hnfc->CC.Version == 0 &&
        //    hnfc->CC.MLEN == 0 && hnfc->CC.AccessControl == 0)    {

        NFC_CCTypeDef cc = {
            .MagicNumber = 0xE1,
            .Version = 0x10,
            .MLEN = 0x6D,
            .AccessControl = 0x00};

        /* Write new compatibility container */
        if ((nt3h_write_cc(dev, &cc)) != NT3H_OK)
            return rslt;
    }

    return rslt;
}

/**
  * @brief  De-initialise NFC device.
  * @param  *hnfc Pointer to NFC handler 
  * @retval HAL status
  */
nt3h_status nt3h_deinit(struct nt3h_dev *dev)
{
    nt3h_status rslt;

    /* Check for null pointer in device structure */
    if ((rslt = null_ptr_check(dev)) != NT3H_OK)
        return rslt;

    return rslt;
}

/**
  * @brief  Write a block(s) of data in blocking mode to NFC memory (EEPROM or SRAM).
  * @param  *hnfc Pointer to NFC handler
  * @param  address Beginning address of first block
  * @param  *block Pointer to NFC Block(s) containing data
  * @param  noOfBlocks Number of NFC Block(s) to write
  * @param  timeout Timeout value for blocking 
  * @retval HAL status
  */
static nt3h_status write_blocks(struct nt3h_dev *dev, uint16_t mem_addr, struct data_block *block, uint16_t block_cnt)
{
    nt3h_status rslt;

    /* Check for null pointer in device structure */
    if ((rslt = null_ptr_check(dev)) != NT3H_OK)
        return rslt;

    /* Check parameters are valid */
    if (block == NULL || block_cnt == 0)
        return NT3H_E_INVALID_ARGS;

    while (block_cnt > 0U)
    {
        if ((rslt = write_mem(dev->dev_id, mem_addr, block->data, NFC_I2C_MEM_BLOCK_SIZE)) != NT3H_OK)
            return rslt;

        if ((mem_addr > NFC_SRAM_ADDRESS) && (mem_addr < (NFC_SRAM_ADDRESS + NFC_SRAM_LENGTH)))
        {
            /* Address is within SRAM memory region. Time to write 1-block = 0.4ms. */
        }
        else
        {
            /* Address is within EEPROM memory region. Time to write 1-block = 4ms */
            delay_ms(4); /* ALlow time for NFC to complete write to its memory */
        }

        block++; /* Move to next block of data */
        mem_addr++;
        block_cnt--;
    }

    return rslt;
}

/**
  * @brief  Read a block(s) of data in blocking mode from NFC memory (EEPROM or SRAM).
  * @param  *hnfc Pointer to NFC handler
  * @param  address Beginning address of first block
  * @param  *block Pointer to NFC Block(s) to dump data
  * @param  noOfBlocks Number of NFC Block(s) to read
  * @param  timeout Timeout value for blocking 
  * @retval HAL status
  */
static nt3h_status read_blocks(struct nt3h_dev *dev, uint16_t mem_addr, struct data_block *block, uint16_t block_cnt)
{
    nt3h_status rslt;

    /* Check for null pointer in device structure */
    if ((rslt = null_ptr_check(dev)) != NT3H_OK)
        return rslt;

    /* Check parameters are valid */
    if (block == NULL || block_cnt == 0)
        return NT3H_E_INVALID_ARGS;

    while (block_cnt > 0U)
    {
        if ((rslt = read_mem(dev->dev_id, mem_addr, block->data, NFC_I2C_MEM_BLOCK_SIZE)) != NT3H_OK)
            return rslt;

        block++; /* Move to next block of data */
        mem_addr++;
        block_cnt--;
    }

    return rslt;
}

/**
  * @brief  Read number of bytes from NFC memory
  * @param  *hnfc Pointer to NFC handler
  * @param  address I2C memory address
  * @param  byteOffset Byte offset from I2C memory address
  * @param  bytes[] Buffer to store bytes read
  * @param  size Number of bytes to read
  * @param  timeout Timeout value 
  * @retval HAL status
  */
nt3h_status nt3h_read_memory(struct nt3h_dev *dev, uint16_t mem_addr, uint16_t byte_offset, uint8_t *data, size_t len)
{
    nt3h_status rslt;

    /* Check for null pointer in device structure */
    if ((rslt = null_ptr_check(dev)) != NT3H_OK)
        return rslt;

    /* Check parameters are valid */
    if (data == NULL || len == 0)
        return NT3H_E_INVALID_ARGS;

    /* Firstly, remove redundant offset if offset is greater than block size */
    mem_addr += (byte_offset / NFC_I2C_MEM_BLOCK_SIZE);                             /* Increment address by number of whole blocks */
    byte_offset -= (byte_offset / NFC_I2C_MEM_BLOCK_SIZE) * NFC_I2C_MEM_BLOCK_SIZE; /* Subtract offset by it's equivalent amount in bytes */

    volatile uint16_t blocksNeeded = len / NFC_I2C_MEM_BLOCK_SIZE; /* Calculate how many whole blocks */
    volatile uint16_t partialBytes = len % NFC_I2C_MEM_BLOCK_SIZE; /* Calculate how many remainder bytes */

    blocksNeeded += (partialBytes) ? 1 : 0;                                 /* Add extra block for remainder */
    blocksNeeded += ((byte_offset + len) > NFC_I2C_MEM_BLOCK_SIZE) ? 1 : 0; /* If operation reads into next block, add another block */

    struct data_block blocks[blocksNeeded];

    if ((rslt = read_blocks(dev, mem_addr, blocks, blocksNeeded)) != NT3H_OK)
        return rslt;

    uint8_t *ptr = ((uint8_t *)blocks) + byte_offset;
    memcpy(data, ptr, len);

    return rslt;
}

/**
  * @brief  Write number of bytes to NFC memory
  * @param  *hnfc Pointer to NFC handler
  * @param  address I2C memory address
  * @param  byteOffset Byte offset from I2C memory address
  * @param  bytes[] Buffer containing bytes to write
  * @param  size Number of bytes to write
  * @param  timeout Timeout value 
  * @retval HAL status
  */
nt3h_status nt3h_write_memory(struct nt3h_dev *dev, uint16_t mem_addr, uint16_t byte_offset, uint8_t *data, size_t len)
{
    nt3h_status rslt;

    /* Check for null pointer in device structure */
    if ((rslt = null_ptr_check(dev)) != NT3H_OK)
        return rslt;

    /*!! Check addresses are within bounds !! */
    if(data == NULL || len == 0)
        return NT3H_E_INVALID_ARGS;

    /* Firstly, remove redundant offset if offset is greater than block size */
    mem_addr += (byte_offset / NFC_I2C_MEM_BLOCK_SIZE);                             /* Increment address by number of whole blocks */
    byte_offset -= (byte_offset / NFC_I2C_MEM_BLOCK_SIZE) * NFC_I2C_MEM_BLOCK_SIZE; /* Subtract offset by it's equivalent amount in bytes */

    volatile uint16_t blocksNeeded = len / NFC_I2C_MEM_BLOCK_SIZE; /* Calculate how many whole blocks */
    volatile uint16_t partialBytes = len % NFC_I2C_MEM_BLOCK_SIZE; /* Calculate how many remainder bytes */

    blocksNeeded += (partialBytes) ? 1 : 0;                                 /* Add extra block for remainder */
    blocksNeeded += ((byte_offset + len) > NFC_I2C_MEM_BLOCK_SIZE) ? 1 : 0; /* If operation reads into next block, add another block */

    NFC_Block blocks[blocksNeeded];

    if (NFC_ReadBlocks(hnfc, address, blocks, blocksNeeded, timeout) != NFC_OK)
    {
        Log_info("NFC: Unable to read block when writing bytes");
        return NFC_ERROR;
    }

    uint8_t *ptr = ((uint8_t *)blocks) + byteOffset;
    memcpy(ptr, bytes, size);

    if (NFC_WriteBlocks(hnfc, address, blocks, blocksNeeded, timeout) != NFC_OK)
    {
        Log_info("NFC: Unable to write block when writing bytes");
        return NFC_ERROR;
    }

    return NFC_OK;
}

/**
  * @brief  Erase number of bytes of NFC memory
  * @param  *hnfc Pointer to NFC handler
  * @param  address I2C memory address
  * @param  byteOffset Byte offset from I2C memory address
  * @param  size Number of bytes to erase
  * @param  timeout Timeout value 
  * @retval HAL status
  */
nt3h_status nt3h_erase_memory(struct nt3h_dev *dev, uint16_t mem_addr, uint16_t byte_offset, size_t len)
{
    nt3h_status rslt;

    /* Check for null pointer in device structure */
    if ((rslt = null_ptr_check(dev)) != NT3H_OK)
        return rslt;

    /*!! Check addresses are within bounds !! */
    if(len == 0)
        return NT3H_E_INVALID_ARGS;

    /* Firstly, remove redundant offset if offset is greater than block size */
    mem_addr += (byte_offset / NFC_I2C_MEM_BLOCK_SIZE);                             /* Increment address by number of whole blocks */
    byte_offset -= (byte_offset / NFC_I2C_MEM_BLOCK_SIZE) * NFC_I2C_MEM_BLOCK_SIZE; /* Subtract offset by it's equivalent amount in bytes */

    volatile uint16_t blocksNeeded = len / NFC_I2C_MEM_BLOCK_SIZE; /* Calculate how many whole blocks */
    volatile uint16_t partialBytes = len % NFC_I2C_MEM_BLOCK_SIZE; /* Calculate how many remainder bytes */

    blocksNeeded += (partialBytes) ? 1 : 0;                                 /* Add extra block for remainder */
    blocksNeeded += ((byte_offset + len) > NFC_I2C_MEM_BLOCK_SIZE) ? 1 : 0; /* If operation reads into next block, add another block */

    struct data_block blocks[blocksNeeded];

    if ((rslt = read_blocks(dev, mem_addr, blocks, blocksNeeded)) != NT3H_OK)
        return rslt;

    uint8_t *ptr = ((uint8_t *)blocks) + byte_offset;
    memset(ptr, NFC_MEMORY_ERASE_VALUE, len);

    if ((rslt = write_blocks(dev, mem_addr, blocks, blocksNeeded)) != NT3H_OK)
        return rslt;

    return rslt;
}

/**
  * @brief  Read a session register from NFC memory.
  * @param  *hnfc Pointer to NFC handler
  * @param  memAddress Beginning address of session registers
  * @param  regAddress Number of register within register block
  * @param  *regData buffer to store register value
  * @param  timeout Timeout value for blocking 
  * @retval HAL status
  */
nt3h_status nt3h_read_register(struct nt3h_dev *dev, uint8_t mem_addr, uint8_t reg_addr, uint8_t *data)
{
    nt3h_status rslt;

    /* Check for null pointer in device structure */
    if ((rslt = null_ptr_check(dev)) != NT3H_OK)
        return rslt;

    /* Check parameters are valid */
    if (data == NULL)
        return NT3H_E_INVALID_ARGS;

    /* Create I2C payload to read from NFC register, according to NFC spec */
    uint8_t txBuff[2] = {mem_addr, reg_addr};

    if ((rslt = write(dev->dev_id, txBuff, sizeof(txBuff))) != NT3H_OK)
        return rslt;

    uint8_t rxBuff;

    if ((rslt = read(dev->dev_id, &rxBuff, sizeof(rxBuff))) != NT3H_OK)
        return rslt;

    *data = rxBuff;

    return rslt;
}

/**
  * @brief  Write to session register in NFC memory.
  * @param  *hnfc Pointer to NFC handler
  * @param  memAddress Beginning address of session registers
  * @param  regAddress Number of register within register block
  * @param  mask Register write mask
  * @param  regData Value to write to register
  * @param  timeout Timeout value for blocking 
  * @retval HAL status
  */
nt3h_status nt3h_write_register(struct nt3h_dev *dev, uint8_t mem_addr, uint8_t reg_addr, uint8_t mask, uint8_t data)
{
    nt3h_status rslt;

    /* Check for null pointer in device structure */
    if ((rslt = null_ptr_check(dev)) != NT3H_OK)
        return rslt;

    /*!! Check addresses are within bounds !! */

    /* Create I2C payload to write to NFC register according to NFC spec */
    uint8_t txBuff[4] = {mem_addr, reg_addr, mask, data};

    if((rslt = write(dev->dev_id, txBuff, sizeof(txBuff))) != NT3H_OK)
        return rslt;

    return rslt;
}

/**
  * @brief  Write to configuration register in NFC memory.
  * @param  *hnfc Pointer to NFC handler
  * @param  memAddress Beginning address of configuration registers
  * @param  regAddress Number of register within register block
  * @param  mask Register write mask
  * @param  regData Value to write to register
  * @param  timeout Timeout value for blocking 
  * @retval HAL status
  */
nt3h_status nt3h_write_config(struct nt3h_dev *dev, uint8_t mem_addr, uint8_t reg_addr, uint8_t mask, uint8_t data)
{
    nt3h_status rslt;

    /* Check for null pointer in device structure */
    if ((rslt = null_ptr_check(dev)) != NT3H_OK)
        return rslt;

    /*!! Check addresses are within bounds !! */

    struct data_block block;

    if ((rslt = read_blocks(dev, mem_addr, &block, 1)) != NT3H_OK)
        return rslt;
    
    block.data[reg_addr] = (block.data[reg_addr] & mask) | data;

    if((rslt = write_blocks(dev, mem_addr, &block, 1)) != NT3H_OK)
        return rslt;

    return rslt;
}

/**
  * @brief  Read configuration register from NFC memory.
  * @param  *hnfc Pointer to NFC handler
  * @param  memAddress Beginning address of configuration registers
  * @param  regAddress Number of register within register block
  * @param  *regData Buffer to store register value
  * @param  timeout Timeout value for blocking 
  * @retval HAL status
  */
nt3h_status nt3h_read_config(struct nt3h_dev *dev, uint8_t mem_addr, uint8_t reg_addr, uint8_t *data)
{
    nt3h_status rslt;

    /* Check for null pointer in device structure */
    if ((rslt = null_ptr_check(dev)) != NT3H_OK)
        return rslt;

    /*!! Check addresses are within bounds !! */

    struct data_block block;

    if ((rslt = read_blocks(dev, mem_addr, &block, 1)) != NT3H_OK)
        return rslt;

    *data = block.data[reg_addr];

    return rslt;
}

/**
  * @brief  Program I2C address of NFC device by writing to Addr field 
  * @param  *hnfc Pointer to NFC handler
  * @param  addr I2C Address (UNSHIFTED). Warning: Must shift left by 1 before calling API.
  * @retval HAL status
  */
nt3h_status nt3h_write_addr(struct nt3h_dev *dev, uint8_t addr)
{
    nt3h_status rslt;

    /* Check for null pointer in device structure */
    if ((rslt = null_ptr_check(dev)) != NT3H_OK)
        return rslt;

    /*!! Check addresses are within bounds !! */

    struct data_block block;

    /* Copy current contents of Block 0 */
    if ((rslt = read_blocks(dev, 0x00, &block, 1)) != NT3H_OK)
        return rslt;
        
    block.data[0] = addr; /* Overwrite block with new addr value */

    /* Write new block back to Block 0 */
    if ((rslt = write_blocks(dev, 0x00, &block, 1)) != NT3H_OK)
        return rslt;

    return rslt;
}

/* Check if there is a NFC field present on the NFC antenna
 * This is achieved by performing an I2C memory read on the
 * NS_REG location and checking the RF_FIELD_PRESENT field
 * 
 * Session Registers Address = 0xFE
 * NS_REG Register = Byte 6
 * RF_FIELD_PRESENT Field = Bit 0
 */
nt3h_status nt3h_read_field_present(struct nt3h_dev *dev, uint8_t *is_field_present)
{
    nt3h_status rslt;

    /* Check for null pointer in device structure */
    if ((rslt = null_ptr_check(dev)) != NT3H_OK)
        return rslt;

    /*!! Check addresses are within bounds !! */
    if(is_field_present == NULL)
        return NT3H_E_NULL_PTR;

    uint8_t NS_REG;

    if ((rslt = nt3h_read_register(dev, 0xFE, 6, &NS_REG)) != NT3H_OK)
        return rslt;

    *is_field_present = NS_REG & 0x01; /* Mask for RF_FIELD_PRESENT */

    return rslt;
}

/* Read Capability Container field */
nt3h_status nt3h_read_cc(struct nt3h_dev *dev)
{
    nt3h_status rslt;

    /* Check for null pointer in device structure */
    if ((rslt = null_ptr_check(dev)) != NT3H_OK)
        return rslt;

    /*!! Check addresses are within bounds !! */

    struct data_block block;

    if ((rslt = nt3h_read_blocks(dev, 0x00, &block, 1)) != NT3H_OK)
        return rslt;

    memcpy(&dev->CC, &block.data[12], 4);

    return rslt;
}

/* Write Capability Container field */
nt3h_status nt3h_write_cc(struct nt3h_dev *dev, NFC_CCTypeDef *cc)
{
    nt3h_status rslt;

    /* Check for null pointer in device structure */
    if ((rslt = null_ptr_check(dev)) != NT3H_OK)
        return rslt;

    /*!! Check addresses are within bounds !! */
    
    struct data_block block;

    if ((rslt = nt3h_read_blocks(dev, 0x00, &block, 1)) != NT3H_OK)
        return rslt;

    memcpy(&block.data[12], cc, 4);

    /* Write new block back to Block 0 */
    if ((rslt = nt3h_write_blocks(dev, 0x00, &block, 1)) != NT3H_OK)
        return rslt;

    return rslt;
}

// /* =================================================================== */
// /* =============== Helpful toString Log Print Functions ============== */
// #if DEBUG_NFC == 1

// void NFC_PrintDeviceProperties(NFC_HandleTypeDef *hnfc)
// {
//     NFC_Block block;

//     Log_info("----- NFC Device Details ----");

//     if (NFC_ReadBlocks(hnfc, 0x00, &block, 1, 1000) == NFC_OK)
//     {
//         Log_info("Addr: %02X", block.Data[0]);

//         Log_info("Serial: %02X:%02X:%02X:%02X:%02X:%02X",
//                  block.Data[1], block.Data[2],
//                  block.Data[3], block.Data[4],
//                  block.Data[5], block.Data[6]);

//         Log_info("Static lock bytes: %02X:%02X", block.Data[10], block.Data[11]);
//     }

//     if (NFC_ReadCC(hnfc) == NFC_OK)
//     {
//         Log_info("CC: %02X:%02X:%02X:%02X",
//                  hnfc->CC.MagicNumber,
//                  hnfc->CC.Version,
//                  hnfc->CC.MLEN,
//                  hnfc->CC.AccessControl);

//         Log_info("Memory Size: %d bytes", hnfc->CC.MLEN * 8);
//     }

//     Log_info("-----------------------------");
// }

// void NFC_PrintMemory(NFC_HandleTypeDef *hnfc, uint16_t blockAddress, uint16_t noOfBlocks)
// {
//     Log_info("****** NFC Memory Dump *****");
//     NFC_Block blocks[noOfBlocks];
//     uint8_t buffer[128];

//     if (NFC_ReadBlocks(hnfc, blockAddress, blocks, noOfBlocks, 1000) == NFC_OK)
//     {
//         for (size_t blockNo = 0; blockNo < noOfBlocks; blockNo++)
//         {
//             uint8_t *ptr = &buffer[0];
//             ptr += sprintf((char *)ptr, "0x%02X", blockAddress);

//             for (int i = 0; i < 16; i += 4)
//             {
//                 ptr += sprintf((char *)ptr, "\t");

//                 for (int j = 0; j < 4; j++)
//                 {
//                     ptr += sprintf((char *)ptr, "%02X ", blocks[blockNo].Data[i + j]);
//                 }

//                 ptr += sprintf((char *)ptr, "| ");
//                 for (int j = 0; j < 4; j++)
//                 {
//                     //if((block->Data[i+j] >= 65 && block->Data[i+j] <= 90) ||
//                     //   (block->Data[i+j] >= 97 && block->Data[i+j] <= 122)) {
//                     if (blocks[blockNo].Data[i + j] >= 33 && blocks[blockNo].Data[i + j] <= 126)
//                     {
//                         ptr += sprintf((char *)ptr, "%c", blocks[blockNo].Data[i + j]);
//                     }
//                     else
//                     {
//                         ptr += sprintf((char *)ptr, ".");
//                     }
//                 }
//                 ptr += sprintf((char *)ptr, " |");

//                 Log_info("%s", buffer);
//                 ptr = &buffer[0];
//             }

//             blockAddress++;
//         }

//         Log_info("****************************");
//     }
// }

// void NFC_PrintSessionRegisters(NFC_HandleTypeDef *hnfc)
// {
//     uint8_t SessionReg[8];
//     HAL_StatusTypeDef resp = 0;

//     resp |= NFC_ReadRegister(hnfc, 0xFE, 0, &SessionReg[0], 1000);
//     resp |= NFC_ReadRegister(hnfc, 0xFE, 1, &SessionReg[1], 1000);
//     resp |= NFC_ReadRegister(hnfc, 0xFE, 2, &SessionReg[2], 1000);
//     resp |= NFC_ReadRegister(hnfc, 0xFE, 3, &SessionReg[3], 1000);
//     resp |= NFC_ReadRegister(hnfc, 0xFE, 4, &SessionReg[4], 1000);
//     resp |= NFC_ReadRegister(hnfc, 0xFE, 5, &SessionReg[5], 1000);
//     resp |= NFC_ReadRegister(hnfc, 0xFE, 6, &SessionReg[6], 1000);
//     //   resp |= NFC_ReadRegister(hnfc, 0xFE, 7, &SessionReg[7], 1000);

//     if (resp == HAL_OK)
//     {
//         Log_info("--- NFC Session Registers ---");
//         Log_info("           NC_REG: 0x%02X", SessionReg[0]);
//         Log_info("  LAST_NDEF_BLOCK: 0x%02X", SessionReg[1]);
//         Log_info("SRAM_MIRROR_BLOCK: 0x%02X", SessionReg[2]);
//         Log_info("           WDT_LS: 0x%02X", SessionReg[3]);
//         Log_info("           WDT_MS: 0x%02X", SessionReg[4]);
//         Log_info("    I2C_CLOCK_STR: 0x%02X", SessionReg[5]);
//         Log_info("           NS_REG: 0x%02X", SessionReg[6]);
//         //    Log_info("              RFU: 0x%02X", SessionReg[7]);
//         Log_info("-----------------------------");
//     }
// }

// void NFC_PrintConfigRegisters(NFC_HandleTypeDef *hnfc)
// {
//     NFC_Block configBlock;

//     if (NFC_ReadBlocks(hnfc, 0x3A, &configBlock, 1, 1000) == NFC_OK)
//     {
//         Log_info("---- NFC Config Registers ----");
//         Log_info("           NC_REG: 0x%02X", configBlock.Data[0]);
//         Log_info("  LAST_NDEF_BLOCK: 0x%02X", configBlock.Data[1]);
//         Log_info("SRAM_MIRROR_BLOCK: 0x%02X", configBlock.Data[2]);
//         Log_info("           WDT_LS: 0x%02X", configBlock.Data[3]);
//         Log_info("           WDT_MS: 0x%02X", configBlock.Data[4]);
//         Log_info("    I2C_CLOCK_STR: 0x%02X", configBlock.Data[5]);
//         Log_info("         REG_LOCK: 0x%02X", configBlock.Data[6]);
//         Log_info("              RFU: 0x%02X", configBlock.Data[7]);
//         Log_info("-----------------------------");
//     }
// }
// #endif