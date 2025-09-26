#!/bin/bash
##############################################################
# File Name: rs.sh
# Author:
# mail:
# Created Time: Sat Sep 20 07:31:44 2025
##############################################################
sudo ./bin/bypass -c 0x2 -n 4 -a c75f:00:02.0 --proc-type=secondary --file-prefix=bypass -- ring_buffer_1
# sudo ../bin/bypass --lcores=0@0 -n 4 --proc-type=secondary --file-prefix=bypass -- ring_buffer_1
