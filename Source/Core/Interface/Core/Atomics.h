#pragma once
#include <__init.h>
EXTERN_C_BEGIN

// ATM_b32 =====================================================================

/**
 * A 32-bit boolean atomic value.
 */
typedef struct alignas(4) ATM_b32 { u8 buffer[4]; } ATM_b32;

/**
 * Atomically loads the value. Sequentially consistent.
 */
b32 ATM_LoadB32(ATM_b32* atom);

/**
 * Atomically stores a value. Sequentially consistent.
 */
void ATM_StoreB32(ATM_b32* atom, b32 value);

/**
 * Atomically replaces the value and returns the old value. Sequentially consistent.
 */
b32 ATM_XchgB32(ATM_b32* atom, b32 value);

/**
 * Atomically compares the value with `expected`; if equal, replaces it with `desired`.
 * Returns true if the exchange occurred, false otherwise.
 * Sequentially consistent.
 */
b8 ATM_CmpXchgB32(ATM_b32* atom, b32 expected, b32 desired);

// ATM_u32 =====================================================================

/**
 * A 32-bit unsigned atomic value.
 */
typedef struct alignas(4) ATM_u32 { u8 buffer[4]; } ATM_u32;

/**
 * Atomically loads the value. Sequentially consistent.
 */
u32 ATM_LoadU32(ATM_u32* atom);

/**
 * Atomically stores a value. Sequentially consistent.
 */
void ATM_StoreU32(ATM_u32* atom, u32 value);

/**
 * Atomically replaces the value and returns the old value. Sequentially consistent.
 */
u32 ATM_XchgU32(ATM_u32* atom, u32 value);

/**
 * Atomically compares the value with `*expected`; if equal, replaces it with `desired`.
 * Returns true if the exchange occurred. On failure, writes the current value into `*expected`.
 * Sequentially consistent.
 */
b8 ATM_CmpXchgU32(ATM_u32* atom, u32* expected, u32 desired);

/**
 * Atomically adds `value` and returns the value prior to the addition. Sequentially consistent.
 */
u32 ATM_FetchAddU32(ATM_u32* atom, u32 value);

/**
 * Atomically subtracts `value` and returns the value prior to the subtraction. Sequentially consistent.
 */
u32 ATM_FetchSubU32(ATM_u32* atom, u32 value);

/**
 * Atomically bitwise-ANDs `value` and returns the value prior to the operation. Sequentially consistent.
 */
u32 ATM_FetchAndU32(ATM_u32* atom, u32 value);

/**
 * Atomically bitwise-ORs `value` and returns the value prior to the operation. Sequentially consistent.
 */
u32 ATM_FetchOrU32(ATM_u32* atom, u32 value);

// ATM_u64 =====================================================================

/**
 * A 64-bit unsigned atomic value.
 */
typedef struct alignas(8) ATM_u64 { u8 buffer[8]; } ATM_u64;

/**
 * Atomically loads the value. Sequentially consistent.
 */
u64 ATM_LoadU64(ATM_u64* atom);

/**
 * Atomically stores a value. Sequentially consistent.
 */
void ATM_StoreU64(ATM_u64* atom, u64 value);

/**
 * Atomically replaces the value and returns the old value. Sequentially consistent.
 */
u64 ATM_XchgU64(ATM_u64* atom, u64 value);

/**
 * Atomically compares the value with `*expected`; if equal, replaces it with `desired`.
 * Returns true if the exchange occurred. On failure, writes the current value into `*expected`.
 * Sequentially consistent.
 */
b8 ATM_CmpXchgU64(ATM_u64* atom, u64* expected, u64 desired);

/**
 * Atomically adds `value` and returns the value prior to the addition. Sequentially consistent.
 */
u64 ATM_FetchAddU64(ATM_u64* atom, u64 value);

/**
 * Atomically subtracts `value` and returns the value prior to the subtraction. Sequentially consistent.
 */
u64 ATM_FetchSubU64(ATM_u64* atom, u64 value);

/**
 * Atomically bitwise-ANDs `value` and returns the value prior to the operation. Sequentially consistent.
 */
u64 ATM_FetchAndU64(ATM_u64* atom, u64 value);

/**
 * Atomically bitwise-ORs `value` and returns the value prior to the operation. Sequentially consistent.
 */
u64 ATM_FetchOrU64(ATM_u64* atom, u64 value);

// ATM_i32 =====================================================================

/**
 * A 32-bit signed atomic value.
 */
typedef struct alignas(4) ATM_i32 { u8 buffer[4]; } ATM_i32;

/**
 * Atomically loads the value. Sequentially consistent.
 */
i32 ATM_LoadI32(ATM_i32* atom);

/**
 * Atomically stores a value. Sequentially consistent.
 */
void ATM_StoreI32(ATM_i32* atom, i32 value);

/**
 * Atomically replaces the value and returns the old value. Sequentially consistent.
 */
i32 ATM_XchgI32(ATM_i32* atom, i32 value);

/**
 * Atomically compares the value with `*expected`; if equal, replaces it with `desired`.
 * Returns true if the exchange occurred. On failure, writes the current value into `*expected`.
 * Sequentially consistent.
 */
b8 ATM_CmpXchgI32(ATM_i32* atom, i32* expected, i32 desired);

/**
 * Atomically adds `value` and returns the value prior to the addition. Sequentially consistent.
 */
i32 ATM_FetchAddI32(ATM_i32* atom, i32 value);

/**
 * Atomically subtracts `value` and returns the value prior to the subtraction. Sequentially consistent.
 */
i32 ATM_FetchSubI32(ATM_i32* atom, i32 value);

/**
 * Atomically bitwise-ANDs `value` and returns the value prior to the operation. Sequentially consistent.
 */
i32 ATM_FetchAndI32(ATM_i32* atom, i32 value);

/**
 * Atomically bitwise-ORs `value` and returns the value prior to the operation. Sequentially consistent.
 */
i32 ATM_FetchOrI32(ATM_i32* atom, i32 value);

// ATM_i64 =====================================================================

/**
 * A 64-bit signed atomic value.
 */
typedef struct alignas(8) ATM_i64 { u8 buffer[8]; } ATM_i64;

/**
 * Atomically loads the value. Sequentially consistent.
 */
i64 ATM_LoadI64(ATM_i64* atom);

/**
 * Atomically stores a value. Sequentially consistent.
 */
void ATM_StoreI64(ATM_i64* atom, i64 value);

/**
 * Atomically replaces the value and returns the old value. Sequentially consistent.
 */
i64 ATM_XchgI64(ATM_i64* atom, i64 value);

/**
 * Atomically compares the value with `*expected`; if equal, replaces it with `desired`.
 * Returns true if the exchange occurred. On failure, writes the current value into `*expected`.
 * Sequentially consistent.
 */
b8 ATM_CmpXchgI64(ATM_i64* atom, i64* expected, i64 desired);

/**
 * Atomically adds `value` and returns the value prior to the addition. Sequentially consistent.
 */
i64 ATM_FetchAddI64(ATM_i64* atom, i64 value);

/**
 * Atomically subtracts `value` and returns the value prior to the subtraction. Sequentially consistent.
 */
i64 ATM_FetchSubI64(ATM_i64* atom, i64 value);

/**
 * Atomically bitwise-ANDs `value` and returns the value prior to the operation. Sequentially consistent.
 */
i64 ATM_FetchAndI64(ATM_i64* atom, i64 value);

/**
 * Atomically bitwise-ORs `value` and returns the value prior to the operation. Sequentially consistent.
 */
i64 ATM_FetchOrI64(ATM_i64* atom, i64 value);

EXTERN_C_END
