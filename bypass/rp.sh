#!/bin/bash
##############################################################
# File Name: rs.sh
# Author:
# mail:
# Created Time: Sat Sep 20 07:31:44 2025
##############################################################
sudo ./bin/bypass -c 0x1 -n 4 --proc-type=primary --file-prefix=bypass -- --nic-pci "c75f:00:02.0" --src-ip 10.2.1.118 --dst-ip 10.2.1.116
#sudo ../bin/bypass --lcores=0@0 -n 4 --proc-type=primary --file-prefix=bypass -- ring_buffer_1
