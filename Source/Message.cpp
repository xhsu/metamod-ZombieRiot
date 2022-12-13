import std;

import progdefs;
import util;

import Plugin;

using std::any;
using std::string;
using std::tuple;
using std::vector;

inline vector<any> g_rgMsgArgs;
inline vector<int> g_rgiMsgBlackList;
inline bool g_bMessageArgsReady = false, g_bIntercepting = false;
inline tuple<int, int, const float *, edict_t *> g_MsgBeginArgs;

// LUNA: warning: this method can only be used on monothread situation.

void fw_MessageBegin(int iMsgDest, int iMsgIndex, const float *prgflOrigin, edict_t *pEdict) noexcept
{
	gpMetaGlobals->mres = MRES_IGNORED;

	if (std::ranges::find(g_rgiMsgBlackList, iMsgIndex) == g_rgiMsgBlackList.cend())
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

	if (g_bIntercepting)
	{
		g_bMessageArgsReady = true;
		g_bIntercepting = false;
	}
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