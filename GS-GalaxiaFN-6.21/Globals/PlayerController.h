#pragma once
#include <random>
#include <unordered_set>
#include <chrono>
#include <thread>

std::string storedSessionId;

std::vector<std::string> split(const std::string& s, char delimiter)
{
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream tokenStream(s);

	while (std::getline(tokenStream, token, delimiter))
	{
		tokens.push_back(token);
	}

	return tokens;
}

namespace PlayerController
{
	// [GALAXIA PATCH] NativeFunc : rempli par pattern, l'appel est ignore (avec log) si le pattern n'est pas trouve en 6.21
	// ABuildingSMActor* (*ReplaceBuildingActor)(ABuildingSMActor* BuildingActor, int32 a2, TSubclassOf<ABuildingSMActor> BuildingClass, int32 BuildingLevel, int32 RotationIterations, bool bMirrored, AFortPlayerController* PlayerController);
	Offsets621::NativeFunc<ABuildingSMActor*(ABuildingSMActor* BuildingActor, int32 a2, TSubclassOf<ABuildingSMActor> BuildingClass, int32 BuildingLevel, int32 RotationIterations, bool bMirrored, AFortPlayerController* PlayerController)> ReplaceBuildingActor;
	// [GALAXIA PATCH] NativeFunc : rempli par pattern, l'appel est ignore (avec log) si le pattern n'est pas trouve en 6.21
	// bool (*CantBuild)(UWorld* World, TSubclassOf<ABuildingActor> BuildingClass, FVector_NetQuantize10& BuildLoc, FRotator& BuildRot, bool bMirrored, TArray<ABuildingActor*>* BuildingsToBeDestroyed, __int64* a7);
	Offsets621::NativeFunc<bool(UWorld* World, TSubclassOf<ABuildingActor> BuildingClass, FVector_NetQuantize10& BuildLoc, FRotator& BuildRot, bool bMirrored, TArray<ABuildingActor*>* BuildingsToBeDestroyed, __int64* a7)> CantBuild;
	// [GALAXIA PATCH] NativeFunc : rempli par pattern, l'appel est ignore (avec log) si le pattern n'est pas trouve en 6.21
	// int32 (*PayRepairCosts)(AFortPlayerController* PlayerController, ABuildingSMActor* BuildingActorToRepair);
	Offsets621::NativeFunc<int32(AFortPlayerController* PlayerController, ABuildingSMActor* BuildingActorToRepair)> PayRepairCosts;
	// [GALAXIA PATCH] NativeFunc : rempli par pattern, l'appel est ignore (avec log) si le pattern n'est pas trouve en 6.21
	// int32 (*PayBuildingCosts)(AFortPlayerController* PlayerController, FBuildingClassData BuildingClassData);
	Offsets621::NativeFunc<int32(AFortPlayerController* PlayerController, FBuildingClassData BuildingClassData)> PayBuildingCosts;
	
	void (*ModifyLoadedAmmo)(void* a1, const FGuid& ItemGuid, int32 CorrectAmmo);

	bool bFirstJump = true;

	DWORD WINAPI StartSafeZoneThread(LPVOID)
	{
		Sleep(1000);

		UKismetSystemLibrary::ExecuteConsoleCommand(Globals::GetWorld(), L"startsafezone", nullptr);
		Globals::GetGameState()->SafeZonesStartTime = 0;
		Globals::GetGameState()->bAircraftIsLocked = false;

		return 0;
	}

	void (*ServerReadyToStartMatchOG)(AFortPlayerControllerAthena* PlayerControllerAthena);
	void ServerReadyToStartMatchHook(AFortPlayerControllerAthena* PlayerControllerAthena)
	{
		if (!PlayerControllerAthena->bReadyToStartMatch)
		{
			UFortMcpProfileAthena* AthenaProfile = PlayerControllerAthena->AthenaProfile;

			if (!PlayerControllerAthena->MatchReport && AthenaProfile)
			{
				UAthenaPlayerMatchReport* NewMatchReport = Cast<UAthenaPlayerMatchReport>(UGameplayStatics::SpawnObject(UAthenaPlayerMatchReport::StaticClass(), PlayerControllerAthena));

				// Faut mettre tout ça mais az c'est une struct de merde
				// https://imgur.com/a/9bPHhks
				NewMatchReport->InitialLevelInfo.AccountLevel;
				NewMatchReport->InitialLevelInfo.LevelXpForLevel;
				NewMatchReport->InitialLevelInfo.BookLevelXpForLevel;

				NewMatchReport->bHasMatchStats = true;
				NewMatchReport->bHasRewards = true;
				NewMatchReport->bHasTeamStats = true;

				PlayerControllerAthena->MatchReport = NewMatchReport;
			}

#ifdef CHEATS
			if (!PlayerControllerAthena->CheatManager)
			{
				UFortCheatManager* CheatManager = (UFortCheatManager*)UGameplayStatics::SpawnObject(UFortCheatManager::StaticClass(), PlayerControllerAthena);

				if (CheatManager)
				{
					PlayerControllerAthena->CheatManager = CheatManager;
					PlayerControllerAthena->CheatManager->ReceiveInitCheatManager();
				}
			}
#endif // CHEATS

			// Init Anti Cheat!
			FindOrCreateAntiCheatGalaxia(PlayerControllerAthena);
		}

		ServerReadyToStartMatchOG(PlayerControllerAthena);
	}

	void (*ServerAttemptAircraftJumpOG)(AFortPlayerControllerAthena* PlayerControllerAthena, const FRotator& ClientRotation);
	void ServerAttemptAircraftJumpHook(AFortPlayerControllerAthena* PlayerControllerAthena, const FRotator& ClientRotation)
	{
		AFortGameModeAthena* GameModeAthena = Cast<AFortGameModeAthena>(Globals::GetGameMode());
		AFortGameStateAthena* GameStateAthena = Cast<AFortGameStateAthena>(Globals::GetGameState());

		if (!GameModeAthena || !GameStateAthena)
			return;

		AFortPlayerState* PlayerState = Cast<AFortPlayerState>(PlayerControllerAthena->PlayerState);
		if (!PlayerState) return;

		ServerAttemptAircraftJumpOG(PlayerControllerAthena, ClientRotation);

		if (PlayerControllerAthena->IsInAircraft() && !PlayerControllerAthena->MyFortPawn)
			GameModeAthena->RestartPlayer(PlayerControllerAthena);
	}

	void ServerTeleportToPlaygroundLobbyIslandHook(AFortPlayerControllerAthena* PlayerControllerAthena)
	{
		AFortGameModeAthena* GameModeAthena = Globals::GetGameMode();
		if (!GameModeAthena) return;

		AFortPlayerPawn* PlayerPawn = PlayerControllerAthena->MyFortPawn;
		if (!PlayerPawn) return;

		AFortPlayerStartCreative* PlayerStartCreative = Cast<AFortPlayerStartCreative>(GameModeAthena->ChoosePlayerStart(PlayerControllerAthena));

		if (!PlayerStartCreative)
		{
			FN_LOG(LogPlayerController, Error, "[AFortPlayerController::ServerTeleportToPlaygroundLobbyIsland] Failed to get PlayerStartCreative!");
			return;
		}

		PlayerPawn->K2_TeleportTo(PlayerStartCreative->K2_GetActorLocation(), PlayerStartCreative->K2_GetActorRotation());
	}

	void ServerPlaySquadQuickChatMessageHook(AFortPlayerControllerAthena* PlayerControllerAthena, const FAthenaQuickChatActiveEntry& ChatEntry, const FUniqueNetIdRepl& SenderID)
	{
		AFortPlayerStateAthena* PlayerStateAthena = Cast<AFortPlayerStateAthena>(PlayerControllerAthena->PlayerState);
		if (!PlayerStateAthena) return;

		static UAthenaEmojiItemDefinition* EmojiComm = FindObjectFast<UAthenaEmojiItemDefinition>("/Game/Athena/Items/Cosmetics/Dances/Emoji/Emoji_Comm.Emoji_Comm");

		if (PlayerControllerAthena && EmojiComm)
		{
			ETeamMemberState TeamMemberState = ETeamMemberState::None;

			switch (ChatEntry.Index)
			{
			case 0:
				TeamMemberState = ETeamMemberState::EnemySpotted;
				break;
			case 1:
				TeamMemberState = ETeamMemberState::NeedBandages;
				break;
			case 2:
				TeamMemberState = ETeamMemberState::NeedShields;
				break;
			case 3:
				TeamMemberState = ETeamMemberState::NeedMaterials;
				break;
			case 4:
				TeamMemberState = ETeamMemberState::NeedWeapon;
				break;
			default:
				TeamMemberState = ETeamMemberState::None;
				break;
			}

			PlayerStateAthena->TeamMemberState = TeamMemberState;
			PlayerStateAthena->ReplicatedTeamMemberState = TeamMemberState;
			PlayerStateAthena->OnRep_ReplicatedTeamMemberState();

			PlayerControllerAthena->ServerPlayEmoteItem(EmojiComm);
		}
	}

	void ServerAttemptInventoryDropHook(AFortPlayerControllerAthena* PlayerControllerAthena, const FGuid& ItemGuid, int32 Count)
	{
		if (Count < 1)
			return;

		AFortPlayerPawn* PlayerPawn = PlayerControllerAthena->MyFortPawn;

		if (!PlayerPawn || PlayerPawn->bIsSkydiving || PlayerPawn->bIsDBNO)
			return;

		if (PlayerControllerAthena->IsInAircraft())
			return;

		UFortWorldItem* WorldItem = Cast<UFortWorldItem>(PlayerControllerAthena->K2_GetInventoryItemWithGuid(ItemGuid));

		if (!WorldItem)
			return;

		UFortWorldItemDefinition* WorldItemDefinition = Cast<UFortWorldItemDefinition>(WorldItem->ItemEntry.ItemDefinition);

		if (!WorldItemDefinition || !WorldItemDefinition->bCanBeDropped)
			return;

		if (WorldItem->ItemEntry.Count <= 0)
		{
			UFortWeaponRangedItemDefinition* WeaponRangedItemDefinition = Cast<UFortWeaponRangedItemDefinition>(WorldItemDefinition);

			if (WeaponRangedItemDefinition && WeaponRangedItemDefinition->bPersistInInventoryWhenFinalStackEmpty)
				return;

			FN_LOG(LogPlayerController, Warning, "[AFortPlayerControllerAthena::ServerAttemptInventoryDrop] The item [%s] has a count of [%i] deletion of the item.", WorldItemDefinition->GetName().c_str(), WorldItem->ItemEntry.Count);

			Inventory::RemoveInventoryItem(PlayerControllerAthena, ItemGuid, Count, true);
			Inventory::UpdateInventory(PlayerControllerAthena->WorldInventory);
			return;
		}

		if (Inventory::RemoveInventoryItem(PlayerControllerAthena, ItemGuid, Count, true))
		{
			const float LootSpawnLocationX = 0.0f;
			const float LootSpawnLocationY = 0.0f;
			const float LootSpawnLocationZ = 60.0f;

			const FVector& SpawnLocation = PlayerPawn->K2_GetActorLocation() +
				PlayerPawn->GetActorForwardVector() * LootSpawnLocationX +
				PlayerPawn->GetActorRightVector() * LootSpawnLocationY +
				PlayerPawn->GetActorUpVector() * LootSpawnLocationZ;

			FFortItemEntry NewItemEntry = Inventory::CreateItemEntry(WorldItemDefinition, Count, WorldItem->ItemEntry.Durability, WorldItem->ItemEntry.Level, WorldItem->ItemEntry.LoadedAmmo);
			// [8.51-ONLY] EFortPickupSpawnSource n'existe pas en 6.21
			// AFortPickup* Pickup = Inventory::SpawnPickup(WorldItemDefinition, PlayerPawn, &NewItemEntry, SpawnLocation, true, EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::Unset);
			AFortPickup* Pickup = Inventory::SpawnPickup(WorldItemDefinition, PlayerPawn, &NewItemEntry, SpawnLocation, true, EFortPickupSourceTypeFlag::Player);

			if (Pickup)
			{
				FRotator PlayerRotation = PlayerPawn->K2_GetActorRotation();
				PlayerRotation.Yaw += UKismetMathLibrary::RandomFloatInRange(-60.0f, 60.0f);

				float RandomDistance = UKismetMathLibrary::RandomFloatInRange(500.0f, 600.0f);
				FVector FinalDirection = UKismetMathLibrary::GetForwardVector(PlayerRotation);

				FVector FinalLocation = SpawnLocation + FinalDirection * RandomDistance;

				Pickup->PawnWhoDroppedPickup = PlayerPawn;
				Pickup->bCombinePickupsWhenTossCompletes = true;
				// [8.51-ONLY] TossPickup prend 4 parametres en 6.21 (pas de source) : la source va dans PickupSourceTypeFlags
				// Pickup->TossPickup(FinalLocation, PlayerPawn, 0, true, EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::Unset);
				Pickup->PickupSourceTypeFlags |= (uint32)EFortPickupSourceTypeFlag::Player;
				Pickup->TossPickup(FinalLocation, PlayerPawn, 0, true);
			}

			Inventory::UpdateInventory(PlayerControllerAthena->WorldInventory);
		}
	}

	void ServerCombineInventoryItemsHook(AFortPlayerControllerAthena* PlayerControllerAthena, const FGuid& TargetItemGuid, const FGuid& SourceItemGuid)
	{
		UFortWorldItem* TargetWorldItem = Cast<UFortWorldItem>(PlayerControllerAthena->K2_GetInventoryItemWithGuid(TargetItemGuid));
		UFortWorldItem* SourceWorldItem = Cast<UFortWorldItem>(PlayerControllerAthena->K2_GetInventoryItemWithGuid(SourceItemGuid));

		if (!TargetWorldItem || !SourceWorldItem)
			return;

		UFortWorldItemDefinition* TargetItemDefinition = Cast<UFortWorldItemDefinition>(TargetWorldItem->ItemEntry.ItemDefinition);
		UFortWorldItemDefinition* SourceItemDefinition = Cast<UFortWorldItemDefinition>(SourceWorldItem->ItemEntry.ItemDefinition);

		if (!TargetItemDefinition || !SourceItemDefinition)
			return;

		if (TargetItemDefinition == SourceItemDefinition && TargetWorldItem->ItemEntry.Count < TargetItemDefinition->MaxStackSize)
		{
			int32 NewTargetCount = TargetWorldItem->ItemEntry.Count + SourceWorldItem->ItemEntry.Count;

			if (NewTargetCount > TargetItemDefinition->MaxStackSize)
			{
				int32 NewSourceCount = NewTargetCount - TargetItemDefinition->MaxStackSize;

				Inventory::ModifyCountItem(PlayerControllerAthena, SourceWorldItem->ItemEntry.ItemGuid, NewSourceCount);

				NewTargetCount = TargetItemDefinition->MaxStackSize;
			}
			else
			{
				Inventory::RemoveItemFromInventory(PlayerControllerAthena->WorldInventory, SourceWorldItem->ItemEntry.ItemGuid);
			}

			Inventory::ModifyCountItem(PlayerControllerAthena, TargetWorldItem->ItemEntry.ItemGuid, NewTargetCount);
			Inventory::UpdateInventory(PlayerControllerAthena->WorldInventory);
		}
	}

	void ServerDropAllItemsHook(AFortPlayerControllerAthena* PlayerControllerAthena, UFortItemDefinition* IgnoreItemDef)
	{
		AFortPlayerPawn* PlayerPawn = PlayerControllerAthena->MyFortPawn;
		AFortInventory* WorldInventory = PlayerControllerAthena->WorldInventory;

		if (!PlayerPawn || !WorldInventory)
			return;

		TArray<UFortWorldItem*> WorldItemToDrops;

		for (int32 i = 0; i < PlayerControllerAthena->WorldInventory->Inventory.ItemInstances.Num(); i++)
		{
			UFortWorldItem* WorldItem = PlayerControllerAthena->WorldInventory->Inventory.ItemInstances[i];
			if (!WorldItem) continue;

			FFortItemEntry* ItemEntry = &WorldItem->ItemEntry;
			if (!ItemEntry) continue;

			UFortWorldItemDefinition* ItemDefinition = Cast<UFortWorldItemDefinition>(ItemEntry->ItemDefinition);
			if (!ItemDefinition) continue;

			if (!ItemDefinition->bCanBeDropped || ItemDefinition == IgnoreItemDef)
				continue;

			WorldItemToDrops.Add(WorldItem);
		}

		const float LootSpawnLocationX = 0.0f;
		const float LootSpawnLocationY = 0.0f;
		const float LootSpawnLocationZ = 70.0f;

		const FVector& SpawnLocation = PlayerPawn->K2_GetActorLocation() +
			PlayerPawn->GetActorForwardVector() * LootSpawnLocationX +
			PlayerPawn->GetActorRightVector() * LootSpawnLocationY +
			PlayerPawn->GetActorUpVector() * LootSpawnLocationZ;

		for (int32 i = 0; i < WorldItemToDrops.Num(); i++)
		{
			UFortWorldItem* WorldItemToDrop = WorldItemToDrops[i];
			if (!WorldItemToDrop) continue;

			FVector FinalLocation = SpawnLocation;
			FVector RandomDirection = UKismetMathLibrary::RandomUnitVector();

			FinalLocation.X += RandomDirection.X * 700.0f;
			FinalLocation.Y += RandomDirection.Y * 700.0f;

			FFortItemEntry NewItemEntry;
			Inventory::CopyItemEntry2(&NewItemEntry, &WorldItemToDrop->ItemEntry);

			if (Inventory::RemoveInventoryItem(PlayerControllerAthena, WorldItemToDrop->ItemEntry.ItemGuid, WorldItemToDrop->ItemEntry.Count, true))
			{
				// [8.51-ONLY] EFortPickupSpawnSource n'existe pas en 6.21
				// AFortPickup* Pickup = Inventory::SpawnPickup(IgnoreItemDef, PlayerPawn, &NewItemEntry, SpawnLocation, true, EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::PlayerElimination);
				AFortPickup* Pickup = Inventory::SpawnPickup(IgnoreItemDef, PlayerPawn, &NewItemEntry, SpawnLocation, true, EFortPickupSourceTypeFlag::Player);

				if (Pickup)
				{
					Pickup->PawnWhoDroppedPickup = PlayerPawn;
					Pickup->bCombinePickupsWhenTossCompletes = true;
					// [8.51-ONLY] TossPickup prend 4 parametres en 6.21 (pas de source) : la source va dans PickupSourceTypeFlags
					// Pickup->TossPickup(FinalLocation, PlayerPawn, 0, true, EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::PlayerElimination);
					Pickup->PickupSourceTypeFlags |= (uint32)EFortPickupSourceTypeFlag::Player;
					Pickup->TossPickup(FinalLocation, PlayerPawn, 0, true);
				}
			}
		}

		Inventory::UpdateInventory(WorldInventory);

		if (WorldItemToDrops.IsValid())
			WorldItemToDrops.Free();
	}

	bool CheckCreateBuildingActor(AFortPlayerController* PlayerController, FBuildingClassData& BuildingClassData)
	{
		UClass* BuildingClass = BuildingClassData.BuildingClass.Get();

		// [8.51-ONLY] BroadcastRemoteClientInfo n'existe pas en 6.21
		// if (!BuildingClass)
		// {
			// AFortPlayerControllerAthena* PlayerControllerAthena = Cast<AFortPlayerControllerAthena>(PlayerController);

			// if (PlayerControllerAthena)
			// {
				// TSubclassOf<ABuildingActor> SubBuildingClass{};
				// SubBuildingClass.ClassPtr = PlayerControllerAthena->BroadcastRemoteClientInfo->RemoteBuildableClass;

				// BuildingClassData.BuildingClass = SubBuildingClass;
				// BuildingClass = BuildingClassData.BuildingClass;
			// }
		// }

		AFortPlayerPawn* PlayerPawn = PlayerController->MyFortPawn;

		if (!PlayerPawn || PlayerPawn->bIsDBNO || !BuildingClass)
			return false;

		UObject* BuildingClassDefault = BuildingClass->ClassDefaultObject;

		if (!BuildingClassDefault->IsA(ABuildingActor::StaticClass()))
			return false;

		if (!Functions::IsPlayerBuildableClasse(BuildingClass))
			return false;

		ABuildingSMActor* BuildingSMActorClassDefault = Cast<ABuildingSMActor>(BuildingClassDefault);

		if (BuildingSMActorClassDefault)
		{
			if (!PlayerController->bBuildFree)
			{
				UFortResourceItemDefinition* ResourceItemDefinition = UFortKismetLibrary::K2_GetResourceItemDefinition(BuildingSMActorClassDefault->ResourceType);
				if (!ResourceItemDefinition) return false;

				// [8.51-ONLY] BP_FindItemInstancesFromDefinition n'existe pas en 6.21 -> Inventory::GetItemInstances
				// TArray<UFortItem*> ItemArray;
				// PlayerController->BP_FindItemInstancesFromDefinition(ResourceItemDefinition, ItemArray);
				TArray<UFortWorldItem*> ItemArray;
				Inventory::GetItemInstances(PlayerController, ResourceItemDefinition, &ItemArray);

				int32 ResourceItemNum = 0;
				for (int32 i = 0; i < ItemArray.Num(); i++)
				{
					UFortWorldItem* WorldItem = Cast<UFortWorldItem>(ItemArray[i]);
					if (!WorldItem) continue;

					ResourceItemNum += WorldItem->ItemEntry.Count;
				}

				// [OFFSET-TODO-6.21] GetCreateResourceAmount (8.51 = 0x16cfcc0)
				// int32 (*GetCreateResourceAmount)(AFortPlayerController* PlayerController, const FBuildingClassData& BuildingClassData) = decltype(GetCreateResourceAmount)(0x16cfcc0 + uintptr_t(GetModuleHandle(0)));
				auto GetCreateResourceAmount = Offsets621::Native<int32(AFortPlayerController* PlayerController, const FBuildingClassData& BuildingClassData)>(Offsets621::GetCreateResourceAmount, "GetCreateResourceAmount");

				if (ResourceItemNum < GetCreateResourceAmount(PlayerController, BuildingClassData))
					return false;
			}
		}

		return true;
	}

	// [8.51-ONLY] FCreateBuildingActorData n'existe pas en 6.21 : ServerCreateBuildingActor recoit les parametres separement
	// void ServerCreateBuildingActorHook(AFortPlayerControllerAthena* PlayerControllerAthena, FCreateBuildingActorData& CreateBuildingData)
	void ServerCreateBuildingActorHook(AFortPlayerControllerAthena* PlayerControllerAthena, const FBuildingClassData& InBuildingClassData, const FVector_NetQuantize10& InBuildLoc, const FRotator& InBuildRot, bool bInMirrored, float SyncKey)
	{
		// Meme forme que FCreateBuildingActorData (8.51) pour garder le code ci-dessous
		struct
		{
			FBuildingClassData BuildingClassData;
			FVector BuildLoc;
			FRotator BuildRot;
			bool bMirrored;
		} CreateBuildingData{ InBuildingClassData, InBuildLoc, InBuildRot, bInMirrored };

		if (!CheckCreateBuildingActor(PlayerControllerAthena, CreateBuildingData.BuildingClassData))
			return;

		// [8.51-ONLY] EFortBuildPreviewMarkerOptionalAdjustment n'existe pas en 6.21 (dernier parametre retire)
		// EFortStructuralGridQueryResults (*CanPlaceBuilableClassInStructuralGrid)(AFortPlayerController* PlayerController, UClass* BuildingClass, const FVector& BuildLoc, const FRotator& BuildRot, bool bMirrored, TArray<ABuildingActor*>* ExistingBuildings, EFortBuildPreviewMarkerOptionalAdjustment* MarkerOptionalAdjustment) = decltype(CanPlaceBuilableClassInStructuralGrid)(0x16d0aa0 + uintptr_t(GetModuleHandle(0)));
		// [OFFSET-TODO-6.21] CanPlaceBuildableClassInStructuralGrid (8.51 = 0x16d0aa0)
		// EFortStructuralGridQueryResults (*CanPlaceBuilableClassInStructuralGrid)(AFortPlayerController* PlayerController, UClass* BuildingClass, const FVector& BuildLoc, const FRotator& BuildRot, bool bMirrored, TArray<ABuildingActor*>* ExistingBuildings) = decltype(CanPlaceBuilableClassInStructuralGrid)(0x16d0aa0 + uintptr_t(GetModuleHandle(0)));
		auto CanPlaceBuilableClassInStructuralGrid = Offsets621::Native<EFortStructuralGridQueryResults(AFortPlayerController* PlayerController, UClass* BuildingClass, const FVector& BuildLoc, const FRotator& BuildRot, bool bMirrored, TArray<ABuildingActor*>* ExistingBuildings)>(Offsets621::CanPlaceBuildableClassInStructuralGrid, "CanPlaceBuildableClassInStructuralGrid");

		TArray<ABuildingActor*> ExistingBuildings;
		// EFortBuildPreviewMarkerOptionalAdjustment MarkerOptionalAdjustment;
		EFortStructuralGridQueryResults GridQueryResults = CanPlaceBuilableClassInStructuralGrid(
			PlayerControllerAthena,
			CreateBuildingData.BuildingClassData.BuildingClass,
			CreateBuildingData.BuildLoc,
			CreateBuildingData.BuildRot,
			CreateBuildingData.bMirrored,
			&ExistingBuildings);
			// &MarkerOptionalAdjustment);

		if (GridQueryResults == EFortStructuralGridQueryResults::CanAdd)
		{
			for (int32 i = 0; i < ExistingBuildings.Num(); i++)
			{
				ABuildingActor* ExistingBuilding = ExistingBuildings[i];
				if (!ExistingBuilding) continue;

				ExistingBuilding->K2_DestroyActor();
			}

			ABuildingSMActor* BuildingSMActor = Util::SpawnActor<ABuildingSMActor>(CreateBuildingData.BuildingClassData.BuildingClass, CreateBuildingData.BuildLoc, CreateBuildingData.BuildRot);
			if (!BuildingSMActor) return;

			BuildingSMActor->CurrentBuildingLevel = CreateBuildingData.BuildingClassData.UpgradeLevel;

			PayBuildingCosts(PlayerControllerAthena, CreateBuildingData.BuildingClassData);

			BuildingSMActor->InitializeKismetSpawnedBuildingActor(BuildingSMActor, PlayerControllerAthena, true);
			BuildingSMActor->bPlayerPlaced = true;

			AFortPlayerStateAthena* PlayerStateAthena = Cast<AFortPlayerStateAthena>(PlayerControllerAthena->PlayerState);

			if (PlayerStateAthena)
			{
				BuildingSMActor->PlacedByPlayer(PlayerStateAthena);

				// SetTeam n'existe pas sur ABuildingSMActor en 6.21 : on ecrit le champ Team (ABuildingActor) puis OnRep_Team
				BuildingSMActor->Team = PlayerStateAthena->TeamIndex;
				BuildingSMActor->OnRep_Team();
			}
		}

		if (ExistingBuildings.IsValid())
			ExistingBuildings.Free();
	}

	void ServerRepairBuildingActorHook(AFortPlayerControllerAthena* PlayerControllerAthena, ABuildingSMActor* BuildingActorToRepair)
	{
		if (!BuildingActorToRepair)
			return;

		AFortPlayerStateAthena* PlayerStateAthena = Cast<AFortPlayerStateAthena>(PlayerControllerAthena->PlayerState);

		if (PlayerStateAthena)
		{
			if (!Functions::IsOnSameTeam(PlayerStateAthena, BuildingActorToRepair))
				return;
		}

		if (!PlayerControllerAthena->bBuildFree)
		{
			/*int32 NumAvailableBuildingResources = UFortKismetLibrary::K2_GetNumAvailableBuildingResources(PlayerControllerAthena, BuildingActorToRepair->ResourceType);
			int32 ResourceAmount = GetRepairResourceAmount(BuildingActorToRepair, PlayerControllerAthena);

			if (NumAvailableBuildingResources < ResourceAmount)
				return;*/
		}

		int32 RepairCosts = PayRepairCosts(PlayerControllerAthena, BuildingActorToRepair);
		BuildingActorToRepair->RepairBuilding(PlayerControllerAthena, RepairCosts);
	}

	void ServerBeginEditingBuildingActorHook(AFortPlayerControllerAthena* PlayerControllerAthena, ABuildingSMActor* BuildingActorToEdit)
	{
		if (!BuildingActorToEdit)
			return;

		AFortPlayerPawn* PlayerPawn = PlayerControllerAthena->MyFortPawn;
		if (!PlayerPawn) return;

		if (BuildingActorToEdit->CheckBeginEditBuildingActor(PlayerControllerAthena))
		{
			AFortPlayerStateZone* PlayerStateZone = Cast<AFortPlayerStateZone>(PlayerControllerAthena->PlayerState);
			if (!PlayerStateZone) return;

			Functions::SetEditingPlayer(BuildingActorToEdit, PlayerStateZone);

			static UFortEditToolItemDefinition* EditToolItemDefinition = FindObjectFast<UFortEditToolItemDefinition>("/Game/Items/Weapons/BuildingTools/EditTool.EditTool");

			UFortWorldItem* WorldItem = Cast<UFortWorldItem>(PlayerControllerAthena->K2_FindExistingItemForDefinition(EditToolItemDefinition, false));

			if (WorldItem &&
				EditToolItemDefinition &&
				EditToolItemDefinition->EquipWeaponDefinition(WorldItem, PlayerControllerAthena))
			{
				AFortWeap_EditingTool* EditingTool = Cast<AFortWeap_EditingTool>(PlayerPawn->CurrentWeapon);

				if (EditingTool)
					Functions::SetEditingActor(EditingTool, BuildingActorToEdit);
			}
		}
	}

	void ServerEditBuildingActorHook(AFortPlayerControllerAthena* PlayerControllerAthena, ABuildingSMActor* BuildingActorToEdit, TSubclassOf<ABuildingSMActor> NewBuildingClass, int32 RotationIterations, bool bMirrored) // RotationIterations : uint8 en 8.51, int32 en 6.21
	{
		if (!BuildingActorToEdit ||
			BuildingActorToEdit->EditingPlayer != PlayerControllerAthena->PlayerState ||
			BuildingActorToEdit->bDestroyed)
			return;

		// This security is not in the real function but it allows you not to replace everything with anything
		if (!Functions::IsPlayerBuildableClasse(NewBuildingClass.Get()))
			return;

		Functions::SetEditingPlayer(BuildingActorToEdit, nullptr);

		int32 CurrentBuildingLevel = BuildingActorToEdit->CurrentBuildingLevel;
		ReplaceBuildingActor(BuildingActorToEdit, 0, NewBuildingClass, CurrentBuildingLevel, RotationIterations, bMirrored, PlayerControllerAthena);
	}

	void ServerEndEditingBuildingActorHook(AFortPlayerControllerAthena* PlayerControllerAthena, ABuildingSMActor* BuildingActorToStopEditing)
	{
		if (!BuildingActorToStopEditing)
			return;

		AFortPlayerPawn* PlayerPawn = PlayerControllerAthena->MyFortPawn;
		if (!PlayerPawn) return;

		if (BuildingActorToStopEditing->EditingPlayer != PlayerControllerAthena->PlayerState ||
			BuildingActorToStopEditing->bDestroyed)
			return;

		Functions::SetEditingPlayer(BuildingActorToStopEditing, nullptr);

		AFortWeap_EditingTool* EditingTool = Cast<AFortWeap_EditingTool>(PlayerPawn->CurrentWeapon);

		if (EditingTool)
			Functions::SetEditingActor(EditingTool, nullptr);
	}

	void ServerExecuteInventoryItemHook(AFortPlayerControllerAthena* PlayerControllerAthena, const FGuid& ItemGuid)
	{
		AFortPlayerPawn* PlayerPawn = PlayerControllerAthena->MyFortPawn;

		if (!PlayerPawn || PlayerPawn->bIsDBNO)
			return;

		UFortWorldItem* WorldItem = Cast<UFortWorldItem>(PlayerControllerAthena->K2_GetInventoryItemWithGuid(ItemGuid));
		if (!WorldItem) return;

		UFortWorldItemDefinition* WorldItemDefinition = Cast<UFortWorldItemDefinition>(WorldItem->ItemEntry.ItemDefinition);
		if (!WorldItemDefinition) return;

		UFortGadgetItemDefinition* GadgetItemDefinition = Cast<UFortGadgetItemDefinition>(WorldItemDefinition);

		if (GadgetItemDefinition)
		{
			// [8.51-ONLY] UFortGadgetItemDefinition::GetWeaponItemDefinition n'existe pas en 6.21
			// WorldItemDefinition = GadgetItemDefinition->GetWeaponItemDefinition();
		}

		UFortWeaponItemDefinition* WeaponItemDefinition = Cast<UFortWeaponItemDefinition>(WorldItemDefinition);
		if (!WeaponItemDefinition) return;

		UFortDecoItemDefinition* DecoItemDefinition = Cast<UFortDecoItemDefinition>(WorldItemDefinition);

		if (DecoItemDefinition)
		{
			DecoItemDefinition->EquipDecoDefinition(WorldItem, PlayerControllerAthena);

			UFortContextTrapItemDefinition* ContextTrapItemDefinition = Cast<UFortContextTrapItemDefinition>(DecoItemDefinition);

			if (ContextTrapItemDefinition)
			{
				ContextTrapItemDefinition->EquipContextTrapDefinition(WorldItem, PlayerControllerAthena);
			}

			return;
		}

		WeaponItemDefinition->EquipWeaponDefinition(WorldItem, PlayerControllerAthena);
	}

	void ServerPlayEmoteItemHook(AFortPlayerControllerAthena* PlayerControllerAthena, UFortMontageItemDefinitionBase* EmoteAsset)
	{
		if (!EmoteAsset)
			return;

		AFortPlayerPawn* PlayerPawn = PlayerControllerAthena->MyFortPawn;

		if (!PlayerPawn || PlayerPawn->bIsSkydiving || PlayerPawn->bIsDBNO)
			return;

		UFortAbilitySystemComponent* AbilitySystemComponent = PlayerPawn->AbilitySystemComponent;
		if (!AbilitySystemComponent) return;

		UFortGameplayAbility* GameplayAbility = nullptr;

		if (EmoteAsset->IsA(UAthenaSprayItemDefinition::StaticClass()))
		{
			GameplayAbility = UGAB_Spray_Generic_C::GetDefaultObj();
		}
		else if (EmoteAsset->IsA(UAthenaToyItemDefinition::StaticClass()) && false)
		{
			UAthenaToyItemDefinition* ToyItemDefinition = Cast<UAthenaToyItemDefinition>(EmoteAsset);

			if (ToyItemDefinition)
			{
				UFortGameplayAbility* ToySpawnAbility = Functions::LoadGameplayAbility(ToyItemDefinition->ToySpawnAbility);
				GameplayAbility = ToySpawnAbility;

				UClass* ToyActorClass = Functions::LoadClass(ToyItemDefinition->ToyActorClass);

				if (ToyActorClass)
				{
					for (int32 i = 0; i < PlayerControllerAthena->ActiveToyInstances.Num(); i++)
					{
						ABuildingGameplayActor* ActiveToyInstance = Cast<ABuildingGameplayActor>(PlayerControllerAthena->ActiveToyInstances[i]);
						if (!ActiveToyInstance) continue;

						if (ActiveToyInstance->bActorIsBeingDestroyed)
							continue;

						if (ActiveToyInstance->IsA(ToyActorClass))
						{
							PlayerControllerAthena->ActiveToyInstances.Remove(i);
							ActiveToyInstance->FlushNetDormancy();
							ActiveToyInstance->ForceNetUpdate();
							ActiveToyInstance->SilentDie();
							ActiveToyInstance->K2_DestroyActor();
						}
					}
				}
			}
		}

		if (!GameplayAbility && EmoteAsset->IsA(UAthenaDanceItemDefinition::StaticClass()))
		{
			GameplayAbility = UGAB_Emote_Generic_C::GetDefaultObj();
		}

		if (GameplayAbility)
		{
			UAthenaDanceItemDefinition* DanceItemDefinition = Cast<UAthenaDanceItemDefinition>(EmoteAsset);

			if (DanceItemDefinition)
			{
				PlayerPawn->bMovingEmote = DanceItemDefinition->bMovingEmote;
				PlayerPawn->EmoteWalkSpeed = DanceItemDefinition->WalkForwardSpeed;
				// [8.51-ONLY] bMovingEmoteForwardOnly / bMoveForwardOnly n'existent pas en 6.21
				// PlayerPawn->bMovingEmoteForwardOnly = DanceItemDefinition->bMoveForwardOnly;
			}

			FGameplayAbilitySpecHandle Handle{ rand() };

			FGameplayAbilitySpec Spec{ -1, -1, -1 };
			Spec.Ability = GameplayAbility;
			Spec.Level = 0;
			Spec.InputID = -1;
			Spec.Handle = Handle;
			Spec.SourceObject = EmoteAsset;

			FGameplayAbilitySpecHandle OutHandle;
			Abilities::GiveAbilityAndActivateOnce(AbilitySystemComponent, &OutHandle, Spec);
		}
	}

	void ServerReturnToMainMenuHook(AFortPlayerControllerAthena* PlayerControllerAthena)
	{
		AFortPlayerPawn* PlayerPawn = PlayerControllerAthena->MyFortPawn;

		if (PlayerPawn)
		{
			PlayerControllerAthena->Suicide();

			if (PlayerPawn && PlayerPawn->bIsDBNO)
				PlayerControllerAthena->Suicide();
		}

		PlayerControllerAthena->ClientTravel(L"/Game/Maps/Frontend", ETravelType::TRAVEL_Absolute, false, FGuid());
	}

	void ServerSuicideHook(AFortPlayerControllerAthena* PlayerControllerAthena)
	{
		AFortPlayerPawn* PlayerPawn = PlayerControllerAthena->MyFortPawn;
		if (!PlayerPawn) return;

		PlayerPawn->ForceKill(FGameplayTag(), PlayerControllerAthena, nullptr);
	}

	AActor* SpawnToyInstanceHook(AFortPlayerController* PlayerController, FFrame& Stack, AActor** Ret)
	{
		TSubclassOf<AActor> ToyClass{};
		FTransform SpawnPosition = FTransform();

		Stack.StepCompiledIn(&ToyClass);
		Stack.StepCompiledIn(&SpawnPosition);

		Stack.Code += Stack.Code != nullptr;

		ABuildingGameplayActor* ToyInstance = Util::SpawnActorTransfrom<ABuildingGameplayActor>(ToyClass, SpawnPosition);

		if (ToyInstance)
		{
			ToyInstance->SetOwner(PlayerController);
			ToyInstance->OnRep_Owner();

			PlayerController->ActiveToyInstances.Add(ToyInstance);
		}

		*Ret = ToyInstance;
		return *Ret;
	}

	void ServerAcknowledgePossessionHook(APlayerController* PlayerController, APawn* P)
	{
		if (!PlayerController || !P)
			return;

		if (PlayerController->AcknowledgedPawn == P)
			return;

		PlayerController->AcknowledgedPawn = P;
	}
	
	void (*ServerCheatOG)(AFortPlayerControllerAthena* PlayerControllerAthena, const FString& Msg);
	void ServerCheatHook(AFortPlayerControllerAthena* PlayerControllerAthena, const FString& Msg)
	{
		if (!Msg.IsValid())
			return;

		UFortCheatManager* CheatManager = Cast<UFortCheatManager>(PlayerControllerAthena->CheatManager);

		if (!CheatManager)
		{
			FN_LOG(LogPlayerController, Error, "[AFortPlayerControllerAthena::ServerCheat] Cheat Manager invalid or not FortCheatManager!");
			return;
		}

		/*
			// faire des vérifs prc vzi les fdp vont en profiter
		*/

		UKismetSystemLibrary::ExecuteConsoleCommand(PlayerControllerAthena, Msg, PlayerControllerAthena);
	}

	bool RemoveInventoryItemHook(void* a1, const FGuid& ItemGuid, int32 Count, int a4, char a5)
	{
		AFortPlayerController* PlayerController = (AFortPlayerController*)(__int64(a1) - Offsets621::AFortPlayerController_InventoryOwnerInterface); // [GALAXIA PATCH] 0x598 en 8.51 : mauvais controleur en 6.21 (retrait refuse, objets dupliques)

		if (!PlayerController)
		{
			FN_LOG(LogPlayerController, Error, "[AFortPlayerController::RemoveInventoryItem] Failed to get PlayerController!");
			return false;
		}

		bool bResult = Inventory::RemoveInventoryItem(PlayerController, ItemGuid, Count);

		if (bResult)
			Inventory::UpdateInventory(PlayerController->WorldInventory);

		return bResult;
	}

	// Juste une envie de mourrir
	void ModifyLoadedAmmoHook(void* a1, const FGuid& ItemGuid, int32 CorrectAmmo)
	{
		AFortPlayerController* PlayerController = (AFortPlayerController*)(__int64(a1) - Offsets621::AFortPlayerController_InventoryOwnerInterface); // [GALAXIA PATCH] 0x598 en 8.51 : mauvais controleur en 6.21 (retrait refuse, objets dupliques)

		if (!PlayerController)
		{
			FN_LOG(LogPlayerController, Error, "[AFortPlayerController::ModifyLoadedAmmo] Failed to get PlayerController!");
			return;
		}

		Inventory::ModifyLoadedAmmoItem(PlayerController, ItemGuid, CorrectAmmo);
		Inventory::UpdateInventory(PlayerController->WorldInventory);

		ModifyLoadedAmmo(a1, ItemGuid, CorrectAmmo);
	}

	// [8.51-ONLY] UFortControllerComponent_Interaction n'existe pas en 6.21 : ServerAttemptInteract est sur AFortPlayerController
	// void (*ServerAttemptInteractOG)(UFortControllerComponent_Interaction* ControllerComponent, AActor* ReceivingActor, UPrimitiveComponent* InteractComponent, ETInteractionType InteractType, UObject* OptionalObjectData);
	// void ServerAttemptInteractHook(UFortControllerComponent_Interaction* ControllerComponent, AActor* ReceivingActor, UPrimitiveComponent* InteractComponent, ETInteractionType InteractType, UObject* OptionalObjectData)
	void (*ServerAttemptInteractOG)(AFortPlayerController* PlayerController, AActor* ReceivingActor, UPrimitiveComponent* InteractComponent, ETInteractionType InteractType, UObject* OptionalObjectData);
	void ServerAttemptInteractHook(AFortPlayerController* PlayerController, AActor* ReceivingActor, UPrimitiveComponent* InteractComponent, ETInteractionType InteractType, UObject* OptionalObjectData)
	{
		ServerAttemptInteractOG(PlayerController, ReceivingActor, InteractComponent, InteractType, OptionalObjectData);

		// AFortPlayerControllerAthena* PlayerControllerAthena = Cast<AFortPlayerControllerAthena>(ControllerComponent->GetOwner());
		AFortPlayerControllerAthena* PlayerControllerAthena = Cast<AFortPlayerControllerAthena>(PlayerController);

		if (!ReceivingActor || !PlayerControllerAthena)
			return;

		ABuildingItemCollectorActor* BuildingItemCollectorActor = Cast<ABuildingItemCollectorActor>(ReceivingActor);

		if (BuildingItemCollectorActor)
		{
			UFortWorldItemDefinition* ActiveInputItem = BuildingItemCollectorActor->ActiveInputItem;
			if (!ActiveInputItem) return;

			const float LootSpawnLocationX = BuildingItemCollectorActor->LootSpawnLocation.X;
			const float LootSpawnLocationY = BuildingItemCollectorActor->LootSpawnLocation.Y;
			const float LootSpawnLocationZ = BuildingItemCollectorActor->LootSpawnLocation.Z;

			const FVector& SpawnLocation = BuildingItemCollectorActor->K2_GetActorLocation() +
				BuildingItemCollectorActor->GetActorForwardVector() * LootSpawnLocationX +
				BuildingItemCollectorActor->GetActorRightVector() * LootSpawnLocationY +
				BuildingItemCollectorActor->GetActorUpVector() * LootSpawnLocationZ;

			for (int32 i = 0; i < BuildingItemCollectorActor->ItemCollections.Num(); i++)
			{
				const FCollectorUnitInfo& ItemCollection = BuildingItemCollectorActor->ItemCollections[i];

				if (ItemCollection.InputItem != ActiveInputItem)
					continue;

				TArray<FFortItemEntry> OutputItemEntry = ItemCollection.OutputItemEntry;

				for (int32 j = 0; j < OutputItemEntry.Num(); j++)
				{
					FFortItemEntry ItemEntry = OutputItemEntry[j];

					UFortWorldItemDefinition* ItemDefinition = Cast<UFortWorldItemDefinition>(ItemEntry.ItemDefinition);
					if (!ItemDefinition) continue;

					FFortItemEntry NewItemEntry = Inventory::CreateItemEntry(ItemDefinition, ItemEntry.Count, ItemEntry.Durability, ItemEntry.Level, ItemEntry.LoadedAmmo);
					AFortPickupAthena* Pickup = Util::SpawnActor<AFortPickupAthena>(AFortPickupAthena::StaticClass(), SpawnLocation);

					if (Pickup)
					{
						// [OFFSET-TODO-6.21] PickupInitialize (8.51 = 0xFECD10)
						// void (*PickupInitialize)(AFortPickupAthena * Pickup, FFortItemEntry * ItemEntry, TArray<FFortItemEntry> MultiItemEntries, bool bSplitOnPickup) = decltype(PickupInitialize)(0xFECD10 + uintptr_t(GetModuleHandle(0)));
						auto PickupInitialize = Offsets621::Native<void(AFortPickupAthena * Pickup, FFortItemEntry * ItemEntry, TArray<FFortItemEntry> MultiItemEntries, bool bSplitOnPickup)>(Offsets621::PickupInitialize, "PickupInitialize", &Fallbacks621::PickupInitialize);
						PickupInitialize(Pickup, &NewItemEntry, {}, false);

						Pickup->bRandomRotation = true;

						Functions::TossPickupFromContainer(
							BuildingItemCollectorActor,
							Pickup,
							OutputItemEntry.Num(),
							(j + 1),
							BuildingItemCollectorActor->LootTossConeHalfAngle,
							BuildingItemCollectorActor->LootTossDirection,
							BuildingItemCollectorActor->LootTossSpeed,
							BuildingItemCollectorActor->bTossOnGround
						);
					}
				}

				static UFunction* DoVendDeath = FindObjectFast<UFunction>("/Game/Athena/Items/Gameplay/VendingMachine/B_Athena_VendingMachine.B_Athena_VendingMachine_C.DoVendDeath");
				// [GALAXIA PATCH] 6.21 : DoVendDeath est introuvable a ce chemin, ProcessEvent plantait avec une fonction nulle
				if (DoVendDeath)
					BuildingItemCollectorActor->ProcessEvent(DoVendDeath, nullptr);

				BuildingItemCollectorActor->K2_DestroyActor();
			}
		}
	}

	void InitPlayerController()
	{
		AFortPlayerControllerAthena* FortPlayerControllerAthenaDefault = AFortPlayerControllerAthena::GetDefaultObj();
		UClass* FortPlayerControllerAthenaClass = AFortPlayerControllerAthena::StaticClass();

		// Hook par ProcessEvent (parametres verifies au demarrage) (8.51 = 0x2198 / 8)
		// MinHook::HookVTable(FortPlayerControllerAthenaDefault, 0x2198 / 8, ServerAttemptAircraftJumpHook, (LPVOID*)(&ServerAttemptAircraftJumpOG), "AFortPlayerControllerAthena::ServerAttemptAircraftJump");
		MinHook::HookRPC<ServerAttemptAircraftJumpHook>(FortPlayerControllerAthenaDefault, "FortPlayerControllerAthena", "ServerAttemptAircraftJump", (void**)(&ServerAttemptAircraftJumpOG), 0x2198 / 8);
		// [8.51-ONLY] ServerTeleportToPlaygroundLobbyIsland n'existe pas en 6.21
		// MinHook::HookVTable(FortPlayerControllerAthenaDefault, 0x22A0 / 8, ServerTeleportToPlaygroundLobbyIslandHook, nullptr, "AFortPlayerControllerAthena::ServerTeleportToPlaygroundLobbyIsland");

		// Hook par ProcessEvent (parametres verifies au demarrage) (8.51 = 0x12F8 / 8)
		// MinHook::HookVTable(FortPlayerControllerAthenaDefault, 0x12F8 / 8, ServerReadyToStartMatchHook, (LPVOID*)(&ServerReadyToStartMatchOG), "AFortPlayerControllerAthena::ServerReadyToStartMatch");
		MinHook::HookRPC<ServerReadyToStartMatchHook>(FortPlayerControllerAthenaDefault, "FortPlayerController", "ServerReadyToStartMatch", (void**)(&ServerReadyToStartMatchOG), 0x12F8 / 8);
		// Hook par ProcessEvent (parametres verifies au demarrage) (8.51 = 0x22F0 / 8)
		// MinHook::HookVTable(FortPlayerControllerAthenaDefault, 0x22F0 / 8, ServerPlaySquadQuickChatMessageHook, nullptr, "AFortPlayerControllerAthena::ServerPlaySquadQuickChatMessage");
		MinHook::HookRPC<ServerPlaySquadQuickChatMessageHook>(FortPlayerControllerAthenaDefault, "FortPlayerControllerAthena", "ServerPlaySquadQuickChatMessage", nullptr, 0x22F0 / 8);
		// Hook par ProcessEvent (parametres verifies au demarrage) (8.51 = 0x1080 / 8)
		// MinHook::HookVTable(FortPlayerControllerAthenaDefault, 0x1080 / 8, ServerAttemptInventoryDropHook, nullptr, "AFortPlayerControllerAthena::ServerAttemptInventoryDrop");
		MinHook::HookRPC<ServerAttemptInventoryDropHook>(FortPlayerControllerAthenaDefault, "FortPlayerController", "ServerAttemptInventoryDrop", nullptr, 0x1080 / 8);
		// Hook par ProcessEvent (parametres verifies au demarrage) (8.51 = 0x1090 / 8)
		// MinHook::HookVTable(FortPlayerControllerAthenaDefault, 0x1090 / 8, ServerCombineInventoryItemsHook, nullptr, "AFortPlayerControllerAthena::ServerCombineInventoryItems");
		MinHook::HookRPC<ServerCombineInventoryItemsHook>(FortPlayerControllerAthenaDefault, "FortPlayerController", "ServerCombineInventoryItems", nullptr, 0x1090 / 8);
		// Hook par ProcessEvent (parametres verifies au demarrage) (8.51 = 0xD80 / 8)
		// MinHook::HookVTable(FortPlayerControllerAthenaDefault, 0xD80 / 8, ServerDropAllItemsHook, nullptr, "AFortPlayerControllerAthena::ServerDropAllItems");
		MinHook::HookRPC<ServerDropAllItemsHook>(FortPlayerControllerAthenaDefault, "FortPlayerController", "ServerDropAllItems", nullptr, 0xD80 / 8);
		// Hook par ProcessEvent (parametres verifies au demarrage) (8.51 = 0x1118 / 8)
		// MinHook::HookVTable(FortPlayerControllerAthenaDefault, 0x1118 / 8, ServerCreateBuildingActorHook, nullptr, "AFortPlayerControllerAthena::ServerCreateBuildingActor");
		MinHook::HookRPC<ServerCreateBuildingActorHook>(FortPlayerControllerAthenaDefault, "FortPlayerController", "ServerCreateBuildingActor", nullptr, 0x1118 / 8);
		// Hook par ProcessEvent (parametres verifies au demarrage) (8.51 = 0x10F8 / 8)
		// MinHook::HookVTable(FortPlayerControllerAthenaDefault, 0x10F8 / 8, ServerRepairBuildingActorHook, nullptr, "AFortPlayerControllerAthena::ServerRepairBuildingActor");
		MinHook::HookRPC<ServerRepairBuildingActorHook>(FortPlayerControllerAthenaDefault, "FortPlayerController", "ServerRepairBuildingActor", nullptr, 0x10F8 / 8);
		// Hook par ProcessEvent (parametres verifies au demarrage) (8.51 = 0x1150 / 8)
		// MinHook::HookVTable(FortPlayerControllerAthenaDefault, 0x1150 / 8, ServerBeginEditingBuildingActorHook, nullptr, "AFortPlayerControllerAthena::ServerBeginEditingBuildingActor");
		MinHook::HookRPC<ServerBeginEditingBuildingActorHook>(FortPlayerControllerAthenaDefault, "FortPlayerController", "ServerBeginEditingBuildingActor", nullptr, 0x1150 / 8);
		// Hook par ProcessEvent (parametres verifies au demarrage) (8.51 = 0x1128 / 8)
		// MinHook::HookVTable(FortPlayerControllerAthenaDefault, 0x1128 / 8, ServerEditBuildingActorHook, nullptr, "AFortPlayerControllerAthena::ServerEditBuildingActor");
		MinHook::HookRPC<ServerEditBuildingActorHook>(FortPlayerControllerAthenaDefault, "FortPlayerController", "ServerEditBuildingActor", nullptr, 0x1128 / 8);
		// Hook par ProcessEvent (parametres verifies au demarrage) (8.51 = 0x1140 / 8)
		// MinHook::HookVTable(FortPlayerControllerAthenaDefault, 0x1140 / 8, ServerEndEditingBuildingActorHook, nullptr, "AFortPlayerControllerAthena::ServerEndEditingBuildingActor");
		MinHook::HookRPC<ServerEndEditingBuildingActorHook>(FortPlayerControllerAthenaDefault, "FortPlayerController", "ServerEndEditingBuildingActor", nullptr, 0x1140 / 8);
		// Hook par ProcessEvent (parametres verifies au demarrage) (8.51 = 0xFE8 / 8)
		// MinHook::HookVTable(FortPlayerControllerAthenaDefault, 0xFE8 / 8, ServerExecuteInventoryItemHook, nullptr, "AFortPlayerControllerAthena::ServerExecuteInventoryItem");
		MinHook::HookRPC<ServerExecuteInventoryItemHook>(FortPlayerControllerAthenaDefault, "FortPlayerController", "ServerExecuteInventoryItem", nullptr, 0xFE8 / 8);
		// Hook par ProcessEvent (parametres verifies au demarrage) (8.51 = 0xDE0 / 8)
		// MinHook::HookVTable(FortPlayerControllerAthenaDefault, 0xDE0 / 8, ServerPlayEmoteItemHook, nullptr, "AFortPlayerControllerAthena::ServerPlayEmoteItem");
		MinHook::HookRPC<ServerPlayEmoteItemHook>(FortPlayerControllerAthenaDefault, "FortPlayerController", "ServerPlayEmoteItem", nullptr, 0xDE0 / 8);
		// Hook par ProcessEvent (parametres verifies au demarrage) (8.51 = 0x12D8 / 8)
		// MinHook::HookVTable(FortPlayerControllerAthenaDefault, 0x12D8 / 8, ServerReturnToMainMenuHook, nullptr, "AFortPlayerControllerAthena::ServerReturnToMainMenu");
		MinHook::HookRPC<ServerReturnToMainMenuHook>(FortPlayerControllerAthenaDefault, "FortPlayerController", "ServerReturnToMainMenu", nullptr, 0x12D8 / 8);
		// Hook par ProcessEvent (parametres verifies au demarrage) (8.51 = 0x1348 / 8)
		// MinHook::HookVTable(FortPlayerControllerAthenaDefault, 0x1348 / 8, ServerSuicideHook, nullptr, "AFortPlayerControllerAthena::ServerSuicide");
		MinHook::HookRPC<ServerSuicideHook>(FortPlayerControllerAthenaDefault, "FortPlayerController", "ServerSuicide", nullptr, 0x1348 / 8);
		// Hook par ProcessEvent (parametres verifies au demarrage) (8.51 = 0x840 / 8)
		// MinHook::HookVTable(FortPlayerControllerAthenaDefault, 0x840 / 8, ServerAcknowledgePossessionHook, nullptr, "AFortPlayerControllerAthena::ServerAcknowledgePossession");
		MinHook::HookRPC<ServerAcknowledgePossessionHook>(FortPlayerControllerAthenaDefault, "PlayerController", "ServerAcknowledgePossession", nullptr, 0x840 / 8);
		// Hook par ProcessEvent (parametres verifies au demarrage) (8.51 = 0xDD0 / 8)
		// MinHook::HookVTable(FortPlayerControllerAthenaDefault, 0xDD0 / 8, ServerCheatHook, (LPVOID*)(&ServerCheatOG), "AFortPlayerControllerAthena::ServerCheat");
		MinHook::HookRPC<ServerCheatHook>(FortPlayerControllerAthenaDefault, "FortPlayerController", "ServerCheat", (void**)(&ServerCheatOG), 0xDD0 / 8);

		UFunction* SpawnToyInstanceFunc = FortPlayerControllerAthenaClass->GetFunction("FortPlayerController", "SpawnToyInstance");
		MinHook::HookFunctionExec(SpawnToyInstanceFunc, SpawnToyInstanceHook, nullptr);

		// [8.51-ONLY] UFortControllerComponent_Interaction n'existe pas en 6.21 : ServerAttemptInteract est accroche sur AFortPlayerControllerAthena juste en dessous
		// UFortControllerComponent_Interaction* FortControllerComponent_InteractionDefault = UFortControllerComponent_Interaction::GetDefaultObj();
		// UClass* FortControllerComponent_InteractionClass = UFortControllerComponent_Interaction::StaticClass();

		// MinHook::HookVTable(FortControllerComponent_InteractionDefault, 0x400 / 8, ServerAttemptInteractHook, (LPVOID*)(&ServerAttemptInteractOG), "UFortControllerComponent_Interaction::ServerAttemptInteract");

		// Hook par ProcessEvent (parametres verifies au demarrage)
		MinHook::HookRPC<ServerAttemptInteractHook>(FortPlayerControllerAthenaDefault, "FortPlayerController", "ServerAttemptInteract", (void**)(&ServerAttemptInteractOG));

		uintptr_t AddressRemoveInventoryItem = MinHook::FindPatternUnique(Patterns::RemoveInventoryItem, "RemoveInventoryItem");
		uintptr_t AddressModifyLoadedAmmo = MinHook::FindPatternUnique(Patterns::ModifyLoadedAmmo, "ModifyLoadedAmmo");
		uintptr_t AddressPayBuildingCosts = MinHook::FindPatternUnique(Patterns::PayBuildingCosts, "PayBuildingCosts");
		uintptr_t AddressPayRepairCosts = MinHook::FindPatternUnique(Patterns::PayRepairCosts, "PayRepairCosts");
		uintptr_t AddressCantBuild = MinHook::FindPatternUnique(Patterns::CantBuild, "CantBuild");
		uintptr_t AddressReplaceBuildingActor = MinHook::FindPatternUnique(Patterns::ReplaceBuildingActor, "ReplaceBuildingActor");
		uintptr_t AddressApplyCharacterCustomization = MinHook::FindPatternUnique(Patterns::ApplyCharacterCustomization, "ApplyCharacterCustomization");

		MinHook::HookAddress(AddressRemoveInventoryItem, "RemoveInventoryItem", RemoveInventoryItemHook, nullptr); // [GALAXIA PATCH] rien n'est installe si le pattern n'est pas trouve
		MinHook::HookAddress(AddressModifyLoadedAmmo, "ModifyLoadedAmmo", ModifyLoadedAmmoHook, (void**)(&ModifyLoadedAmmo)); // [GALAXIA PATCH] rien n'est installe si le pattern n'est pas trouve

		PayBuildingCosts.Func = decltype(PayBuildingCosts.Func)(AddressPayBuildingCosts);
		PayBuildingCosts.Name = "PayBuildingCosts";
		PayRepairCosts.Func = decltype(PayRepairCosts.Func)(AddressPayRepairCosts);
		PayRepairCosts.Name = "PayRepairCosts";
		CantBuild.Func = decltype(CantBuild.Func)(AddressCantBuild);
		CantBuild.Name = "CantBuild";
		ReplaceBuildingActor.Func = decltype(ReplaceBuildingActor.Func)(AddressReplaceBuildingActor);
		ReplaceBuildingActor.Name = "ReplaceBuildingActor";

		FN_LOG(LogInit, Log, "InitPlayerController Success!");
	}
}
