import std;

import util;

import Plugin;
import Player;
import Round;

using std::bit_cast;

static cell ZR_PatchRoundEnd(AMX *amx, cell *params) noexcept
{
	Round::SetPatchRoundEnd((bool)params[1]);
	return true;
}

static cell ZR_TerminateRound(AMX *amx, cell *params) noexcept
{
	Round::TerminateRound(bit_cast<float>(params[1]), bit_cast<EWinStatus>(params[2]));
	return true;
}

static cell ZR_GetTeamScore(AMX *amx, cell *params) noexcept
{
	return Round::TeamScore(bit_cast<ECounterStrikeTeam>(params[1]));
}

static cell ZR_SetTeamScore(AMX *amx, cell *params) noexcept
{
	Round::TeamScore(bit_cast<ECounterStrikeTeam>(params[1])) = (short)params[2];
	return true;
}

static cell ZR_UpdateTeamScore(AMX *amx, cell *params) noexcept
{
	Round::UpdateTeamScore(bit_cast<ECounterStrikeTeam>(params[1]));
	return true;
}

// native ZR_SetAnimation(iPlayer, Float:Time, Anim, GaitAnim = -1)
static cell ZR_SetAnimation(AMX *amx, cell *params) noexcept
{
	auto const &iPlayer = params[1];
	auto const flTime = bit_cast<float>(params[2]);
	auto const &iAnim = params[3];
	auto const &iGaitAnim = params[4];

	Player::PlayTime[iPlayer] = bit_cast<float>(flTime);
	Player::Sequence[iPlayer] = iAnim;
	Player::GaitSequence[iPlayer] = iGaitAnim;

	if (iGaitAnim < 0)
	{
		Player::SetAnimation(iPlayer, PLAYER_CUSTOM);
		return true;
	}

	EHANDLE<CBasePlayer> pPlayer((short)iPlayer);

	Player::RecordTime[iPlayer] = gpGlobals->time;
	pPlayer->pev->sequence = Player::Sequence[iPlayer];
	pPlayer->pev->gaitsequence = Player::GaitSequence[iPlayer];
	pPlayer->pev->frame = 0;
	ResetSequenceInfo(pPlayer);

	return true;
}

static cell SetPenetrationToGhost(AMX *amx, cell *params) noexcept
{
	if (params[2])
	{
		ent_cast<edict_t *>(params[1])->v.iuser2 = SEMICLIPKEY;
	}
	else
	{
		ent_cast<edict_t *>(params[1])->v.iuser2 = 0;
	}

	return true;
}

void DeployNatives(void) noexcept
{
	static constexpr AMX_NATIVE_INFO rgAmxNativeInfo[] =
	{
		{ "ZR_PatchRoundEnd",		&ZR_PatchRoundEnd },
		{ "ZR_TerminateRound",		&ZR_TerminateRound },
		{ "ZR_GetTeamScore",		&ZR_GetTeamScore },
		{ "ZR_SetTeamScore",		&ZR_SetTeamScore },
		{ "ZR_UpdateTeamScore",		&ZR_UpdateTeamScore },
		{ "ZR_SetAnimation",		&ZR_SetAnimation },
		{ "SetPenetrationToGhost",	&SetPenetrationToGhost },
		{ nullptr, nullptr },
	};

	MF_AddNatives(rgAmxNativeInfo);
}
