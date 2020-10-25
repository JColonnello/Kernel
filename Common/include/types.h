#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef int64_t ssize_t;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef struct
{
    u64 rax;
    u64 rbx;
    u64 rcx;
    u64 rdx;
    u64 r8;
    u64 r9;
    u64 r10;
    u64 r11;
    u64 r12;
    u64 r13;
    u64 r14;
    u64 r15;
    u64 rdi;
    u64 rsi;
    u64 rbp;
    u64 irqNumber;
    u32 errorCode;
    long : 32;
    u64 rip;
    u16 cs;
    long : 48;
    u64 rflags;
    u64 rsp;
    u16 ss;
    long : 48;
} RegisterStatus;
