#include "CorePrivate.h"

static_assert(sizeof  (ATM_b32) == 4,    "ATM_b32 must be 4 bytes");
static_assert(alignof (ATM_b32) == 4,    "ATM_b32 must be 4-byte aligned");
static_assert(sizeof  (ATM_u32) == 4,    "ATM_u32 must be 4 bytes");
static_assert(alignof (ATM_u32) == 4,    "ATM_u32 must be 4-byte aligned");
static_assert(sizeof  (ATM_u64) == 8,    "ATM_u64 must be 8 bytes");
static_assert(alignof (ATM_u64) == 8,    "ATM_u64 must be 8-byte aligned");
static_assert(sizeof  (ATM_i32) == 4,    "ATM_i32 must be 4 bytes");
static_assert(alignof (ATM_i32) == 4,    "ATM_i32 must be 4-byte aligned");
static_assert(sizeof  (ATM_i64) == 8,    "ATM_i64 must be 8 bytes");
static_assert(alignof (ATM_i64) == 8,    "ATM_i64 must be 8-byte aligned");

// ATM_b32 =====================================================================

b32 ATM_LoadB32(ATM_b32* atom)
{
    #if MSR_WINDOWS
        return (b32) _InterlockedOr((LONG volatile*) atom->buffer, 0);
    #elif MSR_UNIX
        return (b32) __atomic_load_n((u32 volatile*) atom->buffer, __ATOMIC_SEQ_CST);
    #else
        #error "Unsupported platform."
    #endif
}

void ATM_StoreB32(ATM_b32* atom, b32 value)
{
    #if MSR_WINDOWS
        (void) InterlockedExchange((LONG volatile*) atom->buffer, (LONG) value);
    #elif MSR_UNIX
        __atomic_store_n((u32 volatile*) atom->buffer, (u32) value, __ATOMIC_SEQ_CST);
    #else
        #error "Unsupported platform."
    #endif
}

b32 ATM_XchgB32(ATM_b32* atom, b32 value)
{
    #if MSR_WINDOWS
        return (b32) InterlockedExchange((LONG volatile*) atom->buffer, (LONG) value);
    #elif MSR_UNIX
        return (b32) __atomic_exchange_n((u32 volatile*) atom->buffer, (u32) value, __ATOMIC_SEQ_CST);
    #else
        #error "Unsupported platform."
    #endif
}

b8 ATM_CmpXchgB32(ATM_b32* atom, b32 expected, b32 desired)
{
    #if MSR_WINDOWS
        LONG old = InterlockedCompareExchange((LONG volatile*) atom->buffer, (LONG) desired, (LONG) expected);
        return (b8) (old == (LONG) expected);
    #elif MSR_UNIX
        u32 exp = (u32) expected;
        return (b8) __atomic_compare_exchange_n((u32 volatile*) atom->buffer, &exp, (u32) desired, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    #else
        #error "Unsupported platform."
    #endif
}

// ATM_u32 =====================================================================

u32 ATM_LoadU32(ATM_u32* atom)
{
    #if MSR_WINDOWS
        return (u32) _InterlockedOr((LONG volatile*) atom->buffer, 0);
    #elif MSR_UNIX
        return __atomic_load_n((u32 volatile*) atom->buffer, __ATOMIC_SEQ_CST);
    #else
        #error "Unsupported platform."
    #endif
}

void ATM_StoreU32(ATM_u32* atom, u32 value)
{
    #if MSR_WINDOWS
        (void) InterlockedExchange((LONG volatile*) atom->buffer, (LONG) value);
    #elif MSR_UNIX
        __atomic_store_n((u32 volatile*) atom->buffer, value, __ATOMIC_SEQ_CST);
    #else
        #error "Unsupported platform."
    #endif
}

u32 ATM_XchgU32(ATM_u32* atom, u32 value)
{
    #if MSR_WINDOWS
        return (u32) InterlockedExchange((LONG volatile*) atom->buffer, (LONG) value);
    #elif MSR_UNIX
        return __atomic_exchange_n((u32 volatile*) atom->buffer, value, __ATOMIC_SEQ_CST);
    #else
        #error "Unsupported platform."
    #endif
}

b8 ATM_CmpXchgU32(ATM_u32* atom, u32* expected, u32 desired)
{
    #if MSR_WINDOWS
        LONG old = InterlockedCompareExchange((LONG volatile*) atom->buffer, (LONG) desired, (LONG) *expected);
        b8 success = (b8) (old == (LONG) *expected);
        if (!success) { *expected = (u32) old; }
        return success;
    #elif MSR_UNIX
        return (b8) __atomic_compare_exchange_n((u32 volatile*) atom->buffer, expected, desired, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    #else
        #error "Unsupported platform."
    #endif
}

u32 ATM_FetchAddU32(ATM_u32* atom, u32 value)
{
    #if MSR_WINDOWS
        return (u32) InterlockedExchangeAdd((LONG volatile*) atom->buffer, (LONG) value);
    #elif MSR_UNIX
        return __atomic_fetch_add((u32 volatile*) atom->buffer, value, __ATOMIC_SEQ_CST);
    #else
        #error "Unsupported platform."
    #endif
}

u32 ATM_FetchSubU32(ATM_u32* atom, u32 value)
{
    #if MSR_WINDOWS
        return (u32) InterlockedExchangeAdd((LONG volatile*) atom->buffer, -(LONG) value);
    #elif MSR_UNIX
        return __atomic_fetch_sub((u32 volatile*) atom->buffer, value, __ATOMIC_SEQ_CST);
    #else
        #error "Unsupported platform."
    #endif
}

u32 ATM_FetchAndU32(ATM_u32* atom, u32 value)
{
    #if MSR_WINDOWS
        return (u32) InterlockedAnd((LONG volatile*) atom->buffer, (LONG) value);
    #elif MSR_UNIX
        return __atomic_fetch_and((u32 volatile*) atom->buffer, value, __ATOMIC_SEQ_CST);
    #else
        #error "Unsupported platform."
    #endif
}

u32 ATM_FetchOrU32(ATM_u32* atom, u32 value)
{
    #if MSR_WINDOWS
        return (u32) InterlockedOr((LONG volatile*) atom->buffer, (LONG) value);
    #elif MSR_UNIX
        return __atomic_fetch_or((u32 volatile*) atom->buffer, value, __ATOMIC_SEQ_CST);
    #else
        #error "Unsupported platform."
    #endif
}

// ATM_u64 =====================================================================

u64 ATM_LoadU64(ATM_u64* atom)
{
    #if MSR_WINDOWS
        return (u64) _InterlockedOr64((LONG64 volatile*) atom->buffer, 0);
    #elif MSR_UNIX
        return __atomic_load_n((u64 volatile*) atom->buffer, __ATOMIC_SEQ_CST);
    #else
        #error "Unsupported platform."
    #endif
}

void ATM_StoreU64(ATM_u64* atom, u64 value)
{
    #if MSR_WINDOWS
        (void) InterlockedExchange64((LONG64 volatile*) atom->buffer, (LONG64) value);
    #elif MSR_UNIX
        __atomic_store_n((u64 volatile*) atom->buffer, value, __ATOMIC_SEQ_CST);
    #else
        #error "Unsupported platform."
    #endif
}

u64 ATM_XchgU64(ATM_u64* atom, u64 value)
{
    #if MSR_WINDOWS
        return (u64) InterlockedExchange64((LONG64 volatile*) atom->buffer, (LONG64) value);
    #elif MSR_UNIX
        return __atomic_exchange_n((u64 volatile*) atom->buffer, value, __ATOMIC_SEQ_CST);
    #else
        #error "Unsupported platform."
    #endif
}

b8 ATM_CmpXchgU64(ATM_u64* atom, u64* expected, u64 desired)
{
    #if MSR_WINDOWS
        LONG64 old = InterlockedCompareExchange64((LONG64 volatile*) atom->buffer, (LONG64) desired, (LONG64) *expected);
        b8 success = (b8) (old == (LONG64) *expected);
        if (!success) { *expected = (u64) old; }
        return success;
    #elif MSR_UNIX
        return (b8) __atomic_compare_exchange_n((u64 volatile*) atom->buffer, expected, desired, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    #else
        #error "Unsupported platform."
    #endif
}

u64 ATM_FetchAddU64(ATM_u64* atom, u64 value)
{
    #if MSR_WINDOWS
        return (u64) InterlockedExchangeAdd64((LONG64 volatile*) atom->buffer, (LONG64) value);
    #elif MSR_UNIX
        return __atomic_fetch_add((u64 volatile*) atom->buffer, value, __ATOMIC_SEQ_CST);
    #else
        #error "Unsupported platform."
    #endif
}

u64 ATM_FetchSubU64(ATM_u64* atom, u64 value)
{
    #if MSR_WINDOWS
        return (u64) InterlockedExchangeAdd64((LONG64 volatile*) atom->buffer, -(LONG64) value);
    #elif MSR_UNIX
        return __atomic_fetch_sub((u64 volatile*) atom->buffer, value, __ATOMIC_SEQ_CST);
    #else
        #error "Unsupported platform."
    #endif
}

u64 ATM_FetchAndU64(ATM_u64* atom, u64 value)
{
    #if MSR_WINDOWS
        return (u64) InterlockedAnd64((LONG64 volatile*) atom->buffer, (LONG64) value);
    #elif MSR_UNIX
        return __atomic_fetch_and((u64 volatile*) atom->buffer, value, __ATOMIC_SEQ_CST);
    #else
        #error "Unsupported platform."
    #endif
}

u64 ATM_FetchOrU64(ATM_u64* atom, u64 value)
{
    #if MSR_WINDOWS
        return (u64) InterlockedOr64((LONG64 volatile*) atom->buffer, (LONG64) value);
    #elif MSR_UNIX
        return __atomic_fetch_or((u64 volatile*) atom->buffer, value, __ATOMIC_SEQ_CST);
    #else
        #error "Unsupported platform."
    #endif
}

// ATM_i32 =====================================================================

i32 ATM_LoadI32(ATM_i32* atom)
{
    #if MSR_WINDOWS
        return (i32) _InterlockedOr((LONG volatile*) atom->buffer, 0);
    #elif MSR_UNIX
        return __atomic_load_n((i32 volatile*) atom->buffer, __ATOMIC_SEQ_CST);
    #else
        #error "Unsupported platform."
    #endif
}

void ATM_StoreI32(ATM_i32* atom, i32 value)
{
    #if MSR_WINDOWS
        (void) InterlockedExchange((LONG volatile*) atom->buffer, (LONG) value);
    #elif MSR_UNIX
        __atomic_store_n((i32 volatile*) atom->buffer, value, __ATOMIC_SEQ_CST);
    #else
        #error "Unsupported platform."
    #endif
}

i32 ATM_XchgI32(ATM_i32* atom, i32 value)
{
    #if MSR_WINDOWS
        return (i32) InterlockedExchange((LONG volatile*) atom->buffer, (LONG) value);
    #elif MSR_UNIX
        return __atomic_exchange_n((i32 volatile*) atom->buffer, value, __ATOMIC_SEQ_CST);
    #else
        #error "Unsupported platform."
    #endif
}

b8 ATM_CmpXchgI32(ATM_i32* atom, i32* expected, i32 desired)
{
    #if MSR_WINDOWS
        LONG old = InterlockedCompareExchange((LONG volatile*) atom->buffer, (LONG) desired, (LONG) *expected);
        b8 success = (b8) (old == (LONG) *expected);
        if (!success) { *expected = (i32) old; }
        return success;
    #elif MSR_UNIX
        return (b8) __atomic_compare_exchange_n((i32 volatile*) atom->buffer, expected, desired, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    #else
        #error "Unsupported platform."
    #endif
}

i32 ATM_FetchAddI32(ATM_i32* atom, i32 value)
{
    #if MSR_WINDOWS
        return (i32) InterlockedExchangeAdd((LONG volatile*) atom->buffer, (LONG) value);
    #elif MSR_UNIX
        return __atomic_fetch_add((i32 volatile*) atom->buffer, value, __ATOMIC_SEQ_CST);
    #else
        #error "Unsupported platform."
    #endif
}

i32 ATM_FetchSubI32(ATM_i32* atom, i32 value)
{
    #if MSR_WINDOWS
        return (i32) InterlockedExchangeAdd((LONG volatile*) atom->buffer, -(LONG) value);
    #elif MSR_UNIX
        return __atomic_fetch_sub((i32 volatile*) atom->buffer, value, __ATOMIC_SEQ_CST);
    #else
        #error "Unsupported platform."
    #endif
}

i32 ATM_FetchAndI32(ATM_i32* atom, i32 value)
{
    #if MSR_WINDOWS
        return (i32) InterlockedAnd((LONG volatile*) atom->buffer, (LONG) value);
    #elif MSR_UNIX
        return __atomic_fetch_and((i32 volatile*) atom->buffer, value, __ATOMIC_SEQ_CST);
    #else
        #error "Unsupported platform."
    #endif
}

i32 ATM_FetchOrI32(ATM_i32* atom, i32 value)
{
    #if MSR_WINDOWS
        return (i32) InterlockedOr((LONG volatile*) atom->buffer, (LONG) value);
    #elif MSR_UNIX
        return __atomic_fetch_or((i32 volatile*) atom->buffer, value, __ATOMIC_SEQ_CST);
    #else
        #error "Unsupported platform."
    #endif
}

// ATM_i64 =====================================================================

i64 ATM_LoadI64(ATM_i64* atom)
{
    #if MSR_WINDOWS
        return (i64) _InterlockedOr64((LONG64 volatile*) atom->buffer, 0);
    #elif MSR_UNIX
        return __atomic_load_n((i64 volatile*) atom->buffer, __ATOMIC_SEQ_CST);
    #else
        #error "Unsupported platform."
    #endif
}

void ATM_StoreI64(ATM_i64* atom, i64 value)
{
    #if MSR_WINDOWS
        (void) InterlockedExchange64((LONG64 volatile*) atom->buffer, (LONG64) value);
    #elif MSR_UNIX
        __atomic_store_n((i64 volatile*) atom->buffer, value, __ATOMIC_SEQ_CST);
    #else
        #error "Unsupported platform."
    #endif
}

i64 ATM_XchgI64(ATM_i64* atom, i64 value)
{
    #if MSR_WINDOWS
        return (i64) InterlockedExchange64((LONG64 volatile*) atom->buffer, (LONG64) value);
    #elif MSR_UNIX
        return __atomic_exchange_n((i64 volatile*) atom->buffer, value, __ATOMIC_SEQ_CST);
    #else
        #error "Unsupported platform."
    #endif
}

b8 ATM_CmpXchgI64(ATM_i64* atom, i64* expected, i64 desired)
{
    #if MSR_WINDOWS
        LONG64 old = InterlockedCompareExchange64((LONG64 volatile*) atom->buffer, (LONG64) desired, (LONG64) *expected);
        b8 success = (b8) (old == (LONG64) *expected);
        if (!success) { *expected = (i64) old; }
        return success;
    #elif MSR_UNIX
        return (b8) __atomic_compare_exchange_n((i64 volatile*) atom->buffer, expected, desired, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    #else
        #error "Unsupported platform."
    #endif
}

i64 ATM_FetchAddI64(ATM_i64* atom, i64 value)
{
    #if MSR_WINDOWS
        return (i64) InterlockedExchangeAdd64((LONG64 volatile*) atom->buffer, (LONG64) value);
    #elif MSR_UNIX
        return __atomic_fetch_add((i64 volatile*) atom->buffer, value, __ATOMIC_SEQ_CST);
    #else
        #error "Unsupported platform."
    #endif
}

i64 ATM_FetchSubI64(ATM_i64* atom, i64 value)
{
    #if MSR_WINDOWS
        return (i64) InterlockedExchangeAdd64((LONG64 volatile*) atom->buffer, -(LONG64) value);
    #elif MSR_UNIX
        return __atomic_fetch_sub((i64 volatile*) atom->buffer, value, __ATOMIC_SEQ_CST);
    #else
        #error "Unsupported platform."
    #endif
}

i64 ATM_FetchAndI64(ATM_i64* atom, i64 value)
{
    #if MSR_WINDOWS
        return (i64) InterlockedAnd64((LONG64 volatile*) atom->buffer, (LONG64) value);
    #elif MSR_UNIX
        return __atomic_fetch_and((i64 volatile*) atom->buffer, value, __ATOMIC_SEQ_CST);
    #else
        #error "Unsupported platform."
    #endif
}

i64 ATM_FetchOrI64(ATM_i64* atom, i64 value)
{
    #if MSR_WINDOWS
        return (i64) InterlockedOr64((LONG64 volatile*) atom->buffer, (LONG64) value);
    #elif MSR_UNIX
        return __atomic_fetch_or((i64 volatile*) atom->buffer, value, __ATOMIC_SEQ_CST);
    #else
        #error "Unsupported platform."
    #endif
}
