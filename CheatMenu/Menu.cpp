#include "pch.h"
#include "MenuInfo.h"
#include "Menu.h"
#include "Teleport.h"
#include "Weapon.h"
#include "Vehicle.h"
#include "Ui.h"
#include "Util.h"
#include "Updater.h"

Menu::Menu()
{
	// TODO: use structs
	// Load config data
	overlay::bCoord = config.GetValue("overlay.bCoord", false);
	overlay::bCpuUsage = config.GetValue("overlay.bCpuUsage", false);
	overlay::bFPS = config.GetValue("overlay.bFPS", false);
	overlay::bLocName = config.GetValue("overlay.bLocName", false);
	overlay::bTransparent = config.GetValue("overlay.bTransparent", false);
	overlay::bMemUsage = config.GetValue("overlay.bMemUsage", false);
	overlay::bVehHealth = config.GetValue("overlay.bVehHealth", false);
	overlay::bVehSpeed = config.GetValue("overlay.bVehSpeed", false);
	overlay::mSelectedPos = config.GetValue("overlay.mSelectedPos", 4);
	overlay::fPosX = config.GetValue("overlay.fPosX", 0);
	overlay::fPosY = config.GetValue("overlay.fPosY", 0);

	// Hotkeys
	hotkeys::aim_skin_changer.key1 = config.GetValue("hotkey.aim_skin_changer.key1", VK_RETURN);
	hotkeys::aim_skin_changer.key2 = config.GetValue("hotkey.aim_skin_changer.key2", VK_RETURN);

	hotkeys::freecam.key1 = config.GetValue("hotkey.freecam.key1", VK_F6);
	hotkeys::freecam.key2 = config.GetValue("hotkey.freecam.key2", VK_F6);

	hotkeys::quick_ss.key1 = config.GetValue("hotkey.quick_screenshot.key1", VK_F5);
	hotkeys::quick_ss.key2 = config.GetValue("hotkey.quick_screenshot.key2", VK_F5);

	hotkeys::quick_tp.key1 = config.GetValue("hotkey.quick_tp.key1", VK_KEY_X);
	hotkeys::quick_tp.key2 = config.GetValue("hotkey.quick_tp.key2", VK_KEY_Y);

	hotkeys::menu_open.key1 = config.GetValue("hotkey.menu_open.key1", VK_LCONTROL);
	hotkeys::menu_open.key2 = config.GetValue("hotkey.menu_open.key2", VK_KEY_M);

	hotkeys::command_window.key1 = config.GetValue("hotkey.command_window.key1", VK_LMENU);
	hotkeys::command_window.key2 = config.GetValue("hotkey.command_window.key2", VK_KEY_C);

	hotkeys::flip_veh.key1 = config.GetValue("hotkey.flip_veh.key1", VK_NONE);
	hotkeys::flip_veh.key2 = config.GetValue("hotkey.flip_veh.key2", VK_NONE);

	hotkeys::fix_veh.key1 = config.GetValue("hotkey.fix_veh.key1", VK_NONE);
	hotkeys::fix_veh.key2 = config.GetValue("hotkey.fix_veh.key2", VK_NONE);

	hotkeys::god_mode.key1 = config.GetValue("hotkey.god_mode.key1", VK_NONE);
	hotkeys::god_mode.key2 = config.GetValue("hotkey.god_mode.key2", VK_NONE);

	hotkeys::veh_engine.key1 = config.GetValue("hotkey.veh_engine.key1", VK_NONE);
	hotkeys::veh_engine.key2 = config.GetValue("hotkey.veh_engine.key2", VK_NONE);

	hotkeys::veh_instant_start.key1 = config.GetValue("hotkey.veh_instant_start.key1", VK_NONE);
	hotkeys::veh_instant_start.key2 = config.GetValue("hotkey.veh_instant_start.key2", VK_NONE);

	hotkeys::veh_instant_stop.key1 = config.GetValue("hotkey.veh_instant_stop.key1", VK_NONE);
	hotkeys::veh_instant_stop.key2 = config.GetValue("hotkey.veh_instant_stop.key2", VK_NONE);

	Util::GetCPUUsageInit();
	MEMORYSTATUSEX memInfo;
	memInfo.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&memInfo);

	overlay::mTotalRam = int(memInfo.ullTotalPhys * 1e-6); // Bytes -> MegaBytes
}

void Menu::DrawOverlay()
{
	CPlayerPed* player = FindPlayerPed();
	bool show_menu = overlay::bCoord || overlay::bFPS || overlay::bLocName ||
		((overlay::bVehHealth || overlay::bVehSpeed) && player->m_pVehicle && player->m_pVehicle->IsDriver(player));

	int corner = overlay::mSelectedPos - 1;
	const float offset = 10.0f;
	ImGuiIO& io = ImGui::GetIO();
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

	if (corner != -1)
	{
		window_flags |= ImGuiWindowFlags_NoMove;
		ImVec2 window_pos = ImVec2((corner & 1) ? io.DisplaySize.x - offset : offset, (corner & 2) ? io.DisplaySize.y - offset : offset);
		ImVec2 window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
		ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
	}
	else
	{
		if (overlay::fPosX != NULL && overlay::fPosY != NULL)
		{
			config.SetValue("overlay.fPosX", overlay::fPosX);
			config.SetValue("overlay.fPosY", overlay::fPosY);
			ImGui::SetNextWindowPos(ImVec2(overlay::fPosX, overlay::fPosY), ImGuiCond_Once);
		}
	}

	ImGui::SetNextWindowBgAlpha(overlay::bTransparent ? 0.0f : 0.5f);

	if (show_menu && ImGui::Begin("Overlay", NULL, window_flags))
	{
		CPlayerPed* player = FindPlayerPed();
		CVector pos = player->GetPosition();
		size_t game_ms = CTimer::m_snTimeInMilliseconds;

		if (game_ms - overlay::mLastInterval > overlay::mInterval)
		{
			overlay::fCpuUsage = (float)Util::GetCurrentCPUUsage();

			MEMORYSTATUSEX memInfo;
			memInfo.dwLength = sizeof(MEMORYSTATUSEX);
			GlobalMemoryStatusEx(&memInfo);
			int mUsedRam = int((memInfo.ullTotalPhys - memInfo.ullAvailPhys) * 1e-6);
			overlay::fMemUsage = 100.0f * (float(mUsedRam) / float(overlay::mTotalRam));
			overlay::mFPS = (size_t)CTimer::game_FPS;

			overlay::mLastInterval = game_ms;
		}

		if (overlay::bCoord)
			ImGui::Text("Coord: %.2f, %.2f, %.2f", pos.x, pos.y, pos.z);

		if (overlay::bCpuUsage)
			ImGui::Text("CPU usage: %.2f%%", overlay::fCpuUsage);

		if (overlay::bFPS)
			ImGui::Text("Frames: %d", overlay::mFPS);

		if (overlay::bLocName)
			ImGui::Text("Location: %s", Util::GetLocationName(&pos).c_str());

		if (overlay::bMemUsage)
			ImGui::Text("RAM usage: %.2f%%", overlay::fMemUsage);

		if (player->m_pVehicle && player->m_pVehicle->IsDriver(player))
		{
			if (overlay::bVehHealth)
				ImGui::Text("Veh Health: %.f", player->m_pVehicle->m_fHealth);

			if (overlay::bVehSpeed)
			{
				int speed = player->m_pVehicle->m_vecMoveSpeed.Magnitude() * 50; // 02E3 - GET_CAR_SPEED
				ImGui::Text("Veh Speed: %d", speed);
			}
		}

		ImVec2 windowPos = ImGui::GetWindowPos();
		overlay::fPosX = windowPos.x;
		overlay::fPosY = windowPos.y;

		ImGui::End();
	}
}

void Menu::DrawShortcutsWindow()
{
	int resX = int(screen::GetScreenWidth());
	int resY = int(screen::GetScreenHeight());

	ImGui::SetNextWindowPos(ImVec2(0, resY - 40), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(resX, 40));

	ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration + ImGuiWindowFlags_AlwaysAutoResize + ImGuiWindowFlags_NoSavedSettings
		+ ImGuiWindowFlags_NoMove;

	if (ImGui::Begin("Shortcuts window", NULL, flags))
	{
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(ImGui::GetStyle().FramePadding.x, resY / 130));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(3, 3));

		ImGui::SetNextItemWidth(resX);
		ImGui::SetKeyboardFocusHere(-1);

		if (ImGui::InputTextWithHint("##TEXTFIELD", "Enter command", commands::input_buffer, INPUT_BUFFER_SIZE, ImGuiInputTextFlags_EnterReturnsTrue))
		{
			ProcessCommands();
			commands::show_menu = false;
			strcpy(commands::input_buffer, "");
		}

		ImGui::PopStyleVar(2);
		ImGui::End();
	}
}

void Menu::ProcessCommands()
{
	std::stringstream ss(commands::input_buffer);

	std::string command;
	ss >> command;

	if (command == "hp")
	{
		try
		{
			std::string temp;
			ss >> temp;
			FindPlayerPed()->m_fHealth = std::stof(temp);
		}
		catch (...) {
			CHud::SetHelpMessage("Invalid value", false, false, false);
		}
	}

	if (command == "time")
	{
		try
		{
			std::string temp;
			ss >> temp;
			CClock::ms_nGameClockHours = std::stoi(temp);

			ss >> temp;
			CClock::ms_nGameClockMinutes = std::stoi(temp);
		}
		catch (...) {
			CHud::SetHelpMessage("Invalid value", false, false, false);
		}
	}

	if (command == "tp")
	{
		try {
			CVector pos;
			std::string temp;

			ss >> temp;
			pos.x = std::stof(temp);

			ss >> temp;
			pos.y = std::stof(temp);

			ss >> temp;
			pos.z = std::stof(temp);

			Teleport::TeleportPlayer(false, pos, 0);
		}
		catch (...) {
			CHud::SetHelpMessage("Invalid location", false, false, false);
		}
	}

	if (command == "wep")
	{
		std::string wep_name;
		ss >> wep_name;

		if (wep_name == "jetpack")
		{
			std::string weapon = "-1";
			Weapon::GiveWeaponToPlayer(weapon);
			CHud::SetHelpMessage("Weapon given", false, false, false);
		}
		else
		{
			eWeaponType weapon = CWeaponInfo::FindWeaponType((char*)wep_name.c_str());
			std::string weapon_name = std::to_string(weapon);
			CWeaponInfo* pweaponinfo = CWeaponInfo::GetWeaponInfo(weapon, 1);

			if (wep_name != "" && pweaponinfo->m_nModelId1 != -1)
			{
				Weapon::GiveWeaponToPlayer(weapon_name);
				CHud::SetHelpMessage("Weapon given", false, false, false);
			}
			else
				CHud::SetHelpMessage("Invalid command", false, false, false);
		}

		return;
	}
	if (command == "veh")
	{
		std::string veh_name;
		ss >> veh_name;

		int model = Vehicle::GetModelFromName(veh_name.c_str());
		if (model != 0)
		{
			std::string smodel = std::to_string(model);
			Vehicle::SpawnVehicle(smodel);
			CHud::SetHelpMessage("Vehicle spawned", false, false, false);
		}
		else
			CHud::SetHelpMessage("Invalid command", false, false, false);

		return;
	}
}

void Menu::Draw()
{
	if (ImGui::BeginTabBar("Menu", ImGuiTabBarFlags_NoTooltip + ImGuiTabBarFlags_FittingPolicyScroll))
	{
		if (ImGui::BeginTabItem("Overlay"))
		{
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::SameLine();
			if (Ui::ListBox("Overlay", overlay::posNames, overlay::mSelectedPos))
				config.SetValue("overlay.mSelectedPos", overlay::mSelectedPos);

			ImGui::Spacing();

			ImGui::Columns(2, NULL, false);
			if (ImGui::Checkbox("No background", &overlay::bTransparent))
				config.SetValue("overlay.bTransparent", overlay::bTransparent);

			if (ImGui::Checkbox("Show coordinates", &overlay::bCoord))
				config.SetValue("overlay.bCoord", overlay::bCoord);

			if (ImGui::Checkbox("Show CPU usage", &overlay::bCpuUsage))
				config.SetValue("overlay.bCpuUsage", overlay::bCpuUsage);

			if (ImGui::Checkbox("Show FPS", &overlay::bFPS))
				config.SetValue("overlay.bFPS", overlay::bFPS);

			ImGui::NextColumn();

			if (ImGui::Checkbox("Show location", &overlay::bLocName))
				config.SetValue("overlay.bLocName", overlay::bLocName);

			if (ImGui::Checkbox("Show RAM usage", &overlay::bMemUsage))
				config.SetValue("overlay.bMemUsage", overlay::bMemUsage);

			if (ImGui::Checkbox("Show veh health", &overlay::bVehHealth))
				config.SetValue("overlay.bVehHealth", overlay::bVehHealth);

			if (ImGui::Checkbox("Show veh speed", &overlay::bVehSpeed))
				config.SetValue("overlay.bVehSpeed", overlay::bVehSpeed);

			ImGui::Columns(1);

			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Hotkeys"))
		{
			ImGui::Spacing();
			ImGui::Text("Usage");
			Ui::ShowTooltip("Left-click selects hotkey.\nLeft clicking outside deselects."
"\nRight click disables hotkey.");
			ImGui::Spacing();
			ImGui::BeginChild("Hotkeys");
			if (Ui::HotKey("Open/ close cheat menu", hotkeys::menu_open))
			{
				config.SetValue("hotkey.menu_open.key1", hotkeys::menu_open.key1);
				config.SetValue("hotkey.menu_open.key2", hotkeys::menu_open.key2);
			}
			if (Ui::HotKey("Open/ close command window", hotkeys::command_window))
			{
				config.SetValue("hotkey.command_window.key1", hotkeys::command_window.key1);
				config.SetValue("hotkey.command_window.key2", hotkeys::command_window.key2);
			}

			ImGui::Dummy(ImVec2(0, 10));

			if (Ui::HotKey("Activate aim skin changer", hotkeys::aim_skin_changer))
			{
				config.SetValue("hotkey.aim_skin_changer.key1", hotkeys::aim_skin_changer.key1);
				config.SetValue("hotkey.aim_skin_changer.key2", hotkeys::aim_skin_changer.key2);
			}
			if (Ui::HotKey("Freecam", hotkeys::freecam))
			{
				config.SetValue("hotkey.freecam.key1", hotkeys::freecam.key1);
				config.SetValue("hotkey.freecam.key2", hotkeys::freecam.key2);
			}
			if (Ui::HotKey("Take quick screenshot", hotkeys::quick_ss))
			{
				config.SetValue("hotkey.quick_screenshot.key1", hotkeys::quick_ss.key1);
				config.SetValue("hotkey.quick_screenshot.key2", hotkeys::quick_ss.key2);
			}
			if (Ui::HotKey("Toggle quick teleport", hotkeys::quick_tp))
			{
				config.SetValue("hotkey.quick_tp.key1", hotkeys::quick_tp.key1);
				config.SetValue("hotkey.quick_tp.key2", hotkeys::quick_tp.key2);
			}

			ImGui::Dummy(ImVec2(0, 10));

			if (Ui::HotKey("Fix current vehicle", hotkeys::fix_veh))
			{
				config.SetValue("hotkey.fix_veh.key1", hotkeys::fix_veh.key1);
				config.SetValue("hotkey.fix_veh.key2", hotkeys::fix_veh.key2);
			}

			if (Ui::HotKey("Flip current vehicle", hotkeys::flip_veh))
			{
				config.SetValue("hotkey.flip_veh.key1", hotkeys::flip_veh.key1);
				config.SetValue("hotkey.flip_veh.key2", hotkeys::flip_veh.key2);
			}

			if (Ui::HotKey("Toggle god mode", hotkeys::god_mode))
			{
				config.SetValue("hotkey.god_mode.key1", hotkeys::god_mode.key1);
				config.SetValue("hotkey.god_mode.key2", hotkeys::god_mode.key2);
			}

			if (Ui::HotKey("Toggle veh engine", hotkeys::veh_engine))
			{
				config.SetValue("hotkey.veh_engine.key1", hotkeys::veh_engine.key1);
				config.SetValue("hotkey.veh_engine.key2", hotkeys::veh_engine.key2);
			}

			if (Ui::HotKey("Vehicle instant start", hotkeys::veh_instant_start))
			{
				config.SetValue("hotkey.veh_instant_start.key1", hotkeys::veh_instant_start.key1);
				config.SetValue("hotkey.veh_instant_start.key2", hotkeys::veh_instant_start.key2);
			}

			if (Ui::HotKey("Vehicle instant stop", hotkeys::veh_instant_stop))
			{
				config.SetValue("hotkey.veh_instant_stop.key1", hotkeys::veh_instant_stop.key1);
				config.SetValue("hotkey.veh_instant_stop.key2", hotkeys::veh_instant_stop.key2);
			}

			ImGui::Dummy(ImVec2(0, 10));

			ImGui::EndChild();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Commands"))
		{
			if (ImGui::BeginChild("CommandsChild"))
			{
				ImGui::TextWrapped(std::string("Open or close command window using " + Ui::GetHotKeyNameString(hotkeys::command_window)).c_str());
				ImGui::Spacing();
				if (ImGui::CollapsingHeader("Set health"))
				{
					ImGui::Spacing();
					ImGui::TextWrapped("Set player health.\nExample: hp (health).");
					ImGui::Spacing();
					ImGui::Separator();
				}
				if (ImGui::CollapsingHeader("Set time"))
				{
					ImGui::Spacing();
					ImGui::TextWrapped("Set current game time.\nExample: time (hour) (minute).\n");
					ImGui::TextWrapped("Writing something like 'time 12' would be interpreted as 'time 12 12'");
					ImGui::Spacing();
					ImGui::Separator();
				}
				if (ImGui::CollapsingHeader("Quick vehicle spawner"))
				{
					ImGui::Spacing();
					ImGui::TextWrapped("Spawn vehicles by typing their model names.\nExample: veh (veh_name)");
					ImGui::Spacing();
					ImGui::Separator();
				}
				if (ImGui::CollapsingHeader("Quick weapon spawner"))
				{
					ImGui::Spacing();
					ImGui::TextWrapped("Spawn weapons by typing their model names.\nExample: wep (wep_name)");
					ImGui::Spacing();
					ImGui::Separator();
				}

				ImGui::EndChild();
			}
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("About"))
		{
			ImGui::Spacing();

			if (ImGui::Button("Check update", ImVec2(Ui::GetSize(3))))
			{	
				if (Updater::state == UPDATER_IDLE)
					Updater::state = UPDATER_CHECKING;
			}

			ImGui::SameLine();

			if (ImGui::Button("Discord server", ImVec2(Ui::GetSize(3))))
				ShellExecute(NULL, "open", DISCORD_INVITE, NULL, NULL, SW_SHOWNORMAL);

			ImGui::SameLine();

			if (ImGui::Button("GitHub repo", ImVec2(Ui::GetSize(3))))
				ShellExecute(NULL, "open", GITHUB_LINK, NULL, NULL, SW_SHOWNORMAL);

			ImGui::Spacing();

			if (ImGui::BeginChild("AboutChild"))
			{
				ImGui::Columns(2, NULL, false);
				ImGui::Text("Author: Grinch_");

				ImGui::Text("Version: %s",MENU_VERSION);

				ImGui::NextColumn();
				ImGui::Text("ImGui: %s", ImGui::GetVersion());
				ImGui::Text("Build: %s", BUILD_NUMBER);

				ImGui::Columns(1);

				ImGui::Dummy(ImVec2(0, 10));
				ImGui::TextWrapped("If you find bugs or have suggestions, let me know on discord.");
				ImGui::Dummy(ImVec2(0, 10));
				ImGui::TextWrapped("Thanks to Junior-Djjr");
				ImGui::Dummy(ImVec2(0, 10));
				Ui::CenterdText("Copyright GPLv3 2019-2021 Grinch_");

				ImGui::EndChild();
			}

			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
}