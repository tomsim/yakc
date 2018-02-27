#pragma once
//------------------------------------------------------------------------------
/**
    @class YAKC::M6567Window
    @brief show debug info about the m6567 VIC-II chip
*/
#include "yakc_ui/WindowBase.h"
#include "IMUI/IMUI.h"

namespace YAKC {

class M6567Window : public WindowBase {
    OryolClassDecl(M6567Window);
public:
    /// setup the window
    virtual void Setup(yakc& emu) override;
    /// draw method
    virtual bool Draw(yakc& emu) override;

    void drawColor(const char* text, uint8_t palIndex);

    ImVec4 paletteColors[16];
    bool badline = false;
};

} // namespace YAKC
