import std;

import Config;
import Platform;
import Plugin;
import Resources;

import UtlString;

using std::array;
using std::string;
using std::string_view;
using std::uint8_t;
using std::vector;

using namespace std::literals;

void Config::Load(void) noexcept
{
	char sz[64]{};
	g_engfuncs.pfnGetGameDir(sz);

	m_GameDir = sz;

	std::filesystem::path const ConfigDir = m_GameDir / L"addons/amxmodx/configs/";

	auto const Settings = ConfigDir / L"settingmenu.ini";
	auto const ZombieConfigFile = ConfigDir / L"zombie.ini";
	auto const HumanConfigFile = ConfigDir / L"human.ini";
	auto const MapAmbianceConfigFile = ConfigDir / std::format("ambience_{}.ini", STRING(gpGlobals->mapname));
	auto const GlobalAmbianceConfigFile = ConfigDir / L"ambience.ini";

	if (std::filesystem::exists(Settings))
		LoadSettings(Settings);

	if (std::filesystem::exists(ZombieConfigFile))
		LoadZombies(ZombieConfigFile);

	if (std::filesystem::exists(HumanConfigFile))
		LoadHumans(HumanConfigFile);

	if (std::filesystem::exists(MapAmbianceConfigFile))
		LoadAmbiance(MapAmbianceConfigFile);
	else if (std::filesystem::exists(GlobalAmbianceConfigFile))
		LoadAmbiance(GlobalAmbianceConfigFile);

	// Failure detections

	if (m_rgHumanInfo.empty()) [[unlikely]]
		UTIL_Terminate(L"ZombieRiot至少需要定義一種人類類型以遊玩。");
	else if (m_rgZombieInfo.empty()) [[unlikely]]
		UTIL_Terminate(L"ZombieRiot至少需要定義一種人類類型以遊玩。");
	else if (m_rgWeather.empty()) [[unlikely]]
		UTIL_Terminate(L"ZombieRiot至少需要定義一項「天氣&亮度」組合以遊玩。");
}

void Config::LoadZombies(std::filesystem::path const& hPath) noexcept
{
	m_rgZombieInfo.clear();

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
	m_rgHumanInfo.clear();

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
	m_rgWeather.clear();

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

void Config::LoadSettings(std::filesystem::path const &hPath) noexcept
{
	if (std::ifstream file(hPath); file)
	{
		vector<uint8_t> rgiColor{};

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

			if (szKey == u8"是否支持ZBOT")
				m_bUsingZBOT = (szValue == u8"是" || szValue == "True");
			else if (szKey == u8"是否缓存玩家T模型")
				m_bPrecacheTModel = (szValue == u8"是" || szValue == "True");
			else if (szKey == u8"隐藏僵尸准星")
				m_bHideZombieCrosshair = (szValue == u8"是" || szValue == "True");

#define LOAD_STRING(KEY, VAR) \
	else if (szKey == KEY) \
		VAR = szValue

			LOAD_STRING(u8"断局提示", HintMessage::m_GameBreaks);
			LOAD_STRING(u8"自动平衡队伍(人类)", HintMessage::m_ForcedIntoHuman);
			LOAD_STRING(u8"自动平衡队伍(僵尸)", HintMessage::m_ForcedIntoZombie);
			LOAD_STRING(u8"换队伍提示一", HintMessage::m_OneTeamChangePerRound);
			LOAD_STRING(u8"换队伍提示二", HintMessage::m_NotNow);
			LOAD_STRING(u8"换队伍提示三", HintMessage::m_TheOpponentTeamHasFull);
			LOAD_STRING(u8"成为BOSS提示", HintMessage::m_BecomingBoss);
			LOAD_STRING(u8"成为普通僵尸提示", HintMessage::m_BecomingRegularZombie);
			LOAD_STRING(u8"复活倒数文字", HintMessage::m_Respawning);
			LOAD_STRING(u8"禁止复活文字1", HintMessage::m_HumanCanSeeYe);
			LOAD_STRING(u8"禁止复活文字2", HintMessage::m_HumanIsNearby);
			LOAD_STRING(u8"禁止复活文字3", HintMessage::m_Clipping);
			LOAD_STRING(u8"允许复活文字", HintMessage::m_RespawningKeyHint);
			LOAD_STRING(u8"复活提示文字", HintMessage::m_PostRespawning);
			LOAD_STRING(u8"更换僵尸类型提示", HintMessage::m_ChangingZombieType);
			LOAD_STRING(u8"更换人类类型提示", HintMessage::m_ChangingHumanType);
			LOAD_STRING(u8"人类胜利消息", HintMessage::m_HumanWins);
			LOAD_STRING(u8"僵尸胜利消息", HintMessage::m_ZombieWins);
			LOAD_STRING(u8"加入提示", HintMessage::m_Joining);
			LOAD_STRING(u8"离开提示", HintMessage::m_Leaving);

#undef LOAD_STRING

#define LOAD_COLOR(KEY, VAR)	\
			else if (szKey == KEY)\
			{\
				rgiColor = UTIL_SplitIntoNums<uint8_t>(szValue, ", ") | std::ranges::to<std::vector>();\
\
				[[unlikely]]\
				if (rgiColor.size() != 3)\
					UTIL_Terminate(L"參數「%s」的格式不規範：顏色應由三個0-255之間的整數經逗號隔開而構成", string(szKey));\
\
				VAR.r = rgiColor[0];\
				VAR.g = rgiColor[1];\
				VAR.b = rgiColor[2];\
			}\

			LOAD_COLOR(u8"倒数复活文字颜色", HintMessageColor::m_Respawning)
			LOAD_COLOR(u8"禁止复活文字颜色1", HintMessageColor::m_HumanCanSeeYe)
			LOAD_COLOR(u8"禁止复活文字颜色2", HintMessageColor::m_HumanIsNearby)
			LOAD_COLOR(u8"禁止复活文字颜色3", HintMessageColor::m_Clipping)
			LOAD_COLOR(u8"允许复活文字颜色", HintMessageColor::m_RespawningKeyHint)
			LOAD_COLOR(u8"复活提示文字颜色", HintMessageColor::m_PostRespawning)

#undef LOAD_COLOR

			else if (szKey == u8"开局音乐")
				Sounds::m_Beginning = std::format("sound/zombieriot/{}", szValue);

#define LOAD_SOUND(KEY, VAR)\
			else if (szKey == KEY)\
				VAR = std::format("zombieriot/{}", szValue)

			LOAD_SOUND(u8"倒数声音", Sounds::m_Beep);
			LOAD_SOUND(u8"倒数完毕声音", Sounds::m_Starting);
			LOAD_SOUND(u8"第一次提示声音", Sounds::m_CheckPoints[0]);
			LOAD_SOUND(u8"第二次提示声音", Sounds::m_CheckPoints[1]);
			LOAD_SOUND(u8"最后提示声音", Sounds::m_CheckPoints[2]);
			LOAD_SOUND(u8"人类胜利声音", Sounds::m_HumanWins);
			LOAD_SOUND(u8"僵尸胜利声音", Sounds::m_ZombieWins);
			LOAD_SOUND(u8"允许复活的声音", Sounds::m_AllowRespawn);
			LOAD_SOUND(u8"已复活的声音", Sounds::m_PostRespawn);
			LOAD_SOUND(u8"成为幽灵的声音", Sounds::m_GhostSpawn);

#undef LOAD_SOUND

#define LOAD_SOUND_ARRAY(KEY, VAR)\
			else if (szKey == KEY)\
				VAR = UTIL_Split(szValue, ", ") | std::views::transform([](auto &&s) noexcept { return std::format("zombieriot/{}", s); }) | std::ranges::to<vector>()

			LOAD_SOUND_ARRAY(u8"僵尸击空声", Sounds::m_rgszClawMiss);
			LOAD_SOUND_ARRAY(u8"僵尸击墙声", Sounds::m_rgszScratchingWall);
			LOAD_SOUND_ARRAY(u8"僵尸轻击声音", Sounds::m_rgszClawSlashingFlesh);
			LOAD_SOUND_ARRAY(u8"僵尸重击声音", Sounds::m_rgszClawSlicingFlesh);
			LOAD_SOUND_ARRAY(u8"BOSS受伤声音", Sounds::m_rgszBossHurt);
			LOAD_SOUND_ARRAY(u8"僵尸受伤声音", Sounds::m_rgszZombieHurt);
			LOAD_SOUND_ARRAY(u8"BOSS死亡声音", Sounds::m_rgszBossDeath);
			LOAD_SOUND_ARRAY(u8"僵尸死亡声音", Sounds::m_rgszZombieDeath);

#undef LOAD_SOUND_ARRAY
		}

		file.close();
	}
}
