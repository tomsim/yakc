#pragma once
//------------------------------------------------------------------------------
/**
    @class PIOWindow
    @brief visualize the current PIO state
*/
#include "ui/HexInputWidget.h"
#include "ui/WindowBase.h"

class PIOWindow : public WindowBase {
    OryolClassDecl(PIOWindow);
public:
    /// setup the window
    virtual void Setup(yakc::kc85& kc) override;
    /// draw method
    virtual bool Draw(yakc::kc85& kc) override;

    HexInputWidget pioAData;
    HexInputWidget pioBData;
};