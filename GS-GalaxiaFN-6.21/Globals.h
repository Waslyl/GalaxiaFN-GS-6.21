#pragma once
#include <map>
#include <chrono>
#include "Discord.h"

//#define BOTS
//#define CHEATS
#define SESSION

// Toujours en définir un des 5
//#define SERVER1 // Port 7001
//#define SERVER2 // Port 7002
//#define SERVER3 // Port 7003
//#define SERVER4 // Port 7004
#define SERVER5 // Port 7777

#define LATE_GAME // Mode Lategame
//#define ARENA // Mode Arene
//#define ONESHOT //Mode Lategame Oneshot
//#define NORMAL // Tout les modes sauf Custom (c surtout pour les webhook sinon rien s'envoie)
//#define PLAYGROUND // Terrain de jeu + commandes cheats (bot, loot...)
//#define CREATIVE // Creative
//#define DODGE_ROCKET //Mode Esquive Rocket
//#define AVENGERS // Mode Avengers

//#define CUSTOM_POI

//#define DUMP_621 // [GALAXIA PATCH] Ecrit une copie du jeu 6.21 dechiffre (a cote de l'exe) pour retrouver les adresses manquantes. A activer une seule fois.

#define RESULT_DECL void*const RESULT_PARAM

template<typename T = UObject>
static T* Cast(UObject* Object)
{
	if (Object && Object->IsA(T::StaticClass()))
	{
		return (T*)Object;
	}

	return nullptr;
}

namespace Globals
{
	extern inline bool bSendWebhook = true;
	extern inline std::string EUWebhook = "https://discord.com/api/webhooks/1262743021213057097/J6b-smvwtbpH7o_OKBPHbE7nRonsHFf9W8XCnuxR2aCVnN6vcjm97lUmcFMaXTwVmm8r"; // Webhook Log Serveur
	extern inline std::string WebhookAntiCheat = "https://discord.com/api/webhooks/1299354235179372625/OSGDY0MNY6Wzw9JbIlvePRRCl8PolVmOb5xFK-htTZj36M8a4YDDknn0M3gzsUmx7dnF"; // Webhook Log Anticheat
	extern inline std::string Webhook = EUWebhook;
	extern inline std::string ACWebhook = WebhookAntiCheat;

	extern inline int UPTime = 0;

	extern inline bool bStartedListening = false;

	extern inline bool UPTimeStarted = false;

	extern inline bool wasWebhookSent = false;

	extern inline int secBeforeKillGame = 5;

	UWorld* World = nullptr;
	UFortEngine* FortEngine = nullptr;

	UFortEngine* GetFortEngine(bool SkipCheck = false)
	{
		if (FortEngine == nullptr && !SkipCheck)
			FortEngine = UObject::FindObjectSlow<UFortEngine>("FortEngine_");

		return FortEngine;
	}

	UWorld* GetWorld(bool SkipCheck = false)
	{
		if (World == nullptr && !SkipCheck)
		{
			UFortEngine* FortEngine = GetFortEngine();

			if (FortEngine)
			{
				if (FortEngine->GameViewport)
				{
					if (FortEngine->GameViewport->World)
						return FortEngine->GameViewport->World;
				}
			}

			return nullptr;
		}

		return World;
	}

	TArray<ULocalPlayer*> GetLocalPlayers()
	{
		UFortEngine* FortEngine = GetFortEngine();

		if (FortEngine)
		{
			if (FortEngine->GameInstance)
				return FortEngine->GameInstance->LocalPlayers;
		}
	}

	APlayerController* GetServerPlayerController()
	{
		TArray<ULocalPlayer*> LocalPlayers = GetLocalPlayers();
		ULocalPlayer* LocalPlayer = LocalPlayers[0];

		if (LocalPlayer)
		{
			APlayerController* PlayerController = LocalPlayer->PlayerController;

			if (PlayerController)
				return PlayerController;
		}

		return nullptr;
	}

	AFortGameModeAthena* GetGameMode()
	{
		UWorld* World = UWorld::GetWorld();

		if (World)
		{
			AFortGameModeAthena* GameMode = (AFortGameModeAthena*)World->AuthorityGameMode;

			if (GameMode)
				return GameMode;
		}

		return nullptr;
	}

	AFortGameStateAthena* GetGameState()
	{
		UWorld* World = UWorld::GetWorld();

		if (World)
		{
			AFortGameStateAthena* GameState = Cast<AFortGameStateAthena>(World->GameState);

			if (GameState)
				return GameState;
		}

		return nullptr;
	}

	UFortPlaylistAthena* GetPlaylist()
	{
		AFortGameStateAthena* GameState = GetGameState();

		if (GameState)
		{
			UFortPlaylistAthena* Playlist = GameState->CurrentPlaylistInfo.BasePlaylist;

			if (Playlist)
				return Playlist;
		}

		return nullptr;
	}

	UFortGameData* GetGameData()
	{
		UFortAssetManager* AssetManager = Cast<UFortAssetManager>(GetFortEngine()->AssetManager);

		if (AssetManager)
		{
			UFortGameData* GameData = AssetManager->GameData;

			if (GameData)
				return GameData;
		}

		return nullptr;
	}
	
	UCurveTable* GetAthenaGameData()
	{
		UFortPlaylistAthena* PlaylistAthena = GetPlaylist();

		if (PlaylistAthena)
		{
			UCurveTable* AthenaGameData = PlaylistAthena->GameData.Get();

			if (!AthenaGameData)
				AthenaGameData = FindObjectFast<UCurveTable>("/Game/Athena/Balance/DataTables/AthenaGameData.AthenaGameData");

			return AthenaGameData;
		}

		return nullptr;
	}

	template <typename T>
	static T* GetDataTableRowFromName(UDataTable* DataTable, FName RowName)
	{
		if (!DataTable)
			return nullptr;

		auto& RowMap = DataTable->GetRowMap();

		for (int i = 0; i < RowMap.Elements.Elements.Num(); ++i)
		{
			auto& Pair = RowMap.Elements.Elements.Data[i].ElementData.Value;

			if (Pair.Key() != RowName)
				continue;

			return (T*)Pair.Value();
		}

		return nullptr;
	}
}

namespace Announcement
{
	static std::string UptimeWebHook = (Globals::Webhook);
	static std::string AntiCheatWebHook = (Globals::ACWebhook);
}

static DiscordWebhook UptimeWebHook(Announcement::UptimeWebHook.c_str());
static DiscordWebhook AntiCheatWebHook(Announcement::AntiCheatWebHook.c_str());

