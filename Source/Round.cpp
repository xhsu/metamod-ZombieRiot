
import Engine;
import GameRules;
import Platform;

import UtlHook;

import Plugin;
import Round;


inline constexpr size_t VFTIDX_CHalfLifeMultiplay_CheckWinConditions = 65;

inline std::uintptr_t g_dwCheckWinConditions = 0;
inline std::uint8_t g_CheckWinConditionsRestoreByte = 0;

void Round::Hook(void) noexcept
{
	RetrieveGameRules();

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
