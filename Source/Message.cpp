import std;

import progdefs;
import util;

import Hook;
import Plugin;

using std::any;
using std::string;
using std::tuple;
using std::vector;
using std::unordered_set;

//
// Retrieve Info
//

inline unordered_set<int> g_rgiMsgBlackList;

void MessageBlacklist(void) noexcept
{
	MF_MessageBlock(MSGBLOCK_SET, gmsgNVGToggle::m_iMessageIndex, (int *)&BLOCK_SET);
	MF_MessageBlock(MSGBLOCK_SET, gmsgSendCorpse::m_iMessageIndex, (int *)&BLOCK_SET);
	MF_MessageBlock(MSGBLOCK_SET, gmsgScoreAttrib::m_iMessageIndex, (int *)&BLOCK_SET);
	MF_MessageBlock(MSGBLOCK_SET, gmsgRoundTime::m_iMessageIndex, (int *)&BLOCK_SET);

	g_rgiMsgBlackList =
	{
		gmsgCurWeapon::m_iMessageIndex,
		gmsgDeathMsg::m_iMessageIndex,
		gmsgResetHUD::m_iMessageIndex,
		gmsgShowMenu::m_iMessageIndex,
		gmsgStatusIcon::m_iMessageIndex,
		gmsgMoney::m_iMessageIndex,
		gmsgAmmoPickup::m_iMessageIndex,
		gmsgFade::m_iMessageIndex,
		gmsgSendAudio::m_iMessageIndex,
		gmsgHealth::m_iMessageIndex,
		gmsgBattery::m_iMessageIndex,
		gmsgTextMsg::m_iMessageIndex,
		gmsgHideWeapon::m_iMessageIndex,
	};
}

void RetrieveMessageIndex(void) noexcept
{
	gmsgCurWeapon::Retrieve();
	gmsgNVGToggle::Retrieve();
	gmsgSendCorpse::Retrieve();
	gmsgScoreAttrib::Retrieve();
	gmsgRoundTime::Retrieve();
	gmsgDeathMsg::Retrieve();
	gmsgResetHUD::Retrieve();
	gmsgShowMenu::Retrieve();
	gmsgStatusIcon::Retrieve();
	gmsgMoney::Retrieve();
	gmsgAmmoPickup::Retrieve();
	gmsgFade::Retrieve();
	gmsgSendAudio::Retrieve();
	gmsgHealth::Retrieve();
	gmsgBattery::Retrieve();
	gmsgTextMsg::Retrieve();
	gmsgHideWeapon::Retrieve();
}

//
// Msg Hack
//

inline vector<any> g_rgMsgArgs;
inline bool g_bMessageArgsReady = false, g_bIntercepting = false;
inline tuple<int, int, const float *, edict_t *> g_MsgBeginArgs;

// LUNA: warning: this method can only be used on monothread situation.

void fw_MessageBegin(int iMsgDest, int iMsgIndex, const float *prgflOrigin, edict_t *pEdict) noexcept
{
	gpMetaGlobals->mres = MRES_IGNORED;

	if (!g_rgiMsgBlackList.contains(iMsgIndex))
		return;

	g_rgMsgArgs.clear();
	g_bMessageArgsReady = false;
	g_bIntercepting = true;
	g_MsgBeginArgs = std::make_tuple(iMsgDest, iMsgIndex, prgflOrigin, pEdict);

	gpMetaGlobals->mres = MRES_SUPERCEDE;
}

void fw_MessageEnd(void) noexcept
{
	gpMetaGlobals->mres = g_bIntercepting ? MRES_SUPERCEDE : MRES_IGNORED;

	if (!g_bIntercepting)
		return;

	g_bMessageArgsReady = true;
	g_bIntercepting = false;
}

void fw_WriteByte(int iValue) noexcept
{
	gpMetaGlobals->mres = g_bIntercepting ? MRES_SUPERCEDE : MRES_IGNORED;

	if (g_bIntercepting)
	{
		g_rgMsgArgs.emplace_back(static_cast<byte>(iValue));
	}
}

void fw_WriteChar(int iValue) noexcept
{
	gpMetaGlobals->mres = g_bIntercepting ? MRES_SUPERCEDE : MRES_IGNORED;

	if (g_bIntercepting)
	{
		g_rgMsgArgs.emplace_back(static_cast<char>(iValue));
	}
}

void fw_WriteShort(int iValue) noexcept
{
	gpMetaGlobals->mres = g_bIntercepting ? MRES_SUPERCEDE : MRES_IGNORED;

	if (g_bIntercepting)
	{
		g_rgMsgArgs.emplace_back(static_cast<std::int16_t>(iValue));
	}
}

void fw_WriteLong(int iValue) noexcept
{
	gpMetaGlobals->mres = g_bIntercepting ? MRES_SUPERCEDE : MRES_IGNORED;

	if (g_bIntercepting)
	{
		g_rgMsgArgs.emplace_back(static_cast<std::int32_t>(iValue));
	}
}

void fw_WriteAngle(float flValue) noexcept
{
	gpMetaGlobals->mres = g_bIntercepting ? MRES_SUPERCEDE : MRES_IGNORED;

	if (g_bIntercepting)
	{
		g_rgMsgArgs.emplace_back(flValue);
	}
}

void fw_WriteCoord(float flValue) noexcept
{
	gpMetaGlobals->mres = g_bIntercepting ? MRES_SUPERCEDE : MRES_IGNORED;

	if (g_bIntercepting)
	{
		g_rgMsgArgs.emplace_back(flValue);
	}
}

void fw_WriteString(const char *sz) noexcept
{
	gpMetaGlobals->mres = g_bIntercepting ? MRES_SUPERCEDE : MRES_IGNORED;

	if (g_bIntercepting)
	{
		g_rgMsgArgs.emplace_back(string(sz));
	}
}

void fw_WriteEntity(int iValue) noexcept	// LUNA: actual writes: 2 bytes. like a short.
{
	gpMetaGlobals->mres = g_bIntercepting ? MRES_SUPERCEDE : MRES_IGNORED;

	if (g_bIntercepting)
	{
		g_rgMsgArgs.emplace_back(ent_cast<edict_t *>(iValue));
	}
}

extern "C++" namespace Message
{
	void Clear(void) noexcept;
}

void Message::Clear(void) noexcept
{
	g_rgMsgArgs.clear();
	g_rgiMsgBlackList.clear();
	g_bMessageArgsReady = false;
	g_bIntercepting = false;
	g_MsgBeginArgs = { 0, 0, nullptr, nullptr };
}

//
// Message Receiver
//
