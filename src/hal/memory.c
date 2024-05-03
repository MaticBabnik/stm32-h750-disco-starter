#include "badhal.h"

#define SCB_SHCSR_MEMFAULTENA (1UL << 16)
#define MPU_CTRL_ENABLE 1
#define MPU_PRIVILEGED_DEFAULT 0x4

#define MPU_RASR_ATTRS_Pos 16
#define MPU_RASR_XN_Pos 28
#define MPU_RASR_AP_Pos 24
#define MPU_RASR_TEX_Pos 19
#define MPU_RASR_S_Pos 18
#define MPU_RASR_C_Pos 17
#define MPU_RASR_B_Pos 16
#define MPU_RASR_SRD_Pos 8
#define MPU_RASR_SIZE_Pos 1

#define MPU_RASR_ENABLE 1

#define MPU_REGION_SIZE_32MB 0x18
#define MPU_REGION_FULL_ACCESS 0x3

void mem_mpu_enable(u32 ctrl)
{
    // Enable MPU
    MPU->CTRL = ctrl | MPU_CTRL_ENABLE;
    // Enable fault exceptions
    SCB->SHCSR |= SCB_SHCSR_MEMFAULTENA;
    // sync data & flush instruction pipeline
    asm volatile("dsb");
    asm volatile("isb");
}

void mem_mpu_disable()
{
    // finish up all memory accesses
    asm volatile("dmb");
    // Disable fault exceptions
    SCB->SHCSR &= ~SCB_SHCSR_MEMFAULTENA;
    // Disable MPU
    MPU->CTRL = 0;
}

void mem_mpu_setup_sdram()
{
    mem_mpu_disable();

    // region 1
    MPU->RNR = 1;
    MPU->RBAR = SDRAM_BASE;
    MPU->RASR = MPU_RASR_ENABLE |
                (MPU_REGION_SIZE_32MB << MPU_RASR_SIZE_Pos) | // SDRAM is 32M
                (1 << MPU_RASR_C_Pos) |                       // cacheable
                (MPU_REGION_FULL_ACCESS << MPU_RASR_AP_Pos);  // full access

    mem_mpu_enable(MPU_PRIVILEGED_DEFAULT);
}


#define SCB_CCR_IC (1<<17)

void mem_enable_icache() {
    asm volatile("dsb");
    asm volatile("isb");
    SCB->ICIALLU = 0; // invalidate I-cache
    asm volatile("dsb");
    asm volatile("isb");
    SCB->CCR |= SCB_CCR_IC; // enable I-cache
    asm volatile("dsb");
    asm volatile("isb");
}