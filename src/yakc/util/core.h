#pragma once
//------------------------------------------------------------------------------
/**
    @file yakc/core.h
    @brief yakc core definitions
*/
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#if __clang_analyzer__
    #include <assert.h>
    #define YAKC_ASSERT(cond) assert(cond)
#else
    #if defined(_DEBUG)
        #if !(__GNUC__ || __GNUC__)
            // on Visual Studio, replace __PRETTY_FUNCTION__ with __FUNCSIG__
            #define __PRETTY_FUNCTION__ __FUNCSIG__
        #endif
        #define YAKC_ASSERT(cond) do { if(!(cond)) { YAKC::func.assertmsg_func(#cond,nullptr,__FILE__,__LINE__,__PRETTY_FUNCTION__); abort(); } } while(0)
    #else
        #define YAKC_ASSERT(cond)
    #endif
#endif

namespace YAKC {

static const int SOUND_SAMPLE_RATE = 44100;

struct ext_funcs {
    void (*assertmsg_func)(const char* cond, const char* msg, const char* file, int line, const char* func) = nullptr;
    void* (*malloc_func)(size_t) = nullptr;
    void (*free_func)(void*) = nullptr;
};

// biggest host texture resolution needed by any system (see breadboard.h)
static const int global_max_fb_width = 1024;
static const int global_max_fb_height = 312;

/// jump table for externally provided functions
extern struct ext_funcs func;
/// helper to clear a chunk of memory
extern void clear(void* ptr, int num_bytes);
/// helper to fill a chunk of memory with random noise
extern void fill_random(void* ptr, int num_bytes);

#define YAKC_MALLOC(s) func.malloc_func(s)
#define YAKC_FREE(p) func.free_func(p)

enum class system {
    kc85_2          = (1<<0),
    kc85_3          = (1<<1),
    kc85_4          = (1<<2),
    z1013_01        = (1<<3),
    z1013_16        = (1<<5),
    z1013_64        = (1<<6),
    z9001           = (1<<7),
    kc87            = (1<<8),
    zxspectrum48k   = (1<<9),
    zxspectrum128k  = (1<<10),
    cpc464          = (1<<11),
    cpc6128         = (1<<12),
    kccompact       = (1<<13),
    acorn_atom      = (1<<15),
    c64_pal         = (1<<16),
    c64_ntsc        = (1<<17),
    none = 0,
    any_kc85        = (kc85_2|kc85_3|kc85_4),
    any_z1013       = (z1013_01|z1013_16|z1013_64),
    any_z9001       = (z9001|kc87),
    any_zx          = (zxspectrum48k|zxspectrum128k),
    any_cpc         = (cpc464|cpc6128|kccompact),
    any_c64         = (c64_pal|c64_ntsc),
    any = 0x7FFFFFFF
};


enum class cpu_model {
    z80,
    m6502,
};

enum class os_rom {
    caos_hc900,
    caos_2_2,
    caos_3_1,
    caos_3_4,
    caos_4_2,
    z1013_mon202,
    z1013_mon_a2,
    z9001_os_1_2,
    kc87_os_2,
    amstrad_zx48k,
    amstrad_zx128k,
    cpc464_os,
    cpc464_basic,
    cpc6128,
    cpc6128_basic,
    kcc_os,
    kcc_basic,
    none,
};

namespace chip {
    enum id {
        z80 = (1<<0),
        z80pio = (1<<1),
        z80pio_2 = (1<<2),
        z80ctc = (1<<3),
        ay38910 = (1<<4),
        m6502 = (1<<5),
        m6522 = (1<<6),
        i8255 = (1<<7),
        mc6847 = (1<<8),
        mc6845 = (1<<9),
        m6526 = (1<<10),    // C64 CIA
        m6526_2 = (1<<11),
        m6569 = (1<<12),    // C64 VIC-II
        m6581 = (1<<13)     // C64 SID
    };
    typedef uint32_t mask;
};

extern system system_from_string(const char* str);
extern const char* string_from_system(system sys);
extern os_rom os_from_string(const char* str);

class joystick {
public:
    enum {
        left = (1<<0),
        right = (1<<1),
        up = (1<<2),
        down = (1<<3),
        btn0 = (1<<4),
        btn1 = (1<<5)
    };
};

} // namespace YAKC
