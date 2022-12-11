#include <cassert>

import GameRules;

import UtlHook;

import Plugin;
import Round;

inline constexpr unsigned char CWORLD_PRECACHE_FN_PATTERN[] = "\x90\x55\x57\x33\xFF\x68\x2A\x2A\x2A\x2A\x68\x2A\x2A\x2A\x2A\x8B\xE9\x89\x3D\x2A\x2A\x2A\x2A\x89\x3D\x2A\x2A\x2A\x2A\x89\x3D";

inline constexpr size_t VFTIDX_CHalfLifeMultiplay_CheckWinConditions = 65;

inline std::uintptr_t g_dwCheckWinConditions = 0;
inline std::uint8_t g_CheckWinConditionsRestoreByte = 0;

void Round::Hook(void) noexcept
{
	auto addr = (std::uintptr_t)UTIL_SearchPattern("mp.dll", CWORLD_PRECACHE_FN_PATTERN, 1);

#ifdef _DEBUG
	assert(addr != 0);
#else
	[[unlikely]]
	if (!addr)
		gpMetaUtilFuncs->pfnLogError(PLID, "Function \"CWorld::Precache\" no found!");
#endif

	addr += (std::ptrdiff_t)(0xD29B4 - 0xD2940);
	g_pGameRules = *(CHalfLifeMultiplay **)(void **)(*(long *)addr);

	assert(g_pGameRules != nullptr);

	// However, the hook status remains even if the game reloaded.
	// Still need this method to make sure the hooks are happened only once.

	static bool bGameRuleHooked = false;

	[[unlikely]]
	if (!bGameRuleHooked)
	{
		auto const rgpfn = UTIL_RetrieveVirtualFunctionTable(g_pGameRules);

		g_dwCheckWinConditions = (std::uintptr_t)rgpfn[VFTIDX_CHalfLifeMultiplay_CheckWinConditions];
		g_CheckWinConditionsRestoreByte = *(std::uint8_t *)g_dwCheckWinConditions;

		bGameRuleHooked = true;
	}
}

void Round::SetPatchRoundEnd(bool const bShouldPatch) noexcept
{
	UTIL_WriteMemory(
		(void *)g_dwCheckWinConditions,
		bShouldPatch ? (std::uint8_t)0xC3 : g_CheckWinConditionsRestoreByte
	);
}

void Round::TerminateRound(float flDelay, EWinStatus iWinStatus) noexcept
{
	g_pGameRules->m_iRoundWinStatus = iWinStatus;
	g_pGameRules->m_bRoundTerminating = true;
	g_pGameRules->m_flRestartRoundTime = gpGlobals->time + flDelay;
}

short &Round::TeamScore(ECounterStrikeTeam iTeam) noexcept
{
	static short iPlaceholder{};

	switch (iTeam)
	{
	case TEAM_TERRORIST:
		return g_pGameRules->m_iNumTerroristWins;

	case TEAM_CT:
		return g_pGameRules->m_iNumCTWins;

	default:
		return iPlaceholder;
	}
}

const char *Round::TeamName(ECounterStrikeTeam iTeam) noexcept
{
	switch (iTeam)
	{
	case TEAM_TERRORIST:
		return "TERRORIST";

	case TEAM_CT:
		return "CT";

	case TEAM_SPECTATOR:
		return "SPECTATOR";

	default:
		return "";
	}
}

void Round::UpdateTeamScore(ECounterStrikeTeam iTeam) noexcept
{
	static int const gmsgTeamScore = gpMetaUtilFuncs->pfnGetUserMsgID(PLID, "TeamScore", nullptr);

	switch (iTeam)
	{
	case TEAM_CT:
	case TEAM_TERRORIST:
		g_engfuncs.pfnMessageBegin(MSG_ALL, gmsgTeamScore, nullptr, nullptr);
		g_engfuncs.pfnWriteString(TeamName(iTeam));
		g_engfuncs.pfnWriteShort(TeamScore(iTeam));
		g_engfuncs.pfnMessageEnd();

	default:
		return;
	}
}
