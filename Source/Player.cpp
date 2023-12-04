import CBase;
import GameRules;

import Plugin;
import Player;

import UtlHook;
import UtlRandom;

int LookupActivity(studiohdr_t *pstudiohdr, entvars_t *pev, Activity activity) noexcept
{
	if (!pstudiohdr)
		return 0;

	int i{};
	int weightTotal = 0;
	int activitySequenceCount = 0;
	int weight = 0;
	int select{};

	auto pseqdesc = (mstudioseqdesc_t *)((byte *)pstudiohdr + pstudiohdr->seqindex);

	for (i = 0; i < pstudiohdr->numseq; i++)
	{
		if (pseqdesc[i].activity == activity)
		{
			weightTotal += pseqdesc[i].actweight;
			activitySequenceCount++;
		}
	}

	if (activitySequenceCount > 0)
	{
		if (weightTotal)
		{
			auto const which = UTIL_Random(0, weightTotal - 1);

			for (i = 0; i < pstudiohdr->numseq; i++)
			{
				if (pseqdesc[i].activity == activity)
				{
					weight += pseqdesc[i].actweight;

					if (weight > which)
					{
						return (Activity)i;
					}
				}
			}
		}
		else
		{
			select = UTIL_Random(0, activitySequenceCount - 1);

			for (i = 0; i < pstudiohdr->numseq; i++)
			{
				if (pseqdesc[i].activity == activity)
				{
					if (select == 0)
					{
						return (Activity)i;
					}

					select--;
				}
			}
		}
	}

	return -1;
}

int LookupSequence(studiohdr_t *pstudiohdr, const char *label) noexcept
{
	if (!pstudiohdr)
		return 0;

	// Look up by sequence name.
	mstudioseqdesc_t *pseqdesc = (mstudioseqdesc_t *)((byte *)pstudiohdr + pstudiohdr->seqindex);
	for (int i = 0; i < pstudiohdr->numseq; i++)
	{
		if (!_stricmp(pseqdesc[i].label, label))
			return i;
	}

	// Not found
	return -1;
}

void GetSequenceInfo(studiohdr_t *pstudiohdr, entvars_t *pev, float *pflFrameRate, float *pflGroundSpeed) noexcept
{
	if (!pstudiohdr)
		return;

	if (pev->sequence >= pstudiohdr->numseq)
	{
		*pflFrameRate = 0;
		*pflGroundSpeed = 0;
		return;
	}

	mstudioseqdesc_t *pseqdesc = (mstudioseqdesc_t *)((byte *)pstudiohdr + pstudiohdr->seqindex) + int(pev->sequence);
	if (pseqdesc->numframes <= 1)
	{
		*pflFrameRate = 256.0f;
		*pflGroundSpeed = 0.0f;
		return;
	}

	*pflFrameRate = pseqdesc->fps * 256.0f / (pseqdesc->numframes - 1);
	*pflGroundSpeed = sqrt(pseqdesc->linearmovement[0] * pseqdesc->linearmovement[0] + pseqdesc->linearmovement[1] * pseqdesc->linearmovement[1] + pseqdesc->linearmovement[2] * pseqdesc->linearmovement[2]);
	*pflGroundSpeed = *pflGroundSpeed * pseqdesc->fps / (pseqdesc->numframes - 1);
}

int GetSequenceFlags(studiohdr_t *pstudiohdr, entvars_t *pev) noexcept
{
	if (!pstudiohdr || pev->sequence >= pstudiohdr->numseq)
		return 0;

	mstudioseqdesc_t *pseqdesc = (mstudioseqdesc_t *)((byte *)pstudiohdr + pstudiohdr->seqindex) + int(pev->sequence);
	return pseqdesc->flags;
}

void ResetSequenceInfo(CBaseAnimating* pEntity) noexcept
{
	auto const pModel = g_engfuncs.pfnGetModelPtr(pEntity->edict());

	GetSequenceInfo(pModel, pEntity->pev, &pEntity->m_flFrameRate, &pEntity->m_flGroundSpeed);
	pEntity->m_fSequenceLoops = ((GetSequenceFlags(pModel, pEntity->pev) & STUDIO_LOOPING) != 0);
	pEntity->pev->animtime = gpGlobals->time;
	pEntity->pev->framerate = 1.0f;

	pEntity->m_fSequenceFinished = false;
	pEntity->m_flLastEventCheck = gpGlobals->time;
}

void __fastcall HamF_SetAnimation(CBasePlayer *pPlayer, int, PLAYER_ANIM playerAnim) noexcept
{
	auto const iPlayer = pPlayer->entindex();

	if (Player::GaitSequence[iPlayer] >= 0 && pPlayer->pev->deadflag == DEAD_NO)
	{
		if (gpGlobals->time > Player::RecordTime[iPlayer] + Player::PlayTime[iPlayer])
			Player::GaitSequence[iPlayer] = -1;

		pPlayer->pev->sequence = Player::Sequence[iPlayer];
		pPlayer->pev->gaitsequence = Player::GaitSequence[iPlayer];

		return;
	}

	if (!pPlayer->pev->modelindex)
		return;

	if ((playerAnim == PLAYER_FLINCH || playerAnim == PLAYER_LARGE_FLINCH) && pPlayer->m_bOwnsShield)
		return;

	if (playerAnim != PLAYER_FLINCH && playerAnim != PLAYER_LARGE_FLINCH && pPlayer->m_flFlinchTime > gpGlobals->time && pPlayer->pev->health > 0.0f)
		return;

	auto speed = pPlayer->pev->velocity.Length2D();

	if (pPlayer->pev->flags & FL_FROZEN)
	{
		speed = 0;
		playerAnim = PLAYER_IDLE;
	}

	auto const pModel = g_engfuncs.pfnGetModelPtr(pPlayer->edict());
	auto hopSeq = LookupActivity(pModel, pPlayer->pev, ACT_HOP);
	auto leapSeq = LookupActivity(pModel, pPlayer->pev, ACT_LEAP);

	switch (playerAnim)
	{
	case PLAYER_CUSTOM:
	{
		if (pPlayer->m_Activity == ACT_SWIM || pPlayer->m_Activity == ACT_DIESIMPLE || pPlayer->m_Activity == ACT_HOVER)
			pPlayer->m_IdealActivity = pPlayer->m_Activity;
		else
			pPlayer->m_IdealActivity = ACT_CUSTOM;

		break;
	}
	case PLAYER_JUMP:
	{
		if (pPlayer->m_Activity == ACT_SWIM || pPlayer->m_Activity == ACT_DIESIMPLE || pPlayer->m_Activity == ACT_HOVER)
			pPlayer->m_IdealActivity = pPlayer->m_Activity;
		else
		{
			pPlayer->m_IdealActivity = ACT_HOP;
			//if (TheBots)
			//	TheBots->OnEvent(EVENT_PLAYER_JUMPED, this);
		}
		break;
	}
	case PLAYER_SUPERJUMP:
	{
		if (pPlayer->m_Activity == ACT_SWIM || pPlayer->m_Activity == ACT_DIESIMPLE || pPlayer->m_Activity == ACT_HOVER)
			pPlayer->m_IdealActivity = pPlayer->m_Activity;
		else
			pPlayer->m_IdealActivity = ACT_LEAP;
		break;
	}
	case PLAYER_DIE:
	{
		pPlayer->m_IdealActivity = ACT_DIESIMPLE;
		break;
	}
	case PLAYER_ATTACK1:
	{
		if (pPlayer->m_Activity == ACT_SWIM || pPlayer->m_Activity == ACT_DIESIMPLE || pPlayer->m_Activity == ACT_HOVER)
			pPlayer->m_IdealActivity = pPlayer->m_Activity;
		else
		{
			pPlayer->m_IdealActivity = ACT_RANGE_ATTACK1;
			//if (TheBots)
			//	TheBots->OnEvent(EVENT_WEAPON_FIRED, this);
		}
		break;
	}
	case PLAYER_ATTACK2:
	{
		if (pPlayer->m_Activity == ACT_SWIM || pPlayer->m_Activity == ACT_DIESIMPLE || pPlayer->m_Activity == ACT_HOVER)
			pPlayer->m_IdealActivity = pPlayer->m_Activity;
		else
		{
			pPlayer->m_IdealActivity = ACT_RANGE_ATTACK2;
			//if (TheBots)
			//	TheBots->OnEvent(EVENT_WEAPON_FIRED, this);
		}
		break;
	}
	case PLAYER_RELOAD:
	{
		if (pPlayer->m_Activity == ACT_SWIM || pPlayer->m_Activity == ACT_DIESIMPLE || pPlayer->m_Activity == ACT_HOVER)
			pPlayer->m_IdealActivity = pPlayer->m_Activity;
		else
		{
			pPlayer->m_IdealActivity = ACT_RELOAD;
			//if (TheBots)
			//	TheBots->OnEvent(EVENT_WEAPON_RELOADED, this);
		}
		break;
	}
	case PLAYER_IDLE:
	case PLAYER_WALK:
	{
		if (pPlayer->pev->flags & FL_ONGROUND || (pPlayer->m_Activity != ACT_HOP && pPlayer->m_Activity != ACT_LEAP))
		{
			if (pPlayer->pev->waterlevel <= 1)
				pPlayer->m_IdealActivity = ACT_WALK;

			else if (speed == 0.0f)
				pPlayer->m_IdealActivity = ACT_HOVER;

			else
				pPlayer->m_IdealActivity = ACT_SWIM;
		}
		else
			pPlayer->m_IdealActivity = pPlayer->m_Activity;
		break;
	}
	case PLAYER_HOLDBOMB:
		pPlayer->m_IdealActivity = ACT_HOLDBOMB;
		break;
	case PLAYER_FLINCH:
		pPlayer->m_IdealActivity = ACT_FLINCH;
		break;
	case PLAYER_LARGE_FLINCH:
		pPlayer->m_IdealActivity = ACT_LARGE_FLINCH;
		break;
	default:
		break;
	}

	char szAnim[64]{};
	int animDesired{};

	switch (pPlayer->m_IdealActivity)
	{
	case ACT_CUSTOM:
	{
		Player::RecordTime[iPlayer] = gpGlobals->time;

		animDesired = Player::Sequence[iPlayer];

		if (animDesired == -1)
			animDesired = 0;

		pPlayer->pev->sequence = animDesired;
		pPlayer->pev->frame = 0;
		ResetSequenceInfo(pPlayer);
		pPlayer->m_Activity = pPlayer->m_IdealActivity;

		break;
	}
	case ACT_HOP:
	case ACT_LEAP:
	{
		if (pPlayer->m_Activity == pPlayer->m_IdealActivity)
			return;

		if (pPlayer->m_Activity == ACT_CUSTOM)
			animDesired = Player::Sequence[iPlayer];
		else
		{
			switch (pPlayer->m_Activity)
			{
			case ACT_RANGE_ATTACK1:
				strcpy(szAnim, "ref_shoot_");
				break;

			case ACT_RANGE_ATTACK2:
				strcpy(szAnim, "ref_shoot2_");
				break;

			case ACT_RELOAD:
				strcpy(szAnim, "ref_reload_");
				break;

			default:
				strcpy(szAnim, "ref_aim_");
				break;
			}

			strcat(szAnim, pPlayer->m_szAnimExtention);
			animDesired = LookupSequence(pModel, szAnim);
		}

		if (animDesired == -1)
			animDesired = 0;

		if (pPlayer->pev->sequence != animDesired || !pPlayer->m_fSequenceLoops)
			pPlayer->pev->frame = 0;

		if (!pPlayer->m_fSequenceLoops)
			pPlayer->pev->effects |= EF_NOINTERP;

		if (pPlayer->m_IdealActivity == ACT_LEAP)
			pPlayer->pev->gaitsequence = LookupActivity(pModel, pPlayer->pev, ACT_LEAP);
		else
			pPlayer->pev->gaitsequence = LookupActivity(pModel, pPlayer->pev, ACT_HOP);

		pPlayer->m_Activity = pPlayer->m_IdealActivity;
		break;
	}
	case ACT_RANGE_ATTACK1:
	{
		pPlayer->m_flLastFired = gpGlobals->time;

		if (pPlayer->pev->flags & FL_DUCKING)
			strcpy(szAnim, "crouch_shoot_");
		else
			strcpy(szAnim, "ref_shoot_");

		strcat(szAnim, pPlayer->m_szAnimExtention);
		animDesired = LookupSequence(pModel, szAnim);
		if (animDesired == -1)
			animDesired = 0;

		pPlayer->pev->sequence = animDesired;
		pPlayer->pev->frame = 0;

		ResetSequenceInfo(pPlayer);
		pPlayer->m_Activity = pPlayer->m_IdealActivity;
		break;
	}
	case ACT_RANGE_ATTACK2:
	{
		pPlayer->m_flLastFired = gpGlobals->time;

		if (pPlayer->pev->flags & FL_DUCKING)
			strcpy(szAnim, "crouch_shoot2_");
		else
			strcpy(szAnim, "ref_shoot2_");

		strcat(szAnim, pPlayer->m_szAnimExtention);
		animDesired = LookupSequence(pModel, szAnim);
		if (animDesired == -1)
			animDesired = 0;

		pPlayer->pev->sequence = animDesired;
		pPlayer->pev->frame = 0;

		ResetSequenceInfo(pPlayer);
		pPlayer->m_Activity = pPlayer->m_IdealActivity;
		break;
	}
	case ACT_RELOAD:
	{
		if (pPlayer->pev->flags & FL_DUCKING)
			strcpy(szAnim, "crouch_reload_");
		else
			strcpy(szAnim, "ref_reload_");

		strcat(szAnim, pPlayer->m_szAnimExtention);
		animDesired = LookupSequence(pModel, szAnim);
		if (animDesired == -1)
			animDesired = 0;

		if (pPlayer->pev->sequence != animDesired || !pPlayer->m_fSequenceLoops)
			pPlayer->pev->frame = 0;

		if (!pPlayer->m_fSequenceLoops)
			pPlayer->pev->effects |= EF_NOINTERP;

		pPlayer->m_Activity = pPlayer->m_IdealActivity;
		break;
	}
	case ACT_HOLDBOMB:
	{
		if (pPlayer->pev->flags & FL_DUCKING)
			strcpy(szAnim, "crouch_aim_");
		else
			strcpy(szAnim, "ref_aim_");

		strcat(szAnim, pPlayer->m_szAnimExtention);
		animDesired = LookupSequence(pModel, szAnim);
		if (animDesired == -1)
			animDesired = 0;

		pPlayer->m_Activity = pPlayer->m_IdealActivity;
		break;
	}
	case ACT_WALK:
	{
		if ((pPlayer->m_Activity != ACT_CUSTOM || pPlayer->m_fSequenceFinished)
			&& (pPlayer->m_Activity != ACT_RANGE_ATTACK1 || pPlayer->m_fSequenceFinished)
			&& (pPlayer->m_Activity != ACT_RANGE_ATTACK2 || pPlayer->m_fSequenceFinished)
			&& (pPlayer->m_Activity != ACT_FLINCH || pPlayer->m_fSequenceFinished)
			&& (pPlayer->m_Activity != ACT_LARGE_FLINCH || pPlayer->m_fSequenceFinished)
			&& (pPlayer->m_Activity != ACT_RELOAD || pPlayer->m_fSequenceFinished))
		{
			if (speed <= 135.0f || gpGlobals->time <= pPlayer->m_flLastFired + 2.f || gpGlobals->time <= Player::RecordTime[iPlayer] + Player::PlayTime[iPlayer])
			{
				if (pPlayer->pev->flags & FL_DUCKING)
					strcpy(szAnim, "crouch_aim_");
				else
					strcpy(szAnim, "ref_aim_");

				strcat(szAnim, pPlayer->m_szAnimExtention);
				animDesired = LookupSequence(pModel, szAnim);
				if (animDesired == -1)
					animDesired = 0;

				pPlayer->m_Activity = ACT_WALK;
			}
			else
			{
				strcpy(szAnim, "run_");
				strcat(szAnim, pPlayer->m_szAnimExtention);
				animDesired = LookupSequence(pModel, szAnim);

				if (animDesired == -1)
				{
					if (pPlayer->pev->flags & FL_DUCKING)
						strcpy(szAnim, "crouch_aim_");
					else
						strcpy(szAnim, "ref_aim_");

					strcat(szAnim, pPlayer->m_szAnimExtention);
					animDesired = LookupSequence(pModel, szAnim);
					if (animDesired == -1)
						animDesired = 0;

					pPlayer->m_Activity = ACT_RUN;
					pPlayer->pev->gaitsequence = LookupActivity(pModel, pPlayer->pev, ACT_RUN);
				}
				else
				{
					pPlayer->m_Activity = ACT_RUN;
					pPlayer->pev->gaitsequence = animDesired;
				}

				//if (pPlayer->m_Activity == ACT_RUN)
				//{
					// TODO: maybe away used variable 'speed'?
					//if (speed > 150.0f)
					//if (pPlayer->pev->velocity.Length2D() > 150.0f)
					//{
					//	if (TheBots)
					//		TheBots->OnEvent(EVENT_PLAYER_FOOTSTEP, this);
					//}
				//}
			}
		}
		else
			animDesired = pPlayer->pev->sequence;

		if (speed > 135.0f)
			pPlayer->pev->gaitsequence = LookupActivity(pModel, pPlayer->pev, ACT_RUN);
		else
			pPlayer->pev->gaitsequence = LookupActivity(pModel, pPlayer->pev, ACT_WALK);
		break;
	}
	case ACT_FLINCH:
	case ACT_LARGE_FLINCH:
	{
		pPlayer->m_Activity = pPlayer->m_IdealActivity;

		switch (pPlayer->m_LastHitGroup)
		{
		case HITGROUP_GENERIC:
		{
			if (UTIL_Random())
				animDesired = LookupSequence(pModel, "head_flinch");
			else
				animDesired = LookupSequence(pModel, "gut_flinch");
			break;
		}
		case HITGROUP_HEAD:
		case HITGROUP_CHEST:
			animDesired = LookupSequence(pModel, "head_flinch");
			break;
		case HITGROUP_SHIELD:
			animDesired = 0;
			break;
		default:
			animDesired = LookupSequence(pModel, "gut_flinch");
			break;
		}

		if (animDesired == -1)
			animDesired = 0;

		break;
	}
	case ACT_DIESIMPLE:
	{
		if (pPlayer->m_Activity == pPlayer->m_IdealActivity)
			return;

		pPlayer->m_Activity = pPlayer->m_IdealActivity;
		pPlayer->m_flDeathThrowTime = 0;
		pPlayer->m_iThrowDirection = THROW_NONE;

		switch (pPlayer->m_LastHitGroup)
		{
		case HITGROUP_GENERIC:
		{
			switch (UTIL_Random(0, 8))
			{
			case 0:
				animDesired = LookupActivity(pModel, pPlayer->pev, ACT_DIE_HEADSHOT);
				pPlayer->m_iThrowDirection = THROW_BACKWARD;
				break;
			case 1:
				animDesired = LookupActivity(pModel, pPlayer->pev, ACT_DIE_GUTSHOT);
				break;
			case 2:
				animDesired = LookupActivity(pModel, pPlayer->pev, ACT_DIE_BACKSHOT);
				pPlayer->m_iThrowDirection = THROW_HITVEL;
				break;
			case 4:
				animDesired = LookupActivity(pModel, pPlayer->pev, ACT_DIEBACKWARD);
				pPlayer->m_iThrowDirection = THROW_HITVEL;
				break;
			case 5:
				animDesired = LookupActivity(pModel, pPlayer->pev, ACT_DIEFORWARD);
				pPlayer->m_iThrowDirection = THROW_FORWARD;
				break;
			case 6:
				animDesired = LookupActivity(pModel, pPlayer->pev, ACT_DIE_CHESTSHOT);
				break;
			case 7:
				animDesired = LookupActivity(pModel, pPlayer->pev, ACT_DIE_GUTSHOT);
				break;
			case 8:
				animDesired = LookupActivity(pModel, pPlayer->pev, ACT_DIE_HEADSHOT);
				break;
			default:
				animDesired = LookupActivity(pModel, pPlayer->pev, ACT_DIESIMPLE);
				break;
			}
			break;
		}
		case HITGROUP_HEAD:
		{
			int random = UTIL_Random(0, 8);
			pPlayer->m_bHeadshotKilled = true;

			if (pPlayer->m_bHighDamage)
				++random;

			switch (random)
			{
			case 1:
			case 2:
				pPlayer->m_iThrowDirection = THROW_BACKWARD;
				break;
			case 3:
			case 4:
			case 5:
			case 6:
				pPlayer->m_iThrowDirection = THROW_HITVEL;
				break;
			default:
				pPlayer->m_iThrowDirection = THROW_NONE;
				break;
			}

			animDesired = LookupActivity(pModel, pPlayer->pev, ACT_DIE_HEADSHOT);
			break;
		}
		case HITGROUP_CHEST:
			animDesired = LookupActivity(pModel, pPlayer->pev, ACT_DIE_CHESTSHOT);
			break;
		case HITGROUP_STOMACH:
			animDesired = LookupActivity(pModel, pPlayer->pev, ACT_DIE_GUTSHOT);
			break;
		case HITGROUP_LEFTARM:
			animDesired = LookupSequence(pModel, "left");
			break;
		case HITGROUP_RIGHTARM:
		{
			pPlayer->m_iThrowDirection = UTIL_Random() ? THROW_HITVEL : THROW_HITVEL_MINUS_AIRVEL;
			animDesired = LookupSequence(pModel, "right");
			break;
		}
		default:
			animDesired = LookupActivity(pModel, pPlayer->pev, ACT_DIESIMPLE);
			break;
		}

		if (pPlayer->pev->flags & FL_DUCKING)
		{
			animDesired = LookupSequence(pModel, "crouch_die");
			pPlayer->m_iThrowDirection = THROW_BACKWARD;
		}
		else if (pPlayer->m_bKilledByBomb || pPlayer->m_bKilledByGrenade)
		{
			g_engfuncs.pfnMakeVectors(pPlayer->pev->angles);

			if (DotProduct(gpGlobals->v_forward, pPlayer->m_vBlastVector) > 0.0f)
				animDesired = LookupSequence(pModel, "left");

			else
			{
				if (UTIL_Random())
					animDesired = LookupSequence(pModel, "crouch_die");
				else
					animDesired = LookupActivity(pModel, pPlayer->pev, ACT_DIE_HEADSHOT);
			}

			if (pPlayer->m_bKilledByBomb)
				pPlayer->m_iThrowDirection = THROW_BOMB;

			else if (pPlayer->m_bKilledByGrenade)
				pPlayer->m_iThrowDirection = THROW_GRENADE;
		}

		if (animDesired == -1)
			animDesired = 0;

		if (pPlayer->pev->sequence != animDesired)
		{
			pPlayer->pev->gaitsequence = 0;
			pPlayer->pev->sequence = animDesired;
			pPlayer->pev->frame = 0.0f;
			ResetSequenceInfo(pPlayer);
		}
		return;
	}
	default:
	{
		if (pPlayer->m_Activity == pPlayer->m_IdealActivity)
			return;

		pPlayer->m_Activity = pPlayer->m_IdealActivity;
		animDesired = LookupActivity(pModel, pPlayer->pev, pPlayer->m_IdealActivity);

		if (pPlayer->pev->sequence != animDesired)
		{
			pPlayer->pev->gaitsequence = 0;
			pPlayer->pev->sequence = animDesired;
			pPlayer->pev->frame = 0;

			ResetSequenceInfo(pPlayer);
		}
		return;
	}
	}

	if (pPlayer->pev->gaitsequence != hopSeq && pPlayer->pev->gaitsequence != leapSeq)
	{
		if (pPlayer->pev->flags & FL_DUCKING)
		{
			if (speed != 0.0f)
				pPlayer->pev->gaitsequence = LookupActivity(pModel, pPlayer->pev, ACT_CROUCH);
			else
				pPlayer->pev->gaitsequence = LookupActivity(pModel, pPlayer->pev, ACT_CROUCHIDLE);
		}
		else
		{
			if (speed > 135.f)
			{
				if (gpGlobals->time > pPlayer->m_flLastFired + 2.f && gpGlobals->time > Player::RecordTime[iPlayer] + Player::PlayTime[iPlayer])
				{
					if (pPlayer->m_Activity != ACT_FLINCH && pPlayer->m_Activity != ACT_LARGE_FLINCH)
					{
						strcpy(szAnim, "run_");
						strcat(szAnim, pPlayer->m_szAnimExtention);

						animDesired = LookupSequence(pModel, szAnim);
						if (animDesired == -1)
						{
							if (pPlayer->m_iTeam == TEAM_CT)
							{
								if (pPlayer->pev->flags & FL_DUCKING)
									strcpy(szAnim, "crouch_aim_");
								else
									strcpy(szAnim, "ref_aim_");

								strcat(szAnim, pPlayer->m_szAnimExtention);
								animDesired = LookupSequence(pModel, szAnim);
							}
							else
								animDesired = LookupSequence(pModel, "run");
						}
						else
							pPlayer->pev->gaitsequence = animDesired;

						pPlayer->m_Activity = ACT_RUN;
					}
				}
				pPlayer->pev->gaitsequence = LookupActivity(pModel, pPlayer->pev, ACT_RUN);
			}
			else
			{
				if (speed > 0.0f)
					pPlayer->pev->gaitsequence = LookupActivity(pModel, pPlayer->pev, ACT_WALK);
				else
					pPlayer->pev->gaitsequence = LookupActivity(pModel, pPlayer->pev, ACT_IDLE);
			}
		}
	}
	if (pPlayer->pev->sequence != animDesired)
	{
		pPlayer->pev->sequence = animDesired;
		pPlayer->pev->frame = 0;

		ResetSequenceInfo(pPlayer);
	}
}

void Player::SetAnimation(int iPlayer, PLAYER_ANIM iAnim) noexcept
{
	if (EHANDLE<CBasePlayer> pPlayer{ g_engfuncs.pfnPEntityOfEntIndex(iPlayer) }; pPlayer)
		HamF_SetAnimation(pPlayer, 0, iAnim);
}

inline constexpr unsigned char SET_ANIMATION_FN_NEW_PATTERN[] = "\x90\x83\xEC\x4C\x53\x55\x8B\x2A\x56\x57\x8B\x4D\x04\x8B\x2A\x2A\x2A\x2A\x2A\x85\xC0";
inline constexpr unsigned char SET_ANIMATION_FN_ANNIV_PATTERN[] = "\xCC\x55\x8B\xEC\x83\xE4\xF8\x83\xEC\x54\xA1\x2A\x2A\x2A\x2A\x33\xC4\x89\x44\x24\x50";

void Player::Hook(void) noexcept
{
	Player::GaitSequence.fill(-1);
	Player::Sequence.fill(0);
	Player::PlayTime.fill(0.f);
	Player::RecordTime.fill(0.f);

	static bool bHooked = false;

	[[unlikely]]
	if (!bHooked)
	{
		bHooked = true;

		unsigned char rgPatch[5]{}, rgOriginalBytes[5]{};
		auto const addr = UTIL_SearchPattern("mp.dll", 1, SET_ANIMATION_FN_NEW_PATTERN, SET_ANIMATION_FN_ANNIV_PATTERN);
		UTIL_PreparePatch(addr, &HamF_SetAnimation, rgPatch, rgOriginalBytes);
		UTIL_DoPatch(addr, rgPatch);
	}
}
