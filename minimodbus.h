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

#ifndef MINIMODBUS_H
#define MINIMODBUS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define MINIMODBUS_BUFFER_SIZE 256

typedef enum MiniModbusError
{
    MiniModbusError_Success = 0,

    // exception codes from Modbus standard
    MiniModbusError_IllegalFunction = 0x01,
    MiniModbusError_IllegalDataAddress = 0x02,
    MiniModbusError_IllegalDataValue = 0x03,
    MiniModbusError_ServerDeviceFailure = 0x04,
    MiniModbusError_Acknowledge = 0x05,
    MiniModbusError_ServerDeviceBusy = 0x06,
    MiniModbusError_MemoryParityError = 0x08,
    MiniModbusError_GatewayPathUnavailable = 0x0a,
    MiniModbusError_GatewayTargetDeviceFailedToRespond = 0x0b,

    // application error codes
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

typedef enum MiniModbusMode
{
    MiniModbusMode_RTU = 0,
    MiniModbusMode_TCP = 1,
} MiniModbusMode_t;

typedef struct MiniModbusConfig {
    MiniModbusMode_t mode;
    uint8_t slave_address;
    void *user_data;
    int (*receive)(void *user_data, uint8_t *data, size_t length);
    int (*send)(void *user_data, const uint8_t *data, size_t length);
} MiniModbusConfig_t;

typedef struct MiniModbusContext {
    MiniModbusConfig_t config;
    size_t buffer_position;
    uint16_t current_tcp_transaction_identifier;
    uint8_t request_code;
    uint8_t response_length;
    uint8_t buffer[MINIMODBUS_BUFFER_SIZE];
} MiniModbusContext_t;

MiniModbusError_t MiniModbus_Init(MiniModbusContext_t *ctx, const MiniModbusConfig_t *config);
MiniModbusError_t MiniModbus_ReadHoldingRegister(MiniModbusContext_t *ctx, uint16_t reg, uint16_t *value);
MiniModbusError_t MiniModbus_WriteSingleRegister(MiniModbusContext_t *ctx, uint16_t reg, uint16_t value);

#ifdef __cplusplus
};

namespace minimodbus {

class MiniModbus {
    MiniModbusContext _ctx;

  public:
    MiniModbus(const MiniModbusConfig &config)
    {
        MiniModbus_Init(&_ctx, &config);
    }

    MiniModbusError ReadHoldingRegister(uint16_t reg, uint16_t *value)
    {
        return MiniModbus_ReadHoldingRegister(&_ctx, reg, value);
    }

    MiniModbusError WriteSingleRegister(uint16_t reg, uint16_t value)
    {
        return MiniModbus_WriteSingleRegister(&_ctx, reg, value);
    }
};

}; // namespace minimodbus

#endif /* __cplusplus */
#endif /* MINIMODBUS_H */

// define the following macro to add implementation
// in a single C file in the project
#ifdef MINIMODBUS_IMPLEMENTATION

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define FUNCTION_READ_HOLDING_REGISTER 0x03
#define FUNCTION_WRITE_SINGLE_REGISTER 0x06
#define ERROR_CODE_BITMASK 0x80
#define RESPONSE_HEADER_LENGTH 2

#define MODBUS_TCP_IP_PROTOCOL_IDENTIFIER 0

static uint16_t MiniModbus_Crc16Table[] = {
    0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241, 0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1,
    0xC481, 0x0440, 0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40, 0x0A00, 0xCAC1, 0xCB81, 0x0B40,
    0xC901, 0x09C0, 0x0880, 0xC841, 0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40, 0x1E00, 0xDEC1,
    0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41, 0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
    0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040, 0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1,
    0xF281, 0x3240, 0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441, 0x3C00, 0xFCC1, 0xFD81, 0x3D40,
    0xFF01, 0x3FC0, 0x3E80, 0xFE41, 0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840, 0x2800, 0xE8C1,
    0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41, 0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
    0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640, 0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0,
    0x2080, 0xE041, 0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240, 0x6600, 0xA6C1, 0xA781, 0x6740,
    0xA501, 0x65C0, 0x6480, 0xA441, 0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41, 0xAA01, 0x6AC0,
    0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840, 0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
    0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40, 0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1,
    0xB681, 0x7640, 0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041, 0x5000, 0x90C1, 0x9181, 0x5140,
    0x9301, 0x53C0, 0x5280, 0x9241, 0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440, 0x9C01, 0x5CC0,
    0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40, 0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
    0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40, 0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0,
    0x4C80, 0x8C41, 0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641, 0x8201, 0x42C0, 0x4380, 0x8341,
    0x4100, 0x81C1, 0x8081, 0x4040,
};

static uint16_t MiniModbus_Crc16(uint8_t *data, size_t length)
{
    uint16_t crc = 0xFFFF;

    while (length-- > 0) {
        crc = (crc >> 8) ^ MiniModbus_Crc16Table[(crc ^ *data++) & 0xFF];
    }

    return crc;
}

static void MiniModbus_RequestAddByte(MiniModbusContext_t *ctx, uint8_t byte)
{
    ctx->buffer[ctx->buffer_position++] = byte;
}

static void MiniModbus_RequestAddUInt16(MiniModbusContext_t *ctx, uint16_t value)
{
    MiniModbus_RequestAddByte(ctx, (value >> 8) & 0xFF);
    MiniModbus_RequestAddByte(ctx, value & 0xFF);
}

static void MiniModbus_RequestStart(MiniModbusContext_t *ctx, uint8_t function_code, uint8_t expected_response_length)
{
    ctx->buffer_position = 0;
    ctx->request_code = function_code;
    ctx->response_length = expected_response_length;

    switch (ctx->config.mode) {
    case MiniModbusMode_TCP:
        // MBAP header
        MiniModbus_RequestAddUInt16(ctx, ++ctx->current_tcp_transaction_identifier);
        MiniModbus_RequestAddUInt16(ctx, MODBUS_TCP_IP_PROTOCOL_IDENTIFIER);
        // NOTE: length populated with dummy values, will be populated later
        MiniModbus_RequestAddUInt16(ctx, 0);
        MiniModbus_RequestAddByte(ctx, ctx->config.slave_address);
        break;
    case MiniModbusMode_RTU:
        MiniModbus_RequestAddByte(ctx, ctx->config.slave_address);
        break;
    }

    MiniModbus_RequestAddByte(ctx, function_code);
}

static MiniModbusError_t MiniModbus_PacketSend(MiniModbusContext_t *ctx)
{
    uint16_t crc;
    uint16_t tcp_length = ctx->buffer_position - 6; // -6 to account for header size
    switch (ctx->config.mode) {
    case MiniModbusMode_RTU:
        crc = MiniModbus_Crc16(ctx->buffer, ctx->buffer_position);
        MiniModbus_RequestAddByte(ctx, crc & 0xFF);
        MiniModbus_RequestAddByte(ctx, (crc >> 8) & 0xFF);
        break;
    case MiniModbusMode_TCP:
        // write length for TCP/IP header
        ctx->buffer[4] = (tcp_length >> 8) & 0xFF;
        ctx->buffer[5] = tcp_length & 0xFF;
        break;
    }

    int sent = ctx->config.send(ctx->config.user_data, ctx->buffer, ctx->buffer_position);

    if (sent < 0 || (size_t)sent != ctx->buffer_position) {
        return MiniModbusError_Send;
    }

    return MiniModbusError_Success;
}

static uint8_t MiniModbus_ResponseReadByte(MiniModbusContext_t *ctx)
{
    return ctx->buffer[ctx->buffer_position++];
}

static uint16_t MiniModbus_ResponseReadUIn16(MiniModbusContext_t *ctx)
{
    uint16_t value = 0;
    value |= MiniModbus_ResponseReadByte(ctx) << 8;
    value |= MiniModbus_ResponseReadByte(ctx);

    return value;
}

static MiniModbusError_t MiniModbus_SendRequestAndWaitResponse(MiniModbusContext_t *ctx)
{
    MiniModbusError_t error = MiniModbus_PacketSend(ctx);
    if (error != MiniModbusError_Success) {
        return error;
    }

    int header_size = RESPONSE_HEADER_LENGTH;

    switch (ctx->config.mode) {
    case MiniModbusMode_RTU:
        header_size += 3; // 2 byte CRC + 1 byte slave address
        break;
    case MiniModbusMode_TCP:
        header_size += 7; // MBAP header
        break;
    }

    // read response
    int received = ctx->config.receive(ctx->config.user_data, ctx->buffer, header_size);
    if (received != header_size) {
        return MiniModbusError_Receive;
    }

    size_t total_received = received;
    size_t tcp_expected_length = 0;
    ctx->buffer_position = 0;

    switch (ctx->config.mode) {
    case MiniModbusMode_RTU:
        if (MiniModbus_ResponseReadByte(ctx) != ctx->config.slave_address) {
            return MiniModbusError_ResponseInvalidSlaveAddress;
        }
        break;
    case MiniModbusMode_TCP:
        if (MiniModbus_ResponseReadUIn16(ctx) != ctx->current_tcp_transaction_identifier) {
            return MiniModbusError_ResponseInvalidTransactionIdentifier;
        }
        if (MiniModbus_ResponseReadUIn16(ctx) != MODBUS_TCP_IP_PROTOCOL_IDENTIFIER) {
            return MiniModbusError_ResponseInvalidProtocolIdentifier;
        }
        tcp_expected_length = MiniModbus_ResponseReadUIn16(ctx);
        if (MiniModbus_ResponseReadByte(ctx) != ctx->config.slave_address) {
            return MiniModbusError_ResponseInvalidSlaveAddress;
        }
        break;
    }

    uint16_t response_code = MiniModbus_ResponseReadByte(ctx);
    uint8_t error_code = 0;

    // if not error, read rest of the response
    if ((response_code & ERROR_CODE_BITMASK) == 0) {
        received = ctx->config.receive(ctx->config.user_data, ctx->buffer + header_size, ctx->response_length - 1);

        // didn't read full response
        if (received != ctx->response_length - 1) {
            return MiniModbusError_Receive;
        }

        total_received += received;
    } else {
        error_code = MiniModbus_ResponseReadByte(ctx);
    }

    uint16_t crc;
    switch (ctx->config.mode) {
    case MiniModbusMode_RTU:
        crc = MiniModbus_Crc16(ctx->buffer, total_received - 2);
        if (ctx->buffer[total_received - 2] != (crc & 0xFF) || ctx->buffer[total_received - 1] != ((crc >> 8) & 0xFF)) {
            return MiniModbusError_InvalidCrc;
        }
        break;
    case MiniModbusMode_TCP:
        if (total_received - 6 != tcp_expected_length) {
            return MiniModbusError_ResponseInvalidLength;
        }
        break;
    }

    if ((response_code & ERROR_CODE_BITMASK) != 0) {
        return (MiniModbusError_t)error_code;
    }

    if (response_code != ctx->request_code) {
        return MiniModbusError_ResponseInvalidCode;
    }

    return MiniModbusError_Success;
}

MiniModbusError_t MiniModbus_Init(MiniModbusContext_t *ctx, const MiniModbusConfig_t *config)
{
    if (ctx == NULL || config == NULL) {
        return MiniModbusError_InvalidArgument;
    }

    memset(ctx, 0, sizeof(MiniModbusContext_t));
    memcpy(&ctx->config, config, sizeof(MiniModbusConfig_t));

    return MiniModbusError_Success;
}

MiniModbusError_t MiniModbus_ReadHoldingRegister(MiniModbusContext_t *ctx, uint16_t reg, uint16_t *value)
{
    if (ctx == NULL || value == NULL) {
        return MiniModbusError_InvalidArgument;
    }

    MiniModbus_RequestStart(ctx, FUNCTION_READ_HOLDING_REGISTER, 3);
    MiniModbus_RequestAddUInt16(ctx, reg);
    MiniModbus_RequestAddUInt16(ctx, 1);

    MiniModbusError_t error = MiniModbus_SendRequestAndWaitResponse(ctx);
    if (error != MiniModbusError_Success) {
        return error;
    }

    uint8_t response_size = MiniModbus_ResponseReadByte(ctx);
    if (response_size != 2) {
        return MiniModbusError_ResponseInvalidLength;
    }

    *value = MiniModbus_ResponseReadUIn16(ctx);

    return MiniModbusError_Success;
}

MiniModbusError_t MiniModbus_WriteSingleRegister(MiniModbusContext_t *ctx, uint16_t reg, uint16_t value)
{
    if (ctx == NULL) {
        return MiniModbusError_InvalidArgument;
    }

    MiniModbus_RequestStart(ctx, FUNCTION_WRITE_SINGLE_REGISTER, 4);
    MiniModbus_RequestAddUInt16(ctx, reg);
    MiniModbus_RequestAddUInt16(ctx, value);

    MiniModbusError_t error = MiniModbus_SendRequestAndWaitResponse(ctx);
    if (error != MiniModbusError_Success) {
        return error;
    }

    uint16_t response_reg = MiniModbus_ResponseReadUIn16(ctx);
    uint16_t response_value = MiniModbus_ResponseReadUIn16(ctx);

    if (response_reg != reg || response_value != value) {
        return MiniModbusError_ResponseInvalid;
    }

    return MiniModbusError_Success;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* MINIMODBUS_IMPLEMENTATION */
