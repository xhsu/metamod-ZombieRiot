export module Round;

export import GameRules;

export enum EWinStatus
{
	WINSTATUS_CT = 1,
	WINSTATUS_TERRORIST,
	WINSTATUS_DRAW
};

export extern "C++" namespace Round
{
	void Hook(void) noexcept;

	void SetPatchRoundEnd(bool const bShouldPatch) noexcept;
	void TerminateRound(float flDelay, EWinStatus iWinStatus) noexcept;
	short &TeamScore(ECounterStrikeTeam iTeam) noexcept;
	const char *TeamName(ECounterStrikeTeam iTeam) noexcept;
	void UpdateTeamScore(ECounterStrikeTeam iTeam) noexcept;
};
