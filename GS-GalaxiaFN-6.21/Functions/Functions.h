#pragma once
#include <iostream>
#include <cctype>

struct FVehicleWeaponTahLesFous
{
public:
	UFortWeaponItemDefinition* VehicleWeapon = nullptr;
	int32 SeatIndex = -1;

	FVehicleWeaponTahLesFous(UFortWeaponItemDefinition* InVehicleWeapon, int32 InSeatIndex)
		: VehicleWeapon(InVehicleWeapon)
		, SeatIndex(InSeatIndex)
	{
	}
};

namespace Functions
{
	std::multimap<UClass*, FVehicleWeaponTahLesFous> GetVehicleWeapons()
	{
		std::multimap<UClass*, FVehicleWeaponTahLesFous> VehicleWeapons;

		// [8.51-ONLY] Armes de vehicules : Baller et canon pirate (8.x), avion Ferret et tourelle montee (7.x). Aucun vehicule arme en 6.21
		// // OctopusVehicle
		// {
			// static UFortWeaponItemDefinition* OctopusWeaponItemDefinition = FindObjectFast<UFortWeaponItemDefinition>("/Game/Athena/Items/Weapons/Vehicles/WID_Octopus_Weapon.WID_Octopus_Weapon");
			// static UClass* OctopusVehicleClass = FindObjectFast<UClass>("/Game/Athena/DrivableVehicles/Octopus/OctopusVehicle.OctopusVehicle_C");

			// if (OctopusWeaponItemDefinition && OctopusVehicleClass)
				// VehicleWeapons.emplace(OctopusVehicleClass, FVehicleWeaponTahLesFous(OctopusWeaponItemDefinition, 0));
		// }

		// // FerretVehicle
		// {
			// static UFortWeaponItemDefinition* FerretWeaponItemDefinition = FindObjectFast<UFortWeaponItemDefinition>("/Game/Athena/Items/Weapons/Ferret_Weapon.Ferret_Weapon");
			// static UClass* FerretVehicleClass = FindObjectFast<UClass>("/Game/Athena/DrivableVehicles/Biplane/Blueprints/FerretVehicle.FerretVehicle_C");

			// if (FerretWeaponItemDefinition && FerretVehicleClass)
				// VehicleWeapons.emplace(FerretVehicleClass, FVehicleWeaponTahLesFous(FerretWeaponItemDefinition, 0));
		// }

		// // TurretVehicle
		// {
			// static UFortWeaponItemDefinition* TurretWeaponItemDefinition = FindObjectFast<UFortWeaponItemDefinition>("/Game/Athena/Items/Traps/MountedTurret/MountedTurret_Weapon.MountedTurret_Weapon");
			// static UClass* TurretVehicleClass = FindObjectFast<UClass>("/Game/Athena/DrivableVehicles/MountedTurret.MountedTurret_C");

			// if (TurretWeaponItemDefinition && TurretVehicleClass)
				// VehicleWeapons.emplace(TurretVehicleClass, FVehicleWeaponTahLesFous(TurretWeaponItemDefinition, 0));
		// }

		// // CannonVehicle
		// {
			// static UFortWeaponItemDefinition* InCannonWeaponItemDefinition = FindObjectFast<UFortWeaponItemDefinition>("/Game/Athena/Items/Weapons/Vehicles/ShipCannon_Weapon_InCannon.ShipCannon_Weapon_InCannon");
			// static UFortWeaponItemDefinition* CannonWeaponItemDefinition = FindObjectFast<UFortWeaponItemDefinition>("/Game/Athena/Items/Weapons/Vehicles/ShipCannon_Weapon.ShipCannon_Weapon");

			// static UClass* CannonVehicleClass = FindObjectFast<UClass>("/Game/Athena/DrivableVehicles/PushCannon.PushCannon_C");

			// if (CannonVehicleClass)
			// {
				// if (InCannonWeaponItemDefinition)
					// VehicleWeapons.emplace(CannonVehicleClass, FVehicleWeaponTahLesFous(InCannonWeaponItemDefinition, 1));

				// if (CannonWeaponItemDefinition)
					// VehicleWeapons.emplace(CannonVehicleClass, FVehicleWeaponTahLesFous(CannonWeaponItemDefinition, 0));
			// }
		// }

		return VehicleWeapons;
	}

	UFortWeaponItemDefinition* GetVehicleWeaponItemDefinition(UClass* VehicleClass, int32 SeatIndex)
	{
		if (!VehicleClass || SeatIndex == -1)
			return nullptr;

		std::multimap<UClass*, FVehicleWeaponTahLesFous> VehicleWeapons = GetVehicleWeapons();

		for (auto& [TestVehicleClass, VehicleWeaponTahLesFou] : VehicleWeapons)
		{
			if (!TestVehicleClass || !VehicleWeaponTahLesFou.VehicleWeapon)
				continue;

			// Check VehicleClass
			if (TestVehicleClass != VehicleClass)
				continue;

			// Check SeatIndex
			if (VehicleWeaponTahLesFou.SeatIndex != SeatIndex)
				continue;

			return VehicleWeaponTahLesFou.VehicleWeapon;
		}

		return nullptr;
	}

	UFortWorldItem* EquipVehicleWeapon(AFortPlayerControllerAthena* PlayerControllerAthena, UFortWeaponItemDefinition* VehicleWeaponItemDefinition)
	{
		if (!PlayerControllerAthena || !VehicleWeaponItemDefinition)
			return nullptr;

		FFortItemEntry ItemEntry = Inventory::CreateItemEntry(VehicleWeaponItemDefinition, 1, 0.f, -1, -1);
		UFortWorldItem* WorldItem = Inventory::AddItem(PlayerControllerAthena->WorldInventory, &ItemEntry);
		Inventory::UpdateInventory(PlayerControllerAthena->WorldInventory);

		if (AFortPlayerPawn* PlayerPawn = PlayerControllerAthena->MyFortPawn)
		{
			FGuid CurrentWeaponGuid = PlayerPawn->CurrentWeapon ? PlayerPawn->CurrentWeapon->ItemEntryGuid : FGuid();
			UFortWorldItem* WorldItem = Cast<UFortWorldItem>(PlayerControllerAthena->K2_GetInventoryItemWithGuid(CurrentWeaponGuid));

			if (WorldItem)
				PlayerControllerAthena->SwappingItemDefinition = Cast<UFortWorldItemDefinition>(WorldItem->ItemEntry.ItemDefinition);
		}

		VehicleWeaponItemDefinition->EquipWeaponDefinition(WorldItem, PlayerControllerAthena);

		return WorldItem;
	}

	void UnEquipVehicleWeapon(AFortPlayerControllerAthena* PlayerControllerAthena, UFortWeaponItemDefinition* VehicleWeaponItemDefinition)
	{
		if (!PlayerControllerAthena || !VehicleWeaponItemDefinition)
			return;

		// [8.51-ONLY] BP_FindItemInstancesFromDefinition n'existe pas en 6.21 : on parcourt directement l'inventaire
		// TArray<UFortItem*> ItemArray;
		// PlayerControllerAthena->BP_FindItemInstancesFromDefinition(VehicleWeaponItemDefinition, ItemArray);
		TArray<UFortWorldItem*> ItemArray;

		if (AFortInventory* WorldInventory = PlayerControllerAthena->WorldInventory)
		{
			for (int32 i = 0; i < WorldInventory->Inventory.ItemInstances.Num(); i++)
			{
				UFortWorldItem* ItemInstance = WorldInventory->Inventory.ItemInstances[i];

				if (ItemInstance && ItemInstance->ItemEntry.ItemDefinition == VehicleWeaponItemDefinition)
					ItemArray.Add(ItemInstance);
			}
		}

		for (int32 i = 0; i < ItemArray.Num(); i++)
		{
			UFortWorldItem* WorldItem = ItemArray[i];
			if (!WorldItem) continue;

			Inventory::RemoveInventoryItem(PlayerControllerAthena, WorldItem->ItemEntry.ItemGuid, WorldItem->ItemEntry.Count, true);
		}

		Inventory::UpdateInventory(PlayerControllerAthena->WorldInventory);

		if (UFortWeaponItemDefinition* SwappingItemDefinition = Cast<UFortWeaponItemDefinition>(PlayerControllerAthena->SwappingItemDefinition))
		{
			UFortWorldItem* WorldItem = Cast<UFortWorldItem>(PlayerControllerAthena->K2_FindExistingItemForDefinition(SwappingItemDefinition, false));
			if (!WorldItem) return;

			SwappingItemDefinition->EquipWeaponDefinition(WorldItem, PlayerControllerAthena);
			// [8.51-ONLY] ClientEquipItem n'existe pas en 6.21 -> ClientExecuteInventoryItem
			// PlayerControllerAthena->ClientEquipItem(WorldItem->ItemEntry.ItemGuid, true);
			PlayerControllerAthena->ClientExecuteInventoryItem(WorldItem->ItemEntry.ItemGuid, 0.0f, true, true);
		}
	}

	bool IsPlayerBuildableClasse(UClass* BuildableClasse)
	{
		AFortGameStateAthena* GameState = Globals::GetGameState();

		if (!BuildableClasse || !GameState)
			return false;

		TArray<TSubclassOf<ABuildingActor>> BuildingActorClasses = GameState->BuildingActorClasses;

		if (BuildingActorClasses.IsValid())
		{
			for (int32 i = 0; i < BuildingActorClasses.Num(); i++)
			{
				TSubclassOf<ABuildingActor> BuildingActorClasse = BuildingActorClasses[i];
				if (!BuildingActorClasse.Get()) continue;

				if (BuildingActorClasse.Get() == BuildableClasse)
					return true;
			}
		}

		return false;
	}

	bool IsOnSameTeam(AFortPlayerStateAthena* PlayerState, ABuildingSMActor* BuildingSMActor)
	{
		if (!PlayerState || !BuildingSMActor)
			return false;

		EFortTeam PlayerTeam = EFortTeam(PlayerState->TeamIndex);
		EFortTeam BuildingTeam = BuildingSMActor->Team;

		if (PlayerTeam == BuildingTeam)
			return true;

		if (BuildingTeam == EFortTeam::Spectator && !BuildingSMActor->bPlayerPlaced)
			return true;

		return false;
	}

	// 7FF66F3270B0
	void SetEditingPlayer(ABuildingSMActor* BuildingActor, AFortPlayerStateZone* EditingPlayer)
	{
		if (BuildingActor->HasAuthority() && (!BuildingActor->EditingPlayer || !EditingPlayer))
		{
			BuildingActor->SetNetDormancy(ENetDormancy(2 - (EditingPlayer != 0)));
			BuildingActor->ForceNetUpdate();
			BuildingActor->EditingPlayer = EditingPlayer;
		}
	}

	// 7FF66F986F80
	void SetEditingActor(AFortWeap_EditingTool* EditingTool, ABuildingSMActor* BuildingActor)
	{
		if (EditingTool->HasAuthority())
		{
			EditingTool->EditActor = BuildingActor;
			EditingTool->OnRep_EditActor();
		}
	}

	UClass* LoadClass(TSoftClassPtr<UClass> SoftClass)
	{
		UClass* Class = SoftClass.Get();

		if (!Class)
		{
			const FString& AssetPathName = UKismetStringLibrary::Conv_NameToString(SoftClass.ObjectID.AssetPathName);
			Class = StaticLoadObject<UClass>(AssetPathName.CStr());
		}

		return Class;
	}

	UFortGameplayAbility* LoadGameplayAbility(TSoftClassPtr<UClass> SoftGameplayAbility)
	{
		UClass* GameplayAbilityClass = SoftGameplayAbility.Get();

		if (!GameplayAbilityClass)
		{
			const FString& AssetPathName = UKismetStringLibrary::Conv_NameToString(SoftGameplayAbility.ObjectID.AssetPathName);
			GameplayAbilityClass = StaticLoadObject<UClass>(AssetPathName.CStr());
		}

		if (GameplayAbilityClass)
			return Cast<UFortGameplayAbility>(GameplayAbilityClass->ClassDefaultObject);

		return nullptr;
	}

	UFortAbilitySet* LoadAbilitySet(TSoftObjectPtr<UFortAbilitySet> SoftAbilitySet)
	{
		UFortAbilitySet* AbilitySet = SoftAbilitySet.Get();

		if (!AbilitySet)
		{
			const FString& AssetPathName = UKismetStringLibrary::Conv_NameToString(SoftAbilitySet.ObjectID.AssetPathName);
			AbilitySet = StaticLoadObject<UFortAbilitySet>(AssetPathName.CStr());
		}

		return AbilitySet;
	}

	UFortGameplayModifierItemDefinition* LoadGameplayModifier(TSoftObjectPtr<UFortGameplayModifierItemDefinition> SoftGameplayModifier)
	{
		UFortGameplayModifierItemDefinition* GameplayModifier = SoftGameplayModifier.Get();

		if (!GameplayModifier)
		{
			const FString& AssetPathName = UKismetStringLibrary::Conv_NameToString(SoftGameplayModifier.ObjectID.AssetPathName);
			GameplayModifier = StaticLoadObject<UFortGameplayModifierItemDefinition>(AssetPathName.CStr());
		}

		return GameplayModifier;
	}

	FORCEINLINE FVector GetSafeNormal2D(FVector Vector, float Tolerance = SMALL_NUMBER)
	{
		const float SquareSum = Vector.X * Vector.X + Vector.Y * Vector.Y;

		if (SquareSum == 1.f)
		{
			if (Vector.Z == 0.f)
			{
				return Vector;
			}
			else
			{
				return FVector(Vector.X, Vector.Y, 0.f);
			}
		}
		else if (SquareSum < Tolerance)
		{
			return FVector(0, 0, 0);
		}

		const float Scale = FMath::InvSqrt(SquareSum);
		return FVector(Vector.X * Scale, Vector.Y * Scale, 0.f);
	}

	void TossPickupFromContainer(ABuildingActor* BuildingActor, AFortPickup* Pickup, const int32 NumSteps, const int32 Step, const float LootTossConeHalfAngle, const FRotator& LootTossDirection, const float LootTossSpeed, bool bToss = true)
	{
		if (!BuildingActor || !Pickup)
			return;

		const FVector SpawnLocation = Pickup->K2_GetActorLocation();

		/*const int32 NumPickups = FMath::Clamp(NumSteps, 1, 10);
		const float AngleStep = (NumPickups > 1) ? (2.0f * LootTossConeHalfAngle / (NumPickups - 1)) : 0.0f;

		const bool bIsLeftSide = Step < NumPickups / 2;
		const float CurrentAngle = bIsLeftSide
			? (-LootTossConeHalfAngle + Step * AngleStep)
			: (-LootTossConeHalfAngle + (Step + 1) * AngleStep);

		const FRotator TossRotation = FRotator(5.0f, LootTossDirection.Yaw + CurrentAngle, 0.0f);
		const FVector TossDirection = UKismetMathLibrary::Conv_RotatorToVector(TossRotation);

		const float SpacingFactor = 30.0f + Step * 8.0f;
		const float ReboundEffect = 15.0f - Step * 3.0f;

		if (NumPickups == 1)
		{
			const float RandomOffset = 30.0f;
			SpawnLocation += FVector(FMath::RandRange(-RandomOffset, RandomOffset), FMath::RandRange(-RandomOffset, RandomOffset), 0);
		}

		FVector FinalLocation = SpawnLocation + (TossDirection * (LootTossSpeed + SpacingFactor)) + FVector(0, 0, ReboundEffect);*/

		// [8.51-ONLY] TossPickup prend 4 parametres en 6.21 (pas de source) : la source va dans PickupSourceTypeFlags
		// Pickup->TossPickup(SpawnLocation /*FinalLocation*/, nullptr, 0, bToss, EFortPickupSourceTypeFlag::Container, Inventory::GetPickupSpawnSource(BuildingActor));
		Pickup->PickupSourceTypeFlags |= (uint32)EFortPickupSourceTypeFlag::Container;
		Pickup->TossPickup(SpawnLocation /*FinalLocation*/, nullptr, 0, bToss);

		ABuildingContainer* BuildingContainer = Cast<ABuildingContainer>(BuildingActor);

		if (BuildingContainer)
		{
			Pickup->bTossedFromContainer = true;
			Pickup->OnRep_TossedFromContainer();
		}
	}

	int32 GetRandomRarity(const std::map<int32, float>& RarityAndWeight)
	{
		std::random_device rd;
		std::mt19937 gen(rd());

		std::vector<float> weights;

		for (const auto& kv : RarityAndWeight)
			weights.push_back(kv.second);

		std::discrete_distribution<> dist(weights.begin(), weights.end());

		return dist(gen);
	}

	void FillVendingMachine(ABuildingItemCollectorActor* ItemCollectorActor, std::map<int32, float> RarityAndWeight)
	{
		if (!ItemCollectorActor)
			return;

		int32 RandomRarity = GetRandomRarity(RarityAndWeight);

		if (RandomRarity == 0)
		{
			ItemCollectorActor->K2_DestroyActor();
			return;
		}

		// Get Real Rarity
		RandomRarity -= 1;

		// 0 = Common
		// 1 = UnCommon
		// 2 = Rare
		// 3 = Epic
		// 4 = Legendary

		TArray<FCollectorUnitInfo> ItemCollections = ItemCollectorActor->ItemCollections;

		for (int32 i = 0; i < ItemCollections.Num(); i++)
		{
			FCollectorUnitInfo& ItemCollection = ItemCollections[i];
			UFortWorldItemDefinition* InputItem = ItemCollection.InputItem;
			if (!InputItem) continue;

			int32 LootLevel = UFortKismetLibrary::GetLootLevel(ItemCollectorActor);

			TArray<FFortItemEntry> LootToDrops;
			bool bValidLoot = false;

			do
			{
				FName LootTierKey = FName(0);
				int32 LootTier = -1;

				bool bSuccess = Loots::PickLootTierKeyAndLootTierFromTierGroup(&LootTierKey, &LootTier, ItemCollectorActor->DefaultItemLootTierGroupName, LootLevel, 0, RandomRarity, ItemCollectorActor->StaticGameplayTags);
				if (!bSuccess) continue;

				Loots::PickLootDrops(&LootToDrops, LootLevel, LootTierKey, 0, 0, ItemCollectorActor->StaticGameplayTags, false, false);

				bValidLoot = true;
				if (LootToDrops.Num() > 0)
				{
					UFortWorldItemDefinition* LootToDropDefinition = Cast<UFortWorldItemDefinition>(LootToDrops[0].ItemDefinition);

					for (const FCollectorUnitInfo& OtherItemCollection : ItemCollections)
					{
						if (OtherItemCollection.OutputItem == LootToDropDefinition)
						{
							bValidLoot = false;
							break;
						}
					}
				}

			} while (!bValidLoot);

			if (ItemCollections[i].OutputItemEntry.IsValid())
				ItemCollections[i].OutputItemEntry.Free();

			for (auto& LootToDrop : LootToDrops)
				ItemCollections[i].OutputItemEntry.Add(LootToDrop);

			if (ItemCollections[i].OutputItemEntry.IsValidIndex(0))
				ItemCollections[i].OutputItem = Cast<UFortWorldItemDefinition>(ItemCollections[i].OutputItemEntry[0].ItemDefinition);
		}

		ItemCollectorActor->StartingGoalLevel = RandomRarity;
	}

	void FillVendingMachines()
	{
		AFortGameStateAthena* GameState = Cast<AFortGameStateAthena>(Globals::GetGameState());

		if (GameState && GameState->MapInfo)
		{
			AFortAthenaMapInfo* MapInfo = GameState->MapInfo;

			TArray<AActor*> VendingMachines;
			UGameplayStatics::GetAllActorsOfClass(MapInfo, MapInfo->VendingMachineClass, &VendingMachines);

			FScalableFloat VendingMachineRarityCount = MapInfo->VendingMachineRarityCount;

			std::map<int32, float> RarityAndWeight;

			// 0 = Destroy - RarityCount: 1.00, Value: 1.00
			// 1 = Common - RarityCount: 0.25, Value: 1.00
			// 2 = UnCommon - RarityCount: 0.75, Value: 1.00
			// 3 = Rare - RarityCount: 0.75, Value: 1.00
			// 4 = Epic - RarityCount: 0.70, Value: 1.00
			// 5 = Legendary - RarityCount: 0.30, Value: 1.00

			for (int32 i = 0; i < 6; i++)
			{
				float RarityCount = MapInfo->VendingMachineRarityCount.GetValueAtLevel(i);
				RarityAndWeight[i] = RarityCount;
			}

			for (int32 i = 0; i < VendingMachines.Num(); i++)
			{
				ABuildingItemCollectorActor* VendingMachine = Cast<ABuildingItemCollectorActor>(VendingMachines[i]);
				if (!VendingMachine) continue;

				FillVendingMachine(VendingMachine, RarityAndWeight);
			}
		}
	}

	ABuildingGameplayActorConsumable* SpawnGameplayActorConsumable(ABGAConsumableSpawner* ConsumableSpawner, UBGAConsumableWrapperItemDefinition* ConsumableWrapperItemDefinition)
	{
		if (!ConsumableSpawner || !ConsumableWrapperItemDefinition)
			return nullptr;

		UClass* ConsumableClass = ConsumableWrapperItemDefinition->ConsumableClass.Get();

		if (!ConsumableClass)
		{
			const FString& AssetPathName = UKismetStringLibrary::Conv_NameToString(ConsumableWrapperItemDefinition->ConsumableClass.ObjectID.AssetPathName);
			ConsumableClass = StaticLoadObject<UClass>(AssetPathName.CStr());
		}

		if (!ConsumableClass)
			return nullptr;

		FVector SpawnLocation = ConsumableSpawner->K2_GetActorLocation();

		UBlueprintGeneratedClass* BGA_RiftPortal_Athena_Spawner = FindObjectFast<UBlueprintGeneratedClass>("/Game/Athena/Items/ForagedItems/Rift/BGA_RiftPortal_Athena_Spawner.BGA_RiftPortal_Athena_Spawner_C");

		if (!BGA_RiftPortal_Athena_Spawner)
			BGA_RiftPortal_Athena_Spawner = StaticLoadObject<UBlueprintGeneratedClass>(L"/Game/Athena/Items/ForagedItems/Rift/BGA_RiftPortal_Athena_Spawner.BGA_RiftPortal_Athena_Spawner_C");

		if (ConsumableClass != BGA_RiftPortal_Athena_Spawner)
		{
			FVector RandomDirection = UKismetMathLibrary::RandomUnitVector();

			SpawnLocation.X += RandomDirection.X * 500.0f;
			SpawnLocation.Y += RandomDirection.Y * 500.0f;

			const FVector& End = SpawnLocation - FVector({ 0, 0, 1000 });

			FHitResult HitResult;
			bool bHit = UKismetSystemLibrary::LineTraceSingle(
				ConsumableSpawner,
				SpawnLocation,
				End,
				ETraceTypeQuery::TraceTypeQuery1,
				false,
				TArray<AActor*>(),
				EDrawDebugTrace::ForDuration,
				&HitResult,
				true,
				FLinearColor(),
				FLinearColor(),
				0.0f);

			if (bHit)
				SpawnLocation = HitResult.Location;
		}

		FRotator SpawnRotation = ConsumableSpawner->K2_GetActorRotation();
		SpawnRotation.Yaw = (float)rand() * 0.010986663f;

		FTransform SpawnTransform = UKismetMathLibrary::MakeTransform(SpawnLocation, SpawnRotation, FVector({ 1, 1, 1 }));

		ABuildingGameplayActorConsumable* GameplayActorConsumable = Util::SpawnActorTransfrom<ABuildingGameplayActorConsumable>(ConsumableClass, SpawnTransform);

		if (GameplayActorConsumable)
			UGameplayStatics::FinishSpawningActor(GameplayActorConsumable, SpawnTransform);

		return GameplayActorConsumable;
	}

	void InitializeTreasureChests()
	{
		AFortGameStateAthena* GameStateAthena = Cast<AFortGameStateAthena>(Globals::GetGameState());

		if (GameStateAthena && GameStateAthena->MapInfo)
		{
			AFortAthenaMapInfo* MapInfo = GameStateAthena->MapInfo;

			float TreasureChestMinSpawnPercent = MapInfo->TreasureChestMinSpawnPercent.GetValueAtLevel(0); // 50%
			float TreasureChestMaxSpawnPercent = MapInfo->TreasureChestMaxSpawnPercent.GetValueAtLevel(0); // 70%

			TArray<AActor*> TreasureChests;
			UGameplayStatics::GetAllActorsOfClass(MapInfo, MapInfo->TreasureChestClass, &TreasureChests);

			int32 TotalTreasureChests = TreasureChests.Num();
			int32 MinTreasureChestsToKeep = std::floor(TotalTreasureChests * (TreasureChestMinSpawnPercent / 100.0f));
			int32 MaxTreasureChestsToKeep = std::floor(TotalTreasureChests * (TreasureChestMaxSpawnPercent / 100.0f));

			int32 NumTreasureChestsToKeep = UKismetMathLibrary::RandomIntegerInRange(MinTreasureChestsToKeep, MaxTreasureChestsToKeep);

			for (int32 i = TotalTreasureChests - 1; i >= NumTreasureChestsToKeep; --i)
			{
				int32 IndexToDestroy = UKismetMathLibrary::RandomIntegerInRange(0, i);

				TreasureChests[IndexToDestroy]->K2_DestroyActor();
				TreasureChests.Remove(IndexToDestroy);
			}

			if (TreasureChests.Num() > 0)
				TreasureChests.Free();
		}
	}

	void InitializeAmmoBoxs()
	{
		AFortGameStateAthena* GameStateAthena = Cast<AFortGameStateAthena>(Globals::GetGameState());

		if (GameStateAthena && GameStateAthena->MapInfo)
		{
			AFortAthenaMapInfo* MapInfo = GameStateAthena->MapInfo;

			float AmmoBoxMinSpawnPercent = MapInfo->AmmoBoxMinSpawnPercent.GetValueAtLevel(0); // 65%
			float AmmoBoxMaxSpawnPercent = MapInfo->AmmoBoxMaxSpawnPercent.GetValueAtLevel(0); // 80%

			TArray<AActor*> AmmoBoxs;
			UGameplayStatics::GetAllActorsOfClass(MapInfo, MapInfo->AmmoBoxClass, &AmmoBoxs);

			int32 TotalAmmoBoxs = AmmoBoxs.Num();
			int32 MinAmmoBoxsToKeep = std::floor(TotalAmmoBoxs * (AmmoBoxMinSpawnPercent / 100.0f));
			int32 MaxAmmoBoxsToKeep = std::floor(TotalAmmoBoxs * (AmmoBoxMaxSpawnPercent / 100.0f));

			int32 NumAmmoBoxsToKeep = UKismetMathLibrary::RandomIntegerInRange(MinAmmoBoxsToKeep, MaxAmmoBoxsToKeep);

			for (int32 i = TotalAmmoBoxs - 1; i >= NumAmmoBoxsToKeep; --i)
			{
				int32 IndexToDestroy = UKismetMathLibrary::RandomIntegerInRange(0, i);

				AmmoBoxs[IndexToDestroy]->K2_DestroyActor();
				AmmoBoxs.Remove(IndexToDestroy);
			}

			if (AmmoBoxs.Num() > 0)
				AmmoBoxs.Free();
		}
	}

	void InitializeLlamas()
	{
		AFortGameStateAthena* GameStateAthena = Cast<AFortGameStateAthena>(Globals::GetGameState());
		AFortGameModeAthena* GameModeAthena = Cast<AFortGameModeAthena>(Globals::GetGameMode());

		if (GameStateAthena && GameModeAthena && GameStateAthena->MapInfo)
		{
			AFortAthenaMapInfo* MapInfo = GameStateAthena->MapInfo;

			float LlamaQuantityMin = MapInfo->LlamaQuantityMin.GetValueAtLevel(0); // 3
			float LlamaQuantityMax = MapInfo->LlamaQuantityMax.GetValueAtLevel(0); // 3

			int32 LlamaQuantityToSpawn = UKismetMathLibrary::RandomIntegerInRange(LlamaQuantityMin, LlamaQuantityMax);

			float SupplyDropMinPlacementHeight = MapInfo->SupplyDropMinPlacementHeight.GetValueAtLevel(0);
			float SupplyDropMaxPlacementHeight = MapInfo->SupplyDropMaxPlacementHeight.GetValueAtLevel(0);

			float SafeZoneStartingRadius = MapInfo->SafeZoneStartingRadius.GetValueAtLevel(0);

			FVector SafeZoneLocation = GameModeAthena->SafeZoneLocations[0];
			SafeZoneLocation.Z = 15000;

			for (int32 i = 0; i < LlamaQuantityToSpawn; i++)
			{
				FVector SpawnLocation = FVector(0, 0, 0);

				// [OFFSET-TODO-6.21] PickSupplyDropLocation (8.51 = 0xf4fe80)
				// FVector* (*PickSupplyDropLocation)(AFortAthenaMapInfo* AthenaMapInfo, FVector* OutLocation, FVector* CenterLocation, float Radius) = decltype(PickSupplyDropLocation)(0xf4fe80 + uintptr_t(GetModuleHandle(0)));
				auto PickSupplyDropLocation = Offsets621::Native<FVector*(AFortAthenaMapInfo* AthenaMapInfo, FVector* OutLocation, FVector* CenterLocation, float Radius)>(Offsets621::PickSupplyDropLocation, "PickSupplyDropLocation");
				PickSupplyDropLocation(MapInfo, &SpawnLocation, &SafeZoneLocation, SafeZoneStartingRadius);

				if (SpawnLocation.IsZero())
					continue;

				FRotator SpawnRotation = FRotator(0, 0, 0);
				SpawnRotation.Yaw = (float)rand() * 0.010986663f;

				// [OFFSET-TODO-6.21] SpawnSupplyDrop (8.51 = 0xf55880)
				// AFortAthenaSupplyDrop* (*SpawnSupplyDrop)(AFortAthenaMapInfo* AthenaMapInfo, FVector const* Location, FRotator const* Rotation, UClass* SupplyDropClass, float TraceStartZ, float TraceEndZ) = decltype(SpawnSupplyDrop)(0xf55880 + uintptr_t(GetModuleHandle(0)));
				auto SpawnSupplyDrop = Offsets621::Native<AFortAthenaSupplyDrop*(AFortAthenaMapInfo* AthenaMapInfo, FVector const* Location, FRotator const* Rotation, UClass* SupplyDropClass, float TraceStartZ, float TraceEndZ)>(Offsets621::SpawnSupplyDrop, "SpawnSupplyDrop");
				SpawnSupplyDrop(MapInfo, &SpawnLocation, &SpawnRotation, MapInfo->LlamaClass, SupplyDropMaxPlacementHeight, SupplyDropMinPlacementHeight);
			}
		}
	}

	void InitializeConsumableBGAs()
	{
		AFortGameStateAthena* GameStateAthena = Cast<AFortGameStateAthena>(Globals::GetGameState());

		if (GameStateAthena)
		{
			UBlueprintGeneratedClass* GameplayActorConsumableClass = StaticLoadObject<UBlueprintGeneratedClass>(L"/Game/Athena/BuildingActors/ConsumableBGAs/Spawner/BP_BGACSpawner.BP_BGACSpawner_C");

			TArray<AActor*> Actors;
			UGameplayStatics::GetAllActorsOfClass(GameStateAthena, GameplayActorConsumableClass, &Actors);

			for (int32 i = 0; i < Actors.Num(); i++)
			{
				ABGAConsumableSpawner* ConsumableSpawner = Cast<ABGAConsumableSpawner>(Actors[i]);
				if (!ConsumableSpawner) continue;

				int32 LootLevel = UFortKismetLibrary::GetLootLevel(GameStateAthena);

				TArray<FFortItemEntry> LootToDrops;
				UFortKismetLibrary::PickLootDrops(GameStateAthena, &LootToDrops, ConsumableSpawner->SpawnLootTierGroup, LootLevel, -1);

				FVector SpawnerLocation = ConsumableSpawner->K2_GetActorLocation();

				for (int32 j = 0; j < LootToDrops.Num(); j++)
				{
					FFortItemEntry LootToDrop = LootToDrops[j];

					UBGAConsumableWrapperItemDefinition* ConsumableWrapperItemDefinition = Cast<UBGAConsumableWrapperItemDefinition>(LootToDrop.ItemDefinition);
					if (!ConsumableWrapperItemDefinition) continue;

					ABuildingGameplayActorConsumable* GameplayActorConsumable = SpawnGameplayActorConsumable(ConsumableSpawner, ConsumableWrapperItemDefinition);

					FVector ConsumableLocation = GameplayActorConsumable->K2_GetActorLocation();

					FN_LOG(LogFunctions, Debug, "[%i/%i] - GameplayActorConsumable: %s, ConsumableLocation: [X: %.2f, Y: %.2f, Z: %.2f]",
						(j + 1), LootToDrops.Num(), GameplayActorConsumable->GetFullName().c_str(), ConsumableLocation.X, ConsumableLocation.Y, ConsumableLocation.Z);

					ConsumableSpawner->ConsumablesToSpawn.Add(LootToDrop);
				}
			}
		}
	}

	bool HasGameplayTags(TArray<FGameplayTag> GameplayTags, FName TagNameRequired)
	{
		const FString& TagNameRequiredString = UKismetStringLibrary::Conv_NameToString(TagNameRequired);
		const FString& TagNameRequiredLower = UKismetStringLibrary::ToLower(TagNameRequiredString);

		for (int32 j = 0; j < GameplayTags.Num(); j++)
		{
			FGameplayTag GameplayTag = GameplayTags[j];

			FName TagName = GameplayTag.TagName;
			if (!TagName.IsValid()) continue;

			const FString& TagNameString = UKismetStringLibrary::Conv_NameToString(TagName);
			const FString& TagNameLower = UKismetStringLibrary::ToLower(TagNameString);

			if (TagNameLower == TagNameRequiredLower)
				return true;
		}

		return false;
	}

	TArray<UFortItemDefinition*> PickOnlyAthena(TArray<UFortItemDefinition*> AllItems)
	{
		static TArray<UFortItemDefinition*> AllAthenaItems;

		if (AllAthenaItems.Num() > 0)
			return AllAthenaItems;

		for (int32 i = 0; i < AllItems.Num(); i++)
		{
			UFortItemDefinition* ItemDefinition = AllItems[i];

			if (!ItemDefinition)
				continue;

			const FString& PathName = UKismetSystemLibrary::GetPathName(ItemDefinition);

			if (!PathName.ToString().contains("/Game/Athena/Items"))
				continue;

			AllAthenaItems.Add(ItemDefinition);
		}

		return AllAthenaItems;
	}

	TArray<UFortItemDefinition*> GetAllItems(bool bOnlyAthena = false)
	{
		static TArray<UFortItemDefinition*> AllItems;

		if (AllItems.Num() > 0)
			return bOnlyAthena ? PickOnlyAthena(AllItems) : AllItems;

		for (int32 i = 0; i < UObject::GObjects->Num(); i++)
		{
			UObject* GObject = UObject::GObjects->GetByIndex(i);

			if (!GObject)
				continue;

			if (GObject->IsA(UFortItemDefinition::StaticClass()))
			{
				UFortItemDefinition* ItemDefinition = Cast<UFortItemDefinition>(GObject);
				if (!ItemDefinition) continue;

				AllItems.Add(ItemDefinition);
			}
		}

		return bOnlyAthena ? PickOnlyAthena(AllItems) : AllItems;
	}

	TArray<UFortWorldItemDefinition*> GetAllItemsWithClass(bool bOnlyAthena, UClass* Class)
	{
		TArray<UFortItemDefinition*> AllItems = GetAllItems(bOnlyAthena);
		TArray<UFortWorldItemDefinition*> AllItemsWithClass;

		for (int32 i = 0; i < AllItems.Num(); i++)
		{
			UFortItemDefinition* ItemDefinition = AllItems[i];

			if (!ItemDefinition)
				continue;

			if (ItemDefinition->IsA(Class))
			{
				UFortWorldItemDefinition* WorldItemDefinition = Cast<UFortWorldItemDefinition>(ItemDefinition);
				if (!WorldItemDefinition) continue;

				AllItemsWithClass.Add(WorldItemDefinition);
			}
		}

		return AllItemsWithClass;
	}

	TArray<UFortWeaponItemDefinition*> GetAllItemsWithTag(const FName& TagNameRequired, TArray<UFortWorldItemDefinition*> BlackListWorldItemDefinitions, TArray<EFortRarity> BlackListRaritys, TArray<EFortItemType> BlackListItemTypes)
	{
		TArray<UFortItemDefinition*> AllItems = GetAllItems(true);
		TArray<UFortWeaponItemDefinition*> AllItemsWithTag;

		if (!TagNameRequired.IsValid())
			return AllItemsWithTag;

		for (int32 i = 0; i < AllItems.Num(); i++)
		{
			UFortWeaponItemDefinition* WeaponRangedItemDefinition = Cast<UFortWeaponItemDefinition>(AllItems[i]);
			if (!WeaponRangedItemDefinition) continue;

			bool bBlacklistItem = false;
			for (int32 j = 0; j < BlackListWorldItemDefinitions.Num(); j++)
			{
				UFortWorldItemDefinition* BlackListWorldItemDefinition = BlackListWorldItemDefinitions[j];
				if (!BlackListWorldItemDefinition) continue;

				if (BlackListWorldItemDefinition == WeaponRangedItemDefinition)
				{
					bBlacklistItem = true;
					break;
				}
			}

			if (bBlacklistItem)
				continue;

			bool bBlackListRariry = false;
			for (int32 j = 0; j < BlackListRaritys.Num(); j++)
			{
				EFortRarity BlackListRarity = BlackListRaritys[j];

				if (BlackListRarity == WeaponRangedItemDefinition->Rarity)
				{
					bBlackListRariry = true;
					break;
				}
			}

			if (bBlackListRariry)
				continue;

			bool bBlackListItemType = false;
			for (int32 j = 0; j < BlackListItemTypes.Num(); j++)
			{
				EFortItemType BlackListItemType = BlackListItemTypes[j];

				if (BlackListItemType == WeaponRangedItemDefinition->ItemType)
				{
					bBlackListItemType = true;
					break;
				}
			}

			if (bBlackListItemType)
				continue;

			TArray<FGameplayTag> GameplayTags = WeaponRangedItemDefinition->GameplayTags.GameplayTags;

			const FString& TagNameRequiredString = UKismetStringLibrary::Conv_NameToString(TagNameRequired);
			const FString& TagNameRequiredLower = UKismetStringLibrary::ToLower(TagNameRequiredString);

			bool bFindTagName = false;
			for (int32 j = 0; j < GameplayTags.Num(); j++)
			{
				FGameplayTag GameplayTag = GameplayTags[j];

				FName TagName = GameplayTag.TagName;
				if (!TagName.IsValid()) continue;

				const FString& TagNameString = UKismetStringLibrary::Conv_NameToString(TagName);
				const FString& TagNameLower = UKismetStringLibrary::ToLower(TagNameString);

				if (TagNameLower == TagNameRequiredLower)
				{
					bFindTagName = true;
					break;
				}
			}

			if (!bFindTagName)
				continue;

			AllItemsWithTag.Add(WeaponRangedItemDefinition);
		}

		return AllItemsWithTag;
	}

	UFortWeaponItemDefinition* ChooseRandomItemWithFilter(const FName& TagNameRequired, TArray<UFortWorldItemDefinition*> BlackListWorldItemDefinitions = {}, TArray<EFortRarity> BlackListRarity = {}, TArray<EFortItemType> BlackListItemTypes = {})
	{
		TArray<UFortWeaponItemDefinition*> AllItemsWithTag = GetAllItemsWithTag(TagNameRequired, BlackListWorldItemDefinitions, BlackListRarity, BlackListItemTypes);

		if (AllItemsWithTag.Num() == 0)
			return nullptr;

		return AllItemsWithTag[std::rand() % AllItemsWithTag.Num()];
	}

	void ApplySiphonEffect(AFortPlayerState* PlayerState)
	{
		if (PlayerState)
		{
			UFortAbilitySystemComponent* AbilitySystemComponent = PlayerState->AbilitySystemComponent;

			if (AbilitySystemComponent)
			{
				FGameplayTag GameplayTag = FGameplayTag();
				GameplayTag.TagName = UKismetStringLibrary::Conv_StringToName(L"GameplayCue.Shield.PotionConsumed");

				AbilitySystemComponent->NetMulticast_InvokeGameplayCueAdded(GameplayTag, FPredictionKey(), FGameplayEffectContextHandle());
				AbilitySystemComponent->NetMulticast_InvokeGameplayCueExecuted(GameplayTag, FPredictionKey(), FGameplayEffectContextHandle());
			}
		}
	}

	bool IsSiphonEnable()
	{
		UCurveTable* AthenaGameData = Globals::GetAthenaGameData();
		if (!AthenaGameData) return false;

		const FName RowName = UKismetStringLibrary::Conv_StringToName(L"Default.Siphon.Enabled");

		float XY;
		EEvaluateCurveTableResult CurveTableResult;
		UDataTableFunctionLibrary::EvaluateCurveTableRow(AthenaGameData, RowName, 0, &CurveTableResult, &XY, L"Function::IsSiphonEnable");

		return bool(XY);
	}

	float GetHealAmount()
	{
		UCurveTable* AthenaGameData = Globals::GetAthenaGameData();
		if (!AthenaGameData) return 0.0f;

		const FName RowName = UKismetStringLibrary::Conv_StringToName(L"Default.Siphon.InitialHealAmount");

		float XY;
		EEvaluateCurveTableResult CurveTableResult;
		UDataTableFunctionLibrary::EvaluateCurveTableRow(AthenaGameData, RowName, 0, &CurveTableResult, &XY, L"Function::GetHealAmount");

		return XY;
	}

	float GetDropWoodAmount()
	{
		UCurveTable* AthenaGameData = Globals::GetAthenaGameData();
		if (!AthenaGameData) return 0.0f;

		const FName RowName = UKismetStringLibrary::Conv_StringToName(L"Default.Siphon.DropWoodOnKill");

		float XY;
		EEvaluateCurveTableResult CurveTableResult;
		UDataTableFunctionLibrary::EvaluateCurveTableRow(AthenaGameData, RowName, 0, &CurveTableResult, &XY, L"Function::GetDropWoodAmount");

		return XY;
	}

	float GetDropStoneAmount()
	{
		UCurveTable* AthenaGameData = Globals::GetAthenaGameData();
		if (!AthenaGameData) return 0.0f;

		const FName RowName = UKismetStringLibrary::Conv_StringToName(L"Default.Siphon.DropStoneOnKill");

		float XY;
		EEvaluateCurveTableResult CurveTableResult;
		UDataTableFunctionLibrary::EvaluateCurveTableRow(AthenaGameData, RowName, 0, &CurveTableResult, &XY, L"Function::GetDropStoneAmount");

		return XY;
	}

	float GetDropMetalAmount()
	{
		UCurveTable* AthenaGameData = Globals::GetAthenaGameData();
		if (!AthenaGameData) return 0.0f;

		const FName RowName = UKismetStringLibrary::Conv_StringToName(L"Default.Siphon.DropMetalOnKill");

		float XY;
		EEvaluateCurveTableResult CurveTableResult;
		UDataTableFunctionLibrary::EvaluateCurveTableRow(AthenaGameData, RowName, 0, &CurveTableResult, &XY, L"Function::GetDropMetalAmount");

		return XY;
	}

	void GiveSiphonBonus(AFortPlayerController* PlayerController, AFortPlayerPawn* PlayerPawn, const FVector& DeathLocation)
	{
		if (PlayerController)
		{
			if (IsSiphonEnable() && PlayerPawn)
			{
				float MaxHealth = PlayerPawn->GetMaxHealth();
				float MaxShield = PlayerPawn->GetMaxShield();

				float Health = PlayerPawn->GetHealth();
				float Shield = PlayerPawn->GetShield();

				float SiphonAmount = GetHealAmount();
				float RemainingSiphonAmount = SiphonAmount;

				if (Health < MaxHealth)
				{
					float NewHealth = std::clamp(Health + SiphonAmount, 0.0f, MaxHealth);

					PlayerPawn->SetHealth(NewHealth);

					RemainingSiphonAmount -= (NewHealth - Health);
				}

				if (RemainingSiphonAmount > 0.0f)
				{
					float NewShield = std::clamp(Shield + RemainingSiphonAmount, 0.0f, MaxShield);

					PlayerPawn->SetShield(NewShield);
				}

				if (!DeathLocation.IsZero())
				{
					UFortResourceItemDefinition* WoodResourceItemDefintion = UFortKismetLibrary::K2_GetResourceItemDefinition(EFortResourceType::Wood);
					UFortResourceItemDefinition* StoneResourceItemDefintion = UFortKismetLibrary::K2_GetResourceItemDefinition(EFortResourceType::Stone);
					UFortResourceItemDefinition* MetalResourceItemDefintion = UFortKismetLibrary::K2_GetResourceItemDefinition(EFortResourceType::Metal);

					UFortKismetLibrary::K2_SpawnPickupInWorld(PlayerController, WoodResourceItemDefintion, GetDropWoodAmount(), DeathLocation, DeathLocation, 0, true, true, false);
					UFortKismetLibrary::K2_SpawnPickupInWorld(PlayerController, StoneResourceItemDefintion, GetDropStoneAmount(), DeathLocation, DeathLocation, 0, true, true, false);
					UFortKismetLibrary::K2_SpawnPickupInWorld(PlayerController, MetalResourceItemDefintion, GetDropMetalAmount(), DeathLocation, DeathLocation, 0, true, true, false);
				}
			}

			AFortPlayerState* PlayerState = Cast<AFortPlayerState>(PlayerController->PlayerState);

			if (PlayerState)
			{
				ApplySiphonEffect(PlayerState);
			}
		}
	}

	void GiveLateGameInventory(AFortPlayerController* PlayerController)
	{
		UFortKismetLibrary::K2_GiveBuildingResource(PlayerController, EFortResourceType::Wood, 500);
		UFortKismetLibrary::K2_GiveBuildingResource(PlayerController, EFortResourceType::Stone, 300);
		UFortKismetLibrary::K2_GiveBuildingResource(PlayerController, EFortResourceType::Metal, 150);

		// Give All Ammos
		{
			TArray<UFortWorldItemDefinition*> AllAmmoItems = Functions::GetAllItemsWithClass(true, UFortAmmoItemDefinition::StaticClass());

			for (int32 i = 0; i < AllAmmoItems.Num(); i++)
			{
				UFortAmmoItemDefinition* AmmoItemDefinition = Cast<UFortAmmoItemDefinition>(AllAmmoItems[i]);
				if (!AmmoItemDefinition) continue;

				if (!AmmoItemDefinition->bCanBeDropped)
					continue;

				UFortKismetLibrary::K2_GiveItemToPlayer(PlayerController, AmmoItemDefinition, ((AmmoItemDefinition->MaxStackSize / 4) + 1), false);
			}
		}

		// Assault Weapon
		{
			const FName& TagNameRequired = UKismetStringLibrary::Conv_StringToName(L"Weapon.Ranged.Assault.Standard");

			TArray<EFortRarity> BlackListRarity;
			BlackListRarity.Add(EFortRarity::Handmade);
			BlackListRarity.Add(EFortRarity::Ordinary);

			UFortWeaponItemDefinition* RandomItemWithTag = Functions::ChooseRandomItemWithFilter(TagNameRequired, {}, BlackListRarity);

			if (RandomItemWithTag)
				UFortKismetLibrary::K2_GiveItemToPlayer(PlayerController, RandomItemWithTag, RandomItemWithTag->MaxStackSize, false);

			if (BlackListRarity.IsValid())
				BlackListRarity.Free();
		}

		// Shotgun Weapon
		{
			const FName& TagNameRequired = UKismetStringLibrary::Conv_StringToName(L"Weapon.Ranged.Shotgun.Pump");

			TArray<EFortRarity> BlackListRarity;
			BlackListRarity.Add(EFortRarity::Handmade);
			BlackListRarity.Add(EFortRarity::Ordinary);

			UFortWeaponItemDefinition* RandomItemWithTag = Functions::ChooseRandomItemWithFilter(TagNameRequired, {}, BlackListRarity);

			if (RandomItemWithTag)
				UFortKismetLibrary::K2_GiveItemToPlayer(PlayerController, RandomItemWithTag, RandomItemWithTag->MaxStackSize, false);

			if (BlackListRarity.IsValid())
				BlackListRarity.Free();
		}

		// Smg Weapon
		{
			const FName& TagNameRequired = UKismetStringLibrary::Conv_StringToName(L"Weapon.Ranged.Smg");

			TArray<EFortRarity> BlackListRarity;
			BlackListRarity.Add(EFortRarity::Handmade);
			BlackListRarity.Add(EFortRarity::Ordinary);

			UFortWeaponItemDefinition* RandomItemWithTag = Functions::ChooseRandomItemWithFilter(TagNameRequired, {}, BlackListRarity);

			if (RandomItemWithTag)
				UFortKismetLibrary::K2_GiveItemToPlayer(PlayerController, RandomItemWithTag, RandomItemWithTag->MaxStackSize, false);

			if (BlackListRarity.IsValid())
				BlackListRarity.Free();
		}

		// Heal Weapon
		{
			const FName& TagNameRequired = UKismetStringLibrary::Conv_StringToName(L"Trait.Restoration.Shields");

			TArray<EFortRarity> BlackListRarity;
			BlackListRarity.Add(EFortRarity::Quality);
			BlackListRarity.Add(EFortRarity::Fine);

			UFortWeaponItemDefinition* RandomItemWithTag = Functions::ChooseRandomItemWithFilter(TagNameRequired, {}, BlackListRarity);

			if (RandomItemWithTag)
				UFortKismetLibrary::K2_GiveItemToPlayer(PlayerController, RandomItemWithTag, RandomItemWithTag->MaxStackSize, false);

			if (BlackListRarity.IsValid())
				BlackListRarity.Free();
		}

		// Rota/Sniper Weapon
		{
			const FName& TagNameRequired = UKismetStringLibrary::Conv_StringToName(L"Bacchus.Shoot.Sniper.Bolt"); // UKismetMathLibrary::RandomBool() ? UKismetStringLibrary::Conv_StringToName(L"Weapon.Ranged.Sniper") : UKismetStringLibrary::Conv_StringToName(L"Weapon.Ranged.Heavy.Grenade");
			UFortWeaponItemDefinition* RandomItemWithTag = Functions::ChooseRandomItemWithFilter(TagNameRequired);

			if (RandomItemWithTag)
				UFortKismetLibrary::K2_GiveItemToPlayer(PlayerController, RandomItemWithTag, RandomItemWithTag->MaxStackSize, false);
		}

		// Trap
		{
			const FName& TagNameRequired = UKismetStringLibrary::Conv_StringToName(L"Bacchus.Place.Trap");
			UFortWeaponItemDefinition* RandomItemWithTag = Functions::ChooseRandomItemWithFilter(TagNameRequired);

			if (RandomItemWithTag)
				UFortKismetLibrary::K2_GiveItemToPlayer(PlayerController, RandomItemWithTag, 3, false);
		}
	}

	void GiveOneShotInventory(AFortPlayerController* PlayerController)
	{
		UFortKismetLibrary::K2_GiveBuildingResource(PlayerController, EFortResourceType::Wood, 150);
		UFortKismetLibrary::K2_GiveBuildingResource(PlayerController, EFortResourceType::Stone, 100);
		UFortKismetLibrary::K2_GiveBuildingResource(PlayerController, EFortResourceType::Metal, 50);

		// Give All Ammos
		{
			TArray<UFortWorldItemDefinition*> AllAmmoItems = Functions::GetAllItemsWithClass(true, UFortAmmoItemDefinition::StaticClass());

			for (int32 i = 0; i < AllAmmoItems.Num(); i++)
			{
				UFortAmmoItemDefinition* AmmoItemDefinition = Cast<UFortAmmoItemDefinition>(AllAmmoItems[i]);
				if (!AmmoItemDefinition) continue;

				if (!AmmoItemDefinition->bCanBeDropped)
					continue;

				UFortKismetLibrary::K2_GiveItemToPlayer(PlayerController, AmmoItemDefinition, ((AmmoItemDefinition->MaxStackSize / 4) + 1), false);
			}
		}

		// Give Random Sniper
		for (int32 i = 0; i < 3; i++)
		{
			const FName& TagNameRequired = UKismetStringLibrary::Conv_StringToName(L"Bacchus.Shoot.Sniper.Bolt");
			UFortWeaponItemDefinition* RandomItemWithTag = Functions::ChooseRandomItemWithFilter(TagNameRequired);

			if (RandomItemWithTag)
				UFortKismetLibrary::K2_GiveItemToPlayer(PlayerController, RandomItemWithTag, RandomItemWithTag->MaxStackSize, false);
		}

		// Heal Weapon
		{
			const FName& TagNameRequired = UKismetStringLibrary::Conv_StringToName(L"Trait.Restoration.Health");

			TArray<EFortRarity> BlackListRarity;
			BlackListRarity.Add(EFortRarity::Quality);
			BlackListRarity.Add(EFortRarity::Sturdy);

			UFortWeaponItemDefinition* RandomItemWithTag = Functions::ChooseRandomItemWithFilter(TagNameRequired, {}, BlackListRarity);

			if (RandomItemWithTag)
				UFortKismetLibrary::K2_GiveItemToPlayer(PlayerController, RandomItemWithTag, RandomItemWithTag->MaxStackSize, false);
		}

		// Others
		{
			static UFortWeaponRangedItemDefinition* ShockGrenadeItemDefinition = FindObjectFast<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Consumables/ShockwaveGrenade/Athena_ShockGrenade.Athena_ShockGrenade");
			static UFortWeaponRangedItemDefinition* KnockGrenadeItemDefinition = FindObjectFast<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Consumables/KnockGrenade/Athena_KnockGrenade.Athena_KnockGrenade");
			static UFortWeaponRangedItemDefinition* HookGunItemDefinition = FindObjectFast<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Weapons/WID_Hook_Gun_VR_Ore_T03.WID_Hook_Gun_VR_Ore_T03");

			if (ShockGrenadeItemDefinition && KnockGrenadeItemDefinition && HookGunItemDefinition)
			{
				UFortWeaponRangedItemDefinition* WeaponRangedItemDefinition = nullptr;

				switch (UKismetMathLibrary::RandomIntegerInRange(1, 3)) // RandomInteger64InRange n'existe pas en 6.21
				{
				case 1:
					WeaponRangedItemDefinition = ShockGrenadeItemDefinition;
					break;
				case 2:
					WeaponRangedItemDefinition = KnockGrenadeItemDefinition;
					break;
				case 3:
					WeaponRangedItemDefinition = HookGunItemDefinition;
					break;
				default:
					break;
				}

				if (WeaponRangedItemDefinition)
					UFortKismetLibrary::K2_GiveItemToPlayer(PlayerController, WeaponRangedItemDefinition, WeaponRangedItemDefinition->MaxStackSize, false);
			}
		}

		// Trap
		{
			const FName& TagNameRequired = UKismetStringLibrary::Conv_StringToName(L"Bacchus.Place.Trap.Bouncer");
			UFortWeaponItemDefinition* RandomItemWithTag = Functions::ChooseRandomItemWithFilter(TagNameRequired);

			if (RandomItemWithTag)
				UFortKismetLibrary::K2_GiveItemToPlayer(PlayerController, RandomItemWithTag, 10, false);
		}
	}

	void GiveRocketInventory(AFortPlayerController* PlayerController)
	{
		UFortKismetLibrary::K2_GiveBuildingResource(PlayerController, EFortResourceType::Wood, 300);
		UFortKismetLibrary::K2_GiveBuildingResource(PlayerController, EFortResourceType::Stone, 250);
		UFortKismetLibrary::K2_GiveBuildingResource(PlayerController, EFortResourceType::Metal, 200);

		// Give All Ammos
		{
			TArray<UFortWorldItemDefinition*> AllAmmoItems = Functions::GetAllItemsWithClass(true, UFortAmmoItemDefinition::StaticClass());

			for (int32 i = 0; i < AllAmmoItems.Num(); i++)
			{
				UFortAmmoItemDefinition* AmmoItemDefinition = Cast<UFortAmmoItemDefinition>(AllAmmoItems[i]);
				if (!AmmoItemDefinition) continue;

				if (!AmmoItemDefinition->bCanBeDropped)
					continue;

				UFortKismetLibrary::K2_GiveItemToPlayer(PlayerController, AmmoItemDefinition, ((AmmoItemDefinition->MaxStackSize / 4) + 1), true);
			}
		}

		// Give Random Rocket
		for (int i = 0; i < 3; i++)
		{
			const FName& TagNameRequired = UKismetStringLibrary::Conv_StringToName(L"Weapon.Ranged.Heavy");
			UFortWeaponItemDefinition* RandomItemWithTag = Functions::ChooseRandomItemWithFilter(TagNameRequired);

			if (RandomItemWithTag)
				UFortKismetLibrary::K2_GiveItemToPlayer(PlayerController, RandomItemWithTag, RandomItemWithTag->MaxStackSize, false);
		}

		// Shockwave
		{
			const FName& TagNameRequired = UKismetStringLibrary::Conv_StringToName(L"Weapon.Ranged.Heavy.Grenade");
			UFortWeaponItemDefinition* RandomItemWithTag = Functions::ChooseRandomItemWithFilter(TagNameRequired);

			if (RandomItemWithTag)
				UFortKismetLibrary::K2_GiveItemToPlayer(PlayerController, RandomItemWithTag, RandomItemWithTag->MaxStackSize, false);
		}

		// Trap
		{
			const FName& TagNameRequired = UKismetStringLibrary::Conv_StringToName(L"Bacchus.Place.Trap.Bouncer");
			UFortWeaponItemDefinition* RandomItemWithTag = Functions::ChooseRandomItemWithFilter(TagNameRequired);

			if (RandomItemWithTag)
				UFortKismetLibrary::K2_GiveItemToPlayer(PlayerController, RandomItemWithTag, 10, false);
		}

		// Heal Weapon
		{
			const FName& TagNameRequired = UKismetStringLibrary::Conv_StringToName(L"Trait.Restoration.Shields");

			TArray<EFortRarity> BlackListRarity;
			BlackListRarity.Add(EFortRarity::Quality);
			BlackListRarity.Add(EFortRarity::Fine);

			UFortWeaponItemDefinition* RandomItemWithTag = Functions::ChooseRandomItemWithFilter(TagNameRequired, {}, BlackListRarity);

			if (RandomItemWithTag)
				UFortKismetLibrary::K2_GiveItemToPlayer(PlayerController, RandomItemWithTag, RandomItemWithTag->MaxStackSize, false);

			if (BlackListRarity.IsValid())
				BlackListRarity.Free();
		}
	}

	std::wstring GetRandomBotName()
	{
		std::vector<std::wstring> botNames = {
			L"Alpha",
			L"Bravo",
			L"Charlie",
			L"Delta",
			L"Echo",
			L"Foxtrot",
			L"Golf",
			L"Hotel",
			L"India",
			L"Jamy",
			L"Zepp",
			L"Lucero",
			L"Amore",
			L"Amour",
			L"Optimus",
			L"Trevor",
			L"Ulima",
			L"Thaddeus",
			L"Ravyn",
			L"Ravyn",
			L"Cody",
			L"Rashad",
			L"Dodie",
			L"Bonamy",
			L"Amicia",
			L"Carine",
			L"Solada",
			L"Dakota",
			L"Rona",
			L"Rinc",
			L"Kelsy",
			L"Faven",
			L"Bayo",
			L"Eagor",
			L"Ryn",
			L"Lyka",
			L"Gedith",
			L"Kaida",
			L"Hydra",
			L"Pendragon",
			L"Ariel",
			L"Estel",
			L"Winona",
			L"Nicole",
			L"Walter",
			L"Neil",
			L"Derek",
			L"Taylor",
			L"Raymond",
			L"Lincoln"
		};


		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<> dis(0, botNames.size() - 1);

		return L"[GalaxiaFN - BOT] " + botNames[dis(gen)];
	}
}