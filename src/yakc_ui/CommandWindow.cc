//------------------------------------------------------------------------------
//  CommandWindow.cc
//------------------------------------------------------------------------------
#include "CommandWindow.h"
#include "IMUI/IMUI.h"
#include "Core/String/StringBuilder.h"
#include "UI.h"
#include "Util.h"
#include "yakc/util/breadboard.h"
#include <ctype.h>

using namespace Oryol;

namespace YAKC {

//------------------------------------------------------------------------------
void
CommandWindow::Setup(yakc& emu) {
    this->setName("Find Commands");
}

//------------------------------------------------------------------------------
bool
CommandWindow::Draw(yakc& emu) {
    ImGui::SetNextWindowSize(ImVec2(200, 250), ImGuiSetCond_Once);
    if (ImGui::Begin(this->title.AsCStr(), &this->Visible)) {
        Util::InputHex8("Prolog Byte", this->prologByte);
        ImGui::SameLine();
        if (ImGui::Button("Scan...")) {
            this->scan(this->prologByte);
        }
        for (int i = 0; i < this->commands.Size(); i++) {
            const Cmd& cmd = this->commands[i];
            if (board.dbg.is_breakpoint(cmd.addr)) {
                ImGui::PushStyleColor(ImGuiCol_Text, UI::EnabledBreakpointColor);
            }
            else {
                ImGui::PushStyleColor(ImGuiCol_Text, UI::DisabledBreakpointColor);
            }
            ImGui::PushID(i);
            if (ImGui::Button(" B ")) {
                board.dbg.toggle_breakpoint(cmd.addr);
            }
            ImGui::PopID();
            ImGui::SameLine();
            ImGui::Text("0x%04X %s", cmd.addr, cmd.name.AsCStr());
            ImGui::PopStyleColor();
        }
    }
    ImGui::End();
    return this->Visible;
}

//------------------------------------------------------------------------------
void
CommandWindow::scan(uint8_t prologByte) {
    StringBuilder strBuilder;

    this->commands.Clear();
    if (board.mem) {
        uint8_t prevByte = mem_rd(board.mem, 0x0000);
        for (unsigned int addr = 0x0001; addr < 0x10000; addr++) {
            const uint8_t curByte = mem_rd(board.mem, addr);
            if ((curByte == prologByte) && (prevByte == prologByte)) {
                // found a header, scan for 00 or 01 byte
                addr++;
                uint8_t c;
                while (isalnum(c = mem_rd(board.mem, addr++))) {
                    strBuilder.Append(c);
                }
                // if it was a valid command, add it to commands array
                if ((c == 0) || (c == 1)) {
                    this->commands.Add(strBuilder.GetString(), addr);
                }
                strBuilder.Clear();
            }
            prevByte = curByte;
        }
    }
}

} // namespace YAKC

