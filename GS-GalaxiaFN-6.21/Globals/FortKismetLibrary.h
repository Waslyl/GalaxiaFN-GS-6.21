#pragma once

namespace FortKismetLibrary
{
	AFortPickup* K2_SpawnPickupInWorldHook(UFortKismetLibrary* KismetLibrary, FFrame& Stack, AFortPickup** Ret)
	{
		UObject* WorldContextObject;
		UFortWorldItemDefinition* ItemDefinition;
		int32 NumberToSpawn;
		FVector Position;
		FVector Direction;
		int32 OverrideMaxStackCount;
		bool bToss;
		bool bRandomRotation;
		bool bBlockedFromAutoPickup;

		Stack.StepCompiledIn(&WorldContextObject);
		Stack.StepCompiledIn(&ItemDefinition);
		Stack.StepCompiledIn(&NumberToSpawn);
		Stack.StepCompiledIn(&Position);
		Stack.StepCompiledIn(&Direction);
		Stack.StepCompiledIn(&OverrideMaxStackCount);
		Stack.StepCompiledIn(&bToss);
		Stack.StepCompiledIn(&bRandomRotation);
		Stack.StepCompiledIn(&bBlockedFromAutoPickup);

		Stack.Code += Stack.Code != nullptr;

		UWorld* World = UWorld::GetWorld();

		if (!World || !ItemDefinition || (NumberToSpawn < 1))
		{
			*Ret = nullptr;
			return *Ret;
		}

		FFortItemEntry ItemEntry = Inventory::CreateItemEntry(ItemDefinition, NumberToSpawn, 0.f, -1, -1);
		// [8.51-ONLY] EFortPickupSpawnSource n'existe pas en 6.21
		// AFortPickup* Pickup = Inventory::SpawnPickup(ItemDefinition, nullptr, &ItemEntry, Position, bToss, EFortPickupSourceTypeFlag::Other, EFortPickupSpawnSource::Unset);
		AFortPickup* Pickup = Inventory::SpawnPickup(ItemDefinition, nullptr, &ItemEntry, Position, bToss, EFortPickupSourceTypeFlag::Other);

		if (Pickup)
			Pickup->bWeaponsCanBeAutoPickups = !bBlockedFromAutoPickup;

		*Ret = Pickup;
		return *Ret;
	}

	void K2_GiveBuildingResourceHook(UFortKismetLibrary* KismetLibrary, FFrame& Stack, void* Ret)
	{
		AFortPlayerController* Controller;
		EFortResourceType ResourceType;
		int32 ResourceAmount;

		Stack.StepCompiledIn(&Controller);
		Stack.StepCompiledIn(&ResourceType);
		Stack.StepCompiledIn(&ResourceAmount);

		Stack.Code += Stack.Code != nullptr;

		UFortResourceItemDefinition* ResourceItemDefinition = UFortKismetLibrary::K2_GetResourceItemDefinition(ResourceType);

		if (!Controller || !ResourceItemDefinition)
			return;

		UFortKismetLibrary::K2_GiveItemToPlayer(Controller, ResourceItemDefinition, ResourceAmount, false);
	}

	void K2_GiveItemToAllPlayersHook(UFortKismetLibrary* KismetLibrary, FFrame& Stack, void* Ret)
	{
		UObject* WorldContextObject;
		UFortWorldItemDefinition* ItemDefinition;
		int32 NumberToGive;
		bool bNotifyPlayer;

		Stack.StepCompiledIn(&WorldContextObject);
		Stack.StepCompiledIn(&ItemDefinition);
		Stack.StepCompiledIn(&NumberToGive);
		Stack.StepCompiledIn(&bNotifyPlayer);

		Stack.Code += Stack.Code != nullptr;

		TArray<AFortPlayerController*> AllFortPlayerControllers = UFortKismetLibrary::GetAllFortPlayerControllers(WorldContextObject, true, false);

		for (int32 i = 0; i < AllFortPlayerControllers.Num(); i++)
		{
			AFortPlayerController* PlayerController = AllFortPlayerControllers[i];
			if (!PlayerController) return;

			UFortKismetLibrary::K2_GiveItemToPlayer(PlayerController, ItemDefinition, NumberToGive, bNotifyPlayer);
		}
	}

	void K2_GiveItemToPlayerHook(UFortKismetLibrary* KismetLibrary, FFrame& Stack, void* Ret)
	{
		AFortPlayerController* PlayerController;
		UFortWorldItemDefinition* ItemDefinition;
		int32 NumberToGive;
		bool bNotifyPlayer;

		Stack.StepCompiledIn(&PlayerController);
		Stack.StepCompiledIn(&ItemDefinition);
		Stack.StepCompiledIn(&NumberToGive);
		Stack.StepCompiledIn(&bNotifyPlayer);

		Stack.Code += Stack.Code != nullptr;

		if (!PlayerController || !ItemDefinition)
			return;

		FFortItemEntry ItemEntry = Inventory::CreateItemEntry(ItemDefinition, NumberToGive, 0.f, -1, -1);
		AFortPlayerPawn* PlayerPawn = PlayerController->MyFortPawn;

		if (bNotifyPlayer && PlayerPawn)
		{
			const FVector& SpawnLocation = PlayerPawn->K2_GetActorLocation();

			AFortPickup* Pickup = UFortKismetLibrary::K2_SpawnPickupInWorld(PlayerController, ItemDefinition, NumberToGive, SpawnLocation, SpawnLocation, 0, false, false, true);

			if (Pickup)
			{
				const FVector& StartDirection = FVector({ 0, 0, 0 });

				// [OFFSET-TODO-6.21] SetPickupTarget (8.51 = 0x1467300)
				// void (*SetPickupTarget)(AFortPickup* Pickup, AFortPlayerPawn* PlayerPawn, float InFlyTime, const FVector& InStartDirection, bool bPlayPickupSound) = decltype(SetPickupTarget)(0x1467300 + uintptr_t(GetModuleHandle(0)));
				auto SetPickupTarget = Offsets621::Native<void(AFortPickup* Pickup, AFortPlayerPawn* PlayerPawn, float InFlyTime, const FVector& InStartDirection, bool bPlayPickupSound)>(Offsets621::SetPickupTarget, "SetPickupTarget", &Inventory::SetPickupTargetSDK);
				SetPickupTarget(Pickup, PlayerPawn, -1.0f, StartDirection, true);
			}
		}
		else
		{
			Inventory::AddInventoryItem(PlayerController, ItemDefinition, &ItemEntry);
			Inventory::UpdateInventory(PlayerController->WorldInventory);
		}
	}

	void GiveItemToInventoryOwnerHook(UFortKismetLibrary* KismetLibrary, FFrame& Stack, void* Ret)
	{
		TScriptInterface<IFortInventoryOwnerInterface> InventoryOwner; 
		UFortWorldItemDefinition* ItemDefinition; 
		int32 NumberToGive; 
		bool bNotifyPlayer; 
		int32 ItemLevel;

		Stack.StepCompiledIn(&InventoryOwner);
		Stack.StepCompiledIn(&ItemDefinition);
		Stack.StepCompiledIn(&NumberToGive);
		Stack.StepCompiledIn(&bNotifyPlayer);
		Stack.StepCompiledIn(&ItemLevel);

		Stack.Code += Stack.Code != nullptr;
		
		if (!InventoryOwner.GetInterfaceRef() || !ItemDefinition)
			return;

		AFortPlayerController* PlayerController = (AFortPlayerController*)(__int64(InventoryOwner.GetInterfaceRef()) - Offsets621::AFortPlayerController_InventoryOwnerInterface); // [GALAXIA PATCH] 0x598 en 8.51
		if (!PlayerController) return;

		FFortItemEntry ItemEntry = Inventory::CreateItemEntry(ItemDefinition, NumberToGive, 0.f, ItemLevel, -1);
		AFortPlayerPawn* PlayerPawn = PlayerController->MyFortPawn;

		if (bNotifyPlayer && PlayerPawn)
		{
			const FVector& SpawnLocation = PlayerPawn->K2_GetActorLocation();

			AFortPickup* Pickup = UFortKismetLibrary::K2_SpawnPickupInWorld(PlayerController, ItemDefinition, NumberToGive, SpawnLocation, SpawnLocation, 0, false, false, true);

			if (Pickup)
			{
				const FVector& StartDirection = FVector({ 0, 0, 0 });

				// [OFFSET-TODO-6.21] SetPickupTarget (8.51 = 0x1467300)
				// void (*SetPickupTarget)(AFortPickup* Pickup, AFortPlayerPawn* PlayerPawn, float InFlyTime, const FVector& InStartDirection, bool bPlayPickupSound) = decltype(SetPickupTarget)(0x1467300 + uintptr_t(GetModuleHandle(0)));
				auto SetPickupTarget = Offsets621::Native<void(AFortPickup* Pickup, AFortPlayerPawn* PlayerPawn, float InFlyTime, const FVector& InStartDirection, bool bPlayPickupSound)>(Offsets621::SetPickupTarget, "SetPickupTarget", &Inventory::SetPickupTargetSDK);
				SetPickupTarget(Pickup, PlayerPawn, -1.0f, StartDirection, true);
			}
		}
		else
		{
			Inventory::AddInventoryItem(PlayerController, ItemDefinition, &ItemEntry);
			Inventory::UpdateInventory(PlayerController->WorldInventory);
		}
	}

	int32 K2_RemoveFortItemFromPlayerHook(UFortKismetLibrary* KismetLibrary, FFrame& Stack, int32* Ret)
	{
		AFortPlayerController* PlayerController; 
		UFortItem* Item;
		int32 AmountToRemove;
		bool bForceRemoval;

		Stack.StepCompiledIn(&PlayerController);
		Stack.StepCompiledIn(&Item);
		Stack.StepCompiledIn(&AmountToRemove);
		Stack.StepCompiledIn(&bForceRemoval);

		Stack.Code += Stack.Code != nullptr;

		if (!PlayerController || !Item)
			return 0;

		int32 ItemCount = Item->GetNumInStack();
		int32 RemovedAmount = FMath::Min(AmountToRemove, ItemCount);

		Inventory::RemoveInventoryItem(PlayerController, Item->GetItemGuid(), RemovedAmount, bForceRemoval);
		Inventory::UpdateInventory(PlayerController->WorldInventory);

		return RemovedAmount;
	}

	void K2_RemoveItemFromAllPlayersHook(UFortKismetLibrary* KismetLibrary, FFrame& Stack, void* Ret)
	{
		UObject* WorldContextObject;
		UFortWorldItemDefinition* ItemDefinition;
		int32 AmountToRemove;

		Stack.StepCompiledIn(&WorldContextObject);
		Stack.StepCompiledIn(&ItemDefinition);
		Stack.StepCompiledIn(&AmountToRemove);

		Stack.Code += Stack.Code != nullptr;

		TArray<AFortPlayerController*> AllFortPlayerControllers = UFortKismetLibrary::GetAllFortPlayerControllers(WorldContextObject, true, false);

		for (int32 i = 0; i < AllFortPlayerControllers.Num(); i++)
		{
			AFortPlayerController* PlayerController = AllFortPlayerControllers[i];
			if (!PlayerController) return;

			UFortKismetLibrary::K2_RemoveItemFromPlayer(PlayerController, ItemDefinition, AmountToRemove, false);
		}
	}

	bool PickLootDropsHook(UFortKismetLibrary* KismetLibrary, FFrame& Stack, bool* Ret)
	{
		UObject* WorldContextObject;
		TArray<FFortItemEntry> OutLootToDrop;
		FName TierGroupName;
		int32 WorldLevel;
		int32 ForcedLootTier;

		Stack.StepCompiledIn(&WorldContextObject);
		TArray<FFortItemEntry>& LootToDrops = Stack.StepCompiledInRef<TArray<FFortItemEntry>>(&OutLootToDrop);
		Stack.StepCompiledIn(&TierGroupName);
		Stack.StepCompiledIn(&WorldLevel);
		Stack.StepCompiledIn(&ForcedLootTier);

		Stack.Code += Stack.Code != nullptr;

		FName LootTierKey = FName(0);
		int32 LootTier = -1;

		bool bResult = Loots::PickLootTierKeyAndLootTierFromTierGroup(&LootTierKey, &LootTier, TierGroupName, WorldLevel, 0, ForcedLootTier, FGameplayTagContainer());

		if (bResult)
		{
			*Ret = Loots::PickLootDrops(&LootToDrops, WorldLevel, LootTierKey, 0, 0, FGameplayTagContainer(), false, false);
			return *Ret;
		}

		*Ret = false;
		return *Ret;
	}

	int32 K2_RemoveItemFromPlayerHook(AFortPlayerController* PlayerController, UFortWorldItemDefinition* ItemDefinition, int32 AmountToRemove, bool bForceRemoval)
	{
		if (!PlayerController || !ItemDefinition)
			return 0;

		UFortWorldItem* WorldItem = Cast<UFortWorldItem>(PlayerController->K2_FindExistingItemForDefinition(ItemDefinition, false));

		if (WorldItem)
		{
			int32 ItemCount = WorldItem->ItemEntry.Count;
			int32 RemovedAmount = FMath::Min(AmountToRemove, ItemCount);

			Inventory::RemoveInventoryItem(PlayerController, WorldItem->ItemEntry.ItemGuid, RemovedAmount, bForceRemoval);
			Inventory::UpdateInventory(PlayerController->WorldInventory);

			return RemovedAmount;
		}

		return 0;
	}

	int32 K2_RemoveItemFromPlayerByGuidHook(AFortPlayerController* PlayerController, const FGuid& ItemGuid, int32 AmountToRemove, bool bForceRemoval)
	{
		if (!PlayerController)
			return 0;

		UFortWorldItem* WorldItem = Cast<UFortWorldItem>(PlayerController->K2_GetInventoryItemWithGuid(ItemGuid));

		if (WorldItem)
		{
			int32 ItemCount = WorldItem->ItemEntry.Count;
			int32 RemovedAmount = FMath::Min(AmountToRemove, ItemCount);

			Inventory::RemoveInventoryItem(PlayerController, WorldItem->ItemEntry.ItemGuid, RemovedAmount, bForceRemoval);
			Inventory::UpdateInventory(PlayerController->WorldInventory);

			return RemovedAmount;
		}

		return 0;
	}

	// [8.51-ONLY] AFortGameModePickup et AFortAthenaSupplyDrop::SpawnGameModePickup n'existent pas en 6.21
	// AFortGameModePickup* SpawnGameModePickupHook(UFortKismetLibrary* KismetLibrary, FFrame& Stack, AFortGameModePickup** Ret)
	// {
		// UFortWorldItemDefinition* ItemDefinition; 
		// TSubclassOf<AFortGameModePickup> PickupClass; 
		// int32 NumberToSpawn; 
		// AFortPawn* TriggeringPawn; 
		// FVector Position; 
		// FVector Direction;

		// Stack.StepCompiledIn(&ItemDefinition);
		// Stack.StepCompiledIn(&PickupClass);
		// Stack.StepCompiledIn(&NumberToSpawn);
		// Stack.StepCompiledIn(&TriggeringPawn);
		// Stack.StepCompiledIn(&Position);
		// Stack.StepCompiledIn(&Direction);

		// Stack.Code += Stack.Code != nullptr;

		// FFortItemEntry ItemEntry = Inventory::CreateItemEntry(ItemDefinition, NumberToSpawn, 0.f, -1, -1);

		// AFortGameModePickup* GameModePickup = Util::SpawnActor<AFortGameModePickup>(AFortGameModePickup::StaticClass(), Position);

		// if (GameModePickup)
		// {
			// // 7FF6B925CD10
			// void (*PickupInitialize)(AFortPickupAthena * Pickup, FFortItemEntry * ItemEntry, TArray<FFortItemEntry> MultiItemEntries, bool bSplitOnPickup) = decltype(PickupInitialize)(0xFECD10 + uintptr_t(GetModuleHandle(0)));

			// TArray<FFortItemEntry> MultiItemEntries{};
			// PickupInitialize(GameModePickup, &ItemEntry, MultiItemEntries, false);

			// GameModePickup->bRandomRotation = true;
			// // [8.51-ONLY] TossPickup prend 4 parametres en 6.21 (pas de source) : la source va dans PickupSourceTypeFlags
			// // GameModePickup->TossPickup(Position, nullptr, 0, true, EFortPickupSourceTypeFlag::Container, EFortPickupSpawnSource::SupplyDrop);
			// GameModePickup->PickupSourceTypeFlags |= (uint32)EFortPickupSourceTypeFlag::Container;
			// GameModePickup->TossPickup(Position, nullptr, 0, true);
		// }

		// *Ret = GameModePickup;
		// return *Ret;
	// }

	AFortPickup* SpawnPickupHook(UFortKismetLibrary* KismetLibrary, FFrame& Stack, AFortPickup** Ret)
	{
		UFortWorldItemDefinition* ItemDefinition; 
		int32 NumberToSpawn; 
		AFortPawn* TriggeringPawn; 
		FVector Position; 
		FVector Direction;

		Stack.StepCompiledIn(&ItemDefinition);
		Stack.StepCompiledIn(&NumberToSpawn);
		Stack.StepCompiledIn(&TriggeringPawn);
		Stack.StepCompiledIn(&Position);
		Stack.StepCompiledIn(&Direction);

		Stack.Code += Stack.Code != nullptr;

		FFortItemEntry ItemEntry = Inventory::CreateItemEntry(ItemDefinition, NumberToSpawn, 0.f, -1, -1);

		// [8.51-ONLY] EFortPickupSpawnSource n'existe pas en 6.21
		// *Ret = Inventory::SpawnPickup(ItemDefinition, nullptr, &ItemEntry, Position, true, EFortPickupSourceTypeFlag::Container, EFortPickupSpawnSource::SupplyDrop);
		*Ret = Inventory::SpawnPickup(ItemDefinition, nullptr, &ItemEntry, Position, true, EFortPickupSourceTypeFlag::Container);
		return *Ret;
	}

	AFortPickup* SpawnPickupFromItemEntryHook(UFortKismetLibrary* KismetLibrary, FFrame& Stack, AFortPickup** Ret)
	{
		UFortWorldItemDefinition* ItemDefinition; 
		FFortItemEntry InItemEntry; 
		int32 NumberToSpawn; 
		AFortPawn* TriggeringPawn; 
		FVector Position; 
		FVector Direction;

		Stack.StepCompiledIn(&ItemDefinition);
		Stack.StepCompiledIn(&InItemEntry);
		Stack.StepCompiledIn(&NumberToSpawn);
		Stack.StepCompiledIn(&TriggeringPawn);
		Stack.StepCompiledIn(&Position);
		Stack.StepCompiledIn(&Direction);

		Stack.Code += Stack.Code != nullptr;

		// [8.51-ONLY] EFortPickupSpawnSource n'existe pas en 6.21
		// *Ret = Inventory::SpawnPickup(ItemDefinition, nullptr, &InItemEntry, Position, true, EFortPickupSourceTypeFlag::Container, EFortPickupSpawnSource::SupplyDrop);
		*Ret = Inventory::SpawnPickup(ItemDefinition, nullptr, &InItemEntry, Position, true, EFortPickupSourceTypeFlag::Container);
		return *Ret;
	}

	AFortAthenaSupplyDrop;

	void InitFortKismetLibrary()
	{
		UFortKismetLibrary* FortKismetLibraryDefault = UFortKismetLibrary::GetDefaultObj();
		UClass* FortKismetLibraryClass = UFortKismetLibrary::StaticClass();

		UFunction* K2_SpawnPickupInWorldFunc = FortKismetLibraryClass->GetFunction("FortKismetLibrary", "K2_SpawnPickupInWorld");
		MinHook::HookFunctionExec(K2_SpawnPickupInWorldFunc, K2_SpawnPickupInWorldHook, nullptr);

		UFunction* GiveItemToInventoryOwnerFunc = FortKismetLibraryClass->GetFunction("FortKismetLibrary", "GiveItemToInventoryOwner");
		MinHook::HookFunctionExec(GiveItemToInventoryOwnerFunc, GiveItemToInventoryOwnerHook, nullptr);

		UFunction* K2_GiveBuildingResourceFunc = FortKismetLibraryClass->GetFunction("FortKismetLibrary", "K2_GiveBuildingResource");
		MinHook::HookFunctionExec(K2_GiveBuildingResourceFunc, K2_GiveBuildingResourceHook, nullptr);

		UFunction* K2_GiveItemToAllPlayersFunc = FortKismetLibraryClass->GetFunction("FortKismetLibrary", "K2_GiveItemToAllPlayers");
		MinHook::HookFunctionExec(K2_GiveItemToAllPlayersFunc, K2_GiveItemToAllPlayersHook, nullptr);

		UFunction* K2_GiveItemToPlayerFunc = FortKismetLibraryClass->GetFunction("FortKismetLibrary", "K2_GiveItemToPlayer");
		MinHook::HookFunctionExec(K2_GiveItemToPlayerFunc, K2_GiveItemToPlayerHook, nullptr);

		UFunction* K2_RemoveItemFromAllPlayersFunc = FortKismetLibraryClass->GetFunction("FortKismetLibrary", "K2_RemoveItemFromAllPlayers");
		MinHook::HookFunctionExec(K2_RemoveItemFromAllPlayersFunc, K2_RemoveItemFromAllPlayersHook, nullptr);

		UFunction* PickLootDropsFunc = FortKismetLibraryClass->GetFunction("FortKismetLibrary", "PickLootDrops");
		MinHook::HookFunctionExec(PickLootDropsFunc, PickLootDropsHook, nullptr);

		// [OFFSET-TODO-6.21] K2_RemoveItemFromPlayer (8.51 = 0x134bb90)
		// MH_CreateHook((LPVOID)(InSDKUtils::GetImageBase() + 0x134bb90), K2_RemoveItemFromPlayerHook, nullptr);
		// MH_EnableHook((LPVOID)(InSDKUtils::GetImageBase() + 0x134bb90));
		MinHook::HookNative(Offsets621::K2_RemoveItemFromPlayer, "K2_RemoveItemFromPlayer", K2_RemoveItemFromPlayerHook, nullptr);

		// [OFFSET-TODO-6.21] K2_RemoveItemFromPlayerByGuid (8.51 = 0x134bbb0)
		// MH_CreateHook((LPVOID)(InSDKUtils::GetImageBase() + 0x134bbb0), K2_RemoveItemFromPlayerByGuidHook, nullptr);
		// MH_EnableHook((LPVOID)(InSDKUtils::GetImageBase() + 0x134bbb0));
		MinHook::HookNative(Offsets621::K2_RemoveItemFromPlayerByGuid, "K2_RemoveItemFromPlayerByGuid", K2_RemoveItemFromPlayerByGuidHook, nullptr);

		FN_LOG(LogInit, Log, "InitFortKismetLibrary Success!");

		AFortAthenaSupplyDrop* FortAthenaSupplyDropDefault = AFortAthenaSupplyDrop::GetDefaultObj();
		UClass* FortAthenaSupplyDropClass = AFortAthenaSupplyDrop::StaticClass();

		// [8.51-ONLY] AFortAthenaSupplyDrop::SpawnGameModePickup n'existe pas en 6.21
		// UFunction* SpawnGameModePickupFunc = FortAthenaSupplyDropClass->GetFunction("FortAthenaSupplyDrop", "SpawnGameModePickup");
		// MinHook::HookFunctionExec(SpawnGameModePickupFunc, SpawnGameModePickupHook, nullptr);

		UFunction* SpawnPickupFunc = FortAthenaSupplyDropClass->GetFunction("FortAthenaSupplyDrop", "SpawnPickup");
		MinHook::HookFunctionExec(SpawnPickupFunc, SpawnPickupHook, nullptr);

		UFunction* SpawnPickupFromItemEntryFunc = FortAthenaSupplyDropClass->GetFunction("FortAthenaSupplyDrop", "SpawnPickupFromItemEntry");
		MinHook::HookFunctionExec(SpawnPickupFromItemEntryFunc, SpawnPickupFromItemEntryHook, nullptr);

		FN_LOG(LogInit, Log, "InitFortAthenaSupplyDrop Success!");
	}
}