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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define MINIMODBUS_IMPLEMENTATION
#include "minimodbus.h"

int main()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(1);
    }
    struct in_addr saddr;

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(5020);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr.s_addr);

    if (connect(sockfd, (const struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("connect");
        exit(1);
    }

    MiniModbusContext_t ctx = {0};
    MiniModbusConfig_t config = {
        .mode = MiniModbusMode_TCP,
        .user_data = (void *)sockfd,
        .slave_address = 1,
        .send = write,
        .receive = read,
    };
    MiniModbus_Init(&ctx, &config);

    uint16_t reg = 0; // register are 0-based!
    uint16_t value = 0;

    // read reg
    MiniModbusError_t error = MiniModbus_ReadHoldingRegister(&ctx, reg, &value);
    printf("read response: %d\n", error);
    printf("read value: %u\n", value);

    // write 42 into reg
    error = MiniModbus_WriteSingleRegister(&ctx, reg, 42);
    printf("write response: %d\n", error);

    // read reg again: value should be 42
    error = MiniModbus_ReadHoldingRegister(&ctx, reg, &value);
    printf("read response: %d\n", error);
    printf("read value: %u\n", value);

    if (close(sockfd) < 0) {
        perror("close");
        exit(1);
    }
}
