#pragma once


#include <game/offsets.hpp>
#include <print>
#include <thread>
#include <chrono>
#include <game/structs.hpp>
#include <game/settings.hpp>
#include <vector>
#include <game/sdk.hpp>
#include "memory.hpp"
template<class T>
class TArray
{
public:
	int Length() const
	{
		return m_nCount;
	}

	bool IsValid() const
	{
		if (m_nCount > m_nMax)
			return false;
		if (!m_Data)
			return false;
		return true;
	}

	uint64_t GetAddress() const
	{
		return m_Data;
	}

	T GetById(int i)
	{
		return mem.Read<T>(m_Data + i * 8);
	}

protected:
	uint64_t m_Data;
	uint32_t m_nCount;
	uint32_t m_nMax;
};

struct FInventoryContainerBase final
{
public:
	int32_t                                         RowNum;
	int32_t                                         ColumnNum;
	int32_t                                         RuleID;
	uint8_t                                         Pad_C[0x4];
	uint8_t                                         Pad_10[0x10];
	int32_t                                         ContainerIndex;
	uint8_t                                         Pad_24[0x4];
	TArray<void*>                         ChildActorList;
	uint8_t                                         Pad_8[0x8];
	uint8_t                                         Pad_40[0x8];
};
struct FString : private TArray<wchar_t>
{
	std::wstring ToWString() const
	{
		if (m_nCount <= 0) return L"";

		wchar_t* buffer = new wchar_t[m_nCount];
		// Utilise la nouvelle fonction ReadArray
		if (mem.ReadArray<wchar_t>(m_Data, buffer, m_nCount))
		{
			std::wstring ws(buffer, m_nCount);
			delete[] buffer;
			return ws;
		}

		delete[] buffer;
		return L"";
	}

	std::string ToString() const
	{
		std::wstring ws = ToWString();
		if (ws.empty()) return "";

		int size_needed = WideCharToMultiByte(CP_UTF8, 0, &ws[0], (int)ws.size(), NULL, 0, NULL, NULL);
		std::string str(size_needed, 0);
		WideCharToMultiByte(CP_UTF8, 0, &ws[0], (int)ws.size(), &str[0], size_needed, NULL, NULL);

		return str;
	}
};
bool init = false;
#define LOG(str) std::cout << "[+] " << str << std::endl

namespace cache {
	inline uintptr_t UWorld = 0;
	inline uintptr_t UGameInstance = 0;
	inline uintptr_t PersistentLevel = 0;
	inline uintptr_t ActorArray = 0;
	inline int ActorCount = 0;
	inline uintptr_t LocalPlayers = 0;
	inline uintptr_t PlayerController = 0;
	inline uintptr_t GameState = 0;
	inline uintptr_t PlayerArray = 0;
	inline int PlayerCount = 0;

	namespace Local {
		inline uintptr_t LocalUPawn = 0;
		inline uintptr_t LocalRootComponent = 0;
		inline uintptr_t LocalPlayerState = 0;
		inline uintptr_t LocalPlayerCameraManager = 0;
		inline FVector Location = {};

		namespace CameraData {
			inline FMinimalViewInfo Camera = {};
		}
	}

	std::vector<Player> ActorList;
	std::vector<Loot> LootList;

	void clear_cache() {
		UWorld = 0;
		UGameInstance = 0;
		LocalPlayers = 0;
		PlayerController = 0;
		GameState = 0;
		PlayerArray = 0;
		PlayerCount = 0;
		ActorArray = 0;
		ActorCount = 0;
		Local::LocalUPawn = 0;
		Local::LocalRootComponent = 0;
		Local::LocalPlayerState = 0;
		Local::LocalPlayerCameraManager = 0;
		Local::Location = {};
		Local::CameraData::Camera = {};
	}

	bool cache() {
		UWorld = mem.Read<uintptr_t>(mem.BaseAddress + offsets::game::OFFSET_GWORLD);
		if (!UWorld) {
			//LOG("Failed to get UWorld");
			clear_cache();
			return false;
		}

		UGameInstance = mem.Read<uintptr_t>(UWorld + offsets::UWorld::OwningGameInstance);
		GameState = mem.Read<uintptr_t>(UWorld + offsets::UWorld::GameState);
		settings::system::real_player_count = mem.Read<int32_t>(GameState + (0x328 + 0x8));

		PersistentLevel = mem.Read<uintptr_t>(UWorld + offsets::UWorld::PersistentLevel);
		if (!UGameInstance || !GameState || !PersistentLevel) {
			LOG("Failed to get gameinstance");
			LOG("Failed to get gamestate");
			LOG("Failed to get persistentlevel");
			clear_cache();
			return false;
		}

		// can bruteforce this offset if needed just a TArray
		ActorArray = mem.Read<uintptr_t>(PersistentLevel + offsets::ULevel::ActorArray);
		ActorCount = mem.Read<int>(PersistentLevel + (offsets::ULevel::ActorArray + sizeof(uintptr_t)));

		if (!ActorArray || !ActorCount) {
			LOG("Failed to get actor array");
			LOG("Failed to get actor count");
			clear_cache();
			return false;
		}

		LocalPlayers = mem.Read<uintptr_t>(mem.Read<uintptr_t>(UGameInstance + offsets::UGameInstance::LocalPlayers));
		if (!LocalPlayers) {
			LOG("Failed to get localplayers");
			clear_cache();
			return false;
		}

		PlayerController = mem.Read<uintptr_t>(LocalPlayers + offsets::UPlayer::PlayerController); // LocalPlayers inherits UPlayer
		if (!PlayerController) {
			LOG("failed to get player controller");
			clear_cache();
			return false;
		}

		Local::LocalPlayerCameraManager = mem.Read<uintptr_t>(PlayerController + offsets::APlayerController::PlayerCameraManager);
		if (!Local::LocalPlayerCameraManager) {
			LOG("failed to get camera manager");
			clear_cache();
			return false;
		}

		Local::LocalUPawn = mem.Read<uintptr_t>(PlayerController + offsets::APlayerController::AcknowledgedPawn);
		if (!Local::LocalUPawn) {
			LOG("Failed to get local upawn");
			clear_cache();
			return false;
		}

		Local::LocalRootComponent = mem.Read<uintptr_t>(Local::LocalUPawn + offsets::AActor::RootComponent); // APawn inherits from AActor
		if (!Local::LocalRootComponent) {
			LOG("Failed to get local root component");
			clear_cache();
			return false;
		}

		PlayerArray = mem.Read<uintptr_t>(GameState + offsets::AGameStateBase::PlayerArray);
		PlayerCount = mem.Read<int>(GameState + (offsets::AGameStateBase::PlayerArray + 0x8));

		if (!PlayerArray || !PlayerCount) {
			LOG("Failed to get player array or player count");
			clear_cache();
			return false;
		}

		return true;
	}

	bool cache_camera() {
		if (!UWorld || !Local::LocalPlayerCameraManager) {
			return false;
		}

		// CameraCachePrivate + 0x10 = a fminimalviewinfo
		Local::CameraData::Camera = mem.Read<FMinimalViewInfo>(Local::LocalPlayerCameraManager + offsets::APlayerCameraManager::CameraCachePrivate + 0x10);

		return true;
	}

	bool cache_localplayer() {
		if (!UWorld || !Local::LocalPlayerCameraManager || !Local::LocalUPawn || !Local::LocalRootComponent)
			return false;

		Local::Location = mem.Read<FVector>(Local::LocalRootComponent + offsets::USceneComponent::RelativeLocation);
	}
	Loot get_loot(uintptr_t pawn, uint32_t object_id, std::string object_name) {
		uintptr_t mesh = mem.Read<uintptr_t>(pawn + offsets::ACharacter::Mesh);
		uintptr_t rootcomponent = mem.Read<uintptr_t>(pawn + offsets::AActor::RootComponent);

		if (!mesh) {
			return {};
		}

		if (!rootcomponent) {
			return {};
		}


//		auto USGInventoryContainerMgrComponent = mem.Read<uintptr_t>(pawn + 0x8B8);

		//TArray<FInventoryContainerBase> InventoryContainerBaseList = mem.Read<TArray<FInventoryContainerBase>>(USGInventoryContainerMgrComponent + 0x140);
		//TArray<uint64_t> ChildActorList = mem.Read<TArray<uint64_t>>(USGInventoryContainerMgrComponent + 0x28);
		int total_price = 0;
		
		//for (uint32_t j = 0; j < ChildActorList.Length(); j++) {
		//	uintptr_t paul = (uintptr_t)ChildActorList.GetById(j);
		//	if (!paul) continue;
		//	uint64_t common_data_comp = mem.Read<uint64_t>(paul + 0x730); //730 or 750
		//	int price = mem.Read<int>(common_data_comp + 0x10C);
		//	total_price += price;
		//	FString item_name = mem.Read<FString>(common_data_comp + 0x138);
		//	LOG(item_name.ToString());

		//}
		//LOG(std::to_string(total_price));
		Loot loot;
		loot.actor_pawn = pawn;
		loot.actor_mesh = mesh;
		loot.actor_id = object_id;
		loot.actor_name = object_name;
		loot.actor_rootcomponent = rootcomponent;
	




		return loot;
	}

	Player get_player(uintptr_t pawn, uint32_t object_id, std::string object_name) {
		uintptr_t mesh = mem.Read<uintptr_t>(pawn + offsets::ACharacter::Mesh);
		uintptr_t playerstate = mem.Read<uintptr_t>(pawn + offsets::APawn::PlayerState);
		uintptr_t rootcomponent = mem.Read<uintptr_t>(pawn + offsets::AActor::RootComponent);
		float health = mem.Read<float>(pawn + offsets::ASGCharacter::Health);

		uint64_t deathcomponent = mem.Read<uint64_t>(pawn + offsets::ASGCharacter::DeathComponent);
		uint8_t isdeadinfo = mem.Read<uint8_t>(deathcomponent + 0x240);
		DWORD isdead = mem.Read<DWORD>(isdeadinfo + 0x0);
		if (!mesh) {
			return {};
		}

		if (!rootcomponent) {
			return {};
		}
		bool isssdead = false;

		if (isdeadinfo	 != 0x0)
			isssdead = true;
		Player player;
		player.actor_pawn = pawn;
		player.actor_mesh = mesh;
		player.actor_state = playerstate;
		player.actor_id = object_id;
		player.actor_name = object_name;
		player.actor_rootcomponent = rootcomponent;
		player.health = health;
		player.is_dead = isssdead;
		//LOG(isssdead);
		FVector headpos = sdk::GetbonePos(mesh, 16);

		player.head_pos = headpos;


		if (object_name.contains("BP_UamCharacter") || object_name.contains("BP_UamRangeCharacter_C"))
		{
			player.bot = false;
		}
		else if (object_name.contains("BP_UamAICharacter_C") ) {
			ECharacterType Charactertype = mem.Read<ECharacterType>(pawn + 0x152d);
	

			if (Charactertype == ECharacterType::ECharacterType_PMC) {
				//LOG("C'est bon mec c'est toi");
				player.bot = false;
			}
			else if (Charactertype == ECharacterType::ECharacterType_SCAV) {
				player.bot = false;

			}
			else {
				player.bot = true;

			}
		}

	

		//player.player_name = player_name;
		//LOG(std::to_string(health));
	
			FString player_name = mem.Read<FString>(playerstate + 0x3f0);
			//if (playerstate)
			//	player.bot = false;

			if (health == 0 || !std::isnan(health)) {
				uintptr_t ability = mem.Read<uintptr_t>(pawn + 0x15e0);	
				float healthh = sdk::GetHealth(ability);
				player.health = healthh;

			}
			auto MyTeamID = mem.Read<uint32_t>(Local::LocalPlayerState + 0x520);
			auto TeamID = mem.Read<uint32_t>(playerstate + 0x520);
			player.player_name = player_name.ToString();


			if (MyTeamID == TeamID) {
				player.IsFriend = true;
			}
			//LOG(player_name.ToString());
			//LOG(MyTeamID);

		
		return player;
	}

	bool cache_players() {
		if (!ActorArray || !ActorCount)
			return false;

		std::vector<Player> player_array;
		std::vector<Loot> loot_array;

		for (int i = 0; i < ActorCount; i++) {
			uintptr_t pawn = mem.Read<uintptr_t>(ActorArray + i * 8);
			if (pawn == Local::LocalUPawn) {
			//	LOG("local upawn found");

				continue;
			}

			uint32_t object_id = mem.Read<uint32_t>(pawn + 24);
			std::string object_name = sdk::GetNameFromFName(object_id);
			//LOG(object_name);
			if (object_name.contains("BP_UamCharacter")  || object_name.contains("BP_UamAICharacter")  || object_name.contains("BP_UamRangeCharacter_C")) {
			Player player = get_player(pawn, object_id,object_name);
				if (player.actor_mesh || player.actor_rootcomponent) {
					player_array.push_back(player);
					//LOG("a player pushed");
				}
			 }

			if (object_name.contains("nade") || object_name.contains("GrenadeBox_C") || object_name.contains("SportBag_C") || object_name.contains("WeaponCase_C") || object_name.contains("ToolBox_C") || object_name.contains("MunitionBox_C") || object_name.contains("Inventory_") || object_name.contains("LootBoxBase_")) {
				Loot loot = get_loot(pawn, object_id, object_name);

				if (loot.actor_mesh || loot.actor_rootcomponent) {
					loot_array.push_back(loot);
					
				}
			}
		}
		ActorList = player_array;
		LootList = loot_array;

	}

	void cache_loop() {
		int loop_count = 0;
		while (settings::running) {
			cache();
			loop_count++;
			if (loop_count >= 5) {
				loop_count = 0;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}
	}

	void cache_important_loop() {
		if (!init) {
			if (!mem.initialize())
			{
				//LOG("Failed to initialize memory manager. Is the driver running and is the game open?");
				//system("pause"); // Met en pause la console pour que vous puissiez lire l'erreur
			
			}
		}

		while (settings::running) {
			cache_camera();
			cache_localplayer();
			std::this_thread::sleep_for(std::chrono::milliseconds(2));
		}
	}

	void cache_player_loop() {
		while (settings::running) {
			cache_players();
			std::this_thread::sleep_for(std::chrono::milliseconds(55));
		}
	}
}