/**
 * Copyright (C) The software Authors. All rights reserved.
 * File Name: bypass/args.cpp
 * Author:
 * mail:
 * Created Time: Wed Sep 24 05:36:09 2025
 * Brief:
 */
#include "args.h"
#include <sstream>
#include "debug_utils.h"

void application_usage(){
    spdlog::info("Application usage:");
    spdlog::info("------------------");
    spdlog::info("sudo ./packet-generator -l <cores_ids> -n 4 --file-prefix=packet-gen -b <port_id_to_skip> -- --output-port <output_port_id> --packets-per-second <packets_per_second>");
    spdlog::info("Example: sudo ./packet-generator -l 4-5 -n 4 --file-prefix=packet-gen -b 0000:00:08.0 -- --output-port 0000:00:09.0 --packets-per-second 30000" );
}


bool ipv4_to_bytes(const std::string& ip_str, uint8_t ip_bytes[4]) {
    std::stringstream ss(ip_str);
    std::string segment;
    int i = 0;

    while (std::getline(ss, segment, '.')) {
        if (i >= 4) return false; // 超过4段
        int value = std::stoi(segment);
        if (value < 0 || value > 255) return false; // 非法值
        ip_bytes[i++] = static_cast<uint8_t>(value);
    }

    return i == 4;
}

bool parse_args(int& argc, char** argv, BenchParam& param){
    for (uint16_t i = 0; i < argc; ++i) {
        if (strcmp(argv[i], "--output-port") == 0) {
            if ((i + 1) < argc) {
                param.port_pci= std::string(argv[i + 1]);
                spdlog::info("PCI device:{}", param.port_pci);
            } else {
                break;
            }
        }

        if (strcmp(argv[i], "--src-ip") == 0) {
            if ((i + 1) < argc) {
                if(ipv4_to_bytes(argv[i+1], param.src_ip)){
                    spdlog::info("src ip:{}",argv[i+1]);
                }else{
                    spdlog::error("Invalid src ip specified. ");
                    exit(1);
                }
            } else {
                break;
            }
        }

        if (strcmp(argv[i], "--dst-ip") == 0) {
            if ((i + 1) < argc) {
                if(ipv4_to_bytes(argv[i+1], param.dst_ip)){
                    spdlog::info("dst ip:{}",argv[i+1]);
                }else{
                    spdlog::error("Invalid src ip specified. ");
                    exit(1);
                }
            } else {
                break;
            }
        }

        // optional arguments
        if (strcmp(argv[i], "--src-port") == 0) {
            if ((i + 1) < argc) {
                try {
                    param.src_port = std::stoi(argv[i + 1]);
                }
                catch(const std::exception& e) {
                    spdlog::error("Invalid src port specified. ");
                    exit(1);
                }
                spdlog::info("src port:{}",param.src_port);
            } else {
                break;
            }
        }

        if (strcmp(argv[i], "--dst-port") == 0) {
            if ((i + 1) < argc) {
                try {
                    param.dst_port = std::stoi(argv[i + 1]);
                }
                catch(const std::exception& e) {
                    spdlog::error("Invalid dst port specified. ");
                    exit(1);
                }
                spdlog::info("dst port:{}",param.dst_port);
            } else {
                break;
            }
        }

    }

    return true;
}
