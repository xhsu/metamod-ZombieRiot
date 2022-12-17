export module CVar;

export import std;

export import cvardef;
export import eiface;

using std::array;
using std::string;

export extern "C++" namespace CVar
{
	inline bool m_bRegistered{ false };
	inline array<cvar_t *, 26> barrier_health{};

	inline void Register() noexcept
	{
		[[likely]]
		if (m_bRegistered)
			return;

		for (std::uint8_t i = 0; i < 26; ++i)
		{
			auto const psz = new string(std::format("zr_barrier_health_{}", static_cast<char>('a' + i)));
			g_engfuncs.pfnCVarRegister(new cvar_t{ .name = psz->c_str(), .string = "28.0", .flags = 0, .value = 28.f, .next = nullptr });

			barrier_health[i] = g_engfuncs.pfnCVarGetPointer(psz->c_str());	// So it's not a leak, i guess?
		}

		m_bRegistered = true;
	}
};
