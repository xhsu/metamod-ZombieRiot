import eiface;

import Config;
import Resources;

using std::string;

void SpriteMgr_t::Precache(void) noexcept
{
	s_rgLibrary[LASER_BEAM] = g_engfuncs.pfnPrecacheModel(LASER_BEAM);
}

void Resources::Precache(void) noexcept
{
	for (auto &&ZombieInfo : Config::m_rgZombieInfo)
	{
		auto const szPlayerModelPath = Config::m_GameDir / std::format("models/player/{0}/{0}.mdl", ZombieInfo.m_szPlayerModel);
		auto const szPlayerTModelPath = Config::m_GameDir / std::format("models/player/{0}/{0}T.mdl", ZombieInfo.m_szPlayerModel);
		auto const szZombieClawPath = Config::m_GameDir / std::format("models/zombieriot/{0}.mdl", ZombieInfo.m_szViewModel);

		g_engfuncs.pfnPrecacheModel(szPlayerModelPath.string().c_str());
		g_engfuncs.pfnPrecacheModel(szZombieClawPath.string().c_str());

		if (Config::m_bPrecacheTModel && std::filesystem::exists(szPlayerTModelPath))
			g_engfuncs.pfnPrecacheModel(szPlayerTModelPath.string().c_str());
	}

	for (auto &&HumanInfo : Config::m_rgHumanInfo)
	{
		auto const szPlayerModelPath = Config::m_GameDir / std::format("models/player/{0}/{0}.mdl", HumanInfo.m_szPlayerModel);
		auto const szPlayerTModelPath = Config::m_GameDir / std::format("models/player/{0}/{0}T.mdl", HumanInfo.m_szPlayerModel);

		g_engfuncs.pfnPrecacheModel(szPlayerModelPath.string().c_str());

		if (Config::m_bPrecacheTModel && std::filesystem::exists(szPlayerTModelPath))
			g_engfuncs.pfnPrecacheModel(szPlayerTModelPath.string().c_str());
	}

	g_engfuncs.pfnPrecacheSound(Sounds::m_Beginning.c_str());
	g_engfuncs.pfnPrecacheSound(Sounds::m_Beep.c_str());
	g_engfuncs.pfnPrecacheSound(Sounds::m_Starting.c_str());
	g_engfuncs.pfnPrecacheSound(Sounds::m_CheckPoints[0].c_str());
	g_engfuncs.pfnPrecacheSound(Sounds::m_CheckPoints[1].c_str());
	g_engfuncs.pfnPrecacheSound(Sounds::m_CheckPoints[2].c_str());
	g_engfuncs.pfnPrecacheSound(Sounds::m_HumanWins.c_str());
	g_engfuncs.pfnPrecacheSound(Sounds::m_ZombieWins.c_str());
	g_engfuncs.pfnPrecacheSound(Sounds::m_AllowRespawn.c_str());
	g_engfuncs.pfnPrecacheSound(Sounds::m_PostRespawn.c_str());
	g_engfuncs.pfnPrecacheSound(Sounds::m_GhostSpawn.c_str());

	std::ranges::for_each(Sounds::m_rgszClawMiss, g_engfuncs.pfnPrecacheSound, [](auto &&s) noexcept { return s.c_str(); });
	std::ranges::for_each(Sounds::m_rgszScratchingWall, g_engfuncs.pfnPrecacheSound, [](auto &&s) noexcept { return s.c_str(); });
	std::ranges::for_each(Sounds::m_rgszClawSlashingFlesh, g_engfuncs.pfnPrecacheSound, [](auto &&s) noexcept { return s.c_str(); });
	std::ranges::for_each(Sounds::m_rgszClawSlicingFlesh, g_engfuncs.pfnPrecacheSound, [](auto &&s) noexcept { return s.c_str(); });
	std::ranges::for_each(Sounds::m_rgszBossHurt, g_engfuncs.pfnPrecacheSound, [](auto &&s) noexcept { return s.c_str(); });
	std::ranges::for_each(Sounds::m_rgszZombieHurt, g_engfuncs.pfnPrecacheSound, [](auto &&s) noexcept { return s.c_str(); });
	std::ranges::for_each(Sounds::m_rgszBossDeath, g_engfuncs.pfnPrecacheSound, [](auto &&s) noexcept { return s.c_str(); });
	std::ranges::for_each(Sounds::m_rgszZombieDeath, g_engfuncs.pfnPrecacheSound, [](auto &&s) noexcept { return s.c_str(); });

	std::ranges::for_each(Sounds::m_rgszRain, g_engfuncs.pfnPrecacheSound);
	std::ranges::for_each(Sounds::m_rgszThunder, g_engfuncs.pfnPrecacheSound);
	std::ranges::for_each(Sounds::m_rgszThunderclap, g_engfuncs.pfnPrecacheSound);
	std::ranges::for_each(Sounds::m_rgszNVG, g_engfuncs.pfnPrecacheSound);

	Sprites.Precache();
}
