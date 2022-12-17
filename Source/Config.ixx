export module Config;

export import std;

export import const_;

using namespace std::literals;

using std::array;
using std::string;
using std::string_view;
using std::uint8_t;
using std::vector;

export extern "C++" namespace Config
{
	inline std::filesystem::path m_GameDir{};

	enum struct ESex : bool
	{
		FEMALE,
		MALE,
	};

	struct Zombie_t
	{
		uint8_t m_iId{ 0 };
		bool m_bHidden{ false };
		bool m_bBoss{ false };
		ESex m_Sex{ ESex::MALE };
		string m_szName{};
		string m_szIntro{};
		string m_szPlayerModel{};
		string m_szViewModel{};
		float m_flMaxHealth{ 1000.f };
		float m_flMaxSpeed{ 250.f };
		float m_flGravity{ 1.f };
		float m_flSlashInterval{};
		float m_flStabInterval{};
		float m_flSlashDist{};
		float m_flStabDist{};
		float m_flMeleeDamageMul{};
		float m_flKnockbackResistance{};
	};

	inline std::deque<Zombie_t> m_rgZombieInfo{};

	struct Human_t
	{
		uint8_t m_iId{ 0 };
		bool m_bHidden{ false };
		ESex m_Sex{ ESex::MALE };
		string m_szName{};
		string m_szIntro{};
		string m_szPlayerModel{};
		float m_flMaxHealth{ 1000.f };
		float m_flMaxSpeed{ 250.f };
		float m_flGravity{ 1.f };
		float m_flKnockbackResistance{};
	};

	inline std::deque<Human_t> m_rgHumanInfo{};

	struct Weather_t
	{
		enum EWeather : uint8_t
		{
			Clear = 1,
			Rain,
			Thunderstorm,
			Tempest,
			Snow,
			Fog,
			BlackFog,
		};

		char m_cLighting{ 'z' };
		EWeather m_iWeather{ Clear };

		static inline string m_szSkyTexture{};
	};

	inline std::deque<Weather_t> m_rgWeather{};

	// General Configs
	inline vector<char> m_rgcAdmin{ 't', 'u' };
	inline bool m_bUsingZBOT = true;
	inline bool m_bPrecacheTModel = false;
	inline bool m_bHideZombieCrosshair = true;

	namespace HintMessage
	{
		inline string m_GameBreaks{ u8"沒有足夠的玩家進行遊戲" };
		inline string m_ForcedIntoHuman{ u8"你被調濟進入人類隊伍" };
		inline string m_ForcedIntoZombie{ u8"你被調濟進入殭屍隊伍" };
		inline string m_OneTeamChangePerRound{ u8"每一回合僅允許一次隊伍變更" };
		inline string m_NotNow{ u8"現在不能變更隊伍" };
		inline string m_TheOpponentTeamHasFull{ u8"對方隊伍已滿" };
		inline string m_BecomingBoss{ u8"成為BOSS" };
		inline string m_BecomingRegularZombie{ u8"成為普通殭屍" };
		inline string m_Respawning{ u8"重新部署剩餘時間：" };
		inline string m_HumanCanSeeYe{ u8"人類可以看見你" };
		inline string m_HumanIsNearby{ u8"人類附近無法部署" };
		inline string m_Clipping{ u8"穿透牆壁時無法部署" };
		inline string m_RespawningKeyHint{ u8"按下攻擊鍵以部署" };
		inline string m_PostRespawning{ u8"已經復活" };
		inline string m_ChangingZombieType{ u8"殭屍介紹：" };
		inline string m_ChangingHumanType{ u8"人類介紹：" };
		inline string m_HumanWins{ u8"人類勝利" };
		inline string m_ZombieWins{ u8"殭屍勝利" };
		inline string m_Joining{ u8"已加入遊戲" };
		inline string m_Leaving{ u8"已離開遊戲" };
	};

	namespace HintMessageColor
	{
		inline color24 m_Respawning{ 255, 255, 0 };
		inline color24 m_HumanCanSeeYe{ 255, 0, 0 };
		inline color24 m_HumanIsNearby{ 255, 0, 0 };
		inline color24 m_Clipping{ 255, 0, 0 };
		inline color24 m_RespawningKeyHint{ 0, 255, 0 };
		inline color24 m_PostRespawning{ 0, 255, 0 };
	};

	void Load(void) noexcept;
	void LoadZombies(std::filesystem::path const &hPath) noexcept;
	void LoadHumans(std::filesystem::path const &hPath) noexcept;
	void LoadAmbiance(std::filesystem::path const &hPath) noexcept;
	void LoadSettings(std::filesystem::path const &hPath) noexcept;
};
