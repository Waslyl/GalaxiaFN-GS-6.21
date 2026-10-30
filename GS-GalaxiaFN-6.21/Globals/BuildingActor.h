#pragma once

namespace BuildingActor
{
	void (*ABuildingContainer_PostUpdateOG)(ABuildingContainer* BuildingContainer, EFortBuildingPersistentState BuildingPersistentState, __int64 a3);
	void (*ABuildingSMActor_PostUpdateOG)(ABuildingSMActor* BuildingSMActor);

	void SelectMeshSet(ABuildingSMActor* BuildingSMActor, FName LootTierKey)
	{
		int32 LootTier = Loots::FindLootTierFromKey(LootTierKey);
		FTierMeshSets* MeshSets = nullptr;

		if (BuildingSMActor->AlternateMeshes.Num() > 0)
		{
			for (int32 i = 0; i < BuildingSMActor->AlternateMeshes.Num(); i++)
			{
				FTierMeshSets* AlternateMeshe = &BuildingSMActor->AlternateMeshes[i];
				int32 Tier = AlternateMeshe->Tier;

				if (Tier <= LootTier && (!MeshSets || MeshSets->Tier < Tier))
				{
					if (AlternateMeshe->MeshSets.Num() <= 0)
					{
						FN_LOG(LogBuildingSMActor, Warning, "No Alternative Mesh available for Building: %s at LootTier: %d", BuildingSMActor->GetName().c_str(), Tier);
					}
					else
					{
						MeshSets = AlternateMeshe;
					}
				}
			}

			if (MeshSets)
			{
				int32 AltMeshIdx = BuildingSMActor->AltMeshIdx;

				if (AltMeshIdx != -1)
				{
					BuildingSMActor->SetMeshSet(MeshSets->MeshSets[AltMeshIdx]);
				}
			}
		}

		FN_LOG(LogBuildingSMActor, Debug, "ABuildingSMActor::SelectMeshSet(FName LootTierKey) Building: %s, AltMeshIdx: %d", BuildingSMActor->GetName().c_str(), BuildingSMActor->AltMeshIdx);
	}

	bool SpawnLootHook(ABuildingContainer* BuildingContainer, AFortPlayerPawn* PlayerPawn)
	{
		if (!BuildingContainer->HasAuthority())
			return false;

		BuildingContainer->ForceNetUpdate();

		if (!BuildingContainer->bAlreadySearched)
		{
			const float LootSpawnLocationX = BuildingContainer->LootSpawnLocation_Athena.X;
			const float LootSpawnLocationY = BuildingContainer->LootSpawnLocation_Athena.Y;
			const float LootSpawnLocationZ = BuildingContainer->LootSpawnLocation_Athena.Z;

			const FVector& SpawnLocation = BuildingContainer->K2_GetActorLocation() +
				BuildingContainer->GetActorForwardVector() * LootSpawnLocationX +
				BuildingContainer->GetActorRightVector() * LootSpawnLocationY +
				BuildingContainer->GetActorUpVector() * LootSpawnLocationZ;

			UFortWorldItemDefinition* LootTestingData = BuildingContainer->LootTestingData;
			int32 LootLevel = UFortKismetLibrary::GetLootLevel(BuildingContainer);

			// [GALAXIA PATCH] 6.21 : coffre passe par PostUpdate avant le choix de la playlist : le loot est choisi a l'ouverture
			if (!BuildingContainer->ContainerLootTierKey.IsValid() && BuildingContainer->SearchLootTierGroup.IsValid())
			{
				auto PickLootTierGroupAthena = Offsets621::Native<FName*(AFortGameMode* GameMode, FName* OutLootTierGroup, FName LootTierGroup)>(Offsets621::PickLootTierGroupAthena, "PickLootTierGroupAthena");
				PickLootTierGroupAthena(Globals::GetGameMode(), &BuildingContainer->SearchLootTierGroup, BuildingContainer->SearchLootTierGroup);

				int32 LootTier = -1;
				Loots::PickLootTierKeyAndLootTierFromTierGroup(&BuildingContainer->ContainerLootTierKey, &LootTier, BuildingContainer->SearchLootTierGroup, LootLevel, 0, -1, BuildingContainer->StaticGameplayTags);
			}

			// [8.51-ONLY] Coffre au tresor enterre (saison 8) : n'existe pas en 6.21, la classe vaut nullptr et la branche plus bas ne s'execute jamais
			// UClass* BootyBuildingContainerClass = FindObjectFast<UClass>("/Game/Athena/Items/ForagedItems/Booty/B_Athena_Booty_Chest.B_Athena_Booty_Chest_C");
			UClass* BootyBuildingContainerClass = nullptr;

			if (LootTestingData)
			{
				FFortItemEntry ItemEntry = Inventory::CreateItemEntry(LootTestingData, 1, 0.f, -1, -1);

				UFortWorldItemDefinition* WorldItemDefinition = Cast<UFortWorldItemDefinition>(ItemEntry.ItemDefinition);

				if (WorldItemDefinition &&
					WorldItemDefinition->ItemType == EFortItemType::WeaponMelee ||
					WorldItemDefinition->ItemType == EFortItemType::WeaponRanged)
				{
					float NewDurability = 1.0f * BuildingContainer->LootedWeaponsDurabilityModifier;

					ItemEntry.Durability = NewDurability;
					Inventory::PeutEtreAddStateValueJsp(&ItemEntry, EFortItemEntryState::DurabilityInitialized, 1);
				}

				// [8.51-ONLY] EFortPickupSpawnSource n'existe pas en 6.21
				// AFortPickup* Pickup = Inventory::SpawnPickup(LootTestingData, nullptr, &ItemEntry, SpawnLocation, true, EFortPickupSourceTypeFlag::Container, Inventory::GetPickupSpawnSource(BuildingContainer));
				AFortPickup* Pickup = Inventory::SpawnPickup(LootTestingData, nullptr, &ItemEntry, SpawnLocation, true, EFortPickupSourceTypeFlag::Container);

				if (Pickup)
				{
					Pickup->bTossedFromContainer = true;
					Pickup->OnRep_TossedFromContainer();
				}
			}
			else if (BuildingContainer->ContainerLootTierKey.IsValid())
			{
				TArray<FFortItemEntry> LootToDrops;
				Loots::PickLootDrops(&LootToDrops, LootLevel, BuildingContainer->ContainerLootTierKey, 0, 0, BuildingContainer->StaticGameplayTags, true, false);

				BuildingContainer->HighestRarity = EFortRarity::Handmade;

				for (int32 i = 0; i < LootToDrops.Num(); i++)
				{
					FFortItemEntry LootToDrop = LootToDrops[i];

					UFortWorldItemDefinition* WorldItemDefinition = Cast<UFortWorldItemDefinition>(LootToDrop.ItemDefinition);

					if (!WorldItemDefinition)
					{
						UFortItemDefinition* ItemDefinition = LootToDrop.ItemDefinition;
						FN_LOG(LogBuildingContainer, Error, "Attempted to spawn non-world item %s!", ItemDefinition->GetName().c_str());
						continue;
					}

					EFortRarity Rarity = WorldItemDefinition->GetRarity();
					EFortRarity HighestRarity = BuildingContainer->HighestRarity;

					if (HighestRarity < Rarity)
						BuildingContainer->HighestRarity = Rarity;

					if (WorldItemDefinition->ItemType == EFortItemType::WeaponMelee ||
						WorldItemDefinition->ItemType == EFortItemType::WeaponRanged)
					{
						float NewDurability = 1.0f * BuildingContainer->LootedWeaponsDurabilityModifier;

						LootToDrop.Durability = NewDurability;
						Inventory::PeutEtreAddStateValueJsp(&LootToDrop, EFortItemEntryState::DurabilityInitialized, 1);
					}

					AFortPickupAthena* Pickup = Util::SpawnActor<AFortPickupAthena>(AFortPickupAthena::StaticClass(), SpawnLocation);

					if (Pickup)
					{
						// [OFFSET-TODO-6.21] PickupInitialize (8.51 = 0xFECD10)
						// void (*PickupInitialize)(AFortPickupAthena* Pickup, FFortItemEntry* ItemEntry, TArray<FFortItemEntry> MultiItemEntries, bool bSplitOnPickup) = decltype(PickupInitialize)(0xFECD10 + uintptr_t(GetModuleHandle(0)));
						auto PickupInitialize = Offsets621::Native<void(AFortPickupAthena* Pickup, FFortItemEntry* ItemEntry, TArray<FFortItemEntry> MultiItemEntries, bool bSplitOnPickup)>(Offsets621::PickupInitialize, "PickupInitialize", &Fallbacks621::PickupInitialize);
						PickupInitialize(Pickup, &LootToDrop, {}, false);

						Pickup->bRandomRotation = true; 

						Functions::TossPickupFromContainer(
							BuildingContainer, 
							Pickup, 
							LootToDrops.Num(),
							(i + 1),
							BuildingContainer->LootTossConeHalfAngle_Athena,
							BuildingContainer->LootTossDirection_Athena,
							BuildingContainer->LootTossSpeed_Athena);
					}
				}
			}
			else if (BootyBuildingContainerClass && BuildingContainer->IsA(BootyBuildingContainerClass)) // [8.51-ONLY] toujours faux en 6.21
			{
				TArray<FFortItemEntry> LootToDrops;
				bool bSuccess = UFortKismetLibrary::PickLootDrops(BuildingContainer, &LootToDrops, BuildingContainer->SearchLootTierGroup, LootLevel, -1);

				if (bSuccess)
				{
					for (int32 i = 0; i < LootToDrops.Num(); i++)
					{
						FFortItemEntry LootToDrop = LootToDrops[i];

						UFortWorldItemDefinition* WorldItemDefinition = Cast<UFortWorldItemDefinition>(LootToDrop.ItemDefinition);

						if (!WorldItemDefinition)
						{
							UFortItemDefinition* ItemDefinition = LootToDrop.ItemDefinition;
							FN_LOG(LogBuildingContainer, Error, "Attempted to spawn non-world item %s!", ItemDefinition->GetName().c_str());
							continue;
						}

						EFortRarity Rarity = WorldItemDefinition->GetRarity();
						EFortRarity HighestRarity = BuildingContainer->HighestRarity;

						if (HighestRarity < Rarity)
							BuildingContainer->HighestRarity = Rarity;

						if (WorldItemDefinition->ItemType == EFortItemType::WeaponMelee ||
							WorldItemDefinition->ItemType == EFortItemType::WeaponRanged)
						{
							float NewDurability = 1.0f * BuildingContainer->LootedWeaponsDurabilityModifier;

							LootToDrop.Durability = NewDurability;
							Inventory::PeutEtreAddStateValueJsp(&LootToDrop, EFortItemEntryState::DurabilityInitialized, 1);
						}

						AFortPickupAthena* Pickup = Util::SpawnActor<AFortPickupAthena>(AFortPickupAthena::StaticClass(), SpawnLocation);

						if (Pickup)
						{
							// [OFFSET-TODO-6.21] PickupInitialize (8.51 = 0xFECD10)
							// void (*PickupInitialize)(AFortPickupAthena * Pickup, FFortItemEntry * ItemEntry, TArray<FFortItemEntry> MultiItemEntries, bool bSplitOnPickup) = decltype(PickupInitialize)(0xFECD10 + uintptr_t(GetModuleHandle(0)));
							auto PickupInitialize = Offsets621::Native<void(AFortPickupAthena * Pickup, FFortItemEntry * ItemEntry, TArray<FFortItemEntry> MultiItemEntries, bool bSplitOnPickup)>(Offsets621::PickupInitialize, "PickupInitialize", &Fallbacks621::PickupInitialize);
							PickupInitialize(Pickup, &LootToDrop, {}, false);

							Pickup->bRandomRotation = true;

							Functions::TossPickupFromContainer(
								BuildingContainer,
								Pickup,
								LootToDrops.Num(),
								(i + 1),
								BuildingContainer->LootTossConeHalfAngle_Athena,
								BuildingContainer->LootTossDirection_Athena,
								BuildingContainer->LootTossSpeed_Athena);
						}
					}
				}
			}

			if (PlayerPawn)
			{
				float LootNoiseRange = BuildingContainer->LootNoiseRange;

				if (LootNoiseRange > 0.0f)
				{
					const FVector& NoiseLocation = BuildingContainer->K2_GetActorLocation();
					UFortAIFunctionLibrary::MakeNoiseEventAtLocation(PlayerPawn, LootNoiseRange, NoiseLocation);
				}
			}

			if (!BuildingContainer->bDestroyed)
				BuildingContainer->SetSearchedContainer(PlayerPawn);
		}

		BuildingContainer->SearchBounceData.SearchAnimationCount++;
		BuildingContainer->BounceContainer();

		return true;
	}

	void ABuildingContainer_PostUpdateHook(ABuildingContainer* BuildingContainer, EFortBuildingPersistentState BuildingPersistentState, __int64 a3)
	{
		ABuildingContainer_PostUpdateOG(BuildingContainer, BuildingPersistentState, a3);

		// [GALAXIA PATCH] 6.21 : sans playlist, pas de tables de loot : le coffre serait marque comme fouille a tort. SpawnLootHook choisira a l'ouverture.
		if (!Globals::GetPlaylist())
			return;

		if (BuildingPersistentState == EFortBuildingPersistentState::New)
		{
			float LootSpawnThreshold = 0.0f;

			bool bSpawnAllLoot = false;

			if (bSpawnAllLoot)
				LootSpawnThreshold = 1.0f;

			if (BuildingContainer->SearchLootTierGroup.IsValid())
			{
				// [OFFSET-TODO-6.21] PickLootTierGroupAthena (8.51 = 0xfad620)
				// FName (*PickLootTierGroupAthena)(AFortGameMode* GameMode, FName* OutLootTierGroup, FName LootTierGroup) = decltype(PickLootTierGroupAthena)(0xfad620 + uintptr_t(GetModuleHandle(0)));
				auto PickLootTierGroupAthena = Offsets621::Native<FName*(AFortGameMode* GameMode, FName* OutLootTierGroup, FName LootTierGroup)>(Offsets621::PickLootTierGroupAthena, "PickLootTierGroupAthena");
				PickLootTierGroupAthena(Globals::GetGameMode(), &BuildingContainer->SearchLootTierGroup, BuildingContainer->SearchLootTierGroup);

				FName* LootTierKey = nullptr;
				int32 ChooseLootTier = -1;
				int32 LootTier = -1;

				LootSpawnThreshold = 1.0f;

				if (BuildingContainer->SearchLootTierChosenQuotaInfo.LootTierKey.ComparisonIndex > 0 ||
					BuildingContainer->SearchLootTierChosenQuotaInfo.LootTierKey.Number > 0)
				{
					LootTierKey = &BuildingContainer->ContainerLootTierKey;
					ChooseLootTier = BuildingContainer->SearchLootTierChosenQuotaInfo.LootTier;

					BuildingContainer->ContainerLootTierKey = BuildingContainer->SearchLootTierChosenQuotaInfo.LootTierKey;

					LootTier = ChooseLootTier;
				}
				else
				{
					int32 LootLevel = UFortKismetLibrary::GetLootLevel(BuildingContainer);

					LootTierKey = &BuildingContainer->ContainerLootTierKey;

					Loots::PickLootTierKeyAndLootTierFromTierGroup(
						&BuildingContainer->ContainerLootTierKey,
						&LootTier,
						BuildingContainer->SearchLootTierGroup,
						LootLevel,
						0,
						-1,
						BuildingContainer->StaticGameplayTags
					);

					ChooseLootTier = LootTier;
				}

				BuildingContainer->SelectMeshSet(ChooseLootTier);

				EFortBuildingPersistentState PersistentState = EFortBuildingPersistentState::Default;

				if (!bSpawnAllLoot &&
					!BuildingContainer->bAlwaysMaintainLoot &&
					!Loots::IsLootDropPossible(*LootTierKey, L"ABuildingContainer::PostUpdate", 0))
				{
					LootTierKey = nullptr;
					PersistentState = EFortBuildingPersistentState::Searched;

					BuildingContainer->SetSearchedContainer(nullptr);

					if (!BuildingContainer->bAlwaysShowContainer)
						LootSpawnThreshold = 0.0f;
				}

				// Retire le floor loot pour la lategame (plus opti)
//#ifndef LATE_GAME
				UBlueprintGeneratedClass* FloorLootClass = FindObjectFast<UBlueprintGeneratedClass>("/Game/Athena/Environments/Blueprints/Tiered_Athena_FloorLoot_01.Tiered_Athena_FloorLoot_01_C");
				UBlueprintGeneratedClass* FloorLootWarmupClass = FindObjectFast<UBlueprintGeneratedClass>("/Game/Athena/Environments/Blueprints/Tiered_Athena_FloorLoot_Warmup.Tiered_Athena_FloorLoot_Warmup_C");

				if (BuildingContainer->IsA(FloorLootClass) || BuildingContainer->IsA(FloorLootWarmupClass))
					SpawnLootHook(BuildingContainer, nullptr);
//#endif // !LATE_GAME
			}

			float v6 = (float)rand() * 0.000030518509f;

			if ((float)((float)(v6 * 1.0f) + 0.0000000099999999f) > LootSpawnThreshold)
			{
				if (BuildingContainer->bBuriedTreasure)
					BuildingContainer->SetSearchedContainer(nullptr);

				BuildingContainer->ContainerLootTierKey = FName(0);
			}
		}
		else
		{
			if (BuildingContainer->ContainerLootTierKey.IsValid())
			{
				SelectMeshSet(BuildingContainer, BuildingContainer->ContainerLootTierKey);
			}
			else
			{
				int32 LootLevel = UFortKismetLibrary::GetLootLevel(BuildingContainer);
				int32 LootTier = -1;

				Loots::PickLootTierKeyAndLootTierFromTierGroup(
					&BuildingContainer->ContainerLootTierKey,
					&LootTier,
					BuildingContainer->SearchLootTierGroup,
					LootLevel,
					0,
					-1,
					BuildingContainer->StaticGameplayTags
				);

				BuildingContainer->SelectMeshSet(LootTier);
			}

			if (BuildingPersistentState == EFortBuildingPersistentState::Searched)
				BuildingContainer->SetSearchedContainer(nullptr);
		}

		FN_LOG(LogBuildingActor, Debug, "ABuildingContainer::PostUpdate() on existing container Container: %s, ContainerLootTierKey: %s, AltMeshIdx: %d",
			BuildingContainer->GetName().c_str(), BuildingContainer->ContainerLootTierKey.ToString().c_str(), BuildingContainer->AltMeshIdx);
	}

	void ABuildingSMActor_PostUpdateHook(ABuildingSMActor* BuildingSMActor)
	{
		EFortBuildingPersistentState PersistentState = EFortBuildingPersistentState::Default;
		FName LootTierKey = BuildingSMActor->DestructionLootTierKey;

		if (BuildingSMActor->DestructionLootTierKey.IsValid())
		{
			SelectMeshSet(BuildingSMActor, LootTierKey);
		}
		else
		{
			int32 LootTier = 0;
			int32 ChooseLootTier = 0;

			if (BuildingSMActor->DestructionLootTierGroup.IsValid())
			{
				LootTier = -1;

				if (BuildingSMActor->DestructionLootTierChosenQuotaInfo.LootTierKey.ComparisonIndex > 0 ||
					BuildingSMActor->DestructionLootTierChosenQuotaInfo.LootTierKey.Number > 0)
				{
					ChooseLootTier = BuildingSMActor->DestructionLootTierChosenQuotaInfo.LootTier;
					LootTierKey = BuildingSMActor->DestructionLootTierChosenQuotaInfo.LootTierKey;

					LootTier = ChooseLootTier;
				}
				else
				{
					int32 LootLevel = UFortKismetLibrary::GetLootLevel(BuildingSMActor);

					Loots::PickLootTierKeyAndLootTierFromTierGroup(
						&BuildingSMActor->DestructionLootTierKey,
						&LootTier,
						BuildingSMActor->DestructionLootTierGroup,
						LootLevel,
						0,
						-1,
						BuildingSMActor->StaticGameplayTags
					);

					ChooseLootTier = LootTier;
				}

				BuildingSMActor->SelectMeshSet(ChooseLootTier);

				if (!Loots::IsLootDropPossible(LootTierKey, L"ABuildingSMActor::AttemptSpawnResources", 0))
				{
					LootTierKey = FName(0);
					PersistentState = EFortBuildingPersistentState::Searched;
				}
			}
		}

		FN_LOG(LogBuildingSMActor, Debug, "ABuildingSMActor::PostUpdate() Building: %s, AltMeshIdx: %d", BuildingSMActor->GetName().c_str(), BuildingSMActor->AltMeshIdx);

		ABuildingSMActor_PostUpdateOG(BuildingSMActor);
	}

    void (*OnDamageServerOG)(ABuildingActor* BuildingActor, float Damage, const FGameplayTagContainer& DamageTags, const FVector& Momentum, const FHitResult& HitInfo, AController* InstigatedBy, AActor* DamageCauser, const FGameplayEffectContextHandle& EffectContext);
    void OnDamageServer(
        ABuildingActor* BuildingActor,
        float Damage,
        const FGameplayTagContainer& DamageTags,
        const FVector& Momentum,
        const FHitResult& HitInfo,
        AController* InstigatedBy,
        AActor* DamageCauser,
        const FGameplayEffectContextHandle& EffectContext
    )
    {
        OnDamageServerOG(
            BuildingActor,
            Damage,
            DamageTags,
            Momentum,
            HitInfo,
            InstigatedBy,
            DamageCauser,
            EffectContext);

        ABuildingSMActor* BuildingSMActor = Cast<ABuildingSMActor>(BuildingActor);
        AFortPlayerController* PlayerController = Cast<AFortPlayerController>(InstigatedBy);
        AFortWeapon* Weapon = Cast<AFortWeapon>(DamageCauser);

        if (!BuildingSMActor || !PlayerController || !Weapon)
            return;

        AFortPlayerPawn* PlayerPawn = PlayerController->MyFortPawn;

        if (!Weapon->WeaponData->IsA(UFortWeaponMeleeItemDefinition::StaticClass()) ||
            !BuildingSMActor->bAllowResourceDrop ||
            BuildingSMActor->bDestroyed ||
            !PlayerPawn)
            return;

        UFortResourceItemDefinition* ResourceItemDefinition = UFortKismetLibrary::K2_GetResourceItemDefinition(BuildingSMActor->ResourceType);

        int32 ResourceDropAmount = 0;
        float ResourceRatio = 0.0f;

        if (ResourceItemDefinition)
        {
            if (BuildingSMActor->MaxResourcesToSpawn < 0)
            {
                // [OFFSET-TODO-6.21] DetermineMaxResourcesToSpawn (8.51 = 0x11114a0)
                // int32 (*DetermineMaxResourcesToSpawn)(ABuildingSMActor* BuildingSMActor, char a2) = decltype(DetermineMaxResourcesToSpawn)(0x11114a0 + uintptr_t(GetModuleHandle(0)));
                auto DetermineMaxResourcesToSpawn = Offsets621::Native<int32(ABuildingSMActor* BuildingSMActor, char a2)>(Offsets621::DetermineMaxResourcesToSpawn, "DetermineMaxResourcesToSpawn");
                BuildingSMActor->MaxResourcesToSpawn = DetermineMaxResourcesToSpawn(BuildingSMActor, 1);
            }

            float MaxResourcesToSpawn = BuildingSMActor->MaxResourcesToSpawn;
            float MaxHealth = BuildingSMActor->GetMaxHealth();

            ResourceRatio = MaxResourcesToSpawn / MaxHealth;
            ResourceDropAmount = (int32)(ResourceRatio * Damage);
        }

        bool bHasHealthLeft = BuildingSMActor->HasHealthLeft();
        bool bDestroyed = false;

        if (!bHasHealthLeft)
        {
            bDestroyed = true;

            int32 MinResourceCount = 0;

            if (!ResourceItemDefinition || (MinResourceCount = 1, ResourceRatio == 0.0))
                MinResourceCount = 0;

            if (ResourceDropAmount < MinResourceCount)
                ResourceDropAmount = MinResourceCount;

			if (BuildingSMActor->DestructionLootTierKey.IsValid())
			{
				int32 LootLevel = UFortKismetLibrary::GetLootLevel(BuildingSMActor);

				TArray<FFortItemEntry> LootToDrops;
				Loots::PickLootDrops(&LootToDrops, LootLevel, BuildingSMActor->DestructionLootTierKey, 0, 0, BuildingSMActor->StaticGameplayTags, true, false);

				for (int32 i = 0; i < LootToDrops.Num(); i++)
				{
					FFortItemEntry LootToDrop = LootToDrops[i];

					UFortWorldItemDefinition* WorldItemDefinition = Cast<UFortWorldItemDefinition>(LootToDrop.ItemDefinition);

					if (!WorldItemDefinition)
					{
						FN_LOG(LogBuildingSMActor, Error, "Loot tier %s dropped entry with no item data!", BuildingSMActor->DestructionLootTierGroup.ToString().c_str());
						continue;
					}

					FVector BuildingSMActorLocation = BuildingSMActor->K2_GetActorLocation();
					FVector RandomDirection = UKismetMathLibrary::RandomUnitVector();

					BuildingSMActorLocation.X += RandomDirection.X * 200.0f;
					BuildingSMActorLocation.Y += RandomDirection.Y * 200.0f;
					BuildingSMActorLocation.Z += RandomDirection.Z * 200.0f;

					FFortItemEntry ItemEntry = Inventory::CreateItemEntry(WorldItemDefinition, LootToDrop.Count, LootToDrop.Durability, LootToDrop.Level, LootToDrop.LoadedAmmo);

					// [8.51-ONLY] EFortPickupSpawnSource n'existe pas en 6.21
					// AFortPickup* Pickup = Inventory::SpawnPickup(WorldItemDefinition, nullptr, &ItemEntry, BuildingSMActorLocation, false, EFortPickupSourceTypeFlag::Destruction, EFortPickupSpawnSource::Unset);
					AFortPickup* Pickup = Inventory::SpawnPickup(WorldItemDefinition, nullptr, &ItemEntry, BuildingSMActorLocation, false, EFortPickupSourceTypeFlag::Destruction);

					if (Pickup)
					{
						FVector ConeDir = { 0, 0, 0 };
						FVector VectorInCone = UKismetMathLibrary::RandomUnitVectorInConeInRadians(ConeDir, 1.0f);

						// [OFFSET-TODO-6.21] GenFlyTime (8.51 = 0x1456e40)
						// float (*GenFlyTime)(AFortPickup* Pickup) = decltype(GenFlyTime)(0x1456e40 + uintptr_t(GetModuleHandle(0)));
						auto GenFlyTime = Offsets621::Native<float(AFortPickup* Pickup)>(Offsets621::GenFlyTime, "GenFlyTime");
						float FlyTime = GenFlyTime(Pickup);

						// [OFFSET-TODO-6.21] SetPickupTarget (8.51 = 0x1467300)
						// void (*SetPickupTarget)(AFortPickup* Pickup, AFortPlayerPawn* PlayerPawn, float InFlyTime, const FVector& InStartDirection, bool bPlayPickupSound) = decltype(SetPickupTarget)(0x1467300 + uintptr_t(GetModuleHandle(0)));
						auto SetPickupTarget = Offsets621::Native<void(AFortPickup* Pickup, AFortPlayerPawn* PlayerPawn, float InFlyTime, const FVector& InStartDirection, bool bPlayPickupSound)>(Offsets621::SetPickupTarget, "SetPickupTarget", &Inventory::SetPickupTargetSDK);
						SetPickupTarget(Pickup, PlayerPawn, (FlyTime + i * 0.30000001192092896f), VectorInCone, true);
					}
				}
			}
        }

        if (ResourceDropAmount > 0)
        {
            FFortItemEntry ItemEntry = Inventory::CreateItemEntry(ResourceItemDefinition, ResourceDropAmount, 0.0f, 0, 0);

            Inventory::PeutEtreAddStateValueJsp(&ItemEntry, EFortItemEntryState::DoNotShowSpawnParticles, 1);

            Inventory::AddInventoryItem(PlayerController, ResourceItemDefinition, &ItemEntry);
            PlayerController->ClientReportDamagedResourceBuilding(BuildingSMActor, BuildingSMActor->ResourceType, ResourceDropAmount, bDestroyed, (Damage == 100.0f));;
        }
    }

	void InitBuildingActor()
	{
        ABuildingContainer* BuildingContainerDefault = ABuildingContainer::GetDefaultObj();
        UClass* BuildingContainerClass = ABuildingContainer::StaticClass();

		// [8.51-ONLY] Index 8.51
		// MinHook::HookVTable(BuildingContainerDefault, 0x7E8 / 8, ABuildingContainer_PostUpdateHook, (LPVOID*)(&ABuildingContainer_PostUpdateOG), "ABuildingContainer::PostUpdate");
		// [GALAXIA PATCH] 6.21 : slot 0x7A8 verifie dans le dump (ABuildingSMActor::PostUpdate occupe le meme slot)
		MinHook::HookVTable(BuildingContainerDefault, 0x7A8 / 8, ABuildingContainer_PostUpdateHook, (void**)(&ABuildingContainer_PostUpdateOG), "ABuildingContainer::PostUpdate");
        // [8.51-ONLY] Index 8.51
        // MinHook::HookVTable(BuildingContainerDefault, 0xE28 / 8, SpawnLootHook, nullptr, "ABuildingContainer::SpawnLoot");
        // [GALAXIA PATCH] 6.21 : slot 0xDD8 verifie dans le dump (appele par ABuildingContainer::ServerOnAttemptInteract avec le pawn et un 3e argument, ignore ici)
        MinHook::HookVTable(BuildingContainerDefault, 0xDD8 / 8, SpawnLootHook, nullptr, "ABuildingContainer::SpawnLoot");

		// [OFFSET-TODO-6.21] ABuildingSMActor_PostUpdate (8.51 = 0x1123260)
		// MH_CreateHook((LPVOID)(InSDKUtils::GetImageBase() + 0x1123260), ABuildingSMActor_PostUpdateHook, (LPVOID*)(&ABuildingSMActor_PostUpdateOG));
		// MH_EnableHook((LPVOID)(InSDKUtils::GetImageBase() + 0x1123260));
		MinHook::HookNative(Offsets621::ABuildingSMActor_PostUpdate, "ABuildingSMActor_PostUpdate", ABuildingSMActor_PostUpdateHook, (void**)(&ABuildingSMActor_PostUpdateOG));

        // [OFFSET-TODO-6.21] OnDamageServer (8.51 = 0x18f0570)
        // MH_CreateHook((LPVOID)(InSDKUtils::GetImageBase() + 0x18f0570), OnDamageServer, (LPVOID*)(&OnDamageServerOG));
        // MH_EnableHook((LPVOID)(InSDKUtils::GetImageBase() + 0x18f0570));
        // MinHook::HookNative(Offsets621::OnDamageServer, "OnDamageServer", OnDamageServer, (void**)(&OnDamageServerOG));
        // [GALAXIA PATCH] 6.21 : ABuildingActor::OnDamageServer est un event Blueprint, hook par ProcessEvent (parametres verifies au demarrage)
        MinHook::HookRPC<OnDamageServer>(ABuildingActor::GetDefaultObj(), "BuildingActor", "OnDamageServer", (void**)(&OnDamageServerOG));

		FN_LOG(LogInit, Log, "InitBuildingActor Success!");
	}
}