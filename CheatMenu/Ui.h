#pragma once
#include "pch.h"

class Ui
{
private:
	inline static std::string m_CurrentHotkey;

public:
	struct NamedMemory
	{
		std::string name;
		int addr;
	};

	struct NamedValue
	{
		std::string name;
		int value;
	};

	struct JsonPopUpData
	{
		std::function<void(std::string&, std::string&, std::string&)> function;
		std::string key;
		std::string rootKey;
		std::string value;
	};

	struct ImgPopUpData
	{
		std::function<void(std::string&)> function;
		std::string value;
	};

	inline static JsonPopUpData jsonPopup;
	inline static ImgPopUpData imgPopup;

	static void CenterdText(const std::string& text);
	static bool ColorButton(int color_id, std::vector<float>& color, ImVec2 size);
	static bool CheckboxAddress(const char* label, int addr = NULL, const char* hint = nullptr);
	static bool CheckboxAddressEx(const char* label, int addr = NULL, int enabled_val = 1, int disabled_val = 0,
	                              const char* hint = nullptr);
	static bool CheckboxAddressVar(const char* label, bool val, int addr, const char* hint = nullptr);
	static bool CheckboxAddressVarEx(const char* label, bool val, int addr, int enabled_val, int disabled_val,
	                                 const char* hint = nullptr);
	static bool CheckboxBitFlag(const char* label, uint flag, const char* hint = nullptr);
	static bool CheckboxWithHint(const char* label, bool* state, const char* hint = nullptr, bool is_disabled = false);
	static void DrawHeaders(CallbackTable& data);

	static void DrawJSON(CJson& json, std::vector<std::string>& combo_items, std::string& selected_item,
	                     ImGuiTextFilter& filter,
	                     std::function<void(std::string&, std::string&, std::string&)> func_left_click,
	                     std::function<void(std::string&, std::string&, std::string&)> func_right_click);
	static void DrawImages(std::vector<std::unique_ptr<STextureStructure>>& img_vec, ImVec2 image_size,
	                       std::vector<std::string>& category_vec, std::string& selected_item, ImGuiTextFilter& filter,
	                       std::function<void(std::string&)> on_left_click,
	                       std::function<void(std::string&)> on_right_click,
	                       std::function<std::string(std::string&)> get_name_func,
	                       std::function<bool(std::string&)> verify_func = nullptr);

	template <typename T>
	static void EditAddress(const char* label, int address, int min = 0, int def = 0, int max = 100);
	static void EditBits(const char* label, int address, const std::vector<std::string>& names);
	static void EditFloat(const char* label, int address, float min, float def, float max, float mul = 1);
	template <typename T>
	static void EditReference(const char* label, T& address, int min = 0, int def = 0, int max = 100);
	static void EditRadioButtonAddress(const char* label, std::vector<NamedMemory>& named_mem);
	static void EditRadioButtonAddressEx(const char* label, int addr, std::vector<NamedValue>& named_val);
	static void EditStat(const char* label, int stat_id, int min = 0, int def = 0, int max = 1000);

	static void FilterWithHint(const char* label, ImGuiTextFilter& filter, const char* hint);

	static ImVec2 GetSize(short count = 1, bool spacing = true);

	static bool HotKey(const char* label, HotKeyData& key_data);
	static bool HotKeyPressed(HotKeyData& hotkey);
	static std::string GetHotKeyNameString(HotKeyData& hotkey);

	static bool ListBox(const char* label, std::vector<std::string>& all_items, int& selected);
	static bool ListBoxStr(const char* label, std::vector<std::string>& all_items, std::string& selected);

	static void RadioButtonAddress(const char* label, std::vector<NamedMemory>& named_mem);
	static void RadioButtonAddressEx(const char* label, int addr, std::vector<NamedValue>& named_val);

	static void ColorPickerAddress(const char* label, int base_addr, ImVec4&& default_color);
	static void ShowTooltip(const char* text);
};

template <typename T>
void Ui::EditAddress(const char* label, const int address, const int min, const int def, const int max)
{
	if (ImGui::CollapsingHeader(label))
	{
		int val = patch::Get<T>(address, false);

		int items = 3;

		if (min == def)
			items = 2;

		ImGui::Columns(items, nullptr, false);
		ImGui::Text(("Min: " + std::to_string(min)).c_str());

		if (items == 3)
		{
			ImGui::NextColumn();
			ImGui::Text(("Def: " + std::to_string(def)).c_str());
		}

		ImGui::NextColumn();
		ImGui::Text(("Max: " + std::to_string(max)).c_str());
		ImGui::Columns(1);

		ImGui::Spacing();

		if (ImGui::InputInt(("Set value##" + std::string(label)).c_str(), &val))
			patch::Set<T>(address, val, false);

		ImGui::Spacing();

		if (val < min)
			val = min;

		if (val > max)
			val = max;

		if (ImGui::Button(("Minimum##" + std::string(label)).c_str(), GetSize(items)))
			patch::Set<T>(address, min, false);

		if (items == 3)
		{
			ImGui::SameLine();

			if (ImGui::Button(("Default##" + std::string(label)).c_str(), GetSize(3)))
				patch::Set<T>(address, def, false);
		}

		ImGui::SameLine();

		if (ImGui::Button(("Maximum##" + std::string(label)).c_str(), GetSize(items)))
			patch::Set<T>(address, max, false);

		ImGui::Spacing();
		ImGui::Separator();
	}
}

template <typename T>
void Ui::EditReference(const char* label, T& address, const int min, const int def, const int max)
{
	if (ImGui::CollapsingHeader(label))
	{
		int val = static_cast<int>(address);

		ImGui::Columns(3, nullptr, false);
		ImGui::Text(("Min: " + std::to_string(min)).c_str());
		ImGui::NextColumn();
		ImGui::Text(("Def: " + std::to_string(def)).c_str());
		ImGui::NextColumn();
		ImGui::Text(("Max: " + std::to_string(max)).c_str());
		ImGui::Columns(1);

		ImGui::Spacing();

		if (ImGui::InputInt(("Set value##" + std::string(label)).c_str(), &val))
			address = static_cast<float>(val);

		ImGui::Spacing();

		if (ImGui::Button(("Minimum##" + std::string(label)).c_str(), GetSize(3)))
			address = static_cast<float>(min);

		ImGui::SameLine();

		if (ImGui::Button(("Default##" + std::string(label)).c_str(), GetSize(3)))
			address = static_cast<float>(def);

		ImGui::SameLine();

		if (ImGui::Button(("Maximum##" + std::string(label)).c_str(), GetSize(3)))
			address = static_cast<float>(max);

		ImGui::Spacing();
		ImGui::Separator();
	}
}
