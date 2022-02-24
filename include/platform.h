#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#include <cstdint>
#include <PR/R4300.h>

#ifdef __cplusplus

namespace n64 {
#endif

// static inline functions here

static inline void disableInterrupts() {
    uint32_t statusReg;
    __asm__ __volatile__("mfc0 %0, $%1" : "=r"(statusReg) : "I"(C0_SR));
    statusReg &= ~SR_IE;
    __asm__ __volatile__("mtc0 %0, $%1" : : "r"(statusReg), "I"(C0_SR));
}

static inline void enableInterrupts() {
    uint32_t statusReg;
    __asm__ __volatile__("mfc0 %0, $%1" : "=r"(statusReg) : "I"(C0_SR));
    statusReg |= SR_IE;
    __asm__ __volatile__("mtc0 %0, $%1" : : "r"(statusReg), "I"(C0_SR));
}

#ifdef __cplusplus
}
#endif

#endif
