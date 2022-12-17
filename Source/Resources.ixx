export module Resources;

export import std;

using std::array;
using std::string;
using std::unordered_map;
using std::vector;

using namespace std::literals;

export extern "C++" namespace Sounds
{
	inline string m_Beginning{ "sound/zombieriot/quarantine_01.mp3" };
	inline string m_Beep{ "zombieriot/beep07.wav" };
	inline string m_Starting{ "zombieriot/pickup_secret01.wav" };
	inline array m_CheckPoints{ "zombieriot/survival_medal.wav"s, "zombieriot/survival_playerrec.wav"s, "zombieriot/survival_teamrec.wav"s };
	inline string m_HumanWins{ "zombieriot/win_human.wav" };
	inline string m_ZombieWins{ "zombieriot/win_zombi.wav" };
	inline string m_AllowRespawn{ "zombieriot/pickup_scifi37.wav" };
	inline string m_PostRespawn{ "zombieriot/pickup_guitarriff10.wav" };
	inline string m_GhostSpawn{ "zombieriot/menu_horror01.wav" };

	inline vector<string> m_rgszClawMiss{ "zombieriot/claw_miss_1.wav", "zombieriot/claw_miss_2.wav" };
	inline vector<string> m_rgszScratchingWall{ "zombieriot/claw_scrape_1.wav", "zombieriot/claw_scrape_2.wav", "zombieriot/claw_scrape_3.wav" };
	inline vector<string> m_rgszClawSlashingFlesh{ "zombieriot/claw_hit_flesh_1.wav", "zombieriot/claw_hit_flesh_2.wav", "zombieriot/claw_hit_flesh_3.wav" };
	inline vector<string> m_rgszClawSlicingFlesh{ "zombieriot/zombie_slice_1.wav", "zombieriot/zombie_slice_2.wav" };
	inline vector<string> m_rgszBossHurt{ "zombieriot/bm_inj_04.wav", "zombieriot/bm_inj_05.wav", "zombieriot/bm_inj_06.wav", "zombieriot/bm_inj_07.wav" };
	inline vector<string> m_rgszZombieHurt{ "zombieriot/been_shot_01.wav", "zombieriot/been_shot_02.wav", "zombieriot/been_shot_03.wav", "zombieriot/been_shot_04.wav" };
	inline vector<string> m_rgszBossDeath{ "zombieriot/bm_death_01.wav", "zombieriot/bm_death_02.wav", "zombieriot/bm_death_03.wav" };
	inline vector<string> m_rgszZombieDeath{ "zombieriot/headless_1.wav", "zombieriot/headless_2.wav", "zombieriot/headless_3.wav", "zombieriot/headless_4.wav" };

	inline constexpr array m_rgszRain = { "ambience/rainsound.wav", "ambience/stormrain.wav" };
	inline constexpr array m_rgszThunder = { "ambience/thunder1.wav", "ambience/thunder2.wav", "ambience/thunder3.wav" };
	inline constexpr array m_rgszThunderclap = { "ambience/thunderflash1.wav", "ambience/thunderflash2.wav", "ambience/thunderflash3.wav", "ambience/thunderflash4.wav", "ambience/thunderflash5.wav" };
	inline constexpr array m_rgszNVG = { "items/nvg_off.wav", "items/nvg_on.wav" };
};

export inline struct SpriteMgr_t
{
	static inline constexpr char LASER_BEAM[] = "sprites/laserbeam.spr";

	static inline unordered_map<string, short> s_rgLibrary{};

	inline decltype(auto) operator[] (auto&& args) noexcept { return s_rgLibrary[std::forward<decltype(args)>(args)]; }

	void Precache(void) noexcept;
}
Sprites;

export extern "C++" namespace Resources
{
	void Precache(void) noexcept;
};
