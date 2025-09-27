/**
 * Copyright (C) The software Authors. All rights reserved.
 * File Name: bypass/args.h
 * Author:
 * mail:
 * Created Time: Wed Sep 24 05:36:09 2025
 * Brief:
 */
#ifndef ARGS_H
#define ARGS_H

#include <string>

typedef struct sBenchParam{
    sBenchParam(){
        dst_mac[0]=0x12;
        dst_mac[1]=0x34;
        dst_mac[2]=0x56;
        dst_mac[3]=0x78;
        dst_mac[4]=0x9A;
        dst_mac[5]=0xBC;

        src_mac[0]=0x12;
        src_mac[1]=0x34;
        src_mac[2]=0x56;
        src_mac[3]=0x78;
        src_mac[4]=0x9A;
        src_mac[5]=0xBC;

        src_port=1234;
        dst_port=4321;
        payload_len=1024;
    };
    uint16_t port_id;
    uint16_t payload_len;
    uint16_t dst_port;
    uint16_t src_port;
    uint8_t dst_ip[4];
    uint8_t src_ip[4];
    uint8_t dst_mac[6];
    uint8_t src_mac[6];
    std::string port_pci;
}BenchParam;

bool parse_args(int& argc, char** argv, BenchParam& param);

#endif
