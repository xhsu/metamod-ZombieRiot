#include <cassert>

import Engine;
import GameRules;
import Platform;

import UtlHook;

import Plugin;
import Round;

inline constexpr unsigned char CWORLD_PRECACHE_FN_NEW_PATTERN[] = "\x90\x55\x57\x33\xFF\x68\x2A\x2A\x2A\x2A\x68\x2A\x2A\x2A\x2A\x8B\xE9\x89\x3D\x2A\x2A\x2A\x2A\x89\x3D\x2A\x2A\x2A\x2A\x89\x3D";
inline constexpr unsigned char CWORLD_PRECACHE_FN_ANNIV_PATTERN[] = "\xCC\x55\x8B\xEC\x51\x57\x68\x2A\x2A\x2A\x2A\x68\x2A\x2A\x2A\x2A\x8B\xF9\xC7\x05";

inline constexpr size_t VFTIDX_CHalfLifeMultiplay_CheckWinConditions = 65;

inline std::uintptr_t g_dwCheckWinConditions = 0;
inline std::uint8_t g_CheckWinConditionsRestoreByte = 0;

void Round::Hook(void) noexcept
{
	auto addr = (std::uintptr_t)UTIL_SearchPattern("mp.dll", 1, CWORLD_PRECACHE_FN_NEW_PATTERN, CWORLD_PRECACHE_FN_ANNIV_PATTERN);

#ifdef _DEBUG
	assert(addr != 0);
#else
	[[unlikely]]
	if (!addr)
		UTIL_Terminate("Function \"CWorld::Precache\" no found!");
#endif
	static constexpr std::ptrdiff_t ofs_anniv = 0xC24E3 - 0xC2440;
	static constexpr std::ptrdiff_t ofs_new = 0xD29B4 - 0xD2940;

	addr += Engine::BUILD_NUMBER >= Engine::ANNIVERSARY ? ofs_anniv : ofs_new;
	g_pGameRules = *(CHalfLifeMultiplay**)(void**)(*(long*)addr);

	assert(g_pGameRules != nullptr);

	// However, the hook status remains even if the game reloaded.
	// Still need this method to make sure the hooks are happened only once.

	static bool bGameRuleHooked = false;

	[[unlikely]]
	if (!bGameRuleHooked)
	{
		auto const vft = UTIL_RetrieveVirtualFunctionTable(g_pGameRules);

		g_dwCheckWinConditions = (std::uintptr_t)vft[VFTIDX_CHalfLifeMultiplay_CheckWinConditions];
		g_CheckWinConditionsRestoreByte = *(std::uint8_t *)g_dwCheckWinConditions;

		bGameRuleHooked = true;
	}
}

void Round::SetPatchRoundEnd(bool const bShouldPatch) noexcept
{
	// the function CHalfLifeMultiplay::CheckWinConditions have no argument at all.
	// so just instruct RETN, as nothing to rewind on stack frame.

	static constexpr std::uint8_t ASM_RETN = 0xC3;	// _asm retn

	UTIL_WriteMemory(
		(void *)g_dwCheckWinConditions,
		bShouldPatch ? ASM_RETN : g_CheckWinConditionsRestoreByte
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
