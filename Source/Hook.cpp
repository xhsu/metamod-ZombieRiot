import std;

import eiface;
import pm_defs;
import progdefs;
import util;

import CBase;
import Config;
import Escape;
import Hook;
import Player;
import Plugin;
import Resources;
import Round;

import UtlString;

using namespace std::string_literals;
using namespace std::string_view_literals;

using std::array;
using std::string_view;

//
// Retrieve info
//

extern void RetrieveMessageIndex(void) noexcept;

//
// callbacks
//

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

inline bool g_bShouldPrecache = true;

int fw_Spawn(edict_t* pEdict) noexcept
{
	static constexpr array g_rgszEntityShouldBeRemoved =
	{
		"func_bomb_target"sv,
		"info_bomb_target"sv,
		"info_vip_start"sv,
		"func_vip_safetyzone"sv,
		"func_escapezone"sv,
		"hostage_entity"sv,
		"monster_scientist"sv,
		"func_hostage_rescue"sv,
		"info_hostage_rescue"sv,
		"env_fog"sv,
		"env_rain"sv,
		"env_snow"sv,
		"armoury_entity"sv,
	};

	gpMetaGlobals->mres = MRES_IGNORED;

	[[unlikely]]
	if (g_bShouldPrecache)
	{
		// plugin_precache

		g_bShouldPrecache = false;

		Config::Load();

		auto const szMapName = std::string_view{ STRING(gpGlobals->mapname) };
		Escape::m_bEnabled = std::ranges::contains_subrange(szMapName, std::string_view{ "zr_" }, {}, tolower_c);

		/*
		* // #UNDONE_ESCAPEMODE
		* // Done part had been rip out.
			if(containi(szMapName, "zr_") != -1)
			{
				register_forward(FM_TraceLine, "fw_BarrierMessage_Post", 1)
				for(new i = 1; i < sizeof g_szGameWeaponClassName; i++)
				{
					if(!g_szGameWeaponClassName[i][0] || i == CSW_KNIFE || i == CSW_HEGRENADE || i == CSW_FLASHBANG || i == CSW_SMOKEGRENADE || i == CSW_C4)
						continue
					RegisterHam(Ham_Weapon_PrimaryAttack, g_szGameWeaponClassName[i], "HAM_FIXBOTPrimaryAttack")
				}
				RegisterHam(Ham_TakeDamage, "func_breakable", "HAM_BarrierTakeDamage")
				RegisterHam(Ham_Touch, "trigger_multiple", "HAM_EscapeModeTouch_Post", 1)
				RegisterHam(Ham_Touch, "func_wall", "HAM_LadderTouch")
				RegisterHam(Ham_Use, "func_button", "HAM_ButtonUse")
				RegisterHam(Ham_Use, "func_breakable", "HAM_BarrierUsed")
			}
		*/

		Resources::Precache();
	}

	// Block unwanted entities
	if (std::ranges::find(g_rgszEntityShouldBeRemoved, STRING(pEdict->v.classname)) != g_rgszEntityShouldBeRemoved.cend())
	{
		pEdict->v.flags |= FL_KILLME;
		gpMetaGlobals->mres = MRES_SUPERCEDE;

		return 0;
	}

	return *reinterpret_cast<int *>(gpMetaGlobals->orig_ret);
}

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
	RetrieveMessageIndex();

	// plugin_cfg

}

void fw_ServerDeactivate_Post(void) noexcept
{
	g_bShouldPrecache = true;
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

int fw_PrecacheModel(const char *s) noexcept
{
	static constexpr array RemovingList =
	{
		"models/w_backpack.mdl",
		"models/w_thighpack.mdl",
		"models/w_kevlar.mdl",
		"models/w_shield.mdl",
		"models/w_antidote.mdl",
		"models/w_security.mdl",
		"models/w_longjump.mdl",
	};

	for (auto &&psz : RemovingList)
	{
		if (!std::strcmp(psz, s) || string_view{ s }.starts_with("models/shield/"))
		{
			gpMetaGlobals->mres = MRES_SUPERCEDE;
			return 0;
		}
	}

	gpMetaGlobals->mres = MRES_IGNORED;
	return *reinterpret_cast<int *>(gpMetaGlobals->orig_ret);
}

int fw_PrecacheSound(const char *s) noexcept
{
	if (s[0] == 'h' && s[1] == 'o')
	{
		gpMetaGlobals->mres = MRES_SUPERCEDE;
		return 0;
	}

	static constexpr array RemovingList =
	{
		"items/suitcharge1.wav",
		"items/suitchargeno1.wav",
		"items/suitchargeok1.wav",
		"player/geiger6.wav",
		"player/geiger5.wav",
		"player/geiger4.wav",
		"player/geiger3.wav",
		"player/geiger2.wav",
		"player/geiger1.wav",
		"weapons/bullet_hit1.wav",
		"weapons/bullet_hit2.wav",
		"items/weapondrop1.wav",
		"weapons/generic_reload.wav",
		"buttons/bell1.wav",
		"buttons/blip1.wav",
		"buttons/blip2.wav",
		"buttons/button11.wav",
		"buttons/latchunlocked2.wav",
		"buttons/lightswitch2.wav",
		"ambience/quail1.wav",
		"events/tutor_msg.wav",
		"events/enemy_died.wav",
		"events/friend_died.wav",
		"events/task_complete.wav",
		"weapons/ak47_clipout.wav",
		"weapons/ak47_clipin.wav",
		"weapons/ak47_boltpull.wav",
		"weapons/aug_clipout.wav",
		"weapons/aug_clipin.wav",
		"weapons/aug_boltpull.wav",
		"weapons/aug_boltslap.wav",
		"weapons/aug_forearm.wav",
		"weapons/c4_click.wav",
		"weapons/c4_beep1.wav",
		"weapons/c4_beep2.wav",
		"weapons/c4_beep3.wav",
		"weapons/c4_beep4.wav",
		"weapons/c4_beep5.wav",
		"weapons/c4_explode1.wav",
		"weapons/c4_plant.wav",
		"weapons/c4_disarm.wav",
		"weapons/c4_disarmed.wav",
		"weapons/elite_reloadstart.wav",
		"weapons/elite_leftclipin.wav",
		"weapons/elite_clipout.wav",
		"weapons/elite_sliderelease.wav",
		"weapons/elite_rightclipin.wav",
		"weapons/elite_deploy.wav",
		"weapons/famas_clipout.wav",
		"weapons/famas_clipin.wav",
		"weapons/famas_boltpull.wav",
		"weapons/famas_boltslap.wav",
		"weapons/famas_forearm.wav",
		"weapons/g3sg1_slide.wav",
		"weapons/g3sg1_clipin.wav",
		"weapons/g3sg1_clipout.wav",
		"weapons/galil_clipout.wav",
		"weapons/galil_clipin.wav",
		"weapons/galil_boltpull.wav",
		"weapons/m4a1_clipin.wav",
		"weapons/m4a1_clipout.wav",
		"weapons/m4a1_boltpull.wav",
		"weapons/m4a1_deploy.wav",
		"weapons/m4a1_silencer_on.wav",
		"weapons/m4a1_silencer_off.wav",
		"weapons/m249_boxout.wav",
		"weapons/m249_boxin.wav",
		"weapons/m249_chain.wav",
		"weapons/m249_coverup.wav",
		"weapons/m249_coverdown.wav",
		"weapons/mac10_clipout.wav",
		"weapons/mac10_clipin.wav",
		"weapons/mac10_boltpull.wav",
		"weapons/mp5_clipout.wav",
		"weapons/mp5_clipin.wav",
		"weapons/mp5_slideback.wav",
		"weapons/p90_clipout.wav",
		"weapons/p90_clipin.wav",
		"weapons/p90_boltpull.wav",
		"weapons/p90_cliprelease.wav",
		"weapons/p228_clipout.wav",
		"weapons/p228_clipin.wav",
		"weapons/p228_sliderelease.wav",
		"weapons/p228_slidepull.wav",
		"weapons/scout_bolt.wav",
		"weapons/scout_clipin.wav",
		"weapons/scout_clipout.wav",
		"weapons/sg550_boltpull.wav",
		"weapons/sg550_clipin.wav",
		"weapons/sg550_clipout.wav",
		"weapons/sg552_clipout.wav",
		"weapons/sg552_clipin.wav",
		"weapons/sg552_boltpull.wav",
		"weapons/ump45_clipout.wav",
		"weapons/ump45_clipin.wav",
		"weapons/ump45_boltslap.wav",
		"weapons/usp_clipout.wav",
		"weapons/usp_clipin.wav",
		"weapons/usp_silencer_on.wav",
		"weapons/usp_silencer_off.wav",
		"weapons/usp_sliderelease.wav",
		"weapons/usp_slideback.wav"
	};

	for (auto &&psz : RemovingList)
	{
		if (!std::strcmp(psz, s))
		{
			gpMetaGlobals->mres = MRES_SUPERCEDE;
			return 0;
		}
	}

	gpMetaGlobals->mres = MRES_IGNORED;
	return *reinterpret_cast<int*>(gpMetaGlobals->orig_ret);
}

void fw_AlertMessage_Post(ALERT_TYPE atype, const char *szFmt, ...) noexcept
{
	gpMetaGlobals->mres = MRES_IGNORED;

	[[unlikely]]
	if (Escape::m_bEnabled && atype == at_logged && !std::strcmp(szFmt, "World triggered \"Round_Start\"\n"))
		Escape::Reset();
}
