export module Player;

export import <array>;

export import CBase;

using std::array;

export inline constexpr auto SEMICLIPKEY = 13854;
export inline constexpr auto ZRWALLCLIPKEY = 242241;

export enum EHitBoxGroup
{
	HITGROUP_GENERIC = 0,
	HITGROUP_HEAD,
	HITGROUP_CHEST,
	HITGROUP_STOMACH,
	HITGROUP_LEFTARM,
	HITGROUP_RIGHTARM,
	HITGROUP_LEFTLEG,
	HITGROUP_RIGHTLEG,
	HITGROUP_SHIELD,
	NUM_HITGROUPS,
};

export enum EThrowDirection
{
	THROW_NONE,
	THROW_FORWARD,
	THROW_BACKWARD,
	THROW_HITVEL,
	THROW_BOMB,
	THROW_GRENADE,
	THROW_HITVEL_MINUS_AIRVEL
};

export inline constexpr auto PLAYER_CUSTOM = static_cast<PLAYER_ANIM>(PLAYER_HOLDBOMB + 1);
export inline constexpr auto ACT_CUSTOM = static_cast<Activity>(ACT_NO + 1);

export extern "C++" namespace Player
{
	inline array<int, 33> GaitSequence{};
	inline array<int, 33> Sequence{};
	inline array<float, 33> PlayTime{};
	inline array<float, 33> RecordTime{};

	void SetAnimation(int iPlayer, PLAYER_ANIM iAnim) noexcept;
	void Hook(void) noexcept;
};

export extern "C++" int LookupActivity(studiohdr_t *pstudiohdr, entvars_t *pev, Activity activity) noexcept;
export extern "C++" int LookupSequence(studiohdr_t *pstudiohdr, const char *label) noexcept;
export extern "C++" void GetSequenceInfo(studiohdr_t *pstudiohdr, entvars_t *pev, float *pflFrameRate, float *pflGroundSpeed) noexcept;
export extern "C++" int GetSequenceFlags(studiohdr_t *pstudiohdr, entvars_t *pev) noexcept;
export extern "C++" void ResetSequenceInfo(CBaseAnimating * pEntity) noexcept;
