#pragma once
#include "imgui/imgui.h"

ImGuiKey ImGui_ImplUwp_VirtualKeyToImGuiKey(int key);
int ImGui_ImplUwp_GetSpecialUppercase(int key);
int ImGui_ImplUwp_GetSpecialCharacter(Windows::System::VirtualKey key);