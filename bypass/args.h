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
    uint16_t pkt_len;
    uint16_t tgtPort;
    uint16_t srcPort;
    std::string tgtIp;
    std::string srcIp;
    std::string tgtMac;
    std::string srcMac;
    std::string port_pci;
}BenchParam;

bool parse_args(int& argc, char** args, BenchParam& param);

#endif
