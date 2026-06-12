#pragma once
#include <__init.h>
#include "Format.h"

EXTERN_C_BEGIN

/**
 * Defines the logging levels.
 */
typedef u8 LOG_Lvl;
enum LOG_Lvls
{
    LOG_Lvl_Debug,
    LOG_Lvl_Info,
    LOG_Lvl_Warning,
    LOG_Lvl_Error,
    LOG_Lvl_Fatal,
};

/**
 * Defines a singular log entry.
 */
typedef struct
{
    LOG_Lvl  lvl;
    u8       cat[7];
    utf8str  msg;
    FMT_Args fmtArgs;
    SrcLoc   loc;
} LOG_Internal_Entry;

void LOG_Internal_AddEntry(const LOG_Internal_Entry* entry);

#define LOG_INTERNAL_ADD_ENTRY(lvl_, cat_, fmtMsg, ...) \
    do \
    { \
        LOG_Internal_Entry NO_CLASH_entry_##__LINE__; \
        NO_CLASH_entry_##__LINE__.lvl = LOG_Lvl_##lvl_; \
        static_assert(sizeof(cat_) >= 2 && sizeof(cat_) <= 8, "Category \"" cat_ "\" must be [1, 7] characters long (excluding null terminator)."); \
        NO_CLASH_entry_##__LINE__.cat[0] = (0 >= 7 - (sizeof(cat_) - 1)) ? (u8)cat_[0 - (7 - (sizeof(cat_) - 1))] : '-'; \
        NO_CLASH_entry_##__LINE__.cat[1] = (1 >= 7 - (sizeof(cat_) - 1)) ? (u8)cat_[1 - (7 - (sizeof(cat_) - 1))] : '-'; \
        NO_CLASH_entry_##__LINE__.cat[2] = (2 >= 7 - (sizeof(cat_) - 1)) ? (u8)cat_[2 - (7 - (sizeof(cat_) - 1))] : '-'; \
        NO_CLASH_entry_##__LINE__.cat[3] = (3 >= 7 - (sizeof(cat_) - 1)) ? (u8)cat_[3 - (7 - (sizeof(cat_) - 1))] : '-'; \
        NO_CLASH_entry_##__LINE__.cat[4] = (4 >= 7 - (sizeof(cat_) - 1)) ? (u8)cat_[4 - (7 - (sizeof(cat_) - 1))] : '-'; \
        NO_CLASH_entry_##__LINE__.cat[5] = (5 >= 7 - (sizeof(cat_) - 1)) ? (u8)cat_[5 - (7 - (sizeof(cat_) - 1))] : '-'; \
        NO_CLASH_entry_##__LINE__.cat[6] = (6 >= 7 - (sizeof(cat_) - 1)) ? (u8)cat_[6 - (7 - (sizeof(cat_) - 1))] : '-'; \
        NO_CLASH_entry_##__LINE__.msg = UTF8STR(fmtMsg); \
        NO_CLASH_entry_##__LINE__.fmtArgs = FMTARGS(__VA_ARGS__); \
        NO_CLASH_entry_##__LINE__.loc = SRC_LOC(); \
        LOG_Internal_AddEntry(&NO_CLASH_entry_##__LINE__); \
    } while (0)

#define LOG_Dbg(cat, fmtMsg, ...) LOG_INTERNAL_ADD_ENTRY(Debug,   #cat, fmtMsg, __VA_ARGS__)
#define LOG_Inf(cat, fmtMsg, ...) LOG_INTERNAL_ADD_ENTRY(Info,    #cat, fmtMsg, __VA_ARGS__)
#define LOG_Wrn(cat, fmtMsg, ...) LOG_INTERNAL_ADD_ENTRY(Warning, #cat, fmtMsg, __VA_ARGS__)
#define LOG_Err(cat, fmtMsg, ...) LOG_INTERNAL_ADD_ENTRY(Error,   #cat, fmtMsg, __VA_ARGS__)
#define LOG_Ftl(cat, fmtMsg, ...) LOG_INTERNAL_ADD_ENTRY(Fatal,   #cat, fmtMsg, __VA_ARGS__)

EXTERN_C_END
