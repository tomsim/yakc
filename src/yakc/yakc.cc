//------------------------------------------------------------------------------
//  yakc.cc
//------------------------------------------------------------------------------
#include "yakc.h"
#include <stdio.h>

namespace YAKC {

//------------------------------------------------------------------------------
void
yakc::init(const ext_funcs& sys_funcs) {
    func = sys_funcs;
    fill_random(board.random, sizeof(board.random));
    this->cpu_ahead = false;
    this->cpu_behind = false;
    this->abs_cycle_count = 0;
    this->overflow_cycles = 0;
/*
    this->cpc.init(&this->board, &this->roms, &this->tapedeck);
    this->bbcmicro.init(&this->board, &this->roms);
*/
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
        return z1013_t::check_roms(m, os);
    }
    else if (is_system(m, system::any_z9001)) {
        return z9001_t::check_roms(m, os);
    }
    else if (is_system(m, system::any_zx)) {
        return zx_t::check_roms(m, os);
    }
    else if (is_system(m, system::any_kc85)) {
        return kc85_t::check_roms(m, os);
    }
    else if (is_system(m, system::acorn_atom)) {
        return atom_t::check_roms(m, os);
    }
    /*
    else if (is_system(m, system::any_cpc)) {
        return cpc::check_roms(m, os);
    }
    else if (is_system(m, system::bbcmicro_b)) {
        return bbcmicro::check_roms(m, os);
    }
    */
    else {
        return false;
    }
}

//------------------------------------------------------------------------------
void
yakc::poweron(system m, os_rom rom) {
    this->model = m;
    this->os = rom;
    this->abs_cycle_count = 0;
    this->overflow_cycles = 0;
    if (this->is_system(system::any_z1013)) {
        z1013.poweron(m);
    }
    else if (this->is_system(system::any_z9001)) {
        z9001.poweron(m, rom);
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
    /*
    else if (this->is_system(system::any_cpc)) {
        this->cpc.poweron(m);
    }
    else if (this->is_system(system::bbcmicro_b)) {
        this->bbcmicro.poweron(m);
    }
    */
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
    /*
    if (this->cpc.on) {
        this->cpc.poweroff();
    }
    if (this->bbcmicro.on) {
        this->bbcmicro.poweroff();
    }
    */
}

//------------------------------------------------------------------------------
bool
yakc::switchedon() const {
    return z1013.on || z9001.on | zx.on | kc85.on | atom.on;
/*
    return this->kc85.on || this->z1013.on || this->z9001.on ||
           this->zx.on || this->cpc.on || this->bbcmicro.on ||
           this->atom.on;
*/
}

//------------------------------------------------------------------------------
void
yakc::reset() {
    this->overflow_cycles = 0;
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
    /*
    if (this->cpc.on) {
        this->cpc.reset();
    }
    if (this->bbcmicro.on) {
        this->bbcmicro.reset();
    }
    */
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
    if (this->is_system(system::bbcmicro_b) || this->is_system(system::acorn_atom)) {
        return cpu_model::mos6502;
    }
    else {
        return cpu_model::z80;
    }
}

//------------------------------------------------------------------------------
void
yakc::step(int micro_secs, uint64_t audio_cycle_count) {
    uint64_t min_cycle_count = 0;
    uint64_t max_cycle_count = 0;
    if (audio_cycle_count > 0) {
        const uint64_t cpu_min_ahead_cycles = board.freq_hz/100;
        const uint64_t cpu_max_ahead_cycles = board.freq_hz/25;
        min_cycle_count = audio_cycle_count + cpu_min_ahead_cycles;
        max_cycle_count = audio_cycle_count + cpu_max_ahead_cycles;
    }
    this->cpu_ahead = false;
    this->cpu_behind = false;
    // compute the end-cycle-count for the current frame
    if (this->abs_cycle_count == 0) {
        this->abs_cycle_count = min_cycle_count;
    }
    int64_t num_cycles = clk_ticks(board.freq_hz, micro_secs) - this->overflow_cycles;
    if (num_cycles < 0) {
        num_cycles = 0;
    }
    uint64_t abs_end_cycles = this->abs_cycle_count + num_cycles;
    if ((max_cycle_count != 0) && (abs_end_cycles > max_cycle_count)) {
        abs_end_cycles = max_cycle_count;
        this->cpu_ahead = true;
    }
    else if ((min_cycle_count != 0) && (abs_end_cycles < min_cycle_count)) {
        abs_end_cycles = min_cycle_count;
        this->cpu_behind = true;
    }
    if (this->abs_cycle_count > abs_end_cycles) {
        this->abs_cycle_count = abs_end_cycles;
    }
    if (!board.dbg.active) {
        if (z1013.on) {
            this->abs_cycle_count = z1013.step(this->abs_cycle_count, abs_end_cycles);
        }
        else if (z9001.on) {
            this->abs_cycle_count = z9001.step(this->abs_cycle_count, abs_end_cycles);
        }
        else if (zx.on) {
            this->abs_cycle_count = zx.step(this->abs_cycle_count, abs_end_cycles);
        }
        else if (kc85.on) {
            this->abs_cycle_count = kc85.step(this->abs_cycle_count, abs_end_cycles);
        }
        else if (atom.on) {
            this->abs_cycle_count = atom.step(this->abs_cycle_count, abs_end_cycles);
        }
        /*
        else if (this->cpc.on) {
            this->abs_cycle_count = this->cpc.step(this->abs_cycle_count, abs_end_cycles);
        }
        else if (this->bbcmicro.on) {
            this->abs_cycle_count = this->bbcmicro.step(this->abs_cycle_count, abs_end_cycles);
        }
        */
        else {
            this->abs_cycle_count = abs_end_cycles;
        }
        YAKC_ASSERT(this->abs_cycle_count >= abs_end_cycles);
        this->overflow_cycles = uint32_t(this->abs_cycle_count - abs_end_cycles);
    }
    else {
        this->abs_cycle_count = abs_end_cycles;
        this->overflow_cycles = 0;
    }
}

//------------------------------------------------------------------------------
uint32_t
yakc::step_debug() {
    if (z1013.on) {
        return z1013.step_debug();
    }
    else if (z9001.on) {
        return z9001.step_debug();
    }
    else if (zx.on) {
        return zx.step_debug();
    }
    else if (kc85.on) {
        return kc85.step_debug();
    }
    else if (atom.on) {
        return atom.step_debug();
    }
    /*
    else if (this->cpc.on) {
        return this->cpc.step_debug();
    }
    else if (this->bbcmicro.on) {
        return this->bbcmicro.step_debug();
    }
    */
    else {
        return 0;
    }
}

//------------------------------------------------------------------------------
void
yakc::put_input(uint8_t ascii, uint8_t joy0_kbd_mask, uint8_t joy0_pad_mask) {
    if (!this->joystick_enabled) {
        joy0_kbd_mask = 0;
    }
    const uint8_t joy0_mask = joy0_kbd_mask|joy0_pad_mask;
    if (z1013.on) {
        z1013.put_key(ascii);
    }
    if (z9001.on) {
        z9001.put_key(ascii);
    }
    if (zx.on) {
        zx.put_input(ascii, joy0_mask);
    }
    if (kc85.on) {
        kc85.put_key(ascii);
    }
    if (atom.on) {
        atom.put_input(ascii, joy0_mask);
    }
    /*
    if (this->cpc.on) {
        this->cpc.put_input(ascii, joy0_mask);
    }
    */
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
    /*
    else if (this->cpc.on) {
        return this->cpc.system_info();
    }
    else if (this->bbcmicro.on) {
        return this->bbcmicro.system_info();
    }
    */
    else {
        return "no info available";
    }
}

//------------------------------------------------------------------------------
void
yakc::fill_sound_samples(float* buffer, int num_samples) {
    if (!board.dbg.active) {
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
    /*
        else if (this->cpc.on) {
            return this->cpc.decode_audio(buffer, num_samples);
        }
    */
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
    /*
    else if (this->cpc.on) {
        return this->cpc.framebuffer(out_width, out_height);
    }
    else if (this->bbcmicro.on) {
        return this->bbcmicro.framebuffer(out_width, out_height);
    }
    */
    else {
        out_width = 0;
        out_height = 0;
        return nullptr;
    }
}

//------------------------------------------------------------------------------
bool
yakc::quickload(const char* name, filetype type, bool start) {
    if (z1013.on) {
        return z1013.quickload(&this->filesystem, name, type, start);
    }
    else if (z9001.on) {
        return z9001.quickload(&this->filesystem, name, type, start);
    }
    else if (zx.on) {
        return zx.quickload(&this->filesystem, name, type, start);
    }
    else if (kc85.on) {
        return kc85.quickload(&this->filesystem, name, type, start);
    }
    /*
    else if (this->cpc.on) {
        return this->cpc.quickload(&this->filesystem, name, type, start);
    }
    */
    else {
        return false;
    }
}

} // namespace YAKC
