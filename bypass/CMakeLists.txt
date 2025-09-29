set(TARGET_NAME "bypass")

find_package(PkgConfig REQUIRED)
pkg_check_modules(SPDK REQUIRED IMPORTED_TARGET spdk_nvme)

find_package(spdlog REQUIRED)

add_executable(${TARGET_NAME}
  args.cpp
  debug_utils.cpp
  eal_utils.cpp
  time_helper.cpp
  base_worker.cpp
  primary_worker.cpp
  mem_worker.cpp
  nic_worker.cpp
  main.cpp
)

include(../dpdk-tutorials.cmake)

target_compile_definitions(${TARGET_NAME} PRIVATE
  RTE_SDK=/usr/local/
  RTE_TARGET=x86_64-default-linuxapp-gcc
)

target_link_libraries(${TARGET_NAME} PUBLIC
  -lrte_eal
  -lrte_ethdev
  -lrte_mempool
  -lrte_mbuf
  -lrte_ring
)

target_link_libraries(${TARGET_NAME} PRIVATE spdlog::spdlog)

# add_custom_command(
    # TARGET ${TARGET_NAME}
    # POST_BUILD
    # COMMAND ${CMAKE_COMMAND} -E copy
    # ${CMAKE_SOURCE_DIR}/rs.sh
    # ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/rs.sh
# )

# add_custom_command(
    # TARGET ${TARGET_NAME}
    # POST_BUILD
    # COMMAND ${CMAKE_COMMAND} -E copy
    # ${CMAKE_SOURCE_DIR}/rp.sh
    # ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/rp.sh
# )

