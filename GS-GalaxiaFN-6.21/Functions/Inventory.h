#pragma once

namespace Inventory
{
	void UpdateInventory(AFortInventory* WorldInventory)
	{
		if (!WorldInventory)
			return;

		WorldInventory->HandleInventoryLocalUpdate();
		WorldInventory->Inventory.MarkArrayDirty();
	}

	int32 GetItemLevel(const FDataTableCategoryHandle& LootLevelData)
	{
		// [8.51-ONLY] Fonction native 8.51 ; en 6.21 le niveau se lit dans la table (comme le GS 6.21 de reference)
		// [OFFSET-TODO-6.21] GetItemLevel (8.51 = 0x14589F0)
		// int32 (*GetItemLevel)(const FDataTableCategoryHandle& DataTableCategoryHandle, int WorldLevel) = decltype(GetItemLevel)(0x14589F0 + uintptr_t(GetModuleHandle(0))); // 7FF6B96C89F0
		// auto GetItemLevel = Offsets621::Native<int32(const FDataTableCategoryHandle& DataTableCategoryHandle, int WorldLevel)>(Offsets621::GetItemLevel, "GetItemLevel"); // 7FF6B96C89F0
		// return GetItemLevel(LootLevelData, Globals::GetGameState()->WorldLevel);
		if (!LootLevelData.DataTable)
			return 0;

		const int32 WorldLevel = Globals::GetGameState()->WorldLevel;
		auto& RowMap = LootLevelData.DataTable->GetRowMap<FFortLootLevelData>();
		FFortLootLevelData* BestRow = nullptr;

		for (int i = 0; i < RowMap.Elements.Elements.Num(); ++i)
		{
			FFortLootLevelData* Row = RowMap.Elements.Elements.Data[i].ElementData.Value.Value();

			if (!Row || Row->Category != LootLevelData.RowContents || Row->LootLevel > WorldLevel)
				continue;

			if (!BestRow || Row->LootLevel > BestRow->LootLevel)
				BestRow = Row;
		}

		if (!BestRow)
			return 0;

		const int32 Range = BestRow->MaxItemLevel - BestRow->MinItemLevel;
		return Range > 0 ? BestRow->MinItemLevel + rand() % (Range + 1) : BestRow->MinItemLevel;
	}

	/*
	---- Fonctions à tester
	
	- 7FF6B936F350 (Spawn Pickup Inshallah)
	- 7FF6B95CD430 (Spawn Pickup Inshallah v2)
	- 7FF6B93851D0 (Peut etre un SetupItemEntry jsp)
	- 7FF6B902F420 (Peut etre un SetupItemEntry jsp v2)
	- 7FF6B9705BF0 (Peut etre une dinguerie à verif)
	-
	-

	*/

	char PeutEtreAddStateValueJsp(FFortItemEntry* ItemEntry, EFortItemEntryState StateType, int32 IntValue) // Marche (Je bande)
	{
		// [OFFSET-TODO-6.21] AddItemEntryStateValue (8.51 = 0x1416E40)
		// char (*TestJsp)(FFortItemEntry* ItemEntry, EFortItemEntryState StateType, int32 IntValue) = decltype(TestJsp)(0x1416E40 + uintptr_t(GetModuleHandle(0))); // 7FF6B9686E40
		auto TestJsp = Offsets621::Native<char(FFortItemEntry* ItemEntry, EFortItemEntryState StateType, int32 IntValue)>(Offsets621::AddItemEntryStateValue, "AddItemEntryStateValue"); // 7FF6B9686E40

		return TestJsp(ItemEntry, StateType, IntValue);
	}

	void JspTestCreate(FFortItemEntry* ItemEntry, UFortItemDefinition* ItemDefinition, int Count, int Level) // Marche
	{
		// [OFFSET-TODO-6.21] CreateItemEntryNative (8.51 = 0x144B200)
		// void (*JspTestCreate)(FFortItemEntry* ItemEntry, UFortItemDefinition* ItemDefinition, int Count, int Level) = decltype(JspTestCreate)(0x144B200 + uintptr_t(GetModuleHandle(0))); // 7FF6B96BB200
		auto JspTestCreate = Offsets621::Native<void(FFortItemEntry* ItemEntry, UFortItemDefinition* ItemDefinition, int Count, int Level)>(Offsets621::CreateItemEntryNative, "CreateItemEntryNative"); // 7FF6B96BB200

		JspTestCreate(ItemEntry, ItemDefinition, Count, Level);
	}

	void ResetItemEntry(FFortItemEntry* ItemEntry) // Marche
	{
		// [OFFSET-TODO-6.21] ResetItemEntry (8.51 = 0x144B580)
		// FFortItemEntry* (*ResetItemEntry)(FFortItemEntry* ItemEntry) = decltype(ResetItemEntry)(0x144B580 + uintptr_t(GetModuleHandle(0))); // 7FF6B96BB580
		auto ResetItemEntry = Offsets621::Native<FFortItemEntry*(FFortItemEntry* ItemEntry)>(Offsets621::ResetItemEntry, "ResetItemEntry", &Fallbacks621::ResetItemEntry); // 7FF6B96BB580

		ResetItemEntry(ItemEntry);
	}

	FFortItemEntry* CopyItemEntry2(FFortItemEntry* PrimaryItemEntry, FFortItemEntry* ItemEntry) // Marche pas j'ai vite test à regarder encore
	{
		// [OFFSET-TODO-6.21] CopyItemEntry2 (8.51 = 0x144B1D0)
		// FFortItemEntry* (*CopyItemEntry2)(FFortItemEntry* PrimaryItemEntry, FFortItemEntry* ItemEntry) = decltype(CopyItemEntry2)(0x144B1D0 + uintptr_t(GetModuleHandle(0))); // 7FF6B96BB1D0
		auto CopyItemEntry2 = Offsets621::Native<FFortItemEntry*(FFortItemEntry* PrimaryItemEntry, FFortItemEntry* ItemEntry)>(Offsets621::CopyItemEntry2, "CopyItemEntry2", &Fallbacks621::CopyItemEntry); // 7FF6B96BB1D0

		return CopyItemEntry2(PrimaryItemEntry, ItemEntry);
	}

	FFortItemEntry* CopyItemEntry(FFortItemEntry* PrimaryItemEntry, FFortItemEntry* ItemEntry)
	{
		// [OFFSET-TODO-6.21] CopyItemEntry (8.51 = 0x144D770)
		// FFortItemEntry* (*CopyItemEntry)(FFortItemEntry* PrimaryItemEntry, FFortItemEntry* ItemEntry) = decltype(CopyItemEntry)(0x144D770 + uintptr_t(GetModuleHandle(0))); // 7FF6B96BD770
		auto CopyItemEntry = Offsets621::Native<FFortItemEntry*(FFortItemEntry* PrimaryItemEntry, FFortItemEntry* ItemEntry)>(Offsets621::CopyItemEntry, "CopyItemEntry", &Fallbacks621::CopyItemEntry); // 7FF6B96BD770

		return CopyItemEntry(PrimaryItemEntry, ItemEntry);
	}

	FFortBaseWeaponStats* GetWeaponStats(UFortWeaponItemDefinition* ItemDefinition)
	{
		if (!ItemDefinition)
			return nullptr;

		if (ItemDefinition->IsA(UFortAmmoItemDefinition::StaticClass()) ||
			ItemDefinition->IsA(UFortTrapItemDefinition::StaticClass()) ||
			ItemDefinition->IsA(UFortIngredientItemDefinition::StaticClass()))
			return nullptr;

		// [OFFSET-TODO-6.21] GetWeaponStats (8.51 = 0xF355D0)
		// FFortBaseWeaponStats* (*GetWeaponStats)(UFortWeaponItemDefinition* ItemDefinition) = decltype(GetWeaponStats)(0xF355D0 + uintptr_t(GetModuleHandle(0))); // 7FF6B91A55D0
		auto GetWeaponStats = Offsets621::Native<FFortBaseWeaponStats*(UFortWeaponItemDefinition* ItemDefinition)>(Offsets621::GetWeaponStats, "GetWeaponStats", &Fallbacks621::GetWeaponStats); // 7FF6B91A55D0

		return GetWeaponStats(ItemDefinition);
	}

	FFortItemEntry CreateItemEntry(UFortItemDefinition* ItemDefinition, int32 Count, float Durability, int32 Level, int32 LoadedAmmo)
	{
		UFortWeaponItemDefinition* WeaponItemDefinition = Cast<UFortWeaponItemDefinition>(ItemDefinition);
		UFortWorldItemDefinition* WorldItemDefinition = Cast<UFortWorldItemDefinition>(ItemDefinition);

		FFortBaseWeaponStats* WeaponStats = GetWeaponStats(WeaponItemDefinition);
		int32 ItemLevel = WorldItemDefinition ? GetItemLevel(WorldItemDefinition->LootLevelData) : 0;

		FFortItemEntry ItemEntry;
		ResetItemEntry(&ItemEntry);
		ItemEntry.ItemDefinition = ItemDefinition;
		ItemEntry.Count = Count;
		ItemEntry.Durability = (Durability == 0.f && WorldItemDefinition) ? WorldItemDefinition->GetMaxDurability(ItemLevel) : Durability;
		ItemEntry.Level = (Level == -1) ? ItemLevel : Level;
		ItemEntry.LoadedAmmo = (WeaponStats && LoadedAmmo == -1) ? WeaponStats->ClipSize : LoadedAmmo;

		return ItemEntry;
	}

	// [8.51-ONLY] EFortPickupSpawnSource n'existe pas en 6.21
	// EFortPickupSpawnSource GetPickupSpawnSource(ABuildingActor* BuildingActor) // peut être un autre moyens jsp
	// {
	// 	ABuildingContainer* BuildingContainer = Cast<ABuildingContainer>(BuildingActor);
// 
	// 	if (!BuildingContainer)
	// 		return EFortPickupSpawnSource::Unset;
// 
	// 	if (!BuildingContainer->SearchLootTierGroup.IsValid())
	// 		return EFortPickupSpawnSource::Unset;
// 
	// 	const std::string& SearchLootTierGroup = BuildingContainer->SearchLootTierGroup.ToString();
// 
	// 	if (SearchLootTierGroup.contains("Loot_AthenaTreasure"))
	// 	{
	// 		return EFortPickupSpawnSource::Chest;
	// 	}
	// 	else if (SearchLootTierGroup.contains("Loot_AthenaAmmo"))
	// 	{
	// 		return EFortPickupSpawnSource::AmmoBox;
	// 	}
	// 	else
	// 	{
	// 		return EFortPickupSpawnSource::Unset;
	// 	}
	// }

	bool GetItemInstances(AFortPlayerController* PlayerController, UFortWorldItemDefinition* ItemDefinition, TArray<UFortWorldItem*>* OutWorldItems)
	{
		auto idk = (void*)(__int64(PlayerController) + Offsets621::AFortPlayerController_InventoryOwnerInterface); // 7FF6B9AFB53A // 0x598 en 8.51

		if (!idk)
			return false;

		// [OFFSET-TODO-6.21] GetItemInstances (8.51 = 0x16DAC30)
		// bool (*GetItemInstances)(void* idk, UFortWorldItemDefinition* ItemDefinition, TArray<UFortWorldItem*>* OutWorldItems) = decltype(GetItemInstances)(0x16DAC30 + uintptr_t(GetModuleHandle(0))); // 7FF6B994AC30 (0x14)
		auto GetItemInstances = Offsets621::Native<bool(void* idk, UFortWorldItemDefinition* ItemDefinition, TArray<UFortWorldItem*>* OutWorldItems)>(Offsets621::GetItemInstances, "GetItemInstances", &Fallbacks621::GetItemInstances); // 7FF6B994AC30 (0x14)

		return GetItemInstances(idk, ItemDefinition, OutWorldItems);
	}

	UFortWorldItem* GetItemInstance(AFortPlayerController* PlayerController, const FGuid& ItemGuid, bool bVerifMaxStackSize = false)
	{
		auto idk = (void*)(__int64(PlayerController) + Offsets621::AFortPlayerController_InventoryOwnerInterface); // 0x598 en 8.51
		
		if (!idk)
			return nullptr;

		// [OFFSET-TODO-6.21] GetItemInstance (8.51 = 0x1004CE0)
		// UFortWorldItem* (*GetItemInstance)(void* idk, const FGuid& ItemGuid) = decltype(GetItemInstance)(0x1004CE0 + uintptr_t(GetModuleHandle(0))); // 7FF6B9274CE0 (0x15)
		auto GetItemInstance = Offsets621::Native<UFortWorldItem*(void* idk, const FGuid& ItemGuid)>(Offsets621::GetItemInstance, "GetItemInstance", &Fallbacks621::GetItemInstance); // 7FF6B9274CE0 (0x15)
		UFortWorldItem* ItemInstance = GetItemInstance(idk, ItemGuid);

		if (ItemInstance)
		{
			UFortItemDefinition* ItemDefinition = ItemInstance->GetItemDefinitionBP();

			if (!ItemDefinition)
				return nullptr;

			if (ItemInstance->ItemEntry.Count >= ItemDefinition->MaxStackSize && bVerifMaxStackSize)
				return nullptr;
		}

		return ItemInstance;
	}

	FGuid GetItemGuid(AFortPlayerController* PlayerController, UFortItemDefinition* ItemDefinition, bool bVerifMaxStackSize = false)
	{
		if (!PlayerController)
		{
			FN_LOG(LogInventory, Error, "[Inventory::GetItemGuid] failed to get PlayerController!");
			return FGuid();
		}

		AFortInventory* WorldInventory = PlayerController->WorldInventory;

		if (!WorldInventory)
		{
			FN_LOG(LogInventory, Error, "[Inventory::GetItemGuid] failed to get WorldInventory!");
			return FGuid();
		}

		for (int i = 0; i < WorldInventory->Inventory.ItemInstances.Num(); i++)
		{
			UFortWorldItem* ItemInstance = WorldInventory->Inventory.ItemInstances[i];
			if (!ItemInstance) continue;

			if (ItemInstance->GetItemDefinitionBP() == ItemDefinition)
			{
				if (ItemInstance->ItemEntry.Count >= ItemDefinition->MaxStackSize && bVerifMaxStackSize)
					continue;

				return ItemInstance->GetItemGuid();
			}
		}

		return FGuid();
	}

	bool IsInventoryFull(AFortPlayerController* PlayerController)
	{
		AFortInventory* WorldInventory = PlayerController->WorldInventory;

		if (!WorldInventory)
			return false;

		int TotalItems = 0;

		for (int i = 0; i < WorldInventory->Inventory.ItemInstances.Num(); i++)
		{
			UFortWorldItem* ItemInstance = WorldInventory->Inventory.ItemInstances[i];
			if (!ItemInstance) continue;

			UFortItemDefinition* ItemDefinition = ItemInstance->ItemEntry.ItemDefinition;
			if (!ItemDefinition) continue;

			if (!ItemDefinition->bInventorySizeLimited)
				continue;

			TotalItems++;
		}

		return (TotalItems >= 5);
	}

	void ModifyCountItem(AFortPlayerController* PlayerController, const FGuid& ItemGuid, int32 Count)
	{
		AFortInventory* WorldInventory = PlayerController->WorldInventory;

		if (!WorldInventory)
			return;

		for (int i = 0; i < WorldInventory->Inventory.ItemInstances.Num(); i++)
		{
			UFortWorldItem* ItemInstance = WorldInventory->Inventory.ItemInstances[i];
			if (!ItemInstance) continue;

			if (ItemInstance->GetItemGuid() == ItemGuid)
			{
				WorldInventory->Inventory.ItemInstances[i]->ItemEntry.Count = Count;
				WorldInventory->Inventory.MarkItemDirty(WorldInventory->Inventory.ItemInstances[i]->ItemEntry);

				for (int j = 0; j < WorldInventory->Inventory.ReplicatedEntries.Num(); j++)
				{
					FFortItemEntry ItemEntry = WorldInventory->Inventory.ReplicatedEntries[j];

					if (ItemEntry.ItemGuid == ItemGuid)
					{
						WorldInventory->Inventory.ReplicatedEntries[j].Count = Count;
						WorldInventory->Inventory.MarkItemDirty(WorldInventory->Inventory.ReplicatedEntries[j]);

						break;
					}
				}
			}
		}
	}

	void ModifyLoadedAmmoItem(AFortPlayerController* PlayerController, const FGuid& ItemGuid, int32 Count)
	{
		AFortInventory* WorldInventory = PlayerController->WorldInventory;

		if (!WorldInventory)
			return;

		for (int i = 0; i < WorldInventory->Inventory.ItemInstances.Num(); i++)
		{
			UFortWorldItem* ItemInstance = WorldInventory->Inventory.ItemInstances[i];
			if (!ItemInstance) continue;

			if (ItemInstance->GetItemGuid() == ItemGuid)
			{
				WorldInventory->Inventory.ItemInstances[i]->ItemEntry.LoadedAmmo = Count;
				WorldInventory->Inventory.MarkItemDirty(WorldInventory->Inventory.ItemInstances[i]->ItemEntry);

				for (int j = 0; j < WorldInventory->Inventory.ReplicatedEntries.Num(); j++)
				{
					FFortItemEntry ItemEntry = WorldInventory->Inventory.ReplicatedEntries[j];

					if (ItemEntry.ItemGuid == ItemGuid)
					{
						WorldInventory->Inventory.ReplicatedEntries[j].LoadedAmmo = Count;
						WorldInventory->Inventory.MarkItemDirty(WorldInventory->Inventory.ReplicatedEntries[j]);

						break;
					}
				}
			}
		}
	}

	// [8.51-ONLY] EFortPickupSpawnSource n'existe pas en 6.21
	// AFortPickupAthena* SpawnPickup(UFortItemDefinition* ItemDefinition, AFortPawn* ItemOwner, FFortItemEntry* ItemEntry, const FVector& Position, bool bToss, const EFortPickupSourceTypeFlag InPickupSourceTypeFlags, const EFortPickupSpawnSource InPickupSpawnSource, bool bCopyGuid = false, const FGuid& OldGuid = FGuid())
	AFortPickupAthena* SpawnPickup(UFortItemDefinition* ItemDefinition, AFortPawn* ItemOwner, FFortItemEntry* ItemEntry, const FVector& Position, bool bToss, const EFortPickupSourceTypeFlag InPickupSourceTypeFlags, bool bCopyGuid = false, const FGuid& OldGuid = FGuid())
	{
		if (ItemEntry->Count <= 0)
			return nullptr;

		AFortPickupAthena* Pickup = Util::SpawnActor<AFortPickupAthena>(AFortPickupAthena::StaticClass(), Position);

		if (!Pickup)
		{
			FN_LOG(LogInventory, Error, "[Inventory::SpawnPickup] failed to spawn Pickup!");
			return nullptr;
		}

		if (ItemOwner)
			Pickup->PawnWhoDroppedPickup = ItemOwner;

		// 7FF6B925CD10
		// [OFFSET-TODO-6.21] PickupInitialize (8.51 = 0xFECD10)
		// void (*PickupInitialize)(AFortPickupAthena* Pickup, FFortItemEntry* ItemEntry, TArray<FFortItemEntry> MultiItemEntries, bool bSplitOnPickup) = decltype(PickupInitialize)(0xFECD10 + uintptr_t(GetModuleHandle(0)));
		auto PickupInitialize = Offsets621::Native<void(AFortPickupAthena* Pickup, FFortItemEntry* ItemEntry, TArray<FFortItemEntry> MultiItemEntries, bool bSplitOnPickup)>(Offsets621::PickupInitialize, "PickupInitialize", &Fallbacks621::PickupInitialize);

		TArray<FFortItemEntry> MultiItemEntries{};
		PickupInitialize(Pickup, ItemEntry, MultiItemEntries, false);

		if (bCopyGuid)
			ItemEntry->ItemGuid = OldGuid;

		Pickup->bRandomRotation = true;
		// [8.51-ONLY] TossPickup prend 4 parametres en 6.21 (pas de source) : la source va dans PickupSourceTypeFlags
		// Pickup->TossPickup(Position, ItemOwner, 0, bToss, InPickupSourceTypeFlags, InPickupSpawnSource);
		Pickup->PickupSourceTypeFlags |= (uint32)InPickupSourceTypeFlags;
		Pickup->TossPickup(Position, ItemOwner, 0, bToss);

		return Pickup;
	}

	void RemoveItemFromInventory(AFortInventory* WorldInventory, const FGuid& ItemGuid)
	{
		if (!WorldInventory)
			return;

		for (int i = 0; i < WorldInventory->Inventory.ItemInstances.Num(); i++)
		{
			UFortWorldItem* ItemInstance = WorldInventory->Inventory.ItemInstances[i];
			if (!ItemInstance) continue;

			if (ItemInstance->GetItemGuid() != ItemGuid)
				continue;

			WorldInventory->Inventory.ItemInstances.Remove(i);

			for (int j = 0; j < WorldInventory->Inventory.ReplicatedEntries.Num(); j++)
			{
				FFortItemEntry* ReplicatedEntry = &WorldInventory->Inventory.ReplicatedEntries[j];

				if (ReplicatedEntry->ItemGuid != ItemGuid)
					continue;

				WorldInventory->Inventory.ReplicatedEntries.Remove(j);
			}
		}
	}

	bool RemoveInventoryItem(AFortPlayerController* PlayerController, const FGuid& ItemGuid, int32 AmountToRemove, bool bForceRemoval = true)
	{
		if (AmountToRemove == 0)
			return true;

		UFortWorldItem* ItemInstance = Inventory::GetItemInstance(PlayerController, ItemGuid);

		if (!ItemInstance)
		{
			FN_LOG(LogPlayerController, Error, "[Inventory::RemoveInventoryItem] Failed to get ItemInstance!");
			return false;
		}

		FFortItemEntry ItemEntry = ItemInstance->ItemEntry;

		if (AmountToRemove >= ItemEntry.Count)
		{
			Inventory::RemoveItemFromInventory(PlayerController->WorldInventory, ItemGuid);
		}
		else if (AmountToRemove < ItemEntry.Count)
		{
			int NewCount = ItemEntry.Count - AmountToRemove;

			Inventory::ModifyCountItem(PlayerController, ItemGuid, NewCount);
		}

		return true;
	}

	void RemoveAllItemsFromInventory(AFortInventory* WorldInventory, bool bRemoveCantBeDropped = false)
	{
		if (!WorldInventory)
			return;

		std::vector<FGuid> ItemGuidToRemove;

		for (int i = 0; i < WorldInventory->Inventory.ItemInstances.Num(); i++)
		{
			UFortWorldItem* ItemInstance = WorldInventory->Inventory.ItemInstances[i];
			if (!ItemInstance) continue;

			FFortItemEntry ItemEntry = ItemInstance->ItemEntry;

			UFortWorldItemDefinition* ItemDefinition = (UFortWorldItemDefinition*)ItemInstance->GetItemDefinitionBP();
			if (!ItemDefinition) continue;

			if (!ItemDefinition->bCanBeDropped && !bRemoveCantBeDropped)
				continue;

			ItemGuidToRemove.push_back(ItemEntry.ItemGuid);
		}

		for (FGuid& ItemGuid : ItemGuidToRemove)
		{
			RemoveItemFromInventory(WorldInventory, ItemGuid);
		}
	}

	void DropAllItemsFromInventory(AFortPlayerController* PlayerController, bool bRemoveCantBeDropped = false)
	{
		AFortInventory* WorldInventory = PlayerController->WorldInventory;

		if (!WorldInventory || !PlayerController->MyFortPawn)
			return;

		for (int i = 0; i < WorldInventory->Inventory.ItemInstances.Num(); i++)
		{
			UFortWorldItem* ItemInstance = WorldInventory->Inventory.ItemInstances[i];
			if (!ItemInstance) continue;

			FFortItemEntry ItemEntry = ItemInstance->ItemEntry;

			UFortWorldItemDefinition* ItemDefinition = (UFortWorldItemDefinition*)ItemInstance->GetItemDefinitionBP();
			if (!ItemDefinition) continue;

			if (!ItemDefinition->bCanBeDropped)
				continue;

			// [8.51-ONLY] EFortPickupSpawnSource n'existe pas en 6.21
			// Inventory::SpawnPickup(ItemDefinition, PlayerController->MyFortPawn, &ItemEntry, PlayerController->MyFortPawn->K2_GetActorLocation(), true, EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::PlayerElimination, true, ItemInstance->ItemEntry.ItemGuid);
			Inventory::SpawnPickup(ItemDefinition, PlayerController->MyFortPawn, &ItemEntry, PlayerController->MyFortPawn->K2_GetActorLocation(), true, EFortPickupSourceTypeFlag::Player, true, ItemInstance->ItemEntry.ItemGuid);
		}

		RemoveAllItemsFromInventory(WorldInventory, bRemoveCantBeDropped);
	}

	UFortWorldItem* AddItem(AFortInventory* WorldInventory, FFortItemEntry* ItemEntry)
	{
		if (ItemEntry->Count == 0)
			return nullptr;

		if (!WorldInventory || !ItemEntry || !ItemEntry->ItemDefinition)
		{
			FN_LOG(LogInventory, Error, "[Inventory::AddItem] failed to get WorldInventory/ItemEntry/ItemDefinition!");
			return nullptr;
		}

		UFortWorldItem* WorldItem = Cast<UFortWorldItem>(ItemEntry->ItemDefinition->CreateTemporaryItemInstanceBP(ItemEntry->Count, ItemEntry->Level));

		if (!WorldItem)
		{
			FN_LOG(LogInventory, Error, "[Inventory::AddItem] failed to create WorldItem!");
			return nullptr;
		}

		// [GALAXIA PATCH] 6.21 : rattacher l'objet temporaire a l'inventaire et au joueur (comme le GS 6.21 de reference)
		WorldItem->OwnerInventory = WorldInventory;

		if (AFortPlayerController* OwningController = Cast<AFortPlayerController>(WorldInventory->Owner))
			WorldItem->SetOwningControllerForTemporaryItem(OwningController);

		ResetItemEntry(&WorldItem->ItemEntry);
		CopyItemEntry(&WorldItem->ItemEntry, ItemEntry);

		// [GALAXIA PATCH] Chaque objet doit avoir son GUID (le GS retrouve les objets par GUID)
		if (!UKismetGuidLibrary::IsValid_Guid(WorldItem->ItemEntry.ItemGuid))
			WorldItem->ItemEntry.ItemGuid = UKismetGuidLibrary::NewGuid();

		// [OFFSET-TODO-6.21] InitializeExistingItemInternal (8.51 = 0x145fdf0)
		// void (*InitializeExistingItemInternal)(AFortInventory* Inventory, UFortWorldItem* ExistingItem) = decltype(InitializeExistingItemInternal)(0x145fdf0 + uintptr_t(GetModuleHandle(0)));
		auto InitializeExistingItemInternal = Offsets621::Native<void(AFortInventory* Inventory, UFortWorldItem* ExistingItem)>(Offsets621::InitializeExistingItemInternal, "InitializeExistingItemInternal", &Fallbacks621::InitializeExistingItem);
		InitializeExistingItemInternal(WorldInventory, WorldItem);

		/*InstanceItemEntry->ParentInventory.ObjectIndex = WorldInventory->Index;

		WorldInventory->Inventory.ReplicatedEntries.Add(*InstanceItemEntry);
		WorldInventory->Inventory.ItemInstances.Add(WorldItem);

		WorldInventory->Inventory.MarkItemDirty(*InstanceItemEntry);*/

		return WorldItem;
	}

	FFortItemEntry* FindEntryByGuid(AFortInventory* WorldInventory, const FGuid& ItemGuid)
	{
		for (int32 i = 0; i < WorldInventory->Inventory.ReplicatedEntries.Num(); i++)
		{
			FFortItemEntry* ItemEntry = &WorldInventory->Inventory.ReplicatedEntries[i];
			if (!ItemEntry) continue;

			if (ItemEntry->ItemGuid== ItemGuid)
				return ItemEntry;
		}

		return nullptr;
	}

	void SetRequiresUpdate(AFortInventory* WorldInventory, FFortItemEntry* ItemEntry)
	{
		WorldInventory->Inventory.MarkItemDirty(*ItemEntry);

		WorldInventory->bRequiresLocalUpdate = true;
		WorldInventory->ForceNetUpdate();
	}

	void CopyStateValues(AFortInventory* WorldInventory, const FGuid& ItemGuid, TArray<FFortItemEntryStateValue> StateValues)
	{
		FFortItemEntry* ItemEntry = FindEntryByGuid(WorldInventory, ItemGuid);

		if (ItemEntry)
		{
			ItemEntry->StateValues.CopyArray(StateValues);

			// [OFFSET-TODO-6.21] SetItemEntryDirty (8.51 = 0x1467a10)
			// void (*SetToDirty)(FFortItemEntry* ItemEntry) = decltype(SetToDirty)(0x1467a10 + uintptr_t(GetModuleHandle(0)));
			auto SetToDirty = Offsets621::Native<void(FFortItemEntry* ItemEntry)>(Offsets621::SetItemEntryDirty, "SetItemEntryDirty", &Fallbacks621::SetItemEntryDirty);
			SetToDirty(ItemEntry);

			SetRequiresUpdate(WorldInventory, ItemEntry);

			// [OFFSET-TODO-6.21] UpdateItemInstances (8.51 = 0x146c170)
			// void (*UpdateItemInstances)(AFortInventory* Inventory) = decltype(UpdateItemInstances)(0x146c170 + uintptr_t(GetModuleHandle(0)));
			auto UpdateItemInstances = Offsets621::Native<void(AFortInventory* Inventory)>(Offsets621::UpdateItemInstances, "UpdateItemInstances");
			UpdateItemInstances(WorldInventory);
		}
	}

	void AddInventoryItem(AFortPlayerController* PlayerController, UFortWorldItemDefinition* ItemDefinition, FFortItemEntry* ItemEntry, FGuid CurrentWeaponGuid = FGuid())
	{
		if (!PlayerController || !ItemDefinition || !ItemEntry)
		{
			FN_LOG(LogInventory, Error, "[Inventory::AddInventoryItem] failed to get PlayerController/ItemDefinition/ItemEntry!");
			return;
		}

		// [8.51-ONLY] bTryPickupSwap (echange au ramassage) n'existe pas en 6.21
		// if (PlayerController->bTryPickupSwap)
		// {
		// 	UFortWorldItem* WorldItem = Cast<UFortWorldItem>(PlayerController->BP_GetInventoryItemWithGuid(CurrentWeaponGuid));
		//
		// 	if (WorldItem)
		// 		PlayerController->ServerAttemptInventoryDrop(WorldItem->ItemEntry.ItemGuid, WorldItem->ItemEntry.Count);
		//
		// 	PlayerController->bTryPickupSwap = false;
		// }

		AFortPlayerPawn* Pawn = PlayerController->MyFortPawn;

		bool bInventoryFull = IsInventoryFull(PlayerController);

		FGuid ItemGuid = GetItemGuid(PlayerController, ItemDefinition, true);
		UFortWorldItem* ItemInstance = GetItemInstance(PlayerController, ItemGuid, true);

		const FGuid& OldItemGuid = ItemEntry->ItemGuid;

		int32 CountToRemove = 0;
		bool bSuccess = false;
		bool bSkip = false;

		Skip:

		if (!bSkip)
		{
			if (ItemInstance)
			{
				int32 NewCount = ItemInstance->ItemEntry.Count + ItemEntry->Count;

				if (NewCount > ItemDefinition->MaxStackSize)
				{
					CountToRemove = NewCount - ItemDefinition->MaxStackSize;
					NewCount = ItemDefinition->MaxStackSize;
				}

				ModifyCountItem(PlayerController, ItemInstance->GetItemGuid(), NewCount);
				CopyStateValues(PlayerController->WorldInventory, ItemInstance->GetItemGuid(), ItemEntry->StateValues);
				bSuccess = true;
			}
			else
			{
				if ((ItemDefinition->IsA(UFortWeaponRangedItemDefinition::StaticClass()) || ItemDefinition->IsA(UFortGadgetItemDefinition::StaticClass())) && bInventoryFull)
				{
					UFortWorldItem* CurrentItemInstance = GetItemInstance(PlayerController, CurrentWeaponGuid);

					if (!CurrentItemInstance && Pawn)
					{
						AFortWeapon* CurrentWeapon = Pawn->CurrentWeapon;

						if (CurrentWeapon)
							CurrentItemInstance = Inventory::GetItemInstance(PlayerController, CurrentWeapon->ItemEntryGuid);
					}

					if (!CurrentItemInstance)
					{
						CountToRemove = ItemEntry->Count;
						bSkip = true;
						goto Skip;
					}

					UFortWorldItemDefinition* CurrentItemDefinition = (UFortWorldItemDefinition*)CurrentItemInstance->GetItemDefinitionBP();

					if (!CurrentItemDefinition)
					{
						CountToRemove = ItemEntry->Count;
						bSkip = true;
						goto Skip;
					}

					if (!CurrentItemDefinition->bCanBeDropped)
					{
						CountToRemove = ItemEntry->Count;
						bSkip = true;
						goto Skip;
					}

					PlayerController->ServerAttemptInventoryDrop(CurrentItemInstance->GetItemGuid(), CurrentItemInstance->GetNumInStack());

					ItemInstance = AddItem(PlayerController->WorldInventory, ItemEntry);

					if (ItemInstance && Pawn)
					{
						if (CurrentItemInstance->GetItemGuid() == Pawn->CurrentWeapon->ItemEntryGuid)
						{
							// [8.51-ONLY] ClientEquipItem n'existe pas en 6.21 -> ClientExecuteInventoryItem
							// PlayerController->ClientEquipItem((FGuid&)ItemInstance->GetItemGuid(), true);
							PlayerController->ClientExecuteInventoryItem(ItemInstance->GetItemGuid(), 0.0f, true, true);
						}
					}

					bSuccess = true;
				}

				if (!bSuccess)
				{
					FGuid ItemGuid = GetItemGuid(PlayerController, ItemDefinition);
					ItemInstance = GetItemInstance(PlayerController, ItemGuid);

					if (ItemInstance)
					{
						if (ItemDefinition->bAllowMultipleStacks)
						{
							ItemInstance = AddItem(PlayerController->WorldInventory, ItemEntry);
							bSuccess = true;
						}
						else
						{
							CountToRemove = ItemEntry->Count;
							bSuccess = true;
						}
					}
					else
					{
						ItemInstance = AddItem(PlayerController->WorldInventory, ItemEntry);
						bSuccess = true;
					}
				}
			}
		}

		if (ItemInstance && bSuccess)
		{
			int32 NewCount = ItemInstance->ItemEntry.Count;

			if (NewCount > ItemDefinition->MaxStackSize)
			{
				CountToRemove = NewCount - ItemDefinition->MaxStackSize;
				NewCount = ItemDefinition->MaxStackSize;

				ModifyCountItem(PlayerController, ItemInstance->GetItemGuid(), NewCount);
			}

			if (ItemDefinition->IsA(UFortGadgetItemDefinition::StaticClass()))
			{
				// 7FF6BA0E3E00
				// [OFFSET-TODO-6.21] GetInterfaceAddress (8.51 = 0x1E73E00)
				// void* (*GetInterfaceAddress)(UObject* Object, UClass* InterfaceClass) = decltype(GetInterfaceAddress)(0x1E73E00 + uintptr_t(GetModuleHandle(0)));
				auto GetInterfaceAddress = Offsets621::Native<void*(UObject* Object, UClass* InterfaceClass)>(Offsets621::GetInterfaceAddress, "GetInterfaceAddress");
				void* InterfaceAddress = GetInterfaceAddress(PlayerController, IFortInventoryOwnerInterface::StaticClass());

				// 7FF6B96D2180
				// [OFFSET-TODO-6.21] ApplyGadgetData (8.51 = 0x1462180)
				// bool (*ApplyGadgetData)(UFortGadgetItemDefinition* GadgetItemDefinition, void* InterfaceAddress, UFortWorldItem* WorldItem, bool bIdk) = decltype(ApplyGadgetData)(0x1462180 + uintptr_t(GetModuleHandle(0)));
				auto ApplyGadgetData = Offsets621::Native<bool(UFortGadgetItemDefinition* GadgetItemDefinition, void* InterfaceAddress, UFortWorldItem* WorldItem, bool bIdk)>(Offsets621::ApplyGadgetData, "ApplyGadgetData");

				if (!ApplyGadgetData((UFortGadgetItemDefinition*)ItemDefinition, InterfaceAddress, ItemInstance, true))
				{
					FN_LOG(LogInventory, Warning, "[Inventory::AddInventoryItem] failed to ApplyGadgetData!");
				}
			}

			UpdateInventory(PlayerController->WorldInventory);
		}

		if (CountToRemove > 0 && Pawn)
		{
			FFortItemEntry NewItemEntry = CreateItemEntry(ItemDefinition, CountToRemove, ItemEntry->Durability, ItemEntry->Level, ItemEntry->LoadedAmmo);
			// [8.51-ONLY] EFortPickupSpawnSource n'existe pas en 6.21
			// SpawnPickup(ItemDefinition, Pawn, &NewItemEntry, Pawn->K2_GetActorLocation(), true, EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::Unset);
			SpawnPickup(ItemDefinition, Pawn, &NewItemEntry, Pawn->K2_GetActorLocation(), true, EFortPickupSourceTypeFlag::Player);
		}
	}

	void SetupInventory(AFortPlayerController* PlayerController, UFortWeaponMeleeItemDefinition* PickaxeDefinition)
	{
		AFortInventory* WorldInventory = PlayerController->WorldInventory;
		AFortGameModeAthena* GameMode = Globals::GetGameMode();

		if (WorldInventory && GameMode)
		{
			TArray<FItemAndCount> StartingItems = GameMode->StartingItems;

			for (int32 i = 0; i < StartingItems.Num(); i++)
			{
				FItemAndCount StartingItem = StartingItems[i];

				UFortWorldItemDefinition* ItemDefinition = (UFortWorldItemDefinition*)StartingItem.Item;

				if (!ItemDefinition)
					continue;

				FFortItemEntry ItemEntry = CreateItemEntry(ItemDefinition, StartingItem.Count, 0.f, -1, -1);
				AddInventoryItem(PlayerController, ItemDefinition, &ItemEntry);
			}

			if (PickaxeDefinition)
			{
				FFortItemEntry ItemEntry = CreateItemEntry(PickaxeDefinition, 1, 0.f, -1, 0);
				AddInventoryItem(PlayerController, PickaxeDefinition, &ItemEntry);
			}

			UpdateInventory(PlayerController->WorldInventory);
		}
	}

	// [GALAXIA PATCH] Repli de AFortPickup::SetPickupTarget (8.51 = 0x1467300) tant que l'adresse 6.21 n'est pas trouvee :
	// meme etat replique que la fonction native (cible, vol, bPickedUp), puis l'objet est donne directement au joueur
	// (en 8.51 c'etait le code natif qui le donnait a la fin du vol) et le pickup disparait a la fin du vol.
	void SetPickupTargetSDK(AFortPickup* Pickup, AFortPlayerPawn* PlayerPawn, float InFlyTime, const FVector& InStartDirection, bool bPlayPickupSound)
	{
		if (!Pickup || !PlayerPawn || Pickup->bPickedUp)
			return;

		Pickup->PickupLocationData.PickupTarget = PlayerPawn;
		Pickup->PickupLocationData.ItemOwner = PlayerPawn;
		Pickup->PickupLocationData.FlyTime = InFlyTime;
		Pickup->PickupLocationData.StartDirection.X = InStartDirection.X;
		Pickup->PickupLocationData.StartDirection.Y = InStartDirection.Y;
		Pickup->PickupLocationData.StartDirection.Z = InStartDirection.Z;
		Pickup->PickupLocationData.bPlayPickupSound = bPlayPickupSound;
		Pickup->OnRep_PickupLocationData();

		Pickup->bPickedUp = true;
		Pickup->OnRep_bPickedUp();

		AFortPlayerController* PlayerController = Cast<AFortPlayerController>(PlayerPawn->Controller);
		UFortWorldItemDefinition* WorldItemDefinition = Cast<UFortWorldItemDefinition>(Pickup->PrimaryPickupItemEntry.ItemDefinition);

		if (PlayerController && WorldItemDefinition)
		{
			FGuid CurrentWeaponGuid = PlayerPawn->CurrentWeapon ? PlayerPawn->CurrentWeapon->ItemEntryGuid : FGuid();
			AddInventoryItem(PlayerController, WorldItemDefinition, &Pickup->PrimaryPickupItemEntry, CurrentWeaponGuid);
		}

		Pickup->SetLifeSpan(InFlyTime > 0.1f ? InFlyTime : 0.1f);
	}
}