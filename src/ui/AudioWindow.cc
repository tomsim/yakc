//------------------------------------------------------------------------------
//  AudioWindow.cc
//------------------------------------------------------------------------------
#include "AudioWindow.h"
#include "IMUI/IMUI.h"
#include "soloud.h"
#include "yakc/Audio.h"
#include "ui/UI.h"

using namespace Oryol;
using namespace yakc;

//------------------------------------------------------------------------------
AudioWindow::AudioWindow(Audio* audio_) :
audio(audio_),
paused(false),
cpuAhead(false),
cpuBehind(false) {
    Memory::Clear(&this->wavBuffer, sizeof(this->wavBuffer));
}

//------------------------------------------------------------------------------
void
AudioWindow::Setup(kc85& kc) {
    this->setName("Audio Debugger");
}

//------------------------------------------------------------------------------
bool
AudioWindow::Draw(kc85& kc) {
    ImGui::SetNextWindowSize(ImVec2(600, 220), ImGuiSetCond_Once);
    if (ImGui::Begin(this->title.AsCStr(), &this->Visible, ImGuiWindowFlags_ShowBorders)) {
        this->audio->soloud.setVisualizationEnable(true);
        ImGui::Checkbox("Pause", &this->paused);
        if (!this->paused) {
            Memory::Copy(this->audio->soloud.getWave(), this->wavBuffer, 256*sizeof(float));
            this->cpuAhead = kc.cpu_ahead;
            this->cpuBehind = kc.cpu_behind;
        }
        ImGui::Text("Backend: %s", this->audio->soloud.getBackendString());
        ImGui::Text("Backend sample rate: source=%d, actual=%d\n",  this->audio->audioSource.sample_rate, 
            this->audio->soloud.getBackendSamplerate());
        ImGui::Text("Backend sample buffer size: %d\n", this->audio->soloud.getBackendBufferSize());
        if (this->cpuAhead) {
            ImGui::TextColored(UI::WarnColor, "*** CPU AHEAD ***");
        }
        else if (this->cpuBehind) {
            ImGui::TextColored(UI::WarnColor, "*** CPU BEHIND ***");
        }
        else {
            ImGui::TextColored(UI::OkColor, "CPU SYNCED");
        }
        for (int chn=0; chn<2; chn++) {
            if (this->audio->audioSource.channels[chn].overflow) {
                ImGui::TextColored(UI::WarnColor, "*** CHANNEL %d: RINGBUFFER OVERFLOW***", chn);
            }
            else {
                ImGui::TextColored(UI::OkColor, "Channel %d: ok", chn);
            }
        }
        ImGui::PlotLines("Wave", this->wavBuffer, 256, 0, nullptr, -1, 1, ImVec2(512, 60));
    }
    else {
        this->audio->soloud.setVisualizationEnable(false);
    }
    ImGui::End();
    return this->Visible;
}
