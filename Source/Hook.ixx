export module Hook;

export import std.compat;

export import Message;

export using gmsgCurWeapon = Message_t<"CurWeapon", bool /* state */, uint8_t /* iId */, uint8_t /* iClip */>;
export using gmsgNVGToggle = Message_t<"NVGToggle", bool /* state */>;
export using gmsgSendCorpse = Message_t<"ClCorpse", const char* /* model */, uint32_t /* coord*128 */, uint32_t, uint32_t, Angles /* dead body angles */, uint32_t /* anim time * 100 */, uint8_t /* sequence */, uint8_t /* body */, uint8_t /* m_iTeam */, uint8_t /* entindex */>;
export using gmsgScoreAttrib = Message_t<"ScoreAttrib", uint8_t /* entindex */, uint8_t /* attrib */>;
export using gmsgRoundTime = Message_t<"RoundTime", uint16_t /* time remaining in sec */>;
export using gmsgDeathMsg = Message_t<"DeathMsg", uint8_t /* killer */, uint8_t /* victim */, bool /* isHeadshot */, const char* /* pszWeapon */>;
export using gmsgResetHUD = Message_t<"ResetHUD">;
export using gmsgShowMenu = Message_t<"ShowMenu", uint16_t /* valid slots */, int8_t /* display time */, bool /* bNeedMore */, const char* /* pszText */>;
export using gmsgStatusIcon = Message_t<"StatusIcon", uint8_t /* status */, const char * /* icon */>;
export using gmsgMoney = Message_t<"Money", uint32_t /* money */, bool /* should flashing */>;
export using gmsgAmmoPickup = Message_t<"AmmoPickup", uint8_t /* ammo index */, uint8_t /* pickup amt */>;
export using gmsgFade = Message_t<"ScreenFade", uint16_t /* transition time */, uint16_t /* color hold */, uint16_t /* flags */, uint8_t /* RGBA */, uint8_t, uint8_t, uint8_t>;
export using gmsgSendAudio = Message_t<"SendAudio", uint8_t /* entindex */, const char* /* text and/or audio?? */, uint16_t /* pitch */>;
export using gmsgHealth = Message_t<"Health", uint8_t /* health */>;
export using gmsgBattery = Message_t<"Battery", uint16_t /* armorvalue */>;
export using gmsgTextMsg = Message_t<"TextMsg", uint8_t /* display location */, const char* /* translation key */>;	// potentially 4 more args, use unmanaged send to do so.
export using gmsgHideWeapon = Message_t<"HideWeapon", uint8_t /* elements to hide */>;
