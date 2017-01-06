#pragma once
//------------------------------------------------------------------------------
/**
    @class YAKC::counter
    
    A counter that triggers every N ticks, keeping the remainder for
    the next update.
*/
#include "yakc/core/core.h"

namespace YAKC {

class counter {
public:
    /// initialize the counter with a period in ticks
    void init(int period);
    /// reset the counter
    void reset();
    /// update the internal counter, after this call step()
    void update(int ticks);
    /// after update, call step() until it returns false
    bool step();
    
    int period = 0;
    int value = 0;
};

//------------------------------------------------------------------------------
inline void
counter::init(int p) {
    this->period = p;
    this->value = 0;
}

//------------------------------------------------------------------------------
inline void
counter::reset() {
    this->value = 0;
}

//------------------------------------------------------------------------------
inline void
counter::update(int ticks) {
    this->value -= ticks;
}

//------------------------------------------------------------------------------
inline bool
counter::step() {
    if (this->value < 0) {
        this->value += this->period;
        return true;
    }
    else {
        return false;
    }
}


} // namespace YAKC

    

