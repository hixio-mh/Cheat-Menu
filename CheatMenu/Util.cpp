#include "pch.h"
#include "Util.h"
#include "../Depend/imgui/stb_image.h"
#include <CCutsceneMgr.h>
#include "psapi.h"

void Util::ClearCharTasksVehCheck(CPed* ped)
{
	uint hped = CPools::GetPedRef(ped);
	uint hveh = NULL;
	bool veh_engine = true;

	if (ped->m_nPedFlags.bInVehicle)
	{
		hveh = CPools::GetVehicleRef(ped->m_pVehicle);
		veh_engine = ped->m_pVehicle->m_nVehicleFlags.bEngineOn;
	}

	Command<Commands::CLEAR_CHAR_TASKS_IMMEDIATELY>(hped);

	if (hveh)
	{
		Command<Commands::TASK_WARP_CHAR_INTO_CAR_AS_DRIVER>(hped, hveh);
		ped->m_pVehicle->m_nVehicleFlags.bEngineOn = veh_engine;
	}
}

void Util::ReleaseTextures(std::vector<std::unique_ptr<STextureStructure>>& store_vec)
{
	// for (auto &it : store_vec)
	// {
	// 	if (Globals::renderer == Render_DirectX9)
	// 	{
	// 		reinterpret_cast<IDirect3DTexture9*>(it->texture)->Release();
	// 		it->texture = nullptr;
	// 	}
	// 	else
	// 		reinterpret_cast<ID3D11ShaderResourceView*>(it->texture)->Release();
	// }
}

void Util::LoadTexturesInDirRecursive(const char* path, const char* file_ext, std::vector<std::string>& category_vec,
                                      std::vector<std::unique_ptr<STextureStructure>>& store_vec)
{
	std::string folder = "";
	for (auto& p : fs::recursive_directory_iterator(path))
	{
		if (p.path().extension() == file_ext)
		{
			store_vec.push_back(std::make_unique<STextureStructure>());
			HRESULT hr = -1;

			if (Globals::renderer == Render_DirectX9)
				hr = D3DXCreateTextureFromFileA(GetD3DDevice(), p.path().string().c_str(),
				                                reinterpret_cast<PDIRECT3DTEXTURE9*>(&store_vec.back().get()->m_pTexture));
			else
			{
				if (LoadTextureFromFileDx11(p.path().string().c_str(),
				                            reinterpret_cast<ID3D11ShaderResourceView**>(&store_vec.back().get()->
					                            m_pTexture)))
					hr = S_OK;
			}

			if (hr == S_OK)
			{
				store_vec.back().get()->m_FileName = p.path().stem().string();
				store_vec.back().get()->m_CategoryName = folder;
			}
			else
			{
				flog << "Failed to load " << p.path().stem().string() << std::endl;
				store_vec.pop_back();
			}
		}
		else
		{
			folder = p.path().stem().string();
			category_vec.push_back(folder);
		}
	}
}

bool Util::LoadTextureFromFileDx11(const char* filename, ID3D11ShaderResourceView** out_srv)
{
	// Load from disk into a raw RGBA buffer
	int image_width = 0;
	int image_height = 0;
	unsigned char* image_data = stbi_load(filename, &image_width, &image_height, nullptr, 4);
	if (image_data == nullptr)
		return false;

	// Create texture
	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Width = image_width;
	desc.Height = image_height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;

	ID3D11Texture2D* pTexture = nullptr;
	D3D11_SUBRESOURCE_DATA subResource;
	subResource.pSysMem = image_data;
	subResource.SysMemPitch = desc.Width * 4;
	subResource.SysMemSlicePitch = 0;

	reinterpret_cast<ID3D11Device*>(Globals::device)->CreateTexture2D(&desc, &subResource, &pTexture);

	// Create texture view
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = desc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;
	reinterpret_cast<ID3D11Device*>(Globals::device)->CreateShaderResourceView(pTexture, &srvDesc, out_srv);
	pTexture->Release();

	stbi_image_free(image_data);

	return true;
}

bool Util::IsOnMission()
{
	return FindPlayerPed()->CanPlayerStartMission() && !*(patch::Get<char*>(0x5D5380, false) +
		CTheScripts::OnAMissionFlag);
}

bool Util::IsOnCutscene()
{
	return CCutsceneMgr::ms_running;
}

std::string Util::GetLocationName(CVector* pos)
{
	int hplayer = CPools::GetPedRef(FindPlayerPed());
	int interior = 0;
	Command<Commands::GET_AREA_VISIBLE>(&interior);

	std::string town = "San Andreas";
	int city;
	Command<Commands::GET_CITY_PLAYER_IS_IN>(&hplayer, &city);

	switch (city)
	{
	case 0:
		town = "CS";
		break;
	case 1:
		town = "LS";
		break;
	case 2:
		town = "SF";
		break;
	case 3:
		town = "LV";
		break;
	}

	if (interior == 0)
		return CTheZones::FindSmallestZoneForPosition(*pos, true)->GetTranslatedName() + std::string(", ") + town;
	return std::string("Interior ") + std::to_string(interior) + ", " + town;
}

int Util::GetLargestGangInZone()
{
	int gang_id = 0, max_density = 0;

	for (int i = 0; i != 10; ++i)
	{
		CVector pos = FindPlayerPed()->GetPosition();
		auto zone = new CZone();

		CZoneExtraInfo* zone_info = CTheZones::GetZoneInfo(&pos, &zone);
		int density = zone_info->m_nGangDensity[i];

		if (density > max_density)
		{
			max_density = density;
			gang_id = i;
		}
		delete zone;
	}

	return gang_id;
}

// implemention of opcode 0AB5 (STORE_CLOSEST_ENTITIES)
// https://github.com/cleolibrary/CLEO4/blob/916d400f4a731ba1dd0ff16e52bdb056f42b7038/source/CCustomOpcodeSystem.cpp#L1671
CVehicle* Util::GetClosestVehicle()
{
	CPlayerPed* player = FindPlayerPed();
	CPedIntelligence* pedintel;
	if (player && (pedintel = player->m_pIntelligence))
	{
		CVehicle* veh = nullptr;
		for (int i = 0; i < 16; i++)
		{
			veh = static_cast<CVehicle*>(pedintel->m_vehicleScanner.m_apEntities[i]);
			if (veh && !veh->m_nVehicleFlags.bFadeOut)
				break;
			veh = nullptr;
		}

		return veh;
	}
	return nullptr;
}

CPed* Util::GetClosestPed()
{
	CPlayerPed* player = FindPlayerPed();
	CPedIntelligence* pedintel;
	if (player && (pedintel = player->m_pIntelligence))
	{
		CPed* ped = nullptr;

		for (int i = 0; i < 16; i++)
		{
			ped = static_cast<CPed*>(pedintel->m_pedScanner.m_apEntities[i]);
			if (ped && ped != player && (ped->m_nCreatedBy & 0xFF) == 1 && !ped->m_nPedFlags.bFadeOut)
				break;
			ped = nullptr;
		}

		return ped;
	}
	return nullptr;
}

void Util::RainbowValues(int& r, int& g, int& b, float speed)
{
	int timer = CTimer::m_snTimeInMilliseconds / 150;
	r = sin(timer * speed) * 127 + 128;
	g = sin(timer * speed + 2) * 127 + 128;
	b = sin(timer * speed + 4) * 127 + 128;
}

RwTexture* CreateRwTextureFromRwImage(RwImage* image)
{
	RwInt32 width, height, depth, flags;
	RwImageFindRasterFormat(image, 4, &width, &height, &depth, &flags);
	RwRaster* raster = RwRasterCreate(width, height, depth, flags);
	RwRasterSetFromImage(raster, image);
	RwImageDestroy(image);
	RwTexture* texture = RwTextureCreate(raster);
	return texture;
}

RwTexture* Util::LoadTextureFromPngFile(fs::path path)
{
	RwImage* image = RtPNGImageRead(path.string().c_str());
	if (!image)
		return nullptr;
	RwTexture* texture = CreateRwTextureFromRwImage(image);
	path.stem().string().copy(texture->name, sizeof(texture->name) - 1);
	return texture;
}

void Util::GetCPUUsageInit()
{
	PdhOpenQuery(nullptr, NULL, &cpuQuery);
	PdhAddEnglishCounter(cpuQuery, "\\Processor(_Total)\\% Processor Time", NULL, &cpuTotal);
	PdhCollectQueryData(cpuQuery);
}

double Util::GetCurrentCPUUsage()
{
	PDH_FMT_COUNTERVALUE counterVal;

	PdhCollectQueryData(cpuQuery);
	PdhGetFormattedCounterValue(cpuTotal, PDH_FMT_DOUBLE, nullptr, &counterVal);
	return counterVal.doubleValue;
}
