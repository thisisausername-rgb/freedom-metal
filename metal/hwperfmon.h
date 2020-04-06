/* Copyright 2020 SiFive, Inc */
/* SPDX-License-Identifier: Apache-2.0 */

#ifndef METAL__HWPERFMON_H
#define METAL__HWPERFMON_H

/*! @brief Macros for valid Event IDs */
#define METAL_HWPERFMON_EVENTID_8 (1UL << 8)
#define METAL_HWPERFMON_EVENTID_9 (1UL << 9)
#define METAL_HWPERFMON_EVENTID_10 (1UL << 10)
#define METAL_HWPERFMON_EVENTID_11 (1UL << 11)
#define METAL_HWPERFMON_EVENTID_12 (1UL << 12)
#define METAL_HWPERFMON_EVENTID_13 (1UL << 13)
#define METAL_HWPERFMON_EVENTID_14 (1UL << 14)
#define METAL_HWPERFMON_EVENTID_15 (1UL << 15)
#define METAL_HWPERFMON_EVENTID_16 (1UL << 16)
#define METAL_HWPERFMON_EVENTID_17 (1UL << 17)
#define METAL_HWPERFMON_EVENTID_18 (1UL << 18)
#define METAL_HWPERFMON_EVENTID_19 (1UL << 19)
#define METAL_HWPERFMON_EVENTID_20 (1UL << 20)
#define METAL_HWPERFMON_EVENTID_21 (1UL << 21)
#define METAL_HWPERFMON_EVENTID_22 (1UL << 22)
#define METAL_HWPERFMON_EVENTID_23 (1UL << 23)
#define METAL_HWPERFMON_EVENTID_24 (1UL << 24)
#define METAL_HWPERFMON_EVENTID_25 (1UL << 25)
#define METAL_HWPERFMON_EVENTID_26 (1UL << 26)
#define METAL_HWPERFMON_EVENTID_27 (1UL << 27)
#define METAL_HWPERFMON_EVENTID_28 (1UL << 28)
#define METAL_HWPERFMON_EVENTID_29 (1UL << 29)
#define METAL_HWPERFMON_EVENTID_30 (1UL << 30)
#define METAL_HWPERFMON_EVENTID_31 (1UL << 31)

/*! @brief Macros for valid Event Class */
#define METAL_HWPERFMON_EVENTCLASS_0 (0UL)
#define METAL_HWPERFMON_EVENTCLASS_1 (1UL)
#define METAL_HWPERFMON_EVENTCLASS_2 (2UL)
#define METAL_HWPERFMON_EVENTCLASS_3 (3UL)
#define METAL_HWPERFMON_EVENTCLASS_4 (4UL)
#define METAL_HWPERFMON_EVENTCLASS_5 (5UL)
#define METAL_HWPERFMON_EVENTCLASS_6 (6UL)
#define METAL_HWPERFMON_EVENTCLASS_7 (7UL)
#define METAL_HWPERFMON_EVENTCLASS_8 (8UL)

/*! @brief Enums for available performance counters */
typedef enum {
    METAL_HWPERFMON_CYCLE = 0,
    METAL_HWPERFMON_TIME = 1,
    METAL_HWPERFMON_INSTRET = 2,
    METAL_HWPERFMON_COUNTER_3 = 3,
    METAL_HWPERFMON_COUNTER_4 = 4,
    METAL_HWPERFMON_COUNTER_5 = 5,
    METAL_HWPERFMON_COUNTER_6 = 6,
    METAL_HWPERFMON_COUNTER_7 = 7,
    METAL_HWPERFMON_COUNTER_8 = 8,
    METAL_HWPERFMON_COUNTER_9 = 9,
    METAL_HWPERFMON_COUNTER_10 = 10,
    METAL_HWPERFMON_COUNTER_11 = 11,
    METAL_HWPERFMON_COUNTER_12 = 12,
    METAL_HWPERFMON_COUNTER_13 = 13,
    METAL_HWPERFMON_COUNTER_14 = 14,
    METAL_HWPERFMON_COUNTER_15 = 15,
    METAL_HWPERFMON_COUNTER_16 = 16,
    METAL_HWPERFMON_COUNTER_17 = 17,
    METAL_HWPERFMON_COUNTER_18 = 18,
    METAL_HWPERFMON_COUNTER_19 = 19,
    METAL_HWPERFMON_COUNTER_20 = 20,
    METAL_HWPERFMON_COUNTER_21 = 21,
    METAL_HWPERFMON_COUNTER_22 = 22,
    METAL_HWPERFMON_COUNTER_23 = 23,
    METAL_HWPERFMON_COUNTER_24 = 24,
    METAL_HWPERFMON_COUNTER_25 = 25,
    METAL_HWPERFMON_COUNTER_26 = 26,
    METAL_HWPERFMON_COUNTER_27 = 27,
    METAL_HWPERFMON_COUNTER_28 = 28,
    METAL_HWPERFMON_COUNTER_29 = 29,
    METAL_HWPERFMON_COUNTER_30 = 30,
    METAL_HWPERFMON_COUNTER_31 = 31
} metal_hwperfmon_counter;

/*! @brief Initialize hardware performance monitor counters.
 * @param None.
 * @return 0 If no error.*/
int metal_hwperfmon_enable(void);

/*! @brief Disable access to hardware performance monitor counters.
 * @param None.
 * @return 0 If no error.*/
int metal_hwperfmon_disable(void);

/*! @brief Set events which will cause the specified counter to increment.
 *         Counter will start incrementing from the moment events are set.
 * @param counter Hardware counter to be incremented by selected events.
 * @param bitmask Bit-mask to select events for a particular counter,
 *                refer core reference manual for selection of events.
 *                Event bit mask is partitioned as follows:
 *                [XLEN-1:8] - Event selection mask [7:0] - Event class
 * @return 0 If no error.*/
int metal_hwperfmon_set_event(metal_hwperfmon_counter counter,
                              unsigned int bitmask);

/*! @brief Get events selection mask set for specified counter.
 * @param counter Hardware counter.
 * @return Event selection bit mask. refer core reference manual for details.*/
unsigned int metal_hwperfmon_get_event(metal_hwperfmon_counter counter);

/*! @brief Clear event selector bits as per specified bit-mask.
 * @param counter Hardware counter.
 * @return 0 If no error.*/
int metal_hwperfmon_clr_event(metal_hwperfmon_counter counter,
                              unsigned int bitmask);

/*! @brief Enable counter access to next lower privilege mode.
 * @param counter Hardware counter.
 * @return 0 If no error.*/
int metal_hwperfmon_enable_access(metal_hwperfmon_counter counter);

/*! @brief Disable counter access to next lower privilege mode.
 * @param counter Hardware counter.
 * @return 0 If no error.*/
int metal_hwperfmon_disable_access(metal_hwperfmon_counter counter);

/*! @brief Reads current value of specified hardware counter.
 *         Note: 'mtime' register is memory mapped into CLINT block.
 *                Use CLINT APIs to access this register.
 * @param counter Hardware counter.
 * @return Current value of hardware counter on success, 0 on failure.*/
unsigned long long
metal_hwperfmon_read_counter(metal_hwperfmon_counter counter);

/*! @brief Clears off specified counter.
 * @param counter Hardware counter.
 * @return 0 If no error.*/
int metal_hwperfmon_clear_counter(metal_hwperfmon_counter counter);

#endif
