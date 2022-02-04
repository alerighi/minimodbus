/*
 * MiniModbus v1.0.0
 * Minimal implementation of the Modbus protocol.
 *
 * Copyright (c) 2021 Alessandro Righi <alessandro.righi@alerighi.it>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * @file minimodbus.h
 * @brief a small library that implements a small but yet useful part of the Modbus protocol
 * @author Alessandro Righi
 * @copyright 2021-2022
 */

#ifndef MINI_MODBUS_H
#define MINI_MODBUS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stddef.h>
#include <stdint.h>

#define MINI_MODBUS_BUFFER_SIZE 256

/**
 * MiniModbus error code.
 * Error codes > 0 are errors from the slave and use the same code defined in the Modbus standard.
 * Error codes < 0 are errors from the MiniModbus library.
 */
typedef enum MiniModbusError {
    MiniModbusError_Success = 0,

    MiniModbusError_IllegalFunction = 0x01,
    MiniModbusError_IllegalDataAddress = 0x02,
    MiniModbusError_IllegalDataValue = 0x03,
    MiniModbusError_ServerDeviceFailure = 0x04,
    MiniModbusError_Acknowledge = 0x05,
    MiniModbusError_ServerDeviceBusy = 0x06,
    MiniModbusError_MemoryParityError = 0x08,
    MiniModbusError_GatewayPathUnavailable = 0x0a,
    MiniModbusError_GatewayTargetDeviceFailedToRespond = 0x0b,

    MiniModbusError_Generic = -1,
    MiniModbusError_InvalidArgument = -2,
    MiniModbusError_Send = -3,
    MiniModbusError_Receive = -4,
    MiniModbusError_InvalidCrc = -5,
    MiniModbusError_ResponseInvalidSlaveAddress = -6,
    MiniModbusError_ResponseUnexpectedErrorCode = -7,
    MiniModbusError_ResponseUnexpectedLength = -8,
    MiniModbusError_ResponseInvalidCode = -9,
    MiniModbusError_ResponseInvalid = -10,
    MiniModbusError_ResponseInvalidTransactionIdentifier = -11,
    MiniModbusError_ResponseInvalidProtocolIdentifier = -12,
    MiniModbusError_ResponseInvalidLength = -13,
} MiniModbusError_t;

/**
 * Modbus mode of operation
 */
typedef enum MiniModbusMode {
    /**
     * RTU mode
     */
    MiniModbusMode_RTU = 0,

    /**
     * TCP/IP mode
     */
    MiniModbusMode_TCP = 1,
} MiniModbusMode_t;

/**
 * MiniModbus configuration structure
 */
typedef struct MiniModbusConfig {
    /**
     * mode of operation (TCP/IP or RTU)
     */
    MiniModbusMode_t mode;

    /**
     * slave address to use
     */
    uint8_t slave_address;

    /**
     * a pointer to data that may be used in the receive/send implementation
     */
    void *user_data;

    /**
     * function to receive data from the serial port or TCP/IP socket.
     * This function should block till the specified amount of data is received,
     * or an error occurred.
     *
     * @param user_data pointer to the custom user_data if specified in config
     * @param data pointer to a buffer where to store the received data
     * @param length number of bytes to receive
     * @return a value < 0 in case of error, otherwise number of bytes received
     *        (that should always be the same of length)
     */
    int (*receive)(void *user_data, void *data, size_t length);

    /**
     * function to send data to the serial port or TCP/IP socket.
     * This function should block till the specified amount of data is written,
     * or an error occurred.
     *
     * @param user_data pointer to the custom user_data if specified in config
     * @param data pointer to the buffer of the data to send
     * @param length number of bytes to send
     * @return an value < 0 in case of an error, otherwise the number of bytes sent
     *         (that should always be the same of length)
     */
    int (*send)(void *user_data, const void *data, size_t length);
} MiniModbusConfig_t;

/**
 * Context of the MiniModbus library. Opaque structure.
 */
typedef struct MiniModbusContext {
    MiniModbusConfig_t config;
    size_t buffer_position;
    uint16_t current_tcp_transaction_identifier;
    uint8_t request_code;
    uint8_t response_length;
    uint8_t buffer[MINI_MODBUS_BUFFER_SIZE];
} MiniModbusContext_t;

/**
 * Initialize the Modbus protocol library
 *
 * @param ctx Modbus context to initialize
 * @param config Configuration object. Can be a temporary object
 * @return MiniModbus_InvalidArgument in case one of the parameters is NULL, otherwise MiniModbus_Success
 */
MiniModbusError_t MiniModbus_Init(MiniModbusContext_t *ctx, const MiniModbusConfig_t *config);

/**
 * Read an holding register (function code 0x03) from the device.
 *
 * @param ctx Modbus context
 * @param reg register to read (zero based)
 * @param value pointer where to store read data
 * @return MiniModbus_Success in case of success, otherwise appropriate error code
 */
MiniModbusError_t MiniModbus_ReadHoldingRegister(MiniModbusContext_t *ctx, uint16_t reg, uint16_t *value);

/**
 * Write an holding register (function code 0x06) on the device.
 *
 * @param ctx Modbus context
 * @param reg register to write (zero based)
 * @param value value to write
 * @return MiniModbus_Success in case of success, otherwise appropriate error code
 */
MiniModbusError_t MiniModbus_WriteSingleRegister(MiniModbusContext_t *ctx, uint16_t reg, uint16_t value);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MINI_MODBUS_H */
