import util;

import CVar;
import Escape;
import GameRules;
import Plugin;
import Query;

using std::string_view;

extern "C++" namespace Escape
{
	inline int KeepSection{ 0 };
	inline int SectionAmount{ 0 };
	inline short HAmount{ 0 }, ZAmount{ 0 };
};

void Escape::Reset(void) noexcept
{
	KeepSection = 0;

	SectionAmount = 0;

	for (edict_t *pEdict : FIND_ENTITY_BY_CLASSNAME("trigger_multiple"))
	{
		if (!std::strcmp(STRING(pEdict->v.targetname), "zr_section"))
		{
			pEdict->v.iuser2 = 0;
			++SectionAmount;
		}
	}

	HAmount = 0;
	ZAmount = 0;
	for (CBasePlayer *pPlayer : Query::all_players())
	{
		switch (pPlayer->m_iTeam)
		{
		case TEAM_CT:
			++HAmount;
			break;

		case TEAM_TERRORIST:
			++ZAmount;
			break;

		default:
			break;
		}
	}

	if (!HAmount || !ZAmount)
		return;

	float Threshold = (float(HAmount) / 16.f) * (float(HAmount) / float(ZAmount));

	for (edict_t *pEdict : FIND_ENTITY_BY_CLASSNAME("func_breakable"))
	{
		string_view szTargetName{ STRING(pEdict->v.targetname) };

		if (!szTargetName.starts_with("zr_barrier"))
			continue;

		pEdict->v.health = pEdict->v.max_health = CVar::barrier_health[szTargetName[11] - 'a']->value * Threshold;
	}
}
