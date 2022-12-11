import std;

import Plugin;
import Platform;

import UtlString;

using std::string;
using std::string_view;
using std::uint8_t;
using std::vector;

extern "C++" namespace Config
{
	enum struct ESex : bool
	{
		FEMALE,
		MALE,
	};

	struct Zombie_t
	{
		uint8_t m_iId{ 0 };
		bool m_bHidden{ false };
		bool m_bBoss{ false };
		ESex m_Sex{ ESex::MALE };
		string m_szName{};
		string m_szIntro{};
		string m_szPlayerModel{};
		string m_szViewModel{};
		float m_flMaxHealth{ 1000.f };
		float m_flMaxSpeed{ 250.f };
		float m_flGravity{ 1.f };
		float m_flSlashInterval{};
		float m_flStabInterval{};
		float m_flSlashDist{};
		float m_flStabDist{};
		float m_flMeleeDamageMul{};
		float m_flKnockbackResistance{};
	};

	inline std::deque<Zombie_t> m_rgZombieInfo{};

	struct Human_t
	{
		uint8_t m_iId{ 0 };
		bool m_bHidden{ false };
		ESex m_Sex{ ESex::MALE };
		string m_szName{};
		string m_szIntro{};
		string m_szPlayerModel{};
		float m_flMaxHealth{ 1000.f };
		float m_flMaxSpeed{ 250.f };
		float m_flGravity{ 1.f };
		float m_flKnockbackResistance{};
	};

	inline std::deque<Human_t> m_rgHumanInfo{};

	struct Weather_t
	{
		enum EWeather : uint8_t
		{
			Clear = 1,
			Rain,
			Thunderstorm,
			Tempest,
			Snow,
			Fog,
			BlackFog,
		};

		char m_cLighting{ 'z' };
		EWeather m_iWeather{ Clear };

		static inline string m_szSkyTexture{};
	};

	inline std::deque<Weather_t> m_rgWeather{};

	void PreparePath(void) noexcept;
	void LoadZombies(std::filesystem::path const &hPath) noexcept;
	void LoadHumans(std::filesystem::path const &hPath) noexcept;
	void LoadAmbiance(std::filesystem::path const &hPath) noexcept;
};

void Config::PreparePath(void) noexcept
{
	char sz[64]{};
	g_engfuncs.pfnGetGameDir(sz);

	std::filesystem::path const ConfigDir = std::format("{}/addons/amxmodx/configs/", sz);

	auto const ZombieConfigFile = ConfigDir / "zombie.ini";
	auto const HumanConfigFile = ConfigDir / "human.ini";
	auto const MapAmbianceConfigFile = std::filesystem::path(std::format("{}/addons/amxmodx/configs/ambience_{}.ini", sz, gpGlobals->mapname));
	auto const GlobalAmbianceConfigFile = ConfigDir / "ambience.ini";

	if (std::filesystem::exists(ZombieConfigFile))
		LoadZombies(ZombieConfigFile);

	if (std::filesystem::exists(HumanConfigFile))
		LoadHumans(HumanConfigFile);

	if (std::filesystem::exists(MapAmbianceConfigFile))
		LoadAmbiance(MapAmbianceConfigFile);
	else if (std::filesystem::exists(GlobalAmbianceConfigFile))
		LoadAmbiance(GlobalAmbianceConfigFile);
}

void Config::LoadZombies(std::filesystem::path const& hPath) noexcept
{
	if (std::ifstream file(hPath); file)
	{
		while (!file.eof())
		{
			string szLine{};
			std::getline(file, szLine);

			if (!szLine.empty() && szLine[0] == ';')
				continue;

			auto const pos = szLine.find_first_of('=');
			if (pos == szLine.npos)
				continue;

			auto const szKey = UTIL_Trim(string_view(szLine).substr(0, pos));
			auto const szValue = UTIL_Trim(string_view(szLine).substr(pos + 1));

			if (szKey == "ID")
			{
				auto const iId = UTIL_StrToNum<std::uint8_t>(szValue);

				[[unlikely]]
				if (iId <= 0)
					UTIL_Terminate(L"殭屍編號註冊失敗：'ID'值必須為正整數！");

				for (auto &&info : m_rgZombieInfo)
				{
					[[unlikely]]
					if (info.m_iId == iId)
						UTIL_Terminate(L"殭屍編號註冊失敗：'ID'值出現重複。編號'%d'已被定義為「%s」！\n", iId, info.m_szName);
				}

				m_rgZombieInfo.emplace_back();
				m_rgZombieInfo.back().m_iId = iId;
			}
			else if (szKey == u8"是否隐藏")
				m_rgZombieInfo.back().m_bHidden = (szValue == u8"是" || szValue == "True");
			else if (szKey == u8"类型")
				m_rgZombieInfo.back().m_bBoss = (szValue == "BOSS");
			else if (szKey == u8"性别")
				m_rgZombieInfo.back().m_Sex = (szValue == u8"男" || szValue == "Male") ? ESex::MALE : ESex::FEMALE;
			else if (szKey == u8"名称")
				m_rgZombieInfo.back().m_szName = szValue;
			else if (szKey == u8"介绍")
				m_rgZombieInfo.back().m_szIntro = szValue;
			else if (szKey == u8"人物模型")
				m_rgZombieInfo.back().m_szPlayerModel = szValue;
			else if (szKey == u8"手臂模型")
				m_rgZombieInfo.back().m_szViewModel = szValue;
			else if (szKey == u8"生命")
				m_rgZombieInfo.back().m_flMaxHealth = UTIL_StrToNum<float>(szValue);
			else if (szKey == u8"速度")
				m_rgZombieInfo.back().m_flMaxSpeed = UTIL_StrToNum<float>(szValue);
			else if (szKey == u8"重量")
				m_rgZombieInfo.back().m_flGravity = UTIL_StrToNum<float>(szValue);
			else if (szKey == u8"轻击速度")
				m_rgZombieInfo.back().m_flSlashInterval = UTIL_StrToNum<float>(szValue);
			else if (szKey == u8"重击速度")
				m_rgZombieInfo.back().m_flStabInterval = UTIL_StrToNum<float>(szValue);
			else if (szKey == u8"轻击范围")
				m_rgZombieInfo.back().m_flSlashDist = UTIL_StrToNum<float>(szValue);
			else if (szKey == u8"重击范围")
				m_rgZombieInfo.back().m_flStabDist = UTIL_StrToNum<float>(szValue);
			else if (szKey == u8"攻击伤害倍数")
				m_rgZombieInfo.back().m_flMeleeDamageMul = UTIL_StrToNum<float>(szValue);
			else if (szKey == u8"抗击退")
				m_rgZombieInfo.back().m_flKnockbackResistance = UTIL_StrToNum<float>(szValue);

			//g_engfuncs.pfnServerPrint(std::format("szKey: \"{}\"\tszValue: \"{}\"\n", szKey, szValue).c_str());
		}

		file.close();
	}
}

void Config::LoadHumans(std::filesystem::path const &hPath) noexcept
{
	if (std::ifstream file(hPath); file)
	{
		while (!file.eof())
		{
			string szLine{};
			std::getline(file, szLine);

			if (!szLine.empty() && szLine[0] == ';')
				continue;

			auto const pos = szLine.find_first_of('=');
			if (pos == szLine.npos)
				continue;

			auto const szKey = UTIL_Trim(string_view(szLine).substr(0, pos));
			auto const szValue = UTIL_Trim(string_view(szLine).substr(pos + 1));

			if (szKey == "ID")
			{
				auto const iId = UTIL_StrToNum<std::uint8_t>(szValue);

				[[unlikely]]
				if (iId <= 0)
					UTIL_Terminate(L"人類編號註冊失敗：'ID'值必須為正整數！");

				for (auto &&info : m_rgHumanInfo)
				{
					[[unlikely]]
					if (info.m_iId == iId)
						UTIL_Terminate(L"人類編號註冊失敗：'ID'值出現重複。編號'%d'已被定義為「%s」！\n", iId, info.m_szName);
				}

				m_rgHumanInfo.emplace_back();
				m_rgHumanInfo.back().m_iId = iId;
			}
			else if (szKey == u8"是否隐藏")
				m_rgHumanInfo.back().m_bHidden = (szValue == u8"是" || szValue == "True");
			else if (szKey == u8"性别")
				m_rgHumanInfo.back().m_Sex = (szValue == u8"男" || szValue == "Male") ? ESex::MALE : ESex::FEMALE;
			else if (szKey == u8"名称")
				m_rgHumanInfo.back().m_szName = szValue;
			else if (szKey == u8"介绍")
				m_rgHumanInfo.back().m_szIntro = szValue;
			else if (szKey == u8"人物模型")
				m_rgHumanInfo.back().m_szPlayerModel = szValue;
			else if (szKey == u8"生命")
				m_rgHumanInfo.back().m_flMaxHealth = UTIL_StrToNum<float>(szValue);
			else if (szKey == u8"速度")
				m_rgHumanInfo.back().m_flMaxSpeed = UTIL_StrToNum<float>(szValue);
			else if (szKey == u8"重量")
				m_rgHumanInfo.back().m_flGravity = UTIL_StrToNum<float>(szValue);
			else if (szKey == u8"抗击退")
				m_rgHumanInfo.back().m_flKnockbackResistance = UTIL_StrToNum<float>(szValue);

			//g_engfuncs.pfnServerPrint(std::format("szKey: \"{}\"\tszValue: \"{}\"\n", szKey, szValue).c_str());
		}

		file.close();
	}
	/*
	for (auto &&info : m_rgHumanInfo)
	{
		auto const str = std::format(
			u8"----------------------------------\n"
			u8"ID: {}\n"
			u8"是否隐藏: {}\n"
			u8"性别: {}\n"
			u8"名称: {}\n"
			u8"介绍: {}\n"
			u8"人物模型: {}\n"
			u8"生命: {}\n"
			u8"速度: {}\n"
			u8"重量: {}\n"
			u8"抗击退: {}\n"
			u8"----------------------------------\n",
			info.m_iId,
			info.m_bHidden,
			info.m_Sex == ESex::MALE ? "男" : "女",
			info.m_szName,
			info.m_szIntro,
			info.m_szPlayerModel,
			info.m_flMaxHealth,
			info.m_flMaxSpeed,
			info.m_flGravity,
			info.m_flKnockbackResistance
		);

		g_engfuncs.pfnServerPrint(str.c_str());
	}
	*/
}

void Config::LoadAmbiance(std::filesystem::path const &hPath) noexcept
{
	if (std::ifstream file(hPath); file)
	{
		while (!file.eof())
		{
			string szLine{};
			std::getline(file, szLine);

			if (!szLine.empty() && szLine[0] == ';')
				continue;

			auto const pos = szLine.find_first_of('=');
			if (pos == szLine.npos)
				continue;

			auto const szKey = UTIL_Trim(string_view(szLine).substr(0, pos));
			auto const szValue = UTIL_Trim(string_view(szLine).substr(pos + 1));

			if (szKey == u8"亮度&天气")
			{
				auto const rgpsz = UTIL_Split(szValue, ", ") | std::ranges::to<std::vector>();

				for (auto &&s : rgpsz)
				{
					[[unlikely]]
					if (auto const iWeather = s[1] - '0'; s.length() != 2 || !std::islower(s[0]) || !std::isdigit(s[1]) || iWeather < 1 || iWeather > 7)
						UTIL_Terminate(L"「亮度與天氣」參數不規範或無效：'%s'", string(s));

					m_rgWeather.emplace_back(Weather_t{ .m_cLighting = s[0], .m_iWeather = static_cast<Weather_t::EWeather>(s[1] - '0') });
				}
			}
			else if (szKey == u8"天空贴图")
			{
				Weather_t::m_szSkyTexture = szValue;
				g_engfuncs.pfnCVarSetString("sv_skyname", Weather_t::m_szSkyTexture.c_str());	// This guaranteed the string is '\0' terminated. 
			}
		}

		file.close();
	}
}
