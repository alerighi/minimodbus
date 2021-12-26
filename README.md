# Minimodbus

A very small footprint Modbus protocol implementation.

## Motivations

I work in the IoT field, mainly with devices related to HVAC, such as heat pumps and boilers, that nowadays use the Modbus protocol.

Working with microcontrollers with limited resources, I needed a Modbus implementation with a very small footprint and that implemented
just the protocol, demanding the communication (via serial port or TCP/IP) to an higher level.

This library has the following limitatoins:

- only master mode it's implemented
- only RTU and TCP/IP communication it's implemented (not ASCII)
- only function codes `0x03` (read holding register) and `0x06` (write single register). Tough it's easy to extend and implement the other codes if you need them
- this library manages only the protocol, the sending/reciving of messages on the serial port/network and the setup of the serial port/network connection is demanded to the user

Features:

- written in standard C99
- header-only library
- no dynamic memory allcation
- very small memory footprint
- easy to extend by implementing more function codes

## Usage

As common with header only library, you must only one time define `MINIMODBUS_IMPLEMENTATION` before including the `minimodbus.h` header file.
In the rest of the project include the header normally.

You need to provide an implementation for the `recieve` and `send` function.

**NOTE**: look at the definition of these functions! They are defined so you can pass the POSIX `read` and `write` functions as-is. Of course this behaviour is undefined, but
works fine for x86 and x86_64 (and maybe also ARM). This is useful for tests though!

First, you need to call the `MiniModbus_Init(MiniModbusContext_t *ctx, const MiniModbusConfig_t *config)` function to initialize the context with the specified config.
The config object doesn't need to persist since it's copied into the context, and can be allocated on the stack of the calling function.

Then you can call the only 2 (for now) implemented functions:

- `MiniModbus_ReadHoldingRegister(MiniModbusContext_t *ctx, uint16_t reg, uint16_t *value)`
- `MiniModbus_WriteSingleRegister(MiniModbusContext_t *ctx, uint16_t reg, uint16_t value)`

**WARNING**: this library doesn't manage opening/closing the connection, and restarting it if it crashes. You need to do that yourself: open the connection/serial port
before calling init, then eventually reopen a closed connection in the `send`/`recieve` handlers, and close it when it's not needed. The library itself doesn't need to be deinitialized since it doesn't to static memory allocation.

Look at the `example.c` for an example implementation!
