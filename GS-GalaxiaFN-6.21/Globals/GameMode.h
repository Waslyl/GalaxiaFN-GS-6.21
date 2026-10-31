#pragma once

#include <iostream>
#include <string>
#include "discord.h"

#include <curl/curl.h>

extern std::string storedSessionId;

enum ENetMode
{
	NM_Standalone,
	NM_DedicatedServer,
	NM_ListenServer,
	NM_Client,
	NM_MAX,
};

// [8.51-ONLY] GEngine lu a une adresse 8.51 au chargement de la DLL (crash a l'injection) et jamais utilise : Globals::GetFortEngine() a la place
// static UFortEngine* GEngine = decltype(GEngine)(*(uintptr_t*)((uintptr_t)GetModuleHandleW(0) + 0x65A40A0));

namespace GameMode
{
	// [GALAXIA PATCH] NativeFunc : rempli par pattern, l'appel est ignore (avec log) si le pattern n'est pas trouve en 6.21
	// void (*RemoveFromAlivePlayers)(AFortGameModeAthena* GameMode, AFortPlayerControllerAthena* PlayerController, AFortPlayerStateAthena* PlayerState, AFortPlayerPawnAthena* Pawn, UFortWeaponItemDefinition* WeaponItemDefinition, EDeathCause DeathCause, char a7);
	Offsets621::NativeFunc<void(AFortGameModeAthena* GameMode, AFortPlayerControllerAthena* PlayerController, AFortPlayerStateAthena* PlayerState, AFortPlayerPawnAthena* Pawn, UFortWeaponItemDefinition* WeaponItemDefinition, EDeathCause DeathCause, char a7)> RemoveFromAlivePlayers;
	// [GALAXIA PATCH] NativeFunc : rempli par pattern, l'appel est ignore (avec log) si le pattern n'est pas trouve en 6.21
	// void (*AddFromAlivePlayers)(AFortGameModeAthena* GameMode, AFortPlayerControllerAthena* PlayerController);
	Offsets621::NativeFunc<void(AFortGameModeAthena* GameMode, AFortPlayerControllerAthena* PlayerController)> AddFromAlivePlayers;

	static AFortPlayerStart* LateGamePlayerStart = nullptr;

	bool bPreReadyToStartMatch = false;
	bool bMatchStarted = false;
	bool bWorldReady = false;
	bool bStartMatch = false;
	bool bRestart = false;

	FVector AircraftLoc;
	int OldSafeZonePhase;
	bool bGetAircraftLoc = false;

	// [GALAXIA PATCH] Ces pointeurs servent seulement d'identite : ne jamais dereferencer l'ancienne carte.
	UWorld* MatchWorld = nullptr;
	AFortGameModeAthena* MatchGameMode = nullptr;
	ULONGLONG RestartAt = 0;
	constexpr ULONGLONG RestartDelayMs = 15000;

	void PrepareNewMatch(UWorld* World, AFortGameModeAthena* GameModeAthena)
	{
		if (MatchWorld == World && MatchGameMode == GameModeAthena &&
			!(bRestart && !static_cast<AFortGameStateAthena*>(GameModeAthena->GameState)->bPlaylistDataIsLoaded))
			return;

		if (MatchWorld)
			FN_LOG(LogGameMode, Log, "[Restart621] Nouvelle carte : etat du GS reinitialise.");

		MatchWorld = World;
		MatchGameMode = GameModeAthena;
		RestartAt = 0;
		bRestart = false;
		bPreReadyToStartMatch = false;
		bMatchStarted = false;
		bWorldReady = false;
		bStartMatch = false;
		LateGamePlayerStart = nullptr;
		AircraftLoc = {};
		OldSafeZonePhase = 0;
		bGetAircraftLoc = false;
		Globals::World = nullptr;
		Beacon::Beacon = nullptr;
	}

	__int64 PreLoginHook() { return 0; };

	// Original ReadyToStartMatch (Faut les choquer)
	bool (*ReadyToStartMatchOG)(AFortGameModeAthena* GameModeAthena);
	bool ReadyToStartMatchHook(AFortGameModeAthena* GameModeAthena)
	{
		AFortGameStateAthena* GameStateAthena = Cast<AFortGameStateAthena>(GameModeAthena->GameState);
		if (!GameStateAthena) return ReadyToStartMatchOG(GameModeAthena);

		AFortGameMode;

		UWorld* World = UWorld::GetWorld();
		if (!World) return ReadyToStartMatchOG(GameModeAthena);

		PrepareNewMatch(World, GameModeAthena);

		if (!World->NetDriver)
			Beacon::Listen(); // Listen

		if (!bPreReadyToStartMatch)
		{
			Globals::GetFortEngine(true);
			Globals::GetWorld(true);

			bPreReadyToStartMatch = true;
		}

		if (GameStateAthena && !GameStateAthena->bPlaylistDataIsLoaded)
		{
#ifdef CREATIVE
			UFortPlaylistAthena* PlaylistAthena = UObject::FindObject<UFortPlaylistAthena>("FortPlaylistAthena Playlist_PlaygroundV2.Playlist_PlaygroundV2");
#endif // CREATIVE

#ifdef LATE_GAME
			UFortPlaylistAthena* PlaylistAthena = UObject::FindObject<UFortPlaylistAthena>("FortPlaylistAthena Playlist_DefaultSolo.Playlist_DefaultSolo");
#endif // LATE_GAME

#ifdef PLAYGROUND
			UFortPlaylistAthena* PlaylistAthena = UObject::FindObject<UFortPlaylistAthena>("FortPlaylistAthena Playlist_Playground.Playlist_Playground");
#endif // PLAYGROUND
#ifdef ARENA
			UFortPlaylistAthena* PlaylistAthena = UObject::FindObject<UFortPlaylistAthena>("FortPlaylistAthena Playlist_ShowdownAlt_Solo.Playlist_ShowdownAlt_Solo"); // Arene
#endif // ARENA
#ifdef NORMAL
			//UFortPlaylistAthena* PlaylistAthena = FindObjectFast<UFortPlaylistAthena>("/Game/Athena/Playlists/Galaxia/Playlist_LateGame_Solo.Playlist_LateGame_Solo");
			UFortPlaylistAthena* PlaylistAthena = UObject::FindObject<UFortPlaylistAthena>("FortPlaylistAthena Playlist_DefaultSolo.Playlist_DefaultSolo"); // Solo
			//UFortPlaylistAthena* PlaylistAthena = UObject::FindObject<UFortPlaylistAthena>("FortPlaylistAthena Playlist_Sword_Solo.Playlist_Sword_Solo"); // Solo
			//UFortPlaylistAthena* PlaylistAthena = UObject::FindObject<UFortPlaylistAthena>("FortPlaylistAthena Playlist_DefaultDuo.Playlist_DefaultDuo"); // Duo
			//UFortPlaylistAthena* PlaylistAthena = UObject::FindObject<UFortPlaylistAthena>("FortPlaylistAthena Playlist_Fill_Solo.Playlist_Fill_Solo"); // Solo
#endif // NORMAL

#ifdef ONESHOT
			UFortPlaylistAthena* PlaylistAthena = UObject::FindObject<UFortPlaylistAthena>("FortPlaylistAthena Playlist_Low_Solo.Playlist_Low_Solo"); //Oneshot
#endif // ONESHOT

#ifdef DODGE_ROCKET
			UFortPlaylistAthena* PlaylistAthena = UObject::FindObject<UFortPlaylistAthena>("FortPlaylistAthena Playlist_FlyExplosives_Solo.Playlist_FlyExplosives_Solo"); //Esquive Rocket
#endif // DODGE_ROCKET

#ifdef AVENGERS
			// [8.51-ONLY] Playlist_Ashton_Sm (LTM Avengers, 8.50) n'existe pas en 6.21 : repli sur Playlist_DefaultSolo juste en dessous
			// UFortPlaylistAthena* PlaylistAthena = UObject::FindObject<UFortPlaylistAthena>("FortPlaylistAthena Playlist_Ashton_Sm.Playlist_Ashton_Sm"); // Avengers
			UFortPlaylistAthena* PlaylistAthena = nullptr;
#endif // AVENGERS

			// [GALAXIA PATCH] 6.21 : si la playlist du mode n'existe pas en 6.21 (ou n'est pas chargee), on prend Playlist_DefaultSolo au lieu de crasher
			if (!PlaylistAthena)
			{
				FN_LOG(LogGameMode, Error, "[6.21] Playlist du mode introuvable en 6.21 ! Repli sur Playlist_DefaultSolo.");
				PlaylistAthena = UObject::FindObject<UFortPlaylistAthena>("FortPlaylistAthena Playlist_DefaultSolo.Playlist_DefaultSolo");
			}

			if (!PlaylistAthena)
			{
				FN_LOG(LogGameMode, Error, "[6.21] Aucune playlist trouvee, la partie ne peut pas demarrer.");
				return false;
			}


			GameStateAthena->CurrentPlaylistInfo.BasePlaylist = PlaylistAthena;
			GameStateAthena->CurrentPlaylistInfo.OverridePlaylist = PlaylistAthena;
			GameStateAthena->CurrentPlaylistInfo.PlaylistReplicationKey++;
			GameStateAthena->CurrentPlaylistInfo.MarkArrayDirty();
			GameStateAthena->OnRep_CurrentPlaylistInfo();
			Loots::EnsurePlaylistLootTables(PlaylistAthena); // [GALAXIA PATCH] 6.21 : tables de loot Athena

			GameStateAthena->AirCraftBehavior = PlaylistAthena->AirCraftBehavior;
			GameStateAthena->bIsLargeTeamGame = PlaylistAthena->bIsLargeTeamGame;
			// [8.51-ONLY] DefaultRebootMachineHotfix (Reboot Van) n'existe pas en 6.21
			// GameStateAthena->DefaultRebootMachineHotfix = 1;

			GameModeAthena->AISettings = PlaylistAthena->AISettings;
			GameModeAthena->WarmupRequiredPlayerCount = 1;//2; // I think it's here for starting with 2 players

			GameModeAthena->GameContextTags.GameplayTags.Clear();
			
			for (int32 i = 0; i < PlaylistAthena->GameplayTagContainer.GameplayTags.Num(); i++) // Pour les défis plus tard
			{
				FGameplayTag GameplayTag = PlaylistAthena->GameplayTagContainer.GameplayTags[i];
				if (!GameplayTag.TagName.IsValid()) continue;

				GameModeAthena->GameContextTags.GameplayTags.Add(GameplayTag);
			}

			AFortGameSession* GameSession = GameModeAthena->FortGameSession;

			if (GameSession)
			{
				GameSession->MaxPlayers = PlaylistAthena->MaxPlayers;
				GameSession->MaxPartySize = PlaylistAthena->MaxSocialPartySize;

				GameSession->SessionName = UKismetStringLibrary::Conv_StringToName(L"GalaxiaSession");
			}

			UFortZoneTheme* ZoneTheme = GameStateAthena->ZoneTheme;

			if (ZoneTheme) // sah jsp si c'est utile mais ta capter faut les choqués
			{
				ZoneTheme->PlaylistId = PlaylistAthena->PlaylistId;
				ZoneTheme->TeamSize = PlaylistAthena->MaxTeamSize;
				ZoneTheme->TeamCount = PlaylistAthena->MaxTeamCount;
				ZoneTheme->MaxPartySize = PlaylistAthena->MaxSocialPartySize;
				ZoneTheme->MaxPlayers = PlaylistAthena->MaxPlayers;
			}

			// [8.51-ONLY] UFortPlaylistAthena::bAllowJoinInProgress n'existe pas en 6.21 (et 0xFAABC0 = adresse 8.51 de PreLogin)
			// if (PlaylistAthena->bAllowJoinInProgress)
			// {
				// MH_CreateHook((LPVOID)(InSDKUtils::GetImageBase() + 0xFAABC0), PreLoginHook, nullptr);
				// MH_EnableHook((LPVOID)(InSDKUtils::GetImageBase() + 0xFAABC0));
			// }

			for (int32 i = 0; i < PlaylistAthena->AdditionalLevels.Num(); i++)
			{
				TSoftObjectPtr<UWorld> AdditionalLevel = PlaylistAthena->AdditionalLevels[i];

				FName FoundationName = AdditionalLevel.ObjectID.AssetPathName;
				if (!FoundationName.IsValid()) continue;

				bool bSuccess = false;
				ULevelStreamingDynamic::LoadLevelInstanceBySoftObjectPtr(GameStateAthena, AdditionalLevel, FVector(), FRotator(), &bSuccess);

				if (bSuccess)
				{
					FName LevelName = AdditionalLevel.ObjectID.AssetPathName;
					if (!LevelName.IsValid()) continue;

					GameStateAthena->AdditionalPlaylistLevelsStreamed.Add(LevelName);
				}
			}

			GameStateAthena->OnRep_AdditionalPlaylistLevelsStreamed();

			TArray<ABuildingFoundation*> BuildingFoundations;

			// [8.51-ONLY] The Block (saison 7) et le volcan (saison 8) n'existent pas sur la carte 6.21
			// BuildingFoundations.Add(FindObjectFast<ABuildingFoundation>("/Game/Athena/Maps/Athena_POI_Foundations.Athena_POI_Foundations.PersistentLevel.SLAB_1")); // The Block
			// BuildingFoundations.Add(FindObjectFast<ABuildingFoundation>("/Game/Athena/Maps/Athena_POI_Foundations.Athena_POI_Foundations.PersistentLevel.LF_Athena_POI_50x53_Volcano")); // Volcano

			for (int32 i = 0; i < BuildingFoundations.Num(); i++)
			{
				ABuildingFoundation* BuildingFoundation = BuildingFoundations[i];
				if (!BuildingFoundation) continue;

				BuildingFoundation->DynamicFoundationType = EDynamicFoundationType::Static;

				BuildingFoundation->bServerStreamedInLevel = true;
				BuildingFoundation->OnRep_ServerStreamedInLevel();

				BuildingFoundation->SetDynamicFoundationEnabled(true);

				BuildingFoundation->FlushNetDormancy();
				BuildingFoundation->ForceNetUpdate();
			}

#ifdef CUSTOM_POI
			// Destroy Foundation
			{
				ABuildingFoundation* BuildingFoundation = FindObjectFast<ABuildingFoundation>("/Game/Athena/Maps/Athena_POI_Foundations.Athena_POI_Foundations.PersistentLevel.LF_Athena_POI_25x28_72");

				if (BuildingFoundation)
				{
					BuildingFoundation->DynamicFoundationType = EDynamicFoundationType::Dynamic_StartDisabled; // StartDisabled en 8.51

					BuildingFoundation->bServerStreamedInLevel = false;
					BuildingFoundation->OnRep_ServerStreamedInLevel();

					// [8.51-ONLY] FoundationEnabledState / EDynamicFoundationEnabledState n'existent pas en 6.21
					// BuildingFoundation->FoundationEnabledState = EDynamicFoundationEnabledState::Disabled;
					BuildingFoundation->OnRep_LevelToStream();

					BuildingFoundation->FoundationType = EBuildingFoundationType::BFT_None;

					BuildingFoundation->SetDynamicFoundationEnabled(false);

					BuildingFoundation->FlushNetDormancy();
					BuildingFoundation->ForceNetUpdate();

					BuildingFoundation->K2_DestroyActor();
					BuildingFoundation->SilentDie();
				}
			}
#endif // CUSTOM_POI

#ifdef BOTS
			Bots::InitializeBotData();
#endif // BOTS
		}

		bool bReadyToStartMatch = ReadyToStartMatchOG(GameModeAthena);

		if (bReadyToStartMatch)
		{
			Functions::InitializeTreasureChests();
			Functions::InitializeAmmoBoxs();
			Functions::InitializeLlamas();
			Functions::InitializeConsumableBGAs();

			Functions::FillVendingMachines();

			FN_LOG(LogGameMode, Log, "ReadyToStartMatch call!");
		}

		return bReadyToStartMatch;
	}

	bool (*StartAircraftPhaseOG)(AFortGameModeAthena* GameModeAthena, char a2);
	bool StartAircraftPhaseHook(AFortGameModeAthena* GameModeAthena, char a2)
	{
#ifdef BOTS
		// Retirer tous les ai pawn pour le bus
		TArray<AActor*> Actors;
		UGameplayStatics::GetAllActorsOfClass(GameModeAthena, AFortAthenaAIBotController::StaticClass(), &Actors);

		for (int32 i = 0; i < Actors.Num(); i++)
		{
			AFortAthenaAIBotController* AthenaAIBotController = Cast<AFortAthenaAIBotController>(Actors[i]);
			if (!AthenaAIBotController) continue;

			AFortPlayerStateAthena* PlayerStateAthena = Cast<AFortPlayerStateAthena>(AthenaAIBotController->PlayerState);
			if (!PlayerStateAthena) continue;

			if (AthenaAIBotController->PlayerBotPawn)
				AthenaAIBotController->PlayerBotPawn->K2_DestroyActor();

			Bots::ChooseRandomTimeToAircraftJump(AthenaAIBotController);

			PlayerStateAthena->bInAircraft = true;
			PlayerStateAthena->ServerSetInAircraft(true);
		}
#endif // BOTS


#ifdef SESSION
#if defined(LATE_GAME) || defined(ONESHOT) || defined(DODGE_ROCKET) || defined(ARENA) || defined(NORMAL) || defined(AVENGERS)
		// Delete le match et remet les joueurs en file d'attente jusqu'a la prochaine partie.
		CURL* curl = curl_easy_init();

		std::string url = "https://galaxia-secure-backend.mega-hosting.fr:2083/fortnite/game/privaterelay/deletematch/" + storedSessionId;

		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "GalaxiaMatchDeleter/1.0");

		CURLcode res = curl_easy_perform(curl);

		long http_code = 0;
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

		if (res != CURLE_OK) {
			std::cerr << "Erreur de requête: " << curl_easy_strerror(res) << std::endl;
		}
		else {
			if (http_code == 200) {
				std::cout << "Réponse 200 OK reçue." << std::endl;
			}
			else {
				std::cout << "Code de réponse HTTP: " << http_code << std::endl;
			}
		}
		curl_easy_cleanup(curl);
#endif // define(LATE_GAME) || defined(ONESHOT) || defined(DODGE_ROCKET) || defined(ARENA) || defined(NORMAL) || defined(AVENGERS)
#endif // SESSION

		TArray<AFortPlayerController*> AllFortPlayerController = UFortKismetLibrary::GetAllFortPlayerControllers(GameModeAthena, true, true);

		for (int32 i = 0; i < AllFortPlayerController.Num(); i++)
		{
			AFortPlayerController* PlayerController = AllFortPlayerController[i];
			if (!PlayerController) continue;

			bool bRemoveCantBeDropped = true;

#if defined(LATE_GAME) || defined(ONESHOT) || defined(DODGE_ROCKET) || defined(ARENA)
			bRemoveCantBeDropped = false;
#endif // (LATE_GAME) || defined(ONESHOT) || defined(DODGE_ROCKET) || defined(ARENA)

			Inventory::RemoveAllItemsFromInventory(PlayerController->WorldInventory, bRemoveCantBeDropped);
			Inventory::UpdateInventory(PlayerController->WorldInventory);

			AFortPlayerPawn* PlayerPawn = PlayerController->MyFortPawn;
			if (!PlayerPawn) continue;

			PlayerPawn->bCanBeDamaged = false;
		}

#if defined(LATE_GAME) || defined(ONESHOT) || defined(DODGE_ROCKET) || defined(ARENA)
		AFortGameStateAthena* GameStateAthena = Cast<AFortGameStateAthena>(GameModeAthena->GameState);

		if (GameStateAthena)
		{
			UKismetSystemLibrary::ExecuteConsoleCommand(GameModeAthena, L"startsafezone", nullptr);
			GameStateAthena->SafeZonesStartTime = 0;
			//GameStateAthena->bAircraftIsLocked = false;
		}

		FVector PlayerStartLocation = GameModeAthena->SafeZoneLocations[3];

		if (!PlayerStartLocation.IsZero())
		{
			PlayerStartLocation.Z = 20000.0f;

			LateGamePlayerStart = Util::SpawnActor<AFortPlayerStart>(AFortPlayerStart::StaticClass(), PlayerStartLocation);
		}

		// [OFFSET-TODO-6.21] "Match Has Started" (8.51 = +0xDC8, champ natif non expose par le dump)
		// *(bool*)(__int64(GameModeAthena) + 0xDC8) = true; // Match Has Started
		if (Offsets621::AFortGameModeAthena_bMatchHasStarted)
			*(bool*)(__int64(GameModeAthena) + Offsets621::AFortGameModeAthena_bMatchHasStarted) = true; // Match Has Started
		else
			Offsets621::LogMissing("AFortGameModeAthena::bMatchHasStarted");

		return true;
#else
		return StartAircraftPhaseOG(GameModeAthena, a2);
#endif // define(LATE_GAME) || defined(ONESHOT) || defined(DODGE_ROCKET) || defined(ARENA)
	}

	APawn* SpawnDefaultPawnForHook(AFortGameModeAthena* GameModeAthena, AController* NewPlayer, AActor* StartSpot)
	{
		if (!NewPlayer || !StartSpot)
		{
			FN_LOG(LogJoin, Error, "SpawnDefaultPawnFor : controleur ou point de depart absent");
			return nullptr;
		}

		AFortPlayerPawn* PlayerPawn = Cast<AFortPlayerPawn>(GameModeAthena->SpawnDefaultPawnAtTransform(NewPlayer, StartSpot->GetTransform()));
		if (!PlayerPawn)
		{
			FN_LOG(LogJoin, Error, "SpawnDefaultPawnAtTransform : aucun pawn cree");
			return nullptr;
		}

		AFortPlayerStateAthena* PlayerStateAthena = Cast<AFortPlayerStateAthena>(NewPlayer->PlayerState);

		if (PlayerStateAthena && PlayerStateAthena->PlayerTeam)
		{
			PlayerStateAthena->SquadId = (uint8)PlayerStateAthena->TeamIndex; // TeamIndex est un EFortTeam en 6.21

			/*PlayerStateAthena->DeathInfo.bDBNO = false;
			PlayerStateAthena->DeathInfo.*/

			AFortGameStateAthena* GameStateAthena = Cast<AFortGameStateAthena>(GameModeAthena->GameState);

			if (GameStateAthena)
			{
				// [8.51-ONLY] GameMemberInfoArray / FGameMemberInfo n'existent pas en 6.21
				// bool bFoundMemberInfo = false;
				// for (int32 i = 0; i < GameStateAthena->GameMemberInfoArray.Members.Num(); i++)
				// {
					// FGameMemberInfo* MemberInfo = &GameStateAthena->GameMemberInfoArray.Members[i];
					// if (!MemberInfo) continue;

					// bool bEqualEqual_UniqueNetId = UFortKismetLibrary::EqualEqual_UniqueNetIdReplUniqueNetIdRepl(MemberInfo->MemberUniqueId, PlayerStateAthena->UniqueId);
					// if (!bEqualEqual_UniqueNetId) continue;

					// MemberInfo->SquadId = PlayerStateAthena->SquadId;
					// MemberInfo->TeamIndex = PlayerStateAthena->TeamIndex;

					// GameStateAthena->GameMemberInfoArray.MarkItemDirty(*MemberInfo);

					// bFoundMemberInfo = true;
				// }

				// if (!bFoundMemberInfo)
				// {
					// FGameMemberInfo MemberInfo;
					// MemberInfo.ReplicationID = -1;
					// MemberInfo.ReplicationKey = -1;
					// MemberInfo.MostRecentArrayReplicationKey = -1;

					// MemberInfo.SquadId = PlayerStateAthena->SquadId;
					// MemberInfo.TeamIndex = PlayerStateAthena->TeamIndex;
					// MemberInfo.MemberUniqueId = PlayerStateAthena->UniqueId;

					// GameStateAthena->GameMemberInfoArray.Members.Add(MemberInfo);
					// GameStateAthena->GameMemberInfoArray.MarkArrayDirty();
				// }
			}

			PlayerStateAthena->WorldPlayerId = PlayerStateAthena->PlayerID;

			static uintptr_t AddressApplyCharacterCustomization = MinHook::FindPatternUnique(Patterns::ApplyCharacterCustomization, "ApplyCharacterCustomization");
			// void (*ApplyCharacterCustomization)(AFortPlayerState* PlayerState, AFortPawn* Pawn) = decltype(ApplyCharacterCustomization)(AddressApplyCharacterCustomization);
			Offsets621::NativeFunc<void(AFortPlayerState* PlayerState, AFortPawn* Pawn)> ApplyCharacterCustomization; // [GALAXIA PATCH] ignore (avec log) si le pattern n'est pas trouve
			ApplyCharacterCustomization.Func = decltype(ApplyCharacterCustomization.Func)(AddressApplyCharacterCustomization);
			ApplyCharacterCustomization.Name = "ApplyCharacterCustomization";

			ApplyCharacterCustomization(PlayerStateAthena, PlayerPawn);

			UFortAbilitySystemComponent* AbilitySystemComponent = PlayerStateAthena->AbilitySystemComponent;

			if (AbilitySystemComponent)
			{
				// [GALAXIA PATCH] 6.21 : si l'asset n'est pas encore en memoire, on le charge (StaticLoadObject) au lieu de garder nullptr pour toute la partie
				// static auto DefaultAbilities = FindObjectFast<UFortAbilitySet>("/Game/Abilities/Player/Generic/Traits/DefaultPlayer/GAS_AthenaPlayer.GAS_AthenaPlayer");
				static UFortAbilitySet* DefaultAbilities = nullptr;

				// [8.51-ONLY] GAS_AthenaPlayer n'existe pas en 6.21 ("Couldn't find file for package") : le jeu definit le set dans GameData->GenericPlayerAbilitySet
				// if (!DefaultAbilities)
				// 	DefaultAbilities = StaticLoadObject<UFortAbilitySet>(L"/Game/Abilities/Player/Generic/Traits/DefaultPlayer/GAS_AthenaPlayer.GAS_AthenaPlayer");
				if (!DefaultAbilities)
				{
					if (UFortGameData* GameData = Globals::GetGameData())
					{
						auto& SoftAbilitySet = GameData->GenericPlayerAbilitySet;
						DefaultAbilities = Cast<UFortAbilitySet>(UKismetSystemLibrary::Conv_SoftObjectReferenceToObject(*reinterpret_cast<TSoftObjectPtr<UObject>*>(&SoftAbilitySet)));

						if (!DefaultAbilities)
						{
							const std::string Path = SoftAbilitySet.ObjectID.AssetPathName.GetRawString();
							const std::wstring WidePath(Path.begin(), Path.end());
							DefaultAbilities = StaticLoadObject<UFortAbilitySet>(WidePath.c_str());
						}
					}
				}

				if (!DefaultAbilities)
					FN_LOG(LogJoin, Error, "GenericPlayerAbilitySet introuvable : le joueur n'aura pas ses abilities (sprint, saut, interaction)");
				Abilities::GrantGameplayAbility(DefaultAbilities, AbilitySystemComponent);
				Abilities::GrantModifierAbilityFromPlaylist(AbilitySystemComponent);
			}
		}

		AFortPlayerControllerAthena* PlayerControllerAthena = Cast<AFortPlayerControllerAthena>(NewPlayer);

		if (PlayerControllerAthena)
		{
			AFortInventory* WorldInventory = PlayerControllerAthena->WorldInventory;

			if (WorldInventory && WorldInventory->Inventory.ItemInstances.Num() <= 0)
			{
				// CosmeticLoadoutPC s'appelle CustomizationLoadout en 6.21 (Pickaxe peut etre nul)
				FFortAthenaLoadout CosmeticLoadoutPC = PlayerControllerAthena->CustomizationLoadout;
				UFortWeaponMeleeItemDefinition* PickaxeDefinition = CosmeticLoadoutPC.Pickaxe ? CosmeticLoadoutPC.Pickaxe->WeaponDefinition : nullptr;

				if (!PickaxeDefinition)
					PickaxeDefinition = FindObjectFast<UFortWeaponMeleeItemDefinition>("/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Athena_C_T01.WID_Harvest_Pickaxe_Athena_C_T01");

				Inventory::SetupInventory(PlayerControllerAthena, PickaxeDefinition);
			}
		}

		return PlayerPawn;
	}

	void (*HandlePostSafeZonePhaseChangedOG)(AFortGameModeAthena* GameModeAthena, int32 MinResetSafeZoneIndex);
	void HandlePostSafeZonePhaseChangedHook(AFortGameModeAthena* GameModeAthena, int32 MinResetSafeZoneIndex)
	{
#if defined(LATE_GAME) || defined(ONESHOT) || defined(DODGE_ROCKET) || defined(ARENA)
		AFortSafeZoneIndicator* SafeZoneIndicator = GameModeAthena->SafeZoneIndicator;
		if (!SafeZoneIndicator) return HandlePostSafeZonePhaseChangedOG(GameModeAthena, MinResetSafeZoneIndex);

		int32& SafeZonePhase = GameModeAthena->SafeZonePhase;

		if (SafeZonePhase < 3)
		{
			SafeZonePhase = 3;

			FVector SafeZoneLocation = GameModeAthena->SafeZoneLocations[SafeZonePhase];

			SafeZoneIndicator->Radius = 10000.0f;
			SafeZoneIndicator->ViewTargetLocation = SafeZoneLocation;
			SafeZoneIndicator->LastCenter = FVector_NetQuantize100(SafeZoneLocation);
			SafeZoneIndicator->NextCenter = FVector_NetQuantize100(SafeZoneLocation);

			if (SafeZonePhase == 3)
			{
				TArray<AFortPlayerController*> AllFortPlayerController = UFortKismetLibrary::GetAllFortPlayerControllers(GameModeAthena, true, true);

				for (int32 i = 0; i < AllFortPlayerController.Num(); i++)
				{
					AFortPlayerController* PlayerController = AllFortPlayerController[i];
					if (!PlayerController) continue;

					AFortPlayerPawn* PlayerPawn = PlayerController->MyFortPawn;
					if (!PlayerPawn) continue;

					FVector TeleportLocation = SafeZoneLocation;
					TeleportLocation.Z = 20000.0f;

					FVector RandomDirection = UKismetMathLibrary::RandomUnitVector();

					TeleportLocation.X += RandomDirection.X * 3000.0f;
					TeleportLocation.Y += RandomDirection.Y * 3000.0f;

					PlayerPawn->K2_TeleportTo(TeleportLocation, FRotator());

					PlayerPawn->bIsSkydiving = true;
					PlayerPawn->OnRep_IsSkydiving(false);

#if defined(LATE_GAME) || defined(ARENA)
					Functions::GiveLateGameInventory(PlayerController);
#endif // defined(LATE_GAME) || defined(ARENA)

#ifdef ONESHOT
					Functions::GiveOneShotInventory(PlayerController);
#endif // ONESHOT

#ifdef DODGE_ROCKET
					Functions::GiveRocketInventory(PlayerController);
#endif // DODGE_ROCKET

#if defined(LATE_GAME) || defined(ARENA)
					PlayerPawn->SetMaxShield(100);
					PlayerPawn->SetShield(100);
					PlayerPawn->SetMaxHealth(100);
					PlayerPawn->SetHealth(100);
#elif defined(ONESHOT)
					PlayerPawn->SetMaxShield(0);
					PlayerPawn->SetShield(0);
					PlayerPawn->SetMaxHealth(50);
					PlayerPawn->SetHealth(50);
#elif defined(DODGE_ROCKET)
					PlayerPawn->SetMaxShield(200);
					PlayerPawn->SetShield(150);
					PlayerPawn->SetMaxHealth(200);
					PlayerPawn->SetHealth(200);
#endif // defined(LATE_GAME) || defined(DODGE_ROCKET) || defined(ONESHOT) || defined(ARENA)

					PlayerPawn->bCanBeDamaged = true;
				}

				UKismetSystemLibrary::ExecuteConsoleCommand(GameModeAthena, L"startshrinksafezone", nullptr);
			}
		}
		
		FN_LOG(LogGameMode, Log, "SafeZoneIndicator: %s", SafeZoneIndicator->GetName().c_str());
		
#endif // defined(LATE_GAME) || defined(ONESHOT) || defined(DODGE_ROCKET) || defined(ARENA)

		HandlePostSafeZonePhaseChangedOG(GameModeAthena, MinResetSafeZoneIndex);

		FN_LOG(LogGameMode, Log, "GameModeAthena: %s", GameModeAthena->GetName().c_str());
		FN_LOG(LogGameMode, Log, "[AFortGameModeAthena::HandlePostSafeZonePhaseChanged] called!");

#ifdef SESSION
		int playersLeft = Globals::GetWorld()->NetDriver->ClientConnections.Num();

#ifdef NORMAL
		std::string title = "Serveur Galaxia Solo";
		std::string fr = "Les joueurs viennent de sauter du bus de combat avec " + std::to_string(playersLeft) + " joueurs, amusez vous bien! <:bush43:1224084858314162426>";
		std::string eng = " The players just jumped out of the battle bus with " + std::to_string(playersLeft) + " players, have a good game! <:bush43:1224084858314162426>";
		
		std::cout << "Webhook message: " << fr << std::endl;
		std::cout << "Webhook message: " << eng << std::endl;
#endif // NORMAL

#ifdef ARENA
		std::string title = "Serveur Galaxia Lategame Arena";
		std::string fr = "Les joueurs viennent de sauter du bus de combat avec " + std::to_string(playersLeft) + " joueurs, amusez vous bien! <:bush43:1224084858314162426>";
		std::string eng = " The players just jumped out of the battle bus with " + std::to_string(playersLeft) + " players, have a good game! <:bush43:1224084858314162426>";
		
		std::cout << "Webhook message: " << fr << std::endl;
		std::cout << "Webhook message: " << eng << std::endl;
#endif // ARENA

#ifdef AVENGER
		std::string title = "Serveur Galaxia Avengers";
		std::string fr = "Les joueurs viennent de sauter du bus de combat avec " + std::to_string(playersLeft) + " joueurs, amusez vous bien! <:bush43:1224084858314162426>";
		std::string eng = " The players just jumped out of the battle bus with " + std::to_string(playersLeft) + " players, have a good game! <:bush43:1224084858314162426>";
		
		std::cout << "Webhook message: " << fr << std::endl;
		std::cout << "Webhook message: " << eng << std::endl;
#endif // AVENGER

#ifdef LATE_GAME
		std::string title = "Serveur Galaxia Lategame";
		std::string fr = "Les joueurs viennent de sauter du bus de combat avec " + std::to_string(playersLeft) + " joueurs, amusez vous bien! <:bush43:1224084858314162426>";
		std::string eng = " The players just jumped out of the battle bus with " + std::to_string(playersLeft) + " players, have a good game! <:bush43:1224084858314162426>";

		std::cout << "Webhook message: " << fr << std::endl;
		std::cout << "Webhook message: " << eng << std::endl;
#endif // LATE_GAME

#ifdef DODGE_ROCKET
		std::string title = "Serveur Galaxia Esquive Rocket";
		std::string fr = "Les joueurs viennent de sauter du bus de combat avec " + std::to_string(playersLeft) + " joueurs, amusez vous bien! <:bush43:1224084858314162426>";
		std::string eng = " The players just jumped out of the battle bus with " + std::to_string(playersLeft) + " players, have a good game! <:bush43:1224084858314162426>";

		std::cout << "Webhook message: " << fr << std::endl;
		std::cout << "Webhook message: " << eng << std::endl;
#endif // DODGE_ROCKET

#ifdef ONESHOT
		std::string title = "Serveur Galaxia Lategame Oneshot";
		std::string fr = "Les joueurs viennent de sauter du bus de combat avec " + std::to_string(playersLeft) + " joueurs, amusez vous bien! <:bush43:1224084858314162426>";
		std::string eng = " The players just jumped out of the battle bus with " + std::to_string(playersLeft) + " players, have a good game! <:bush43:1224084858314162426>";

		std::cout << "Webhook message: " << fr << std::endl;
		std::cout << "Webhook message: " << eng << std::endl;
#endif // ONESHOT

#ifdef PLAYGROUND
		std::string title = "Serveur Galaxia Playground";
		std::string fr = "Les joueurs viennent de sauter du bus de combat avec " + std::to_string(playersLeft) + " joueurs, amusez vous bien! <:bush43:1224084858314162426>";
		std::string eng = " The players just jumped out of the battle bus with " + std::to_string(playersLeft) + " players, have a good game! <:bush43:1224084858314162426>";

		std::cout << "Webhook message: " << fr << std::endl;
		std::cout << "Webhook message: " << eng << std::endl;
#endif // PLAYGROUND

#if defined(SERVER1)
		title += " 1";
#elif defined(SERVER2)
		title += " 2";
#elif defined(SERVER3)
		title += " 3";
#elif defined(SERVER4)
		title += " 4";
#elif defined(SERVER5)
		title += " 5";
#endif

		UptimeWebHook.send_embed(title, fr, eng, 255);

		Globals::wasWebhookSent = true;
#endif // SESSION
	}

	EFortTeam PickTeamHook(AFortGameModeAthena* GameModeAthena, EFortTeam PreferredTeam, AFortPlayerControllerAthena* PlayerControllerAthena)
	{
		AFortGameStateAthena* GameStateAthena = Cast<AFortGameStateAthena>(GameModeAthena->GameState);
		if (!GameStateAthena) return EFortTeam::MAX;

		UFortPlaylistAthena* PlaylistAthena = GameStateAthena->CurrentPlaylistInfo.BasePlaylist;
		if (!GameStateAthena) return EFortTeam::MAX;

		int32 MaxTeamCount = PlaylistAthena->MaxTeamCount;
		int32 MaxTeamSize = PlaylistAthena->MaxTeamSize;

		EFortTeam ChooseTeam = EFortTeam::MAX;

		if (PlaylistAthena->bIsLargeTeamGame || PlaylistAthena->bAllowTeamSwitching)
		{
			int32 MinimumPlayers = INT32_MAX;

			for (int32 i = 0; i < GameStateAthena->Teams.Num(); i++)
			{
				AFortTeamInfo* TeamInfo = GameStateAthena->Teams[i];
				if (!TeamInfo) continue;

				if (i >= MaxTeamCount) break;

				int32 TeamMembersSize = TeamInfo->TeamMembers.Num();

				if (TeamMembersSize < MinimumPlayers && TeamMembersSize < MaxTeamSize)
				{
					MinimumPlayers = TeamMembersSize;
					ChooseTeam = EFortTeam(TeamInfo->Team);
				}
			}

			FN_LOG(LogGameMode, Log, "[AFortGameModeAthena::PickTeam] Large Team Mode: ChosenTeam: %i with %i members", ChooseTeam, MinimumPlayers);
		}
		else
		{
			for (int32 i = 0; i < GameStateAthena->Teams.Num(); i++)
			{
				AFortTeamInfo* TeamInfo = GameStateAthena->Teams[i];
				if (!TeamInfo) continue;

				if (i > MaxTeamCount)
					break;

				int32 TeamMembersSize = TeamInfo->TeamMembers.Num();

				if (TeamMembersSize >= MaxTeamSize)
					continue;

				FN_LOG(LogGameMode, Log, "[AFortGameModeAthena::PickTeam] Normal Team Mode: ChosenTeam: %i with %i members", TeamInfo->Team, TeamMembersSize);

				ChooseTeam = EFortTeam(TeamInfo->Team);
				break;
			}
		}

		return ChooseTeam;
	}
	
	void RestartHook(AGameSession* GameSession)
	{
		UWorld* World = UWorld::GetWorld();
		auto* GameModeAthena = World ? Cast<AFortGameModeAthena>(World->AuthorityGameMode) : nullptr;
		if (!GameModeAthena || !GameSession || GameModeAthena->GameSession != GameSession || bRestart)
			return;

		// [GALAXIA PATCH] Une seule tentative par carte, y compris si un callback de session est reentrant.
		bRestart = true;
		auto* RestartFunction = GameModeAthena->Class->GetFunction("GameMode", "RestartGame");
		if (!RestartFunction || !RestartFunction->ExecFunction)
		{
			FN_LOG(LogGameMode, Error, "[Restart621] GameMode.RestartGame introuvable : relance annulee.");
			return;
		}

#ifdef SESSION

#ifdef NORMAL
		std::string title = "Server Galaxia Solo";
		std::string fr = "La partie vient de se terminer, elle va se relancer... <:lol:1224085003785212016>";
		std::string eng = "The game is finished, it will restart soon... <:lol:1224085003785212016>";
		std::cout << "Webhook message: " << fr << std::endl;

		UptimeWebHook.send_embed(title, fr, eng, 16711680);

		Globals::wasWebhookSent = true;
#endif // NORMAL

#ifdef ARENA
		std::string title = "Server Galaxia Lategame Arena";
		std::string fr = "La partie vient de se terminer, elle va se relancer... <:lol:1224085003785212016>";
		std::string eng = "The game is finished, it will restart soon... <:lol:1224085003785212016>";
		std::cout << "Webhook message: " << fr << std::endl;
#endif // ARENA

#ifdef AVENGERS
		std::string title = "Server Galaxia Avengers";
		std::string fr = "La partie vient de se terminer, elle va se relancer... <:lol:1224085003785212016>";
		std::string eng = "The game is finished, it will restart soon... <:lol:1224085003785212016>";
		std::cout << "Webhook message: " << fr << std::endl;
#endif // AVENGERS

#ifdef LATE_GAME

		std::string title = "Server Galaxia Lategame";
		std::string fr = "La partie vient de se terminer, elle va se relancer... <:lol:1224085003785212016>";
		std::string eng = "The game is finished, it will restart soon... <:lol:1224085003785212016>";
		std::cout << "Webhook message: " << fr << std::endl;
#endif // LATE_GAME

#ifdef DODGE_ROCKET
		std::string title = "Server Galaxia Esquive Rocket";
		std::string fr = "La partie vient de se terminer, elle va se relancer... <:lol:1224085003785212016>";
		std::string eng = "The game is finished, it will restart soon... <:lol:1224085003785212016>";
		std::cout << "Webhook message: " << fr << std::endl;
#endif //DODGE_ROCKET

#ifdef ONESHOT
		std::string title = "Server Galaxia Lategame Oneshot";
		std::string fr = "La partie vient de se terminer, elle va se relancer... <:lol:1224085003785212016>";
		std::string eng = "The game is finished, it will restart soon... <:lol:1224085003785212016>";
		std::cout << "Webhook message: " << fr << std::endl;
#endif // ONESHOT

#ifdef PLAYGROUND
		std::string title = "Server Galaxia Playground";
		std::string fr = "La partie vient de se terminer, elle va se relancer... <:lol:1224085003785212016>";
		std::string eng = "The game is finished, it will restart soon... <:lol:1224085003785212016>";
		std::cout << "Webhook message: " << fr << std::endl;
#endif // PLAYGROUND

#if defined(SERVER1)
		title += " 1";
#elif defined(SERVER2)
		title += " 2";
#elif defined(SERVER3)
		title += " 3";
#elif defined(SERVER4)
		title += " 4";
#elif defined(SERVER5)
		title += " 5";
#endif

		UptimeWebHook.send_embed(title, fr, eng, 16711680);

		Globals::wasWebhookSent = true;

#endif // SESSION

#if defined(SESSION) && defined(PLAYGROUND)
		// Delete le match et remet les joueurs en file d'attente jusqu'a la prochaine partie.
		CURL* curl = curl_easy_init();

		std::string url = "https://galaxia-secure-backend.mega-hosting.fr:2083/fortnite/game/privaterelay/deletematch/" + storedSessionId;

		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "GalaxiaMatchDeleter/1.0");
		CURLcode res = curl_easy_perform(curl);

		long http_code = 0;
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

		if (res != CURLE_OK) {
			std::cerr << "Erreur de requête: " << curl_easy_strerror(res) << std::endl;
		}
		else {
			if (http_code == 200) {
				std::cout << "Réponse 200 OK reçue." << std::endl;
			}
			else {
				std::cout << "Code de réponse HTTP: " << http_code << std::endl;
			}
		}
		curl_easy_cleanup(curl);
#endif // defined(SESSION) && defined(PLAYGROUND)

		// [8.51-ONLY] Ancien redemarrage par arret du processus, conserve pour pouvoir le remettre plus tard.
		// std::system("taskkill /f /im FortniteClient-Win64-Shipping.exe");

		// [GALAXIA PATCH] 6.21 : exec 0x2AB8330 -> RestartGame 0x2643B80 -> ServerTravel("?Restart").
		// Le moteur recharge la carte et gere ses acteurs/connexions dans le meme processus.
		FN_LOG(LogGameMode, Log, "[Restart621] Relance native de la carte (PID %lu).", GetCurrentProcessId());
		auto Flags = RestartFunction->FunctionFlags;
		RestartFunction->FunctionFlags |= 0x400;
		GameModeAthena->ProcessEvent(RestartFunction, nullptr);
		RestartFunction->FunctionFlags = Flags;
	}

	void QueueRestartHook(AGameSession* GameSession)
	{
		UWorld* World = UWorld::GetWorld();
		auto* GameModeAthena = World ? Cast<AFortGameModeAthena>(World->AuthorityGameMode) : nullptr;
		if (!GameModeAthena || !GameSession || GameModeAthena->GameSession != GameSession || bRestart || RestartAt)
			return;

		RestartAt = GetTickCount64() + RestartDelayMs;
		FN_LOG(LogGameMode, Log, "[Restart621] Nouvelle partie dans 15 secondes.");
	}

	void TickRestart(UNetDriver* NetDriver)
	{
		// [GALAXIA PATCH] Tick reseau du monde principal, meme sans joueur. ReceiveTick Blueprint est bloque.
		UWorld* World = UWorld::GetWorld();
		if (!Offsets621::IsExpectedBuild() || !World || !NetDriver || World->NetDriver != NetDriver || NetDriver->World != World)
			return;

		auto* GameModeAthena = Cast<AFortGameModeAthena>(World->AuthorityGameMode);
		if (!GameModeAthena || World != MatchWorld || GameModeAthena != MatchGameMode || bRestart)
			return;

		static const FName WaitingPostMatch = UKismetStringLibrary::Conv_StringToName(L"WaitingPostMatch");
		static const FName LeavingMap = UKismetStringLibrary::Conv_StringToName(L"LeavingMap");
		if (GameModeAthena->MatchState == LeavingMap)
			return;

		// [GALAXIA PATCH] Le getter 6.21 (0xC79660 -> 0xEE51B0) cree une FortGameSession normale.
		// RestartGame accepte AGameSession : exiger Dedicated ici annulait toute relance.
		auto* Session = GameModeAthena->GameSession;
		if (!RestartAt && GameModeAthena->MatchState == WaitingPostMatch)
			QueueRestartHook(Session);

		if (RestartAt && GetTickCount64() >= RestartAt)
			RestartHook(Session);
	}

#if defined(LATE_GAME) || defined(ARENA)
	void (*HandleStartingNewPlayerOG)(AFortGameModeAthena* GameModeAthena, APlayerController* NewPlayer);
	void HandleStartingNewPlayerHook(AFortGameModeAthena* GameModeAthena, APlayerController* NewPlayer)
	{
		AFortPlayerController* PlayerController = Cast<AFortPlayerController>(NewPlayer);
		if (!PlayerController) return HandleStartingNewPlayerOG(GameModeAthena, NewPlayer);

		if (LateGamePlayerStart)
		{
			AFortPlayerPawn* PlayerPawn = Cast<AFortPlayerPawn>(GameModeAthena->SpawnDefaultPawnFor(NewPlayer, LateGamePlayerStart));
			if (!PlayerPawn) return;

			PlayerPawn->Owner = NewPlayer;
			PlayerPawn->OnRep_Owner();

			NewPlayer->Pawn = PlayerPawn;
			NewPlayer->OnRep_Pawn();
			NewPlayer->Possess(PlayerPawn);

			if (NewPlayer)
			{
				PlayerPawn->SetMaxHealth(100);
				PlayerPawn->SetHealth(100);
				PlayerPawn->SetMaxShield(100);
				PlayerPawn->SetShield(0);
			}

			PlayerPawn->bIsSkydiving = true;
			PlayerPawn->OnRep_IsSkydiving(false);

			Functions::GiveLateGameInventory(PlayerController);
		}

		HandleStartingNewPlayerOG(GameModeAthena, NewPlayer);
	}
#endif // defined(LATE_GAME) || defined(ARENA)

	void InitGameMode()
	{
		uintptr_t AddressRemoveFromAlivePlayers = MinHook::FindPatternUnique(Patterns::RemoveFromAlivePlayers, "RemoveFromAlivePlayers");
		uintptr_t AddressAddFromAlivePlayers = MinHook::FindPatternUnique(Patterns::AddFromAlivePlayers, "AddFromAlivePlayers");

		RemoveFromAlivePlayers.Func = decltype(RemoveFromAlivePlayers.Func)(AddressRemoveFromAlivePlayers);
		RemoveFromAlivePlayers.Name = "RemoveFromAlivePlayers";
		AddFromAlivePlayers.Func = decltype(AddFromAlivePlayers.Func)(AddressAddFromAlivePlayers);
		AddFromAlivePlayers.Name = "AddFromAlivePlayers";

		static auto DefaultGameModeAthena = AFortGameModeAthena::GetDefaultObj();

		// Hook par ProcessEvent (parametres verifies au demarrage) (8.51 = 0x7E0 / 8)
		// MinHook::HookVTable(DefaultGameModeAthena, 0x7E0 / 8, ReadyToStartMatchHook, (LPVOID*)(&ReadyToStartMatchOG), "AFortGameModeAthena::ReadyToStartMatch");
		MinHook::HookRPC<ReadyToStartMatchHook>(DefaultGameModeAthena, "GameMode", "ReadyToStartMatch", (void**)(&ReadyToStartMatchOG), 0x7E0 / 8);
		// Hook par ProcessEvent (parametres verifies au demarrage) (8.51 = 0x618 / 8)
		// MinHook::HookVTable(DefaultGameModeAthena, 0x618 / 8, SpawnDefaultPawnForHook, nullptr, "AFortGameModeAthena::SpawnDefaultPawnFor");
		MinHook::HookRPC<SpawnDefaultPawnForHook>(DefaultGameModeAthena, "GameModeBase", "SpawnDefaultPawnFor", nullptr, 0x618 / 8);

#if defined(LATE_GAME) || defined(ARENA)
		// Début d'un fix de merde
		// Hook par ProcessEvent (parametres verifies au demarrage) (8.51 = 0x648 / 8)
		// MinHook::HookVTable(DefaultGameModeAthena, 0x648 / 8, HandleStartingNewPlayerHook, (LPVOID*)(&HandleStartingNewPlayerOG), "AFortGameModeAthena::HandleStartingNewPlayer");
		MinHook::HookRPC<HandleStartingNewPlayerHook>(DefaultGameModeAthena, "GameModeBase", "HandleStartingNewPlayer", (void**)(&HandleStartingNewPlayerOG), 0x648 / 8);
#endif // defined(LATE_GAME) || defined(ARENA)

		// [OFFSET-TODO-6.21] StartAircraftPhase (8.51 = 0xfb8070)
		// MH_CreateHook((LPVOID)(InSDKUtils::GetImageBase() + 0xfb8070), StartAircraftPhaseHook, (LPVOID*)(&StartAircraftPhaseOG));
		// MH_EnableHook((LPVOID)(InSDKUtils::GetImageBase() + 0xfb8070));
		MinHook::HookNative(Offsets621::StartAircraftPhase, "StartAircraftPhase", StartAircraftPhaseHook, (void**)(&StartAircraftPhaseOG));

		// [OFFSET-TODO-6.21] HandlePostSafeZonePhaseChanged (8.51 = 0xfb9830)
		// MH_CreateHook((LPVOID)(InSDKUtils::GetImageBase() + 0xfb9830), HandlePostSafeZonePhaseChangedHook, (LPVOID*)(&HandlePostSafeZonePhaseChangedOG));
		// MH_EnableHook((LPVOID)(InSDKUtils::GetImageBase() + 0xfb9830));
		MinHook::HookNative(Offsets621::HandlePostSafeZonePhaseChanged, "HandlePostSafeZonePhaseChanged", HandlePostSafeZonePhaseChangedHook, (void**)(&HandlePostSafeZonePhaseChangedOG));

		// [OFFSET-TODO-6.21] PickTeam (8.51 = 0xfa9b20)
		// MH_CreateHook((LPVOID)(InSDKUtils::GetImageBase() + 0xfa9b20), PickTeamHook, nullptr);
		// MH_EnableHook((LPVOID)(InSDKUtils::GetImageBase() + 0xfa9b20));
		MinHook::HookNative(Offsets621::PickTeam, "PickTeam", PickTeamHook, nullptr);

		// [8.51-ONLY] Restart (8.51 = 0x153cda0)
		// MH_CreateHook((LPVOID)(InSDKUtils::GetImageBase() + 0x153cda0), RestartHook, nullptr);
		// MH_EnableHook((LPVOID)(InSDKUtils::GetImageBase() + 0x153cda0));
		// [GALAXIA PATCH] Intercepter aussi la demande native sur deconnexion du dernier joueur.
		if (Offsets621::IsExpectedBuild())
			MinHook::HookNative(Offsets621::Restart, "Restart", QueueRestartHook, nullptr);

		FN_LOG(LogInit, Log, "InitGameMode Success!");
	}
}
