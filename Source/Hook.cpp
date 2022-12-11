#include <string>
#include <string_view>

import <algorithm>;
import <array>;
import <vector>;

import eiface;
import pm_defs;
import util;

import CBase;
import Player;
import Plugin;
import Round;

using namespace std::string_literals;
using namespace std::string_view_literals;

using std::array;

inline constexpr array g_rgszEntityCanBeClip =
{
	"func_door"sv,
	"func_door_rotating"sv,
	"momentary_door"sv,
	"momentary_rot_button"sv,
	"func_breakable"sv,
	"hostage_entity"sv,
	"func_pushable"sv,
};

int fw_Spawn_Post(edict_t *pEdict) noexcept
{
	gpMetaGlobals->mres = MRES_IGNORED;

	EHANDLE<CBaseEntity> pEntity{ pEdict };

	if (!pEntity)
		return 0;

	if (!strcmp(STRING(pEntity->pev->classname), "func_wall"))
	{
		if (!strcmp(STRING(pEntity->pev->targetname), "zr_wall"))
		{
			pEntity->pev->iuser2 = ZRWALLCLIPKEY;
			return 0;
		}
	}

	if (!strcmp(STRING(pEntity->pev->classname), "func_breakable") && pEntity->pev->takedamage == DAMAGE_NO)
		return 0;

	if (std::ranges::find(g_rgszEntityCanBeClip, STRING(pEntity->pev->classname)) != g_rgszEntityCanBeClip.cend())
		pEntity->pev->iuser2 = SEMICLIPKEY;

	return 0;
}

void fw_ServerActivate_Post(edict_t *pEdictList, int edictCount, int clientMax) noexcept
{
	// plugin_init

	Round::Hook();
	Player::Hook();

	// plugin_cfg

}

META_RES FN_PM_Move(playermove_s *ppmove, qboolean server) noexcept
{
	if (ppmove->spectator)
		return MRES_IGNORED;

	static std::vector<size_t> rgiPhysEntsIndex{};
	rgiPhysEntsIndex.clear();
	rgiPhysEntsIndex.reserve(MAX_PHYSENTS);

	for (size_t i = 0; i < (size_t)ppmove->numphysent; ++i)
	{
		[[unlikely]]
		if (ppmove->physents[i].info == 0)	// entindex == 0 is the WORLD, mate.
		{
			rgiPhysEntsIndex.emplace_back(i);
			continue;
		}

		if (ppmove->fuser4 == 20520.f && ppmove->physents[i].iuser2 == ZRWALLCLIPKEY)
			continue;

		if (ppmove->deadflag == DEAD_RESPAWNABLE && (ppmove->physents[i].player || ppmove->physents[i].iuser2 == SEMICLIPKEY))
			continue;

		rgiPhysEntsIndex.emplace_back(i);
	}

	for (size_t i = 0; i < rgiPhysEntsIndex.size(); ++i)
	{
		if (i != rgiPhysEntsIndex[i])
			ppmove->physents[i] = ppmove->physents[rgiPhysEntsIndex[i]];
	}

	ppmove->numphysent = std::ssize(rgiPhysEntsIndex);
	return MRES_IGNORED;
}

qboolean fw_AddToFullPack_Post(entity_state_t *pState, int iEntIndex, edict_t *pEdict, edict_t *pClientSendTo, qboolean cl_lw, qboolean bIsPlayer, unsigned char *pSet) noexcept
{
	gpMetaGlobals->mres = MRES_IGNORED;

	if (!MF_IsPlayerIngame(ent_cast<int>(pClientSendTo)))
		return false;

	EHANDLE<CBasePlayer> pPlayer{ pClientSendTo };
	if (pPlayer->m_iTeam == TEAM_TERRORIST && pEdict->v.iuser2 == ZRWALLCLIPKEY)
	{
		pState->solid = SOLID_NOT;
		return false;
	}

	if (!MF_IsPlayerIngame(ent_cast<int>(pEdict)) && pEdict->v.iuser2 != SEMICLIPKEY)
		return false;

	if (pClientSendTo->v.deadflag != DEAD_RESPAWNABLE)
		return false;

	pState->solid = SOLID_NOT;

	if (std::ranges::find(g_rgszEntityCanBeClip, STRING(pEdict->v.classname)) != g_rgszEntityCanBeClip.cend())
	{
		pState->rendermode = kRenderTransTexture;
		pState->renderamt = 130;
	}

	return false;
}
