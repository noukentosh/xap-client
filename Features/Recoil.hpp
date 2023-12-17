#pragma once
#include <iostream>
#include <vector>
#include "../Core/Player.hpp"
#include "../Core/LocalPlayer.hpp"
#include "../Core/Offsets.hpp"

#include "../Math/Vector2D.hpp"
#include "../Math/Vector3D.hpp"
#include "../Math/QAngle.hpp"
#include "../Math/Resolver.hpp"

#include "../Utils/Memory.hpp"
#include "../Utils/XDisplay.hpp"
#include "../Utils/Conversion.hpp"
#include "../Utils/Config.hpp"
#include "../Utils/HitboxType.hpp"

// UI //
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_glfw.h"
#include "../imgui/imgui_impl_opengl3.h"

// Conversion
#define DEG2RAD( x  )  ( (float)(x) * (float)(M_PI / 180.f) )

struct Recoil {
    bool RecoilEnabled = true;
    float PitchPower = 3;
    float YawPower = 3;

    XDisplay* X11Display;
    LocalPlayer* Myself;
    std::vector<Player*>* Players;

    QAngle RCSLastPunch;

    Recoil(XDisplay* X11Display, LocalPlayer* Myself, std::vector<Player*>* GamePlayers) {
        this->X11Display = X11Display;
        this->Myself = Myself;
        this->Players = GamePlayers;
    }

    void RenderUI() {
        if (ImGui::BeginTabItem("Recoil", nullptr, ImGuiTabItemFlags_NoCloseWithMiddleMouseButton | ImGuiTabItemFlags_NoReorder)) {
            ImGui::Checkbox("Recoil Control", &RecoilEnabled);
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                ImGui::SetTooltip("Reduce the intensity of weapon's recoil.");
            ImGui::SliderFloat("Pitch", &PitchPower, 0, 10, "%.1f");
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                ImGui::SetTooltip("Pitch Power");
            ImGui::SliderFloat("Yaw", &YawPower, 0, 10, "%.1f");
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                ImGui::SetTooltip("Yaw Power");

            ImGui::EndTabItem();
        }
    }

    bool Save() {
        try {
            Config::Aimbot::RecoilControl = RecoilEnabled;
            Config::Aimbot::PitchPower = PitchPower;
            Config::Aimbot::YawPower = YawPower;
            return true;
        } catch (...) {
            return false;
        }
    }

    void Update() {
        if(RecoilEnabled && Myself->IsInAttack) {
            StartAiming();
        } else {
            RCSLastPunch = QAngle(0, 0);
        }
    }

    void StartAiming() {
        // Get Target Angle
        QAngle DesiredAngles = QAngle(0, 0);

        // Recoil Control
        RecoilControl(DesiredAngles);

        // Aim angles
        Vector2D aimbotDelta = Vector2D(DesiredAngles.x, DesiredAngles.y);

        int mouseDX = RoundHalfEven(AL1AF0(aimbotDelta.y * Config::dpiY * 10.f));
        int mouseDY = RoundHalfEven(AL1AF0(aimbotDelta.x * -1.f * Config::dpiX * 10.f));

        // Move Mouse
        if (mouseDY == 0 && mouseDX == 0) return;
        
        X11Display->MoveMouse(-mouseDY, -mouseDX);
    }

    int RoundHalfEven(float x) {
        return (x >= 0.0)
            ? static_cast<int>(std::round(x))
            : static_cast<int>(std::round(-x)) * -1;
    }

    float AL1AF0(float num) {
        if (num > 0) return std::max(num, 1.0f);
        return std::min(num, -1.0f);
    }

    void RecoilControl(QAngle& Angle) {
        QAngle CurrentPunch = QAngle(Myself->PunchAngles.x, Myself->PunchAngles.y).NormalizeAngles();
        QAngle NewPunch = { CurrentPunch.x - RCSLastPunch.x, CurrentPunch.y - RCSLastPunch.y };

		Angle.x -= NewPunch.x * (YawPower / 10.f);
		Angle.y -= NewPunch.y * (PitchPower / 10.f);

        RCSLastPunch = CurrentPunch;
    }
};