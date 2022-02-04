# Minimodbus

A very small footprint Modbus protocol implementation.

ModBus is a widely used protocol in the industry. I needed a simple, small footprint yet reliable library to implement
part of this protocol, and I didn't find one. They all were too complex, wanted to do too much, and required integration
with some system API.

This library implements the bare minimum that I needed: it implements only master mode and function codes 0x03 and 0x06
(codes to read/write a holding register). It is written in pure C99 and doesn't have external dependencies beside the C
standard library. It's responsibility of the caller to open the serial connection or TCP/IP socket, and receive/send
data on the chosen transport. This library implements only the protocol part.

I built this library to have a very small footprint su that it can be used on embedded microcontrollers without
problems. This library doesn't do any memory allocation, or use big call stacks.

## Usage

First, you need to call the `MiniModbus_Init(MiniModbusContext_t *ctx, const MiniModbusConfig_t *config)` function to
initialize the context with the specified config. The config object doesn't need to persist since it's copied into the
context, and can be allocated on the stack of the calling function.

**NOTE**: in the MiniModbusConfig_t object you need to pass pointers to a `send` and `receive` functions. If you look at
the signature, you realize that is made to be able to pass the POSIX `read` and `write` functions as-is.
(I think that doing so is sort of undefined behavior but on most platforms it will work fine).

Then you can call the only 2 (for now) implemented functions:

```c
MiniModbus_ReadHoldingRegister(MiniModbusContext_t *ctx, uint16_t reg, uint16_t *value);
MiniModbus_WriteSingleRegister(MiniModbusContext_t *ctx, uint16_t reg, uint16_t value);
```

**WARNING**: this library doesn't manage opening/closing the connection, and restarting it if it crashes. You need to do
that yourself: open the connection/serial port before calling init, then eventually reopen a closed connection in
the `send`/`recieve` handlers, and close it when it's not needed. The library itself doesn't need to be de-initialized
since it doesn't do any dynamic memory allocation.
