cmd_messages.o = gcc -Wp,-MD,./.messages.o.d.tmp -m64 -pthread  -march=native -DRTE_MACHINE_CPUFLAG_SSE -DRTE_MACHINE_CPUFLAG_SSE2 -DRTE_MACHINE_CPUFLAG_SSE3 -DRTE_MACHINE_CPUFLAG_SSSE3 -DRTE_MACHINE_CPUFLAG_SSE4_1 -DRTE_MACHINE_CPUFLAG_SSE4_2 -DRTE_MACHINE_CPUFLAG_AES -DRTE_MACHINE_CPUFLAG_PCLMULQDQ -DRTE_MACHINE_CPUFLAG_AVX -DRTE_MACHINE_CPUFLAG_RDRAND -DRTE_MACHINE_CPUFLAG_FSGSBASE -DRTE_MACHINE_CPUFLAG_F16C -DRTE_MACHINE_CPUFLAG_AVX2 -DRTE_MACHINE_CPUFLAG_SSE -DRTE_MACHINE_CPUFLAG_SSE2 -DRTE_MACHINE_CPUFLAG_SSE3 -DRTE_MACHINE_CPUFLAG_SSSE3 -DRTE_MACHINE_CPUFLAG_SSE4_1 -DRTE_MACHINE_CPUFLAG_SSE4_2 -DRTE_MACHINE_CPUFLAG_AES -DRTE_MACHINE_CPUFLAG_PCLMULQDQ -DRTE_MACHINE_CPUFLAG_AVX -DRTE_MACHINE_CPUFLAG_RDRAND -DRTE_MACHINE_CPUFLAG_FSGSBASE -DRTE_MACHINE_CPUFLAG_F16C -DRTE_MACHINE_CPUFLAG_AVX2  -I/root/workspace/p4c/build/l2_switch_test/build/include -I/root/workspace/dpdk-16.07/x86_64-native-linuxapp-gcc/include -include /root/workspace/dpdk-16.07/x86_64-native-linuxapp-gcc/include/rte_config.h -O3 -Wall  -Wno-unused-function -Wno-unused-but-set-variable -Wno-unused-variable -g -std=gnu99 -I "/root/workspace/p4c/build/l2_switch_test//../../src/hardware_dep/dpdk/includes" -I "/root/workspace/p4c/build/l2_switch_test//../../src/hardware_dep/dpdk/ctrl_plane" -I "/root/workspace/p4c/build/l2_switch_test//../../src/hardware_dep/dpdk/data_plane" -I "/root/workspace/p4c/build/l2_switch_test//../../src/hardware_dep/shared/includes" -I "/root/workspace/p4c/build/l2_switch_test//../../src/hardware_dep/shared/ctrl_plane" -I "/root/workspace/p4c/build/l2_switch_test//../../src/hardware_dep/shared/data_plane" -I "/root/workspace/p4c/build/l2_switch_test//src_hardware_indep"   -o messages.o -c /root/workspace/p4c/build/l2_switch_test//../../src/hardware_dep/shared/ctrl_plane/messages.c 
