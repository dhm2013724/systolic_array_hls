// ==============================================================
// Vivado(TM) HLS - High-Level Synthesis from C, C++ and SystemC v2019.2 (64-bit)
// Copyright 1986-2019 Xilinx, Inc. All Rights Reserved.
// ==============================================================
// CB
// 0x00 : Control signals
//        bit 0  - ap_start (Read/Write/COH)
//        bit 1  - ap_done (Read/COR)
//        bit 2  - ap_idle (Read)
//        bit 3  - ap_ready (Read)
//        bit 7  - auto_restart (Read/Write)
//        others - reserved
// 0x04 : Global Interrupt Enable Register
//        bit 0  - Global Interrupt Enable (Read/Write)
//        others - reserved
// 0x08 : IP Interrupt Enable Register (Read/Write)
//        bit 0  - Channel 0 (ap_done)
//        bit 1  - Channel 1 (ap_ready)
//        others - reserved
// 0x0c : IP Interrupt Status Register (Read/TOW)
//        bit 0  - Channel 0 (ap_done)
//        bit 1  - Channel 1 (ap_ready)
//        others - reserved
// 0x10 : Data signal of A
//        bit 31~0 - A[31:0] (Read/Write)
// 0x14 : reserved
// 0x18 : Data signal of B
//        bit 31~0 - B[31:0] (Read/Write)
// 0x1c : reserved
// 0x20 : Data signal of C
//        bit 31~0 - C[31:0] (Read/Write)
// 0x24 : reserved
// 0x28 : Data signal of M
//        bit 31~0 - M[31:0] (Read/Write)
// 0x2c : reserved
// 0x30 : Data signal of N
//        bit 31~0 - N[31:0] (Read/Write)
// 0x34 : reserved
// 0x38 : Data signal of K
//        bit 31~0 - K[31:0] (Read/Write)
// 0x3c : reserved
// (SC = Self Clear, COR = Clear on Read, TOW = Toggle on Write, COH = Clear on Handshake)

#define XMUL_CB_ADDR_AP_CTRL 0x00
#define XMUL_CB_ADDR_GIE     0x04
#define XMUL_CB_ADDR_IER     0x08
#define XMUL_CB_ADDR_ISR     0x0c
#define XMUL_CB_ADDR_A_DATA  0x10
#define XMUL_CB_BITS_A_DATA  32
#define XMUL_CB_ADDR_B_DATA  0x18
#define XMUL_CB_BITS_B_DATA  32
#define XMUL_CB_ADDR_C_DATA  0x20
#define XMUL_CB_BITS_C_DATA  32
#define XMUL_CB_ADDR_M_DATA  0x28
#define XMUL_CB_BITS_M_DATA  32
#define XMUL_CB_ADDR_N_DATA  0x30
#define XMUL_CB_BITS_N_DATA  32
#define XMUL_CB_ADDR_K_DATA  0x38
#define XMUL_CB_BITS_K_DATA  32

