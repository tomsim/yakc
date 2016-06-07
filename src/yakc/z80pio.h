#pragma once
//------------------------------------------------------------------------------
/**
    @class YAKC::z80pio
    @brief incomplete Z80 PIO emulation (only what needed for KC85)
    
    TODO:
    - STROBE/READY handshake callbacks
    - trigger interrupt

    Resources & References:
        - http://www.z80.info/zip/z80piomn.pdf
        - http://www.z80.info/zip/um0081.pdf
        - MAME z80 pio emulation (https://github.com/mamedev/mame/blob/master/src/devices/machine/z80pio.cpp)
*/
#include "yakc/z80int.h"

namespace YAKC {

class z80pio {
public:
    /// port identifiers
    enum {
        A = 0,
        B = 1,
        num_ports = 2,
    };

    /// port modes
    enum : ubyte {
        mode_output = 0,
        mode_input,
        mode_bidirectional,
        mode_bitcontrol,
    };

    /// interrupt control bits
    enum : ubyte {
        intctrl_enable_int   = (1<<7),
        intctrl_and_or       = (1<<6),
        intctrl_high_low     = (1<<5),
        intctrl_mask_follows = (1<<4)
    };

    /// port expect mode (what is the next control word)
    enum : ubyte {
        expect_any = 0,
        expect_io_select = 1,
        expect_int_mask = 2,
    };

    /// port state
    struct port_t {
        ubyte output = 0;       // output register
        ubyte input = 0;        // input register
        ubyte io_select = 0;    // i/o select bits (for bit-control mode)
        ubyte mode = 0;         // 0=output, 1=input, 2=bidirection, 3=bit-control
        ubyte int_mask = 0xFF;
        ubyte int_vector = 0;
        ubyte int_control = 0;
        ubyte expect = 0;           // next expected control byte
        bool rdy = false;           // ready line active
        bool stb = false;           // strobe line active
        bool bctrl_match = false;   // bitcontrol logic equation result
    };
    port_t port[num_ports];

    /// interrupt controller
    z80int int_ctrl;

    /// callback definitions
    typedef void(*out_cb)(void* userdata, ubyte val);
    typedef ubyte(*in_cb)(void* userdata);
    typedef void(*rdy_cb)(void* userdata, bool active);

    template<typename CBTYPE> struct callback {
        callback() :
            func(0),
            userdata(nullptr)
        { };
        CBTYPE func;
        void* userdata;
    };
    callback<out_cb> out_callback[num_ports];
    callback<in_cb> in_callback[num_ports];
    callback<rdy_cb> rdy_callback[num_ports];

    /// connect out-callback (called when sending data to peripheral)
    void connect_out_cb(int port_id, void* userdata, out_cb cb);
    /// connect in-callback (called when requesting data from peripheral)
    void connect_in_cb(int port_id, void* userdata, in_cb cb);
    /// connect rdy-callback (called when ARDY/BRDY line goes high/low)
    void connect_rdy_cb(int port_id, void* userdata, rdy_cb cb);

    /// initialize the pio
    void init();
    /// reset the pio
    void reset();

    /// write control register
    void write_control(int port_id, ubyte val);
    /// read control register (same result for both ports)
    ubyte read_control();
    /// write data register
    void write_data(int port_id, ubyte data);
    /// read data register
    ubyte read_data(int port_id);
    /// strobe signal on PIO-A from peripheral
    void astb(bool active);
    /// strobe signal on PIO-B from peripheral
    void bstb(bool active);
    /// write data from peripheral into PIO
    void write(int port_id, ubyte val);

private:
    /// set a port's ready line
    void set_rdy(int port_id, bool active);
};

} // namespace YAKC
