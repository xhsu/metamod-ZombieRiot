module;

#include <Windows.h>

#include <wchar.h>

export module Platform;

import std;

export inline std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> g_utf_converter;

export template <typename... Tys> [[noreturn]] void UTIL_Terminate(const char *psz, Tys&&... args) noexcept
{
	static char sz[256]{};
	_snprintf(sz, _countof(sz), psz, std::forward<Tys>(args)...);

	MessageBoxA(nullptr, psz, nullptr, MB_OK);
	std::terminate();
}

export template <typename... Tys> [[noreturn]] void UTIL_Terminate(const wchar_t *psz, Tys&&... args) noexcept
{
	static constexpr auto fnArgHandle = []<typename T>(T &&arg) noexcept -> decltype(auto)
	{
		static char16_t rgu16[64]{};

		if constexpr (std::is_same_v<std::remove_cvref_t<T>, std::string>)
		{
			auto const szu16 = g_utf_converter.from_bytes(arg);
			memcpy(rgu16, szu16.c_str(), szu16.size() * sizeof(char16_t));

			return &rgu16[0];
		}
		else
			return arg;
	};

	static wchar_t wsz[256]{};
	_snwprintf(wsz, _countof(wsz), psz,
		fnArgHandle(args)...);

	MessageBoxW(nullptr, wsz, nullptr, MB_OK);
	std::terminate();
}
