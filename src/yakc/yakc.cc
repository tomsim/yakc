//------------------------------------------------------------------------------
//  yakc.cc
//------------------------------------------------------------------------------
#include "yakc.h"
#include "emus/z1013.h"
#include "emus/z9001.h"
#include "emus/zx.h"
#include "emus/kc85.h"
#include "emus/atom.h"
#include "emus/cpc.h"
#include "emus/c64.h"
#include <stdio.h>

namespace YAKC {

//------------------------------------------------------------------------------
void
yakc::init(const ext_funcs& sys_funcs) {
    func = sys_funcs;
    fill_random(board.random, sizeof(board.random));
}

//------------------------------------------------------------------------------
void
yakc::add_rom(rom_images::rom type, const uint8_t* ptr, int size) {
    roms.add(type, ptr, size);
}

//------------------------------------------------------------------------------
bool
yakc::check_roms(system m, os_rom os) {
    if (is_system(m, system::any_z1013)) {
        return z1013_t::check_roms(m);
    }
    else if (is_system(m, system::any_z9001)) {
        return z9001_t::check_roms(m);
    }
    else if (is_system(m, system::any_zx)) {
        return zx_t::check_roms(m);
    }
    else if (is_system(m, system::any_kc85)) {
        return kc85_t::check_roms(m, os);
    }
    else if (is_system(m, system::acorn_atom)) {
        return atom_t::check_roms(m);
    }
    else if (is_system(m, system::any_cpc)) {
        return cpc_t::check_roms(m);
    }
    else if (is_system(m, system::any_c64)) {
        return c64_t::check_roms(m);
    }
    else {
        return false;
    }
}

//------------------------------------------------------------------------------
void
yakc::poweron(system m, os_rom rom) {
    this->model = m;
    this->os = rom;
    this->enable_joystick(false);
    this->accel = 1;
    board.dbg.init(this->cpu_type());
    if (this->is_system(system::any_z1013)) {
        z1013.poweron(m);
    }
    else if (this->is_system(system::any_z9001)) {
        z9001.poweron(m);
    }
    else if (this->is_system(system::any_zx)) {
        zx.poweron(m);
    }
    else if (this->is_system(system::any_kc85)) {
        kc85.poweron(m, rom);
    }
    else if (this->is_system(system::acorn_atom)) {
        atom.poweron();
    }
    else if (this->is_system(system::any_cpc)) {
        cpc.poweron(m);
    }
    else if (this->is_system(system::any_c64)) {
        c64.poweron(m);
    }
}

//------------------------------------------------------------------------------
void
yakc::poweroff() {
    if (z1013.on) {
        z1013.poweroff();
    }
    if (z9001.on) {
        z9001.poweroff();
    }
    if (zx.on) {
        zx.poweroff();
    }
    if (kc85.on) {
        kc85.poweroff();
    }
    if (atom.on) {
        atom.poweroff();
    }
    if (cpc.on) {
        cpc.poweroff();
    }
    if (c64.on) {
        c64.poweroff();
    }
}

//------------------------------------------------------------------------------
bool
yakc::switchedon() const {
    return z1013.on || z9001.on | zx.on | kc85.on | atom.on | cpc.on | c64.on;
}

//------------------------------------------------------------------------------
void
yakc::reset() {
    this->enable_joystick(false);
    if (z1013.on) {
        z1013.reset();
    }
    if (z9001.on) {
        z9001.reset();
    }
    if (zx.on) {
        zx.reset();
    }
    if (kc85.on) {
        kc85.reset();
    }
    if (atom.on) {
        atom.reset();
    }
    if (cpc.on) {
        cpc.reset();
    }
    if (c64.on) {
        c64.reset();
    }
}

//------------------------------------------------------------------------------
bool
yakc::is_system(system mask) const {
    return 0 != (int(this->model) & int(mask));
}

//------------------------------------------------------------------------------
bool
yakc::is_system(system model, system mask) {
    return 0 != (int(model) & int(mask));
}

//------------------------------------------------------------------------------
cpu_model
yakc::cpu_type() const {
    if (this->is_system(system::acorn_atom) || this->is_system(system::any_c64)) {
        return cpu_model::m6502;
    }
    else {
        return cpu_model::z80;
    }
}

//------------------------------------------------------------------------------
void
yakc::exec(int micro_secs) {
    YAKC_ASSERT(this->accel > 0);
    if (!board.dbg.break_stopped()) {
        if (z1013.on) {
            z1013.exec(micro_secs);
        }
        else if (z9001.on) {
            z9001.exec(micro_secs);
        }
        else if (zx.on) {
            zx.exec(micro_secs);
        }
        else if (kc85.on) {
            kc85.exec(micro_secs);
        }
        else if (atom.on) {
            atom.exec(micro_secs);
        }
        else if (cpc.on) {
            cpc.exec(micro_secs);
        }
        else if (c64.on) {
            c64.exec(micro_secs);
        }
        else {
            //this->abs_cycle_count = abs_end_cycles;
        }

        // check if breakpoint has been hit
        board.dbg.break_check();
    }
}

//------------------------------------------------------------------------------
uint32_t
yakc::step() {
    uint32_t ticks = 0;
    if (board.z80) {
        ticks = z80_exec(board.z80, 0);
        if (!z80_opdone(board.z80)) {
            ticks += z80_exec(board.z80, 0);
        }
        board.dbg.add_history_item(z80_pc(board.z80), ticks);
    }
    else if (board.m6502) {
        ticks = m6502_exec(board.m6502, 0);
        board.dbg.add_history_item(board.m6502->state.PC, ticks);
    }
    return ticks;
}

//------------------------------------------------------------------------------
uint32_t
yakc::step_until(std::function<bool(uint32_t)> fn) {
    uint32_t ticks = 0;
    do {
        ticks += this->step();
    }
    while (!fn(ticks));
    return ticks;
}

//------------------------------------------------------------------------------
void
yakc::on_ascii(uint8_t ascii) {
    if (z1013.on) {
        z1013.on_ascii(ascii);
    }
    if (z9001.on) {
        z9001.on_ascii(ascii);
    }
    if (zx.on) {
        zx.on_ascii(ascii);
    }
    if (kc85.on) {
        kc85.on_ascii(ascii);
    }
    if (atom.on) {
        atom.on_ascii(ascii);
    }
    if (cpc.on) {
        cpc.on_ascii(ascii);
    }
    if (c64.on) {
        c64.on_ascii(ascii);
    }
}

//------------------------------------------------------------------------------
void
yakc::on_key_down(uint8_t key) {
    if (z1013.on) {
        z1013.on_key_down(key);
    }
    if (z9001.on) {
        z9001.on_key_down(key);
    }
    if (zx.on) {
        zx.on_key_down(key);
    }
    if (kc85.on) {
        kc85.on_key_down(key);
    }
    if (atom.on) {
        atom.on_key_down(key);
    }
    if (cpc.on) {
        cpc.on_key_down(key);
    }
    if (c64.on) {
        c64.on_key_down(key);
    }
}

//------------------------------------------------------------------------------
void
yakc::on_key_up(uint8_t key) {
    if (z1013.on) {
        z1013.on_key_up(key);
    }
    if (z9001.on) {
        z9001.on_key_up(key);
    }
    if (zx.on) {
        zx.on_key_up(key);
    }
    if (kc85.on) {
        kc85.on_key_up(key);
    }
    if (atom.on) {
        atom.on_key_up(key);
    }
    if (cpc.on) {
        cpc.on_key_up(key);
    }
    if (c64.on) {
        c64.on_key_up(key);
    }
}

//------------------------------------------------------------------------------
void
yakc::on_joystick(uint8_t joy0_kbd_mask, uint8_t joy0_pad_mask) {
    if (!this->joystick_enabled) {
        joy0_kbd_mask = 0;
    }
    const uint8_t joy0_mask = joy0_kbd_mask|joy0_pad_mask;
    if (zx.on) {
        zx.on_joystick(joy0_mask);
    }
    if (atom.on) {
        atom.on_joystick(joy0_mask);
    }
    if (cpc.on) {
        cpc.on_joystick(joy0_mask);
    }
    if (c64.on) {
        c64.on_joystick(joy0_mask);
    }
}

//------------------------------------------------------------------------------
void
yakc::enable_joystick(bool b) {
    this->joystick_enabled = b;
}

//------------------------------------------------------------------------------
bool
yakc::is_joystick_enabled() const {
    return this->joystick_enabled;
}

//------------------------------------------------------------------------------
int
yakc::num_joysticks() const {
    if (z1013.on) {
        return z1013.num_joysticks();
    }
    else if (z9001.on) {
        return z9001.num_joysticks();
    }
    else if (zx.on) {
        return zx.num_joysticks();
    }
    else if (kc85.on) {
        return kc85.num_joysticks();
    }
    else if (atom.on) {
        return atom.num_joysticks();
    }
    else if (cpc.on) {
        return cpc.num_joysticks();
    }
    else if (c64.on) {
        return c64.num_joysticks();
    }
    else {
        return 0;
    }
}

//------------------------------------------------------------------------------
const char*
yakc::system_info() const {
    if (z1013.on) {
        return z1013.system_info();
    }
    else if (z9001.on) {
        return z9001.system_info();
    }
    else if (zx.on) {
        return zx.system_info();
    }
    else if (kc85.on) {
        return kc85.system_info();
    }
    else if (atom.on) {
        return atom.system_info();
    }
    else if (cpc.on) {
        return cpc.system_info();
    }
    else if (c64.on) {
        return c64.system_info();
    }
    else {
        return "no info available";
    }
}

//------------------------------------------------------------------------------
void
yakc::fill_sound_samples(float* buffer, int num_samples) {
    if (!board.dbg.break_stopped()) {
        if (z9001.on) {
            return z9001.decode_audio(buffer, num_samples);
        }
        else if (zx.on) {
            return zx.decode_audio(buffer, num_samples);
        }
        if (kc85.on) {
            return kc85.decode_audio(buffer, num_samples);
        }
        else if (atom.on) {
            return atom.decode_audio(buffer, num_samples);
        }
        else if (cpc.on) {
            return cpc.decode_audio(buffer, num_samples);
        }
        else if (c64.on) {
            return c64.decode_audio(buffer, num_samples);
        }
    }
    // fallthrough: all systems off, or debugging active: return silence
    clear(buffer, num_samples * sizeof(float));
}

//------------------------------------------------------------------------------
const void*
yakc::framebuffer(int& out_width, int& out_height) {
    if (z1013.on) {
        return z1013.framebuffer(out_width, out_height);
    }
    else if (z9001.on) {
        return z9001.framebuffer(out_width, out_height);
    }
    else if (zx.on) {
        return zx.framebuffer(out_width, out_height);
    }
    else if (kc85.on) {
        return kc85.framebuffer(out_width, out_height);
    }
    else if (atom.on) {
        return atom.framebuffer(out_width, out_height);
    }
    else if (cpc.on) {
        return cpc.framebuffer(out_width, out_height);
    }
    else if (c64.on) {
        return c64.framebuffer(out_width, out_height);
    }
    else {
        out_width = 0;
        out_height = 0;
        return nullptr;
    }
}

//------------------------------------------------------------------------------
const char*
yakc::load_tape_cmd() {
    if (cpc.on) {
        return "|tape\nrun\"\n\n";
    }
    else if (atom.on) {
        return "*LOAD\n\n";
    }
    else if (c64.on) {
        return "LOAD\n";
    }
    else {
        return nullptr;
    }
}

//------------------------------------------------------------------------------
bool
yakc::quickload(const char* name, filetype type, bool start) {
    bool retval = false;
    if (z1013.on) {
        retval = z1013.quickload(&this->filesystem, name, type, start);
    }
    else if (z9001.on) {
        retval = z9001.quickload(&this->filesystem, name, type, start);
    }
    else if (zx.on) {
        retval = zx.quickload(&this->filesystem, name, type, start);
    }
    else if (kc85.on) {
        retval = kc85.quickload(&this->filesystem, name, type, start);
    }
    else if (cpc.on) {
        retval = cpc.quickload(&this->filesystem, name, type, start);
    }
    else if (c64.on) {
        retval = c64.quickload(&this->filesystem, name, type, start);
    }
    else if (atom.on) {
        retval = atom.quickload(&this->filesystem, name, type, start);
    }
    else {
        retval = false;
    }
    return retval;
}

} // namespace YAKC
