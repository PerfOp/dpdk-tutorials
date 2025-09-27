// MIT License
//
// Copyright (c) 2024 Muhammad Awais Khalid
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "primary_worker.h"
#include "mem_worker.h"
#include "nic_worker.h"
#include <spdlog/spdlog.h>
#include "args.h"

int main(int argc, char **argv) {
    // Setting up signals to catch TERM and INT signal.
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = terminate;
    sigaction(SIGTERM, &action, nullptr);
    sigaction(SIGINT, &action, nullptr);

    // 1)
    // Initializing the DPDK EAL (Environment Abstraction Layer). This is the
    // first step of a DPDK program before we call any further DPDK API. The
    // arguments passed to this programs are passed to rte_eal_init() DPDK API.
    // A user must pass DPDK EAL arguments before the application arguments. The
    // DPDK EAL arguments and application arguments must be separated by '--'.
    // For example: ./<dpdk_application> --lcores=0,1 -n 4 -- -s 1 -t 2. `--`
    // will tell the rte_eal_init() that all the DPDK EAL arguments are present
    // before this. In the above example the DPDK EAL arguments are --lcores and
    // -n. The user arguments are -s and -t. DPDK EAL argument `--lcores=0,1`
    // means that there are two logical cores (CPUs) assigned to this DPDK
    // application. The first logical core is 0 and second is 1. DPDK
    // application will launch total_logical_cores worker threads in the
    // application (including main) So in the above example, the DPDK
    // application has two logical cores (0,1). The main function will run on
    // first logical core (0) and an additional worker thread will be launched
    // on the next logical core (1). A DPDK application sets the affinity of
    // execution threads to specific logical cores to achieve performance.
    // DPDK EAL argument `-n 4` means that this DPDK application uses 4 memory
    // channels. The details are DPDK EAL arguments is present at:
    // https://doc.dpdk.org/guides/linux_gsg/linux_eal_parameters.html
    int32_t return_val = rte_eal_init(argc, argv);
    if (return_val < 0) {
        spdlog::error("EAL: Unable to initialize DPDK EAL. Error code: {}",rte_errno);
        exit(1);
    }

    // rte_eal_init() DPDK API will return the number of DPDK EAL arguments
    // processed. So we will subtract the number of DPDK EAL arguments from the
    // total arguments and point argv to the first user argument. For example:
    // ./<dpdk_application> --lcores=0 -n 4 -- -s 1 -t 2 rte_eal_init() will
    // return 4. The total arguments passed to this program is 9. So after
    // subtracting the actual user arguments is (9 - 4 = 5). Setting `argv` to
    // point to the start of user argument which is `--`
    argc -= return_val;
    argv += return_val;

    if (argc < 2) {
        spdlog::error("Ring buffer name not provided in command line arguments.");
        rte_eal_cleanup();
        exit(1);
    }

    BenchParam benchParam;
    parse_args(argc, argv, benchParam);
    // Detecting the logical cores (CPUs) ids passed to this DPDK application.
    uint16_t i = 0;
    std::vector<uint16_t> logicalCores;
    log_info("bypass:", "Logical cores ids (CPU ids): ");
    RTE_LCORE_FOREACH(i) {
        logicalCores.push_back(i);
        log_info(" ", "%d", i);
    }
    log_info("", "\n");

    // We must have atleast one logical cores passed as an argument to this DPDK
    // application.
    if (logicalCores.size() != 1) {
        spdlog::error("EAL:One logical core is required to run this DPDK application.");
        rte_eal_cleanup();
        exit(1);
    }


    // Find the process type of current process. primary/secondary.
    const rte_proc_type_t proc_type = rte_eal_process_type();

    if (proc_type == RTE_PROC_PRIMARY) {
        PrimaryProcess primaryProcess;
        primaryProcess.InitPrimaryResource(benchParam);

        // Start packet generation routine.
        primaryProcess.MainLoop();
        using namespace std::literals;
        std::this_thread::sleep_for(500ms);
    } else if (proc_type == RTE_PROC_SECONDARY) {
        /*
        MemProcess memProcess;
        memProcess.InitMemResource();

        // Start receiving and processing the packets.
        memProcess.MainLoop();
        */
        NicProcess nicProcess;
        nicProcess.InitNicResource();

        // Start receiving and processing the packets.
        nicProcess.MainLoop();
    }

    rte_eal_cleanup();
    return 0;
}
