/* Copyright 2020 SiFive, Inc */
/* SPDX-License-Identifier: Apache-2.0 */

#include <metal/hwperfmon.h>
#include <stdint.h>

/* Macro to generate code within a switch case */
#define METAL_HWPERFMON_HANDLE_SWITCH(m)                                       \
    m(3) m(4) m(5) m(6) m(7) m(8) m(9) m(10) m(11) m(12) m(13) m(14) m(15)     \
        m(16) m(17) m(18) m(19) m(20) m(21) m(22) m(23) m(24) m(25) m(26)      \
            m(27) m(28) m(29) m(30) m(31)

/* Macro to set values into event selector register */
#define METAL_HWPERFMON_SET_EVENT_REG(x)                                       \
    case METAL_HWPERFMON_COUNTER_##x:                                          \
        __asm__ __volatile__("csrr %0, mhpmevent" #x : "=r"(val));             \
        val &= ~bitmask;                                                       \
        val |= bitmask;                                                        \
        __asm__ __volatile__("csrw mhpmevent" #x ", %0" : : "r"(val));         \
        break;

/* Macro to set values into event selector register */
#define METAL_HWPERFMON_CLR_EVENT_REG(x)                                       \
    case METAL_HWPERFMON_COUNTER_##x:                                          \
        __asm__ __volatile__("csrr %0, mhpmevent" #x : "=r"(val));             \
        val &= ~bitmask;                                                       \
        __asm__ __volatile__("csrw mhpmevent" #x ", %0" : : "r"(val));         \
        break;

/* Macro to get values from event selector register */
#define METAL_HWPERFMON_GET_EVENT_REG(x)                                       \
    case METAL_HWPERFMON_COUNTER_##x:                                          \
        __asm__ __volatile__("csrr %0, mhpmevent" #x : "=r"(val));             \
        break;

/* Macro to read HW performance monitor counter values */
#if __riscv_xlen == 32
#define METAL_HWPERFMON_GET_COUNT_REG(x)                                       \
    case METAL_HWPERFMON_COUNTER_##x:                                          \
        __asm__ __volatile__("csrr %0, mhpmcounter" #x "h" : "=r"(vh));        \
        __asm__ __volatile__("csrr %0, mhpmcounter" #x : "=r"(vl));            \
        break;
#else
#define METAL_HWPERFMON_GET_COUNT_REG(x)                                       \
    case METAL_HWPERFMON_COUNTER_##x:                                          \
        __asm__ __volatile__("csrr %0, mhpmcounter" #x : "=r"(vl));            \
        break;
#endif

/* Macro to clear HW performance monitor counter values */
#if __riscv_xlen == 32
#define METAL_HWPERFMON_CLR_COUNT_REG(x)                                       \
    case METAL_HWPERFMON_COUNTER_##x:                                          \
        __asm__ __volatile__("csrw mhpmcounter" #x "h, zero");                 \
        __asm__ __volatile__("csrw mhpmcounter" #x ", zero");                  \
        break;
#else
#define METAL_HWPERFMON_CLR_COUNT_REG(x)                                       \
    case METAL_HWPERFMON_COUNTER_##x:                                          \
        __asm__ __volatile__("csrw mhpmcounter" #x ", zero");                  \
        break;
#endif

/* Macro to check for instruction trap */
#define MCAUSE_ILLEGAL_INST 0x02

/* Return codes */
#define METAL_HWPERFMON_RET_OK 0
#define METAL_HWPERFMON_RET_NOK 1

/* Module enable flag */
static int hwperfmon_enable = 0;

/* Available hardware counters */
static metal_hwperfmon_counter hwperfmon_count = 0;

/* Atomic access flag, place it into ITIM. */
static int flag __attribute__((aligned(4))) __attribute__((section(".itim"))) =
    0;

__attribute__((naked)) static int metal_hwperfmon_cas(int *ptr, int old,
                                                      int new) {
    /* Perform atomic compare and swap */
#ifdef __riscv_atomic
    __asm__ __volatile__("amoswap.w.aq t0, t0, (a0) \n\t"
                         "bne t0, a1, fail # Doesn’t match, so fail. \n\t"
                         "amoswap.w.rl a2, a2, (a0) \n\t"
                         "li a0, 0 # Set return to success. \n\t"
                         "jr ra # Return. \n\t"
                         "fail: \n\t"
                         "amoswap.w.rl t0, t0, (a0) \n\t"
                         "li a0, 1 # Set return to failure. \n\t"
                         "jr ra # Return. \n\t");
#endif
    /* Return error, if no support */
    __asm__ __volatile__("li a0, 1 \n\t jr ra \n\t");
    return 1;
}

int metal_hwperfmon_enable(void) {
    /* Make sure only a single hart executes this code */
    if (metal_hwperfmon_cas(&flag, 0, 1) == METAL_HWPERFMON_RET_NOK) {
        return METAL_HWPERFMON_RET_NOK;
    }
    /* Check enable flag */
    if (hwperfmon_enable == 0) {
        unsigned int temp, r1 = 0, r2 = 0;

        /* Clear 'Illegal instruction' bit in mcause */
        __asm__ __volatile__("csrr %0, mcause" : "=r"(temp));
        temp &= -3;
        __asm__ __volatile__("csrw mcause, %0" : : "r"(temp));

        /* Write and read back all 1s to check number of available counters */
        /* Note that we have setup mtvec to avoid being caught by a trap handler
         * since 'csrw mcounteren, %2' can lead to trap on some old Risc-V cores
         */
        temp = -1;
        __asm__ __volatile__("csrr %0, mtvec \n\t"
                             "la %1, 1f \n\t"
                             "csrw mtvec, %1 \n\t"
                             "csrw mcounteren, %2 \n\t"
                             "nop\n\t"
                             "1: \n\t"
                             "csrw mtvec, %0 \n\t"
                             : "+r"(r1), "+r"(r2)
                             : "r"(temp));

        __asm__ __volatile__("csrr %0, mcause" : "=r"(temp));

        /* Check if there was any Illegal instruction trap */
        if ((temp & MCAUSE_ILLEGAL_INST) == MCAUSE_ILLEGAL_INST)
            return METAL_HWPERFMON_RET_NOK;

        /* Read value from counter enable register */
        __asm__ __volatile__("csrr %0, mcounteren" : "=r"(temp));

        /* Count number of available hardware performance counters */
        while ((temp & 0x01)) {
            hwperfmon_count++;
            temp >>= 1;
        }
        /* Set enable register to zero */
        __asm__ __volatile__("csrw mcounteren, zero");

        /* TODO: mcountinhibit csr is not yet accessible.
         * As per latest RiscV privileged spec v1.11,
         * mcountinhibit controls which of the counters increment.
         * Unused counters can be disabled to reduce power consumption. */
        /* Keep all counters disabled, enable them later on as needed. */
        /* __asm__ __volatile__("csrw mcountinhibit, zero"); */

        /* Clear all counters */
        for (unsigned int i = 0; i < hwperfmon_count; i++) {
            metal_hwperfmon_clr_event(i, 0xFFFFFFFF);
            metal_hwperfmon_clear_counter(i);
        }
        hwperfmon_enable++;
    } else {
        return METAL_HWPERFMON_RET_NOK;
    }

    return METAL_HWPERFMON_RET_OK;
}

int metal_hwperfmon_disable(void) {
    /* Make sure only a single hart executes this code */
    if (metal_hwperfmon_cas(&flag, 1, 0) == METAL_HWPERFMON_RET_NOK) {
        return METAL_HWPERFMON_RET_NOK;
    }
    /* Check enable flag */
    if (hwperfmon_enable) {
        /* Clear registers */
        uint32_t temp = 0;
        __asm__ __volatile__("csrw mcounteren, %0" : : "r"(temp));

        hwperfmon_count = 0;
        hwperfmon_enable = 0;
    } else {
        return METAL_HWPERFMON_RET_NOK;
    }

    return METAL_HWPERFMON_RET_OK;
}

int metal_hwperfmon_set_event(metal_hwperfmon_counter counter,
                              unsigned int bitmask) {
    unsigned int val;

    /* Return error if module isn't enabled or counter is out of range */
    if ((hwperfmon_enable == 0) || (counter >= hwperfmon_count))
        return METAL_HWPERFMON_RET_NOK;

    switch (counter) {
        /* Set event register bit mask as requested */
        METAL_HWPERFMON_HANDLE_SWITCH(METAL_HWPERFMON_SET_EVENT_REG)

    default:
        break;
    }

    return METAL_HWPERFMON_RET_OK;
}

unsigned int metal_hwperfmon_get_event(metal_hwperfmon_counter counter) {
    unsigned int val = 0;

    /* Return error if module isn't enabled or counter is out of range */
    if ((hwperfmon_enable == 0) || (counter >= hwperfmon_count))
        return METAL_HWPERFMON_RET_NOK;

    switch (counter) {
        /* Read event registers */
        METAL_HWPERFMON_HANDLE_SWITCH(METAL_HWPERFMON_GET_EVENT_REG)

    default:
        break;
    }

    return val;
}

int metal_hwperfmon_clr_event(metal_hwperfmon_counter counter,
                              unsigned int bitmask) {
    unsigned int val;

    /* Return error if module isn't enabled or counter is out of range */
    if ((hwperfmon_enable == 0) || (counter >= hwperfmon_count))
        return METAL_HWPERFMON_RET_NOK;

    switch (counter) {
        /* Clear event registers as requested */
        METAL_HWPERFMON_HANDLE_SWITCH(METAL_HWPERFMON_CLR_EVENT_REG)

    default:
        break;
    }

    return METAL_HWPERFMON_RET_OK;
}

int metal_hwperfmon_enable_access(metal_hwperfmon_counter counter) {
    unsigned int val;

    /* Return error if module isn't enabled or counter is out of range */
    if ((hwperfmon_enable == 0) || (counter >= hwperfmon_count))
        return METAL_HWPERFMON_RET_NOK;

    __asm__ __volatile__("csrr %0, mcounteren" : "=r"(val));

    val |= (unsigned int)(1 << counter);

    __asm__ __volatile__("csrw mcounteren"
                         ", %0"
                         :
                         : "r"(val));

    return METAL_HWPERFMON_RET_OK;
}

int metal_hwperfmon_disable_access(metal_hwperfmon_counter counter) {
    unsigned int val;

    /* Return error if module isn't enabled or counter is out of range */
    if ((hwperfmon_enable == 0) || (counter >= hwperfmon_count))
        return METAL_HWPERFMON_RET_NOK;

    __asm__ __volatile__("csrr %0, mcounteren" : "=r"(val));

    val &= (unsigned int)~(1 << counter);

    __asm__ __volatile__("csrw mcounteren"
                         ", %0"
                         :
                         : "r"(val));

    return METAL_HWPERFMON_RET_OK;
}

unsigned long long
metal_hwperfmon_read_counter(metal_hwperfmon_counter counter) {
#if __riscv_xlen == 32
    unsigned int vh = 0, vl = 0;
#else
    unsigned long long vl = 0;
#endif

    /* Return error if module isn't enabled or counter is out of range */
    if ((hwperfmon_enable == 0) || (counter >= hwperfmon_count))
        return METAL_HWPERFMON_RET_NOK;

    switch (counter) {
    case METAL_HWPERFMON_CYCLE:
#if __riscv_xlen == 32
        __asm__ __volatile__("csrr %0, mcycleh" : "=r"(vh));
#endif
        __asm__ __volatile__("csrr %0, mcycle" : "=r"(vl));
        break;

    case METAL_HWPERFMON_TIME:
        /* mtime is memory mapped within CLINT block,
         * Use CLINT APIs to access this register. */
        return METAL_HWPERFMON_RET_NOK;
        break;

    case METAL_HWPERFMON_INSTRET:
#if __riscv_xlen == 32
        __asm__ __volatile__("csrr %0, minstreth" : "=r"(vh));
#endif
        __asm__ __volatile__("csrr %0, minstret" : "=r"(vl));
        break;

        METAL_HWPERFMON_HANDLE_SWITCH(METAL_HWPERFMON_GET_COUNT_REG)

    default:
        break;
    }

#if __riscv_xlen == 32
    return ((((unsigned long long)vh) << 32) | vl);
#else
    return vl;
#endif
}

int metal_hwperfmon_clear_counter(metal_hwperfmon_counter counter) {
    /* Return error if module isn't enabled or counter is out of range */
    if ((hwperfmon_enable == 0) || (counter >= hwperfmon_count))
        return METAL_HWPERFMON_RET_NOK;

    switch (counter) {
    case METAL_HWPERFMON_CYCLE:
#if __riscv_xlen == 32
        __asm__ __volatile__("csrw mcycleh, zero");
#endif
        __asm__ __volatile__("csrw mcycle, zero");
        break;
    case METAL_HWPERFMON_TIME:
        /* mtime is memory mapped within CLINT block */
        return METAL_HWPERFMON_RET_NOK;
        break;
    case METAL_HWPERFMON_INSTRET:
#if __riscv_xlen == 32
        __asm__ __volatile__("csrw minstreth, zero");
#endif
        __asm__ __volatile__("csrw minstret, zero");
        break;

        METAL_HWPERFMON_HANDLE_SWITCH(METAL_HWPERFMON_CLR_COUNT_REG)

    default:
        break;
    }

    return METAL_HWPERFMON_RET_OK;
}
