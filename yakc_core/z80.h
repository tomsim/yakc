#pragma once
//------------------------------------------------------------------------------
/**
    @class yakc::z80
    @brief the Z80 (or rather U880) cpu
*/
#include "yakc_core/common.h"
#include "yakc_core/memory.h"
#include "yakc_core/z80int.h"

namespace yakc {

class z80 {
public:

    /// flag bits
    enum flag {
        CF = (1<<0),        // carry flag
        NF = (1<<1),        // add/subtract
        VF = (1<<2),        // parity/overflow
        PF = VF,            // parity/overflow
        XF = (1<<3),        // undocumented bit 3
        HF = (1<<4),        // half-carry
        YF = (1<<5),        // undocumented bit 5
        ZF = (1<<6),        // zero flag
        SF = (1<<7),        // sign flag
    };
    /// register enum for set_reg/get_reg methods
    enum reg {
        rA=0, rF, rB, rC, rD, rE, rH, rL,
        rAF, rBC, rDE, rHL,
        rAF_,rBC_, rDE_, rHL_,
        rI, rR, rIX, rIY, rSP, rPC,
        rIM,
        num
    };

    /// the cpu state
    struct cpu_state {
        /// main register set
        union {
            struct { ubyte F, A; };
            uword AF;
        };
        union {
            struct { ubyte C, B; };
            uword BC;
        };
        union {
            struct { ubyte E, D; };
            uword DE;
        };
        union {
            struct { ubyte L, H; };
            uword HL;
        };

        /// shadow register set
        uword AF_;
        uword BC_;
        uword DE_;
        uword HL_;

        /// special registers
        ubyte I;
        ubyte R;
        union {
            struct { ubyte IXL, IXH; };
            uword IX;
        };
        union {
            struct { ubyte IYL, IYH; };
            uword IY;
        };
        uword SP;
        uword PC;

        /// CPU is in HALT state
        bool HALT;
        /// the interrupt-enable flip-flops
        bool IFF1;
        bool IFF2;
        /// invalid or unknown instruction has been encountered
        bool INV;
        /// the interrupt mode (0, 1 or 2)
        ubyte IM;
        /// current T cycle counter
        unsigned int T;
    } state;

    /// memory map
    memory mem;
    /// size of PC history ringbuffer (must be 2^N!)
    static const int pc_history_size = 8;
    /// current pc history position
    int pc_history_pos;
    /// a history ringbuffer of previous PC addresses
    uword pc_history[pc_history_size];

    /// input hook typedef, take a port number, return value at that port
    typedef ubyte (*cb_in)(void* userdata, uword port);
    /// output hook typedef, write 8-bit value to output port
    typedef void (*cb_out)(void* userdata, uword port, ubyte val);

    /// user-provided in-handler
    cb_in in_func;
    /// user-provided out-handler
    cb_out out_func;
    /// input/output userdata
    void* inout_userdata;

    /// highest priority interrupt controller in daisy chain
    z80int* irq_device;
    /// an interrupt request has been received
    bool irq_received;
    /// delayed-interrupt-enable flag set bei ei()
    bool enable_interrupt;

    /// constructor
    z80();

    /// one-time init
    void init(cb_in func_in, cb_out func_out, void* userdata);
    /// connect the highest priority interrupt controller device
    void connect_irq_device(z80int* device);
    /// perform a reset (RESET pin triggered)
    void reset();
    /// execute a single instruction, return number of cycles
    unsigned int step();

    /// called when invalid opcode has been hit
    void invalid_opcode(uword opsize);
    /// store current pc in history ringbuffer
    void store_pc_history();
    /// get pc from history ringbuffer (0 is oldest entry)
    uword get_pc_history(int index) const;

    /// receive an interrupt request
    static void irq(void* self);
    /// handle an interrupt request, must be called after step(), return num tstates taken
    int handle_irq();
    /// implement the RETI instruction
    void reti();
    /// implement the EI instruction
    void ei();
    /// implement the DI instruction
    void di();

    /// helper method to swap 2 16-bit registers
    static void swap16(uword& r0, uword& r1);
    /// set an 8-bit register value by enum (slow)
    void set8(reg r, ubyte v);
    /// get an 8-bit register value by enum (slow)
    ubyte get8(reg r) const;
    /// set a 16-bit register value by enum (slow)
    void set16(reg r, uword v);
    /// get a 16-bit register value by enum (slow)
    uword get16(reg r) const;
    /// get a string-name for a register
    static const char* reg_name(reg r);
    /// helper to test expected flag bitmask
    bool test_flags(ubyte expected) const;
    /// fetch an opcode byte and increment R register
    ubyte fetch_op();
    /// call in-handler, return result
    ubyte in(uword port);
    /// call out-handler
    void out(uword port, ubyte val);
    /// perform an 8-bit add, return result, and update flags
    ubyte add8(ubyte acc, ubyte add);
    /// perform an 8-bit adc, return result and update flags
    ubyte adc8(ubyte acc, ubyte add);
    /// perform an 8-bit sub, return result, and update flags
    ubyte sub8(ubyte acc, ubyte sub);
    /// perform an 8-bit sbc, return result and update flags
    ubyte sbc8(ubyte acc, ubyte sub);
    /// perform an 8-bit inc and set flags
    ubyte inc8(ubyte val);
    /// perform an 8-bit dec and set flags
    ubyte dec8(ubyte val);
    /// perform an 16-bit add, update flags and return result
    uword add16(uword acc, uword val);
    /// perform an 16-bit adc, update flags and return result
    uword adc16(uword acc, uword val);
    /// perform an 16-bit sbc, update flags and return result
    uword sbc16(uword acc, uword val);
    /// implement the LDI instruction
    void ldi();
    /// implement the LDIR instruction, return number of T-states
    int ldir();
    /// implement the LDD instruction
    void ldd();
    /// implement the LDDR instruction, return number of T-states
    int lddr();
    /// implement the CPI instruction
    void cpi();
    /// implement the CPIR instruction, return number of T-states
    int cpir();
    /// implement the CPD instruction
    void cpd();
    /// implement the CPDR instruction, return number of T-states
    int cpdr();
    /// get the SF|ZF|PF flags for a value
    static ubyte szp(ubyte val);
    /// return flags for ini/ind instruction
    ubyte ini_ind_flags(ubyte io_val, int c_add);
    /// implement the INI instruction
    void ini();
    /// implement the INIR instruction, return number of T-states
    int inir();
    /// implement the IND instruction
    void ind();
    /// implement the INDR instruction, return number of T-states
    int indr();
    /// return flags for outi/outd instruction
    ubyte outi_outd_flags(ubyte io_val);
    /// implement the OUTI instruction
    void outi();
    /// implement the OTIR instructor, return number of T-states
    int otir();
    /// implment the OUTD instruction
    void outd();
    /// implement the OTDR instruction, return number of T-states
    int otdr();
    /// implement the DAA instruction
    void daa();
    /// get flags for the LD A,I and LD A,R instructions
    static ubyte sziff2(ubyte val, bool iff2);
    /// rotate left, copy sign bit into CF,
    ubyte rlc8(ubyte val, bool flags_szp);
    /// rotate right, copy bit 0 into CF
    ubyte rrc8(ubyte val, bool flags_szp);
    /// rotate left through carry bit
    ubyte rl8(ubyte val, bool flags_szp);
    /// rotate right through carry bit
    ubyte rr8(ubyte val, bool flags_szp);
    /// shift left into carry bit and update flags
    ubyte sla8(ubyte val);
    /// undocumented: shift left into carry bit, update flags
    ubyte sll8(ubyte val);
    /// shift right into carry bit, preserve sign, update flags
    ubyte sra8(ubyte val);
    /// shift right into carry bit, update flags
    ubyte srl8(ubyte val);
    /// implements the RLD instruction
    void rld();
    /// implements the RRD instruction
    void rrd();
    /// implements the BIT test instruction, updates flags
    void bit(ubyte val, ubyte mask);
    /// undocumented register-autocopy for the DD/FD CB instructions
    void undoc_autocopy(ubyte reg, ubyte val);
    /// special handling for the FD/DD CB bit opcodes
    void dd_fd_cb(ubyte lead);
};

//------------------------------------------------------------------------------
inline z80::z80() :
pc_history_pos(0),
in_func(nullptr),
out_func(nullptr),
inout_userdata(nullptr),
irq_device(nullptr),
irq_received(false),
enable_interrupt(false) {
    YAKC_MEMSET(&this->state, 0, sizeof(this->state));
    YAKC_MEMSET(&this->pc_history, 0, sizeof(this->pc_history));
}

//------------------------------------------------------------------------------
inline void
z80::init(cb_in func_in, cb_out func_out, void* userdata) {
    YAKC_ASSERT(func_in && func_out);
    this->reset();
    this->in_func = func_in;
    this->out_func = func_out;
    this->inout_userdata = userdata;
}

//------------------------------------------------------------------------------
inline void
z80::connect_irq_device(z80int* device) {
    YAKC_ASSERT(device);
    this->irq_device = device;
}

//------------------------------------------------------------------------------
inline void
z80::reset() {
    this->state.PC = 0;
    this->state.IM = 0;
    this->state.IFF1 = false;
    this->state.IFF2 = false;
    this->state.I = 0;
    this->state.R = 0;
    this->irq_received = false;
    this->enable_interrupt = false;
}

//------------------------------------------------------------------------------
inline void
z80::irq(void* userdata) {
    z80* self = (z80*) userdata;
    self->irq_received = true;
}

//------------------------------------------------------------------------------
inline int
z80::handle_irq() {
    int tstates = 0;
    if (this->irq_received) {
        tstates += 2;
        // we don't implement MODE0 or MODE1 (yet?)
        YAKC_ASSERT(2 == this->state.IM);
        this->irq_received = false;
        // no point in handling interrupt if no daisy chain is configured
        if (this->irq_device) {
            // interrupt enabled?
            if (this->state.IFF1) {
                // first disable interrupts
                this->state.IFF1 = this->state.IFF2 = false;
                // ask daisy chain for interrupt vector
                ubyte vec = this->irq_device->interrupt_acknowledged();
                // generate interrupt vector address (MODE2)
                uword addr = (this->state.I<<8)|(vec&0xFE);
                // push current PC on stack, and load PC with interrupt routine
                this->state.SP -= 2;
                this->mem.w16(this->state.SP, this->state.PC);
                this->state.PC = this->mem.r16(addr);
                tstates += 19;
            }
            else {
                // interrupts are disabled, notify daisy chain
                this->irq_device->interrupt_cancelled();
            }
        }
    }
    return tstates;
}

//------------------------------------------------------------------------------
inline void
z80::reti() {
    // this is the same as RET
    this->state.PC = mem.r16(state.SP);
    this->state.SP += 2;
    // ...notify daisy chain, if configured
    if (this->irq_device) {
        this->irq_device->reti();
    }
}

//------------------------------------------------------------------------------
inline void
z80::ei() {
    // NOTE: interrupts are actually enabled *after the next instruction*,
    // thus we only set a flag here, the IFF flags will then be changed
    // during the next instruction decode in step()
    this->enable_interrupt = true;
}

//------------------------------------------------------------------------------
inline void
z80::di() {
    this->state.IFF1 = false;
    this->state.IFF2 = false;
}

//------------------------------------------------------------------------------
inline void
z80::store_pc_history() {
    this->pc_history[pc_history_pos++] = this->state.PC;
    this->pc_history_pos &= this->pc_history_size-1;
}

//------------------------------------------------------------------------------
inline uword
z80::get_pc_history(int index) const {
    int i = ((this->pc_history_pos-pc_history_size)+index) & (pc_history_size-1);
    return pc_history[i];
}

//------------------------------------------------------------------------------
inline void
z80::invalid_opcode(uword opsize) {
    state.INV = true;
    state.PC -= opsize;     // stuck on invalid opcode
    state.T  = 4;          // fantasy value
}

//------------------------------------------------------------------------------
inline void
z80::swap16(uword& r0, uword& r1) {
    uword tmp = r0;
    r0 = r1;
    r1 = tmp;
}

//------------------------------------------------------------------------------
inline void
z80::set8(reg r, ubyte v) {
    switch (r) {
        case rA:     state.A = v; break;
        case rF:     state.F = v; break;
        case rB:     state.B = v; break;
        case rC:     state.C = v; break;
        case rD:     state.D = v; break;
        case rE:     state.E = v; break;
        case rH:     state.H = v; break;
        case rL:     state.L = v; break;
        case rI:     state.I = v; break;
        case rR:     state.R = v; break;
        case rIM:    state.IM = v; break;
        default:    YAKC_ASSERT(false); break;
    }
}

//------------------------------------------------------------------------------
inline ubyte
z80::get8(reg r) const {
    switch (r) {
        case rA:    return state.A;
        case rF:    return state.F;
        case rB:    return state.B;
        case rC:    return state.C;
        case rD:    return state.D;
        case rE:    return state.E;
        case rH:    return state.H;
        case rL:    return state.L;
        case rI:    return state.I;
        case rR:    return state.R;
        case rIM:   return state.IM;
        default:
            YAKC_ASSERT(false);
            return 0;
    }
}

//------------------------------------------------------------------------------
inline void
z80::set16(reg r, uword v) {
    switch (r) {
        case rAF:   state.AF = v; break;
        case rBC:   state.BC = v; break;
        case rDE:   state.DE = v; break;
        case rHL:   state.HL = v; break;
        case rAF_:  state.AF_ = v; break;
        case rBC_:  state.BC_ = v; break;
        case rDE_:  state.DE_ = v; break;
        case rHL_:  state.HL_ = v; break;
        case rIX:   state.IX = v; break;
        case rIY:   state.IY = v; break;
        case rSP:   state.SP = v; break;
        case rPC:   state.PC = v; break;
        default:    YAKC_ASSERT(false);
    }
}

//------------------------------------------------------------------------------
inline uword
z80::get16(reg r) const {
    switch (r) {
        case rAF:    return state.AF;
        case rBC:    return state.BC;
        case rDE:    return state.DE;
        case rHL:    return state.HL;
        case rAF_:   return state.AF_;
        case rBC_:   return state.BC_;
        case rDE_:   return state.DE_;
        case rHL_:   return state.HL_;
        case rIX:    return state.IX;
        case rIY:    return state.IY;
        case rSP:    return state.SP;
        case rPC:    return state.PC;
        default:
            YAKC_ASSERT(false);
            return 0;
    }
}

//------------------------------------------------------------------------------
inline const char*
z80::reg_name(reg r) {
    switch (r) {
        case rA: return "A";
        case rF: return "F";
        case rB: return "B";
        case rC: return "C";
        case rD: return "D";
        case rE: return "E";
        case rH: return "H";
        case rL: return "L";
        case rAF: return "AF";
        case rBC: return "BC";
        case rDE: return "DE";
        case rHL: return "HL";
        case rAF_: return "AF'";
        case rBC_: return "BC'";
        case rDE_: return "DE'";
        case rHL_: return "HL'";
        case rI: return "I";
        case rR: return "R";
        case rIX: return "IX";
        case rIY: return "IY";
        case rSP: return "SP";
        case rPC: return "PC";
        case rIM: return "IM";
        default: return "?";
    }
}

//------------------------------------------------------------------------------
inline bool
z80::test_flags(ubyte expected) const {
    // mask out undocumented flags
    ubyte undoc = ~(XF|YF);
    return (state.F & undoc) == expected;
}

//------------------------------------------------------------------------------
inline ubyte
z80::fetch_op() {
    state.R = (state.R + 1) & 0x7F;
    return mem.r8(state.PC++);
}

//------------------------------------------------------------------------------
inline ubyte
z80::in(uword port) {
    if (in_func) {
        return in_func(inout_userdata, port);
    }
    else {
        return 0;
    }
}

//------------------------------------------------------------------------------
inline void
z80::out(uword port, ubyte val) {
    if (out_func) {
        return out_func(inout_userdata, port, val);
    }
}

#define SZ(i) ((i&0xFF)?((i&0x80)?SF:0):ZF)

//------------------------------------------------------------------------------
inline ubyte
z80::add8(ubyte acc, ubyte add) {
    int r = int(acc) + int(add);
    ubyte f = SZ(r);
    if (r > 0xFF) f |= CF;
    if ((r & 0xF) < (acc & 0xF)) f |= HF;
    if (((acc&0x80) == (add&0x80)) && ((r&0x80) != (acc&0x80))) f |= VF;
    state.F = f;
    return (ubyte)r;
}

//------------------------------------------------------------------------------
inline ubyte
z80::adc8(ubyte acc, ubyte add) {
    if (state.F & CF) {
        int r = int(acc) + int(add) + 1;
        ubyte f = SZ(r);
        if (r > 0xFF) f |= CF;
        if ((r & 0xF) <= (acc & 0xF)) f |= HF;
        if (((acc&0x80) == (add&0x80)) && ((r&0x80) != (acc&0x80))) f |= VF;
        state.F = f;
        return (ubyte)r;
    }
    else {
        return add8(acc, add);
    }
}

//------------------------------------------------------------------------------
inline ubyte
z80::sub8(ubyte acc, ubyte sub) {
    int r = int(acc) - int(sub);
    ubyte f = NF | SZ(r);
    if (r < 0) f |= CF;
    if ((r & 0xF) > (acc & 0xF)) f |= HF;
    if (((acc&0x80) != (sub&0x80)) && ((r&0x80) != (acc&0x80))) f |= VF;
    state.F = f;
    return (ubyte)r;
}

//------------------------------------------------------------------------------
inline ubyte
z80::sbc8(ubyte acc, ubyte sub) {
    if (state.F & CF) {
        int r = int(acc) - int(sub) - 1;
        ubyte f = NF | SZ(r);
        if (r < 0) f |= CF;
        if ((r & 0xF) >= (acc & 0xF)) f |= HF;
        if (((acc&0x80) != (sub&0x80)) && ((r&0x80) != (acc&0x80))) f |= VF;
        state.F = f;
        return (ubyte)r;
    }
    else {
        return sub8(acc, sub);
    }
}

//------------------------------------------------------------------------------
inline ubyte
z80::inc8(ubyte val) {
    ubyte r = val + 1;
    ubyte f = SZ(r);
    if ((r & 0xF) == 0) f |= HF;
    if (r == 0x80) f |= VF;
    state.F = f | (state.F & CF);
    return r;
}

//------------------------------------------------------------------------------
inline ubyte
z80::dec8(ubyte val) {
    ubyte r = val - 1;
    ubyte f = NF | SZ(r);
    if ((r & 0xF) == 0xF) f |= HF;
    if (r == 0x7F) f |= VF;
    state.F = f | (state.F & CF);
    return r;
}

//------------------------------------------------------------------------------
inline uword
z80::add16(uword acc, uword val) {
    unsigned int res = acc + val;
    // flag computation taken from MAME
    state.F = (state.F & (SF|ZF|VF)) |
              (((acc^res^val)>>8)&HF)|
              ((res>>16) & CF) | ((res >> 8) & (YF|XF));
    return (uword)res;
}

//------------------------------------------------------------------------------
inline uword
z80::adc16(uword acc, uword val) {
    unsigned int res = acc + val + (state.F & CF);
    // flag computation taken from MAME
    state.F = (((acc^res^val)>>8)&HF) |
              ((res>>16)&CF) |
              ((res>>8)&(SF|YF|XF)) |
              ((res & 0xFFFF) ? 0 : ZF) |
              (((val^acc^0x8000) & (val^res)&0x8000)>>13);
    return res;
}

//------------------------------------------------------------------------------
inline uword
z80::sbc16(uword acc, uword val) {
    unsigned int res = acc - val - (state.F & CF);
    // flag computation taken from MAME
    state.F = (((acc^res^val)>>8)&HF) | NF |
              ((res>>16)&CF) |
              ((res>>8) & (SF|YF|XF)) |
              ((res & 0xFFFF) ? 0 : ZF) |
              (((val^acc) & (acc^res)&0x8000)>>13);
    return res;
}

//------------------------------------------------------------------------------
inline void
z80::ldi() {
    ubyte val = mem.r8(state.HL);
    mem.w8(state.DE, val);
    val += state.A;
    ubyte f = state.F & (SF|ZF|CF);
    if (val & 0x02) f |= YF;
    if (val & 0x08) f |= XF;
    state.HL++;
    state.DE++;
    state.BC--;
    if (state.BC) {
        f |= VF;
    }
    state.F = f;
}

//------------------------------------------------------------------------------
inline int
z80::ldir() {
    ldi();
    if (state.BC != 0) {
        state.PC -= 2;
        return 21;
    }
    else {
        return 16;
    }
}

//------------------------------------------------------------------------------
inline void
z80::ldd() {
    ubyte val = mem.r8(state.HL);
    mem.w8(state.DE, val);
    val += state.A;
    ubyte f = state.F & (SF|ZF|CF);
    if (val & 0x02) f |= YF;
    if (val & 0x08) f |= XF;
    state.HL--;
    state.DE--;
    state.BC--;
    if (state.BC) {
        f |= VF;
    }
    state.F = f;
}

//------------------------------------------------------------------------------
inline int
z80::lddr() {
    ldd();
    if (state.BC != 0) {
        state.PC -= 2;
        return 21;
    }
    else {
        return 16;
    }
}

//------------------------------------------------------------------------------
inline void
z80::cpi() {
    int r = int(state.A) - int(mem.r8(state.HL));
    ubyte f = NF | (state.F & CF) | SZ(r);
    if ((r & 0xF) > (state.A & 0xF)) {
        f |= HF;
        r--;
    }
    if (r & 0x02) f |= YF;
    if (r & 0x08) f |= XF;
    state.HL++;
    state.BC--;
    if (state.BC) {
        f |= VF;
    }
    state.F = f;
}

//------------------------------------------------------------------------------
inline int
z80::cpir() {
    cpi();
    if ((state.BC != 0) && !(state.F & ZF)) {
        state.PC -= 2;
        return 21;
    }
    else {
        return 16;
    }
}

//------------------------------------------------------------------------------
inline void
z80::cpd() {
    int r = int(state.A) - int(mem.r8(state.HL));
    ubyte f = NF | (state.F & CF) | SZ(r);
    if ((r & 0xF) > (state.A & 0xF)) {
        f |= HF;
        r--;
    }
    if (r & 0x02) f |= YF;
    if (r & 0x08) f |= XF;
    state.HL--;
    state.BC--;
    if (state.BC) {
        f |= VF;
    }
    state.F = f;
}

//------------------------------------------------------------------------------
inline int
z80::cpdr() {
    cpd();
    if ((state.BC != 0) && !(state.F & ZF)) {
        state.PC -= 2;
        return 21;
    }
    else {
        return 16;
    }
}

//------------------------------------------------------------------------------
inline ubyte
z80::szp(ubyte val) {
    int p = 0;
    if (val & (1<<0)) p++;
    if (val & (1<<1)) p++;
    if (val & (1<<2)) p++;
    if (val & (1<<3)) p++;
    if (val & (1<<4)) p++;
    if (val & (1<<5)) p++;
    if (val & (1<<6)) p++;
    if (val & (1<<7)) p++;
    ubyte f = SZ(val);
    f |= (val & (YF|XF));   // undocumented flag bits 3 and 5
    f |= p & 1 ? 0 : PF;
    return f;
}

//------------------------------------------------------------------------------
inline ubyte
z80::ini_ind_flags(ubyte io_val, int c_add) {
    // NOTE: most INI flag settings are undocumented in the official
    // docs, so this is taken from MAME, there's also more
    // information here: http://www.z80.info/z80undoc3.txt
    ubyte f = state.B ? state.B & SF : ZF;
    if (io_val & SF) f |= NF;
    unsigned int t = (unsigned int)((state.C+c_add)&0xFF) + (unsigned int)io_val;
    if (t & 0x100) f |= HF|CF;
    f |= szp(ubyte(t & 0x07)^state.B) & PF;
    return f;
}

//------------------------------------------------------------------------------
inline void
z80::ini() {
    ubyte io_val = in(state.BC);
    state.B--;
    mem.w8(state.HL++, io_val);
    state.F = ini_ind_flags(io_val, +1);
}

//------------------------------------------------------------------------------
inline int
z80::inir() {
    ini();
    if (state.B != 0) {
        state.PC -= 2;
        return 21;
    }
    else {
        return 16;
    }
}

//------------------------------------------------------------------------------
inline void
z80::ind() {
    ubyte io_val = in(state.BC);
    state.B--;
    mem.w8(state.HL--, io_val);
    state.F = ini_ind_flags(io_val, -1);
}

//------------------------------------------------------------------------------
inline int
z80::indr() {
    ind();
    if (state.B != 0) {
        state.PC -= 2;
        return 21;
    }
    else {
        return 16;
    }
}

//------------------------------------------------------------------------------
inline ubyte
z80::outi_outd_flags(ubyte io_val) {
    // NOTE: most OUTI flag settings are undocumented in the official
    // docs, so this is taken from MAME, there's also more
    // information here: http://www.z80.info/z80undoc3.txt
    ubyte f = state.B ? state.B & SF : ZF;
    if (io_val & SF) f |= NF;
    unsigned int t = (unsigned int)state.L + (unsigned int)io_val;
    if (t & 0x100) f |= HF|CF;
    f |= szp(ubyte(t & 0x07)^state.B) & PF;
    return f;
}

//------------------------------------------------------------------------------
inline void
z80::outi() {
    ubyte io_val = mem.r8(state.HL++);
    state.B--;
    out(state.BC, io_val);
    state.F = outi_outd_flags(io_val);
}

//------------------------------------------------------------------------------
inline int
z80::otir() {
    outi();
    if (state.B != 0) {
        state.PC -= 2;
        return 21;
    }
    else {
        return 16;
    }
}

//------------------------------------------------------------------------------
inline void
z80::outd() {
    ubyte io_val = mem.r8(state.HL--);
    state.B--;
    out(state.BC, io_val);
    state.F = outi_outd_flags(io_val);
}

//------------------------------------------------------------------------------
inline int
z80::otdr() {
    outd();
    if (state.B != 0) {
        state.PC -= 2;
        return 21;
    }
    else {
        return 16;
    }
}

//------------------------------------------------------------------------------
inline void
z80::daa() {
    // from MAME and http://www.z80.info/zip/z80-documented.pdf
    ubyte val = state.A;
    if (state.F & NF) {
        if (((state.A & 0xF) > 0x9) || (state.F & HF)) {
            val -= 0x06;
        }
        if ((state.A > 0x99) || (state.F & CF)) {
            val -= 0x60;
        }
    }
    else {
        if (((state.A & 0xF) > 0x9) || (state.F & HF)) {
            val += 0x06;
        }
        if ((state.A > 0x99) || (state.F & CF)) {
            val += 0x60;
        }
    }
    state.F &= CF|NF;
    state.F |= (state.A > 0x99) ? CF:0;
    state.F |= (state.A^val) & HF;
    state.F |= szp(val);
    state.A = val;
}

//------------------------------------------------------------------------------
inline ubyte
z80::sziff2(ubyte val, bool iff2) {
    ubyte f = val ? val & SF : ZF;
    f |= (val & (YF|XF));   // undocumented flag bits 3 and 5
    if (iff2) f |= PF;
    return f;
}

//------------------------------------------------------------------------------
inline ubyte
z80::rlc8(ubyte val, bool flags_szp) {
    ubyte r = val<<1|val>>7;
    ubyte f = (val & 0x80) ? CF : 0;
    if (flags_szp) {
        state.F = f | szp(r);
    }
    else {
        state.F = f | (state.F & (SF|ZF|PF));
    }
    return r;
}

//------------------------------------------------------------------------------
inline ubyte
z80::rrc8(ubyte val, bool flags_szp) {
    ubyte r = val>>1|val<<7;
    ubyte f = (val & 0x01) ? CF : 0;
    if (flags_szp) {
        state.F = f | szp(r);
    }
    else {
        state.F = f | (state.F & (SF|ZF|PF));
    }
    return r;
}

//------------------------------------------------------------------------------
inline ubyte
z80::rl8(ubyte val, bool flags_szp) {
    ubyte r = val<<1 | ((state.F & CF) ? 0x01:0x00);
    ubyte f = val & 0x80 ? CF : 0;
    if (flags_szp) {
        state.F = f | szp(r);
    }
    else {
        state.F = f | (state.F & (SF|ZF|PF));
    }
    return r;
}

//------------------------------------------------------------------------------
inline ubyte
z80::rr8(ubyte val, bool flags_szp) {
    ubyte r = val>>1 | ((state.F & CF) ? 0x80:0x00);
    ubyte f = val & 0x01 ? CF : 0;
    if (flags_szp) {
        state.F = f | szp(r);
    }
    else {
        state.F = f | (state.F & (SF|ZF|PF));
    }
    return r;
}

//------------------------------------------------------------------------------
inline ubyte
z80::sla8(ubyte val) {
    ubyte r = val<<1;
    ubyte f = val & 0x80 ? CF : 0;
    state.F = f | szp(r);
    return r;
}

//------------------------------------------------------------------------------
inline ubyte
z80::sll8(ubyte val) {
    // undocument! sll8 is identical with sla8 but inserts a 1 into the LSB
    ubyte r = (val<<1) | 1;
    ubyte f = val & 0x80 ? CF : 0;
    state.F = f | szp(r);
    return r;
}

//------------------------------------------------------------------------------
inline ubyte
z80::sra8(ubyte val) {
    ubyte r = (val>>1) | (val & 0x80);
    ubyte f = val & 0x01 ? CF : 0;
    state.F = f | szp(r);
    return r;
}

//------------------------------------------------------------------------------
inline ubyte
z80::srl8(ubyte val) {
    ubyte r = val>>1;
    ubyte f = val & 0x01 ? CF : 0;
    state.F = f | szp(r);
    return r;
}

//------------------------------------------------------------------------------
inline void
z80::rld() {
    ubyte x = mem.r8(state.HL);
    ubyte tmp = state.A & 0xF;              // store A low nibble
    state.A = (state.A & 0xF0) | (x>>4);    // move (HL) high nibble into A low nibble
    x = (x<<4) | tmp;   // move (HL) low to high nibble, move A low nibble to (HL) low nibble
    mem.w8(state.HL, x);
    state.F = szp(state.A) | (state.F & CF);
}

//------------------------------------------------------------------------------
inline void
z80::rrd() {
    ubyte x = mem.r8(state.HL);
    ubyte tmp = state.A & 0xF;                  // store A low nibble
    state.A = (state.A & 0xF0) | (x & 0x0F);    // move (HL) low nibble to A low nibble
    x = (x >> 4) | (tmp << 4);  // move A low nibble to (HL) high nibble, and (HL) high nibble to (HL) low nibble
    mem.w8(state.HL, x);
    state.F = szp(state.A) | (state.F & CF);
}

//------------------------------------------------------------------------------
inline void
z80::bit(ubyte val, ubyte mask) {
    ubyte r = val & mask;
    ubyte f = HF | (r ? (r & SF) : (ZF|PF));
    f |= (val & (YF|XF));
    state.F = f | (state.F & CF);
}

//------------------------------------------------------------------------------
inline void
z80::undoc_autocopy(ubyte reg, ubyte val) {
    // this is for the undocumented DD CB and FB CB instruction which autocopy
    // the result into an 8-bit register (except for the F register)
    switch (reg) {
        case 0: state.B = val; break;
        case 1: state.C = val; break;
        case 2: state.D = val; break;
        case 3: state.E = val; break;
        case 4: state.H = val; break;
        case 5: state.L = val; break;
        case 7: state.A = val; break;
    }
}

//------------------------------------------------------------------------------
inline void
z80::dd_fd_cb(ubyte lead) {
    int d = mem.rs8(state.PC++);
    uword addr;
    if (lead == 0xDD) {
        addr = state.IX + d;
    }
    else {
        addr = state.IY + d;
    }
    ubyte val;
    ubyte op = mem.r8(state.PC++);
    switch (op) {
        // RLC ([IX|IY]+d) -> r (except F)
        case 0x00:
        case 0x01:
        case 0x02:
        case 0x03:
        case 0x04:
        case 0x05:
        case 0x06:
        case 0x07:
            val = rlc8(mem.r8(addr), true);
            undoc_autocopy(op, val);
            mem.w8(addr, val);
            state.T = 23;
            break;
        // RRC ([IX|IY]+d)
        case 0x08:
        case 0x09:
        case 0x0A:
        case 0x0B:
        case 0x0C:
        case 0x0D:
        case 0x0E:
        case 0x0F:
            val = rrc8(mem.r8(addr), true);
            undoc_autocopy(op-0x08, val);
            mem.w8(addr, val);
            state.T = 23;
            break;
        // RL ([IX|IY]+d)
        case 0x10:
        case 0x11:
        case 0x12:
        case 0x13:
        case 0x14:
        case 0x15:
        case 0x16:
        case 0x17:
            val = rl8(mem.r8(addr), true);
            undoc_autocopy(op-0x10, val);
            mem.w8(addr, val);
            state.T = 23;
            break;
        // RR ([IX|IY]+d)
        case 0x18:
        case 0x19:
        case 0x1A:
        case 0x1B:
        case 0x1C:
        case 0x1D:
        case 0x1E:
        case 0x1F:
            val = rr8(mem.r8(addr), true);
            undoc_autocopy(op-0x18, val);
            mem.w8(addr, val);
            state.T = 23;
            break;
        // SLA ([IX|IY]+d)
        case 0x20:
        case 0x21:
        case 0x22:
        case 0x23:
        case 0x24:
        case 0x25:
        case 0x26:
        case 0x27:
            val = sla8(mem.r8(addr));
            undoc_autocopy(op-0x20, val);
            mem.w8(addr, val);
            state.T = 23;
            break;
        // SRA ([IX|IY]+d)
        case 0x28:
        case 0x29:
        case 0x2A:
        case 0x2B:
        case 0x2C:
        case 0x2D:
        case 0x2E:
        case 0x2F:
            val = sra8(mem.r8(addr));
            undoc_autocopy(op-0x28, val);
            mem.w8(addr, val);
            state.T = 23;
            break;
        // SLL ([IX|IY]+d)
        case 0x30:
        case 0x31:
        case 0x32:
        case 0x33:
        case 0x34:
        case 0x35:
        case 0x36:
        case 0x37:
            val = sll8(mem.r8(addr));
            undoc_autocopy(op-0x30, val);
            mem.w8(addr, val);
            state.T = 23;
            break;
        // SRL ([IX|IY]+d)
        case 0x38:
        case 0x39:
        case 0x3A:
        case 0x3B:
        case 0x3C:
        case 0x3D:
        case 0x3E:
        case 0x3F:
            val = srl8(mem.r8(addr));
            undoc_autocopy(op-0x38, val);
            mem.w8(addr, val);
            state.T = 23;
            break;
        // BIT b,([IX|IY]+d)
        case 0x46 | (0<<3):
        case 0x46 | (1<<3):
        case 0x46 | (2<<3):
        case 0x46 | (3<<3):
        case 0x46 | (4<<3):
        case 0x46 | (5<<3):
        case 0x46 | (6<<3):
        case 0x46 | (7<<3):
            bit(mem.r8(addr), 1<<((op>>3)&7));
            state.T = 20;
            break;
        // RES b,([IX|IY]+d)
        case 0x86 | (0<<3):
        case 0x86 | (1<<3):
        case 0x86 | (2<<3):
        case 0x86 | (3<<3):
        case 0x86 | (4<<3):
        case 0x86 | (5<<3):
        case 0x86 | (6<<3):
        case 0x86 | (7<<3):
            mem.w8(addr, mem.r8(addr) & ~(1<<((op>>3)&7)));
            state.T = 23;
            break;
        // SET b,([IX|IY]+d)
        case 0xC6 | (0<<3):
        case 0xC6 | (1<<3):
        case 0xC6 | (2<<3):
        case 0xC6 | (3<<3):
        case 0xC6 | (4<<3):
        case 0xC6 | (5<<3):
        case 0xC6 | (6<<3):
        case 0xC6 | (7<<3):
            mem.w8(addr, mem.r8(addr) | (1<<((op>>3)&7)));
            state.T = 23;
            break;

        // unknown opcode
        default:
            invalid_opcode(4);
            break;
    }
}

} // namespace
#include "opcodes.h"
