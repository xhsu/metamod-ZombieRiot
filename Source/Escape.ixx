export module Escape;

export extern "C++" namespace Escape
{
	inline bool m_bEnabled = false;

	void Reset(void) noexcept;
};
