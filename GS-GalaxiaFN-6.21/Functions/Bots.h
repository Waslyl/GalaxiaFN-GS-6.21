#pragma once

namespace Bots
{
#ifdef BOTS
	struct FBotAircraftParams
	{
		AFortAthenaAIBotController* AthenaAIBotController = nullptr;
		float TimeToThanks = 0.0f;
		float TimeToDrop = 0.0f;

		FBotAircraftParams(AFortAthenaAIBotController* InAthenaAIBotController, float InTimeToThanks, float InTimeToDrop)
			: AthenaAIBotController(InAthenaAIBotController)
			, TimeToThanks(InTimeToThanks)
			, TimeToDrop(InTimeToDrop)
		{
		}
	};

	static TArray<FBotAircraftParams> BotAircraftParamsArray;
	static bool bIsInitializeBotData = false;

	bool InitializeBotData()
	{
		if (bIsInitializeBotData)
			return true;

		AFortGameStateAthena* GameStateAthena = Globals::GetGameState();
		AFortGameModeAthena* GameModeAthena = Globals::GetGameMode();

		if (!GameStateAthena || !GameModeAthena) 
			return false;

		AAthenaAIDirector* AIDirector = Util::SpawnActor<AAthenaAIDirector>(AAthenaAIDirector::StaticClass());

		if (AIDirector)
		{
			AIDirector->SetOwner(GameModeAthena);
			AIDirector->OnRep_Owner();

			GameModeAthena->AIDirector = AIDirector;
			GameModeAthena->AIDirector->Activate();
		}

		AFortAIGoalManager* AIGoalManager = Util::SpawnActor<AFortAIGoalManager>(AFortAIGoalManager::StaticClass());

		if (AIGoalManager)
		{
			AIGoalManager->SetOwner(GameModeAthena);
			AIGoalManager->OnRep_Owner();

			GameModeAthena->AIGoalManager = AIGoalManager;
		}

		AFortAthenaMutator_Bots* AthenaMutator_Bots = Util::SpawnActor<AFortAthenaMutator_Bots>(AFortAthenaMutator_Bots::StaticClass());
		if (!AthenaMutator_Bots) return false;

		AthenaMutator_Bots->bMutatesGameMode = true;
		AthenaMutator_Bots->CachedGameMode = GameModeAthena;

		AthenaMutator_Bots->bMutatesGameState = true;
		AthenaMutator_Bots->CachedGameState = GameStateAthena;

		GameModeAthena->MutatorListComponent->Mutators.Add(AthenaMutator_Bots);

		UFortServerBotManagerAthena* ServerBotManagerAthena = Cast<UFortServerBotManagerAthena>(UGameplayStatics::SpawnObject(UFortServerBotManagerAthena::StaticClass(), GameModeAthena));
		if (!ServerBotManagerAthena) return false;

		ServerBotManagerAthena->CachedGameMode = GameModeAthena;
		ServerBotManagerAthena->CachedBotMutator = AthenaMutator_Bots;

		GameModeAthena->ServerBotManager = ServerBotManagerAthena;

		bIsInitializeBotData = true;
		return true;
	}

	UFortWorldItem* AddAndEquipItem(AFortInventory* WorldInventory, AFortPlayerPawnAthena* PlayerBotPawn, UFortItemDefinition* ItemDefinition, int32 Count)
	{
		if (!WorldInventory || !PlayerBotPawn || !ItemDefinition || Count < 1)
			return nullptr;

		FFortItemEntry ItemEntry = Inventory::CreateItemEntry(ItemDefinition, Count, 0.f, -1, -1);
		UFortWorldItem* WorldItem = Inventory::AddItem(WorldInventory, &ItemEntry);

		Inventory::UpdateInventory(WorldInventory);

		if (WorldItem)
		{
			UFortWeaponItemDefinition* WeaponItemDefinition = Cast<UFortWeaponItemDefinition>(WorldItem->ItemEntry.ItemDefinition);

			if (WeaponItemDefinition)
				PlayerBotPawn->EquipWeaponDefinition(WeaponItemDefinition, WorldItem->ItemEntry.ItemGuid);

			// Fait full buger quand on équip deux armes (Le move to ne marche plus)

			// essaye ça sub_7FF6B99028D0(PlayerPawnAthena, 0i64);
			//PlayerBotPawn->Equip
		}

		return WorldItem;
	}

	UAthenaCharacterItemDefinition* ChooseRandomCharacter()
	{
		TArray<UAthenaCosmeticItemDefinition*> AllCharacterItems = UFortniteAutomationBlueprintLibrary::GetAllAthenaCosmetics(EFortItemType::AthenaCharacter);
		if (!AllCharacterItems.IsValid()) return nullptr;

		return Cast<UAthenaCharacterItemDefinition>(AllCharacterItems[std::rand() % AllCharacterItems.Num()]);
	}

	UAthenaPickaxeItemDefinition* ChooseRandomPickaxe()
	{
		TArray<UAthenaCosmeticItemDefinition*> AllPickaxeItems = UFortniteAutomationBlueprintLibrary::GetAllAthenaCosmetics(EFortItemType::AthenaPickaxe);
		if (!AllPickaxeItems.IsValid()) return nullptr;

		return Cast<UAthenaPickaxeItemDefinition>(AllPickaxeItems[std::rand() % AllPickaxeItems.Num()]);
	}

	UAthenaGliderItemDefinition* ChooseRandomGlider()
	{
		TArray<UAthenaCosmeticItemDefinition*> AllGliderItems = UFortniteAutomationBlueprintLibrary::GetAllAthenaCosmetics(EFortItemType::AthenaGlider);
		if (!AllGliderItems.IsValid()) return nullptr;

		return Cast<UAthenaGliderItemDefinition>(AllGliderItems[std::rand() % AllGliderItems.Num()]);
	}
	
	UAthenaDanceItemDefinition* ChooseRandomEmote()
	{
		TArray<UAthenaCosmeticItemDefinition*> AllDanceItems = UFortniteAutomationBlueprintLibrary::GetAllAthenaCosmetics(EFortItemType::AthenaDance);
		if (!AllDanceItems.IsValid()) return nullptr;

		return Cast<UAthenaDanceItemDefinition>(AllDanceItems[std::rand() % AllDanceItems.Num()]);
	}

	AFortPickup* GetClosestPickup(AFortAthenaAIBotController* AthenaAIBotController)
	{
		if (!AthenaAIBotController || !AthenaAIBotController->PlayerBotPawn)
			return nullptr;

		TArray<AActor*> Actors;
		UGameplayStatics::GetAllActorsOfClass(AthenaAIBotController, AFortPickup::StaticClass(), &Actors);

		if (!Actors.IsValid())
			return nullptr;

		return Cast<AFortPickup>(UFortKismetLibrary::GetClosestActorFromArray(AthenaAIBotController->PlayerBotPawn, Actors));
	}

	ABuildingFoundation* GetClosestBuildingFoundation(AFortAthenaAIBotController* AthenaAIBotController)
	{
		if (!AthenaAIBotController || !AthenaAIBotController->PlayerBotPawn)
			return nullptr;

		TArray<AActor*> Actors;
		UGameplayStatics::GetAllActorsOfClass(AthenaAIBotController, ABuildingFoundation::StaticClass(), &Actors);

		if (!Actors.IsValid())
			return nullptr;

		ABuildingFoundation* BuildingFoundation = Cast<ABuildingFoundation>(UFortKismetLibrary::GetClosestActorFromArray(AthenaAIBotController->PlayerBotPawn, Actors));
		if (!BuildingFoundation) return nullptr;

		if (BuildingFoundation->FoundationEnabledState != EDynamicFoundationEnabledState::Enabled ||
			BuildingFoundation->DynamicFoundationType != EDynamicFoundationType::Static)
			return nullptr;

		return BuildingFoundation;
	}

	AFortAthenaAIBotController* CreateAIBotController(AFortPlayerPawnAthena* PlayerPawnAthena)
	{
		AFortGameStateAthena* GameStateAthena = Globals::GetGameState();
		AFortGameModeAthena* GameModeAthena = Globals::GetGameMode();

		if (!GameStateAthena || !GameModeAthena)
			return nullptr;

		UFortServerBotManagerAthena* ServerBotManagerAthena = GameModeAthena->ServerBotManager;
		if (!ServerBotManagerAthena) return nullptr;

		AFortAthenaAIBotController* AthenaAIBotController = Util::SpawnActor<AFortAthenaAIBotController>(AFortAthenaAIBotController::StaticClass());
		if (!AthenaAIBotController) return nullptr;

		AthenaAIBotController->PlayerBotPawn = PlayerPawnAthena;
		AthenaAIBotController->CachedBotManager = ServerBotManagerAthena;
		AthenaAIBotController->CachedGameMode = GameModeAthena;
		AthenaAIBotController->CachedBotMutator = ServerBotManagerAthena->CachedBotMutator;

		AFortInventory* FortInventory = Util::SpawnActor<AFortInventory>(AFortInventory::StaticClass());

		if (FortInventory)
		{
			FortInventory->SetOwner(AthenaAIBotController);
			FortInventory->OnRep_Owner();

			AthenaAIBotController->Inventory = FortInventory;
			AthenaAIBotController->OnRep_Inventory();

			FortInventory->HandleInventoryLocalUpdate();
		}

		AFortPlayerStateAthena* PlayerStateAthena = Util::SpawnActor<AFortPlayerStateAthena>(AFortPlayerStateAthena::StaticClass());

		if (PlayerStateAthena)
		{
			PlayerStateAthena->SetOwner(AthenaAIBotController);
			PlayerStateAthena->OnRep_Owner();

			AthenaAIBotController->PlayerState = PlayerStateAthena;
			AthenaAIBotController->OnRep_PlayerState();
		}

		AthenaAIBotController->CustomizationLoadout.Character = ChooseRandomCharacter();
		AthenaAIBotController->CustomizationLoadout.Pickaxe = ChooseRandomPickaxe();
		AthenaAIBotController->CustomizationLoadout.Glider = ChooseRandomGlider();

		PlayerPawnAthena->CosmeticLoadout.Character = AthenaAIBotController->CustomizationLoadout.Character;
		PlayerPawnAthena->CosmeticLoadout.Pickaxe = AthenaAIBotController->CustomizationLoadout.Pickaxe;
		PlayerPawnAthena->CosmeticLoadout.Glider = AthenaAIBotController->CustomizationLoadout.Glider;
		PlayerPawnAthena->OnRep_CosmeticLoadout();

		PlayerPawnAthena->SetOwner(AthenaAIBotController);
		PlayerPawnAthena->OnRep_Owner();

		AthenaAIBotController->Pawn = PlayerPawnAthena;
		AthenaAIBotController->OnRep_Pawn();

		AthenaAIBotController->Possess(PlayerPawnAthena);

		FFortServerBotInfo ServerBotInfo{};
		ServerBotInfo.BotController = AthenaAIBotController;

		// Bon ça doit faire pleins de truc prc ça crash des fois ici: 7FF6B9034794
		//ServerBotManagerAthena->PlayerBots.Add(ServerBotInfo);

		return AthenaAIBotController;
	}

	AFortAthenaAIBotController* SpawnBot(AActor* StartSpot, bool bIsNew = true, AFortAthenaAIBotController* ExistingAthenaAIBotController = nullptr)
	{
		AFortGameStateAthena* GameStateAthena = Globals::GetGameState();
		AFortGameModeAthena* GameModeAthena = Globals::GetGameMode();

		if (!StartSpot ||!GameStateAthena || !GameModeAthena)
			return nullptr;

		AFortPlayerPawnAthena* PlayerPawnAthena = Cast<AFortPlayerPawnAthena>(UAIBlueprintHelperLibrary::SpawnAIFromClass(StartSpot, APlayerPawn_Athena_C::StaticClass(), nullptr, StartSpot->K2_GetActorLocation(), StartSpot->K2_GetActorRotation(), true));
		if (!PlayerPawnAthena) return nullptr;

		AFortAthenaAIBotController* AthenaAIBotController = nullptr;

		if (bIsNew && ExistingAthenaAIBotController == nullptr)
		{
			AthenaAIBotController = CreateAIBotController(PlayerPawnAthena);

			GameStateAthena->PlayerBotsLeft++;
			GameStateAthena->OnRep_PlayersLeft();
		}
		else if (!bIsNew && ExistingAthenaAIBotController)
		{
			AthenaAIBotController = ExistingAthenaAIBotController;
		}

		if (!AthenaAIBotController)
			return nullptr;

		PlayerPawnAthena->Controller = AthenaAIBotController;
		PlayerPawnAthena->OnRep_Controller();

		PlayerPawnAthena->PlayerState = AthenaAIBotController->PlayerState;
		PlayerPawnAthena->OnRep_PlayerState();
		
		PlayerPawnAthena->SetMaxHealth(100);
		PlayerPawnAthena->SetHealth(100);
		
		PlayerPawnAthena->SetMaxShield(100);
		PlayerPawnAthena->SetShield(0);
		
		AFortPlayerState* PlayerState = Cast<AFortPlayerState>(AthenaAIBotController->PlayerState);

		if (PlayerState)
		{
			static uintptr_t AddressApplyCharacterCustomization = MinHook::FindPattern(Patterns::ApplyCharacterCustomization);
			void (*ApplyCharacterCustomization)(AFortPlayerState* PlayerState, AFortPawn* Pawn) = decltype(ApplyCharacterCustomization)(AddressApplyCharacterCustomization);

			ApplyCharacterCustomization(PlayerState, PlayerPawnAthena);

			UFortAbilitySystemComponent* AbilitySystemComponent = PlayerState->AbilitySystemComponent;

			if (AbilitySystemComponent)
			{
				static auto DefaultAbilities = FindObjectFast<UFortAbilitySet>("/Game/Abilities/Player/Generic/Traits/DefaultPlayer/GAS_AthenaPlayer.GAS_AthenaPlayer");
				Abilities::GrantGameplayAbility(DefaultAbilities, AbilitySystemComponent);
				Abilities::GrantModifierAbilityFromPlaylist(AbilitySystemComponent);

				FN_LOG(LogBots, Warning, "GrantGameplayAbility!");
			}

			/*static int32 PlayerBotId = 1;

			std::string PlayerName = "GalaxiaBot-" + std::to_string(PlayerBotId);



			PlayerState->PlayerNamePrivate = std::wstring(PlayerName.begin(), PlayerName.end()).c_str();

			PlayerState->PlayerName = std::wstring(PlayerName.begin(), PlayerName.end()).c_str();
			PlayerState->OnRep_PlayerTeam();

			PlayerBotId++;*/
		}

		AFortInventory* WorldInventory = AthenaAIBotController->Inventory;

		if (WorldInventory && WorldInventory->Inventory.ItemInstances.Num() <= 0 && false)
		{
			UFortWeaponMeleeItemDefinition* PickaxeDefinition = AthenaAIBotController->CustomizationLoadout.Pickaxe->WeaponDefinition;

			if (!PickaxeDefinition)
				PickaxeDefinition = FindObjectFast<UFortWeaponMeleeItemDefinition>("/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Athena_C_T01.WID_Harvest_Pickaxe_Athena_C_T01");

			TArray<FItemAndCount> StartingItems = GameModeAthena->StartingItems;

			for (int32 i = 0; i < StartingItems.Num(); i++)
			{
				FItemAndCount StartingItem = StartingItems[i];

				UFortItemDefinition* ItemDefinition = StartingItem.Item;
				if (!ItemDefinition) continue;

				AddAndEquipItem(WorldInventory, PlayerPawnAthena, ItemDefinition, 1);
			}

			if (PickaxeDefinition)
				AddAndEquipItem(WorldInventory, PlayerPawnAthena, PickaxeDefinition, 1);
		}

		return AthenaAIBotController;
	}

	AFortPlayerPawnAthena* SpawnAIPawnFromAircraft(AFortAthenaAIBotController* AthenaAIBotController)
	{
		AFortGameStateAthena* GameStateAthena = Globals::GetGameState();

		if (!GameStateAthena || !AthenaAIBotController || AthenaAIBotController->PlayerBotPawn)
			return nullptr;

		AFortAthenaAircraft* AthenaAircraft = GameStateAthena->GetAircraft(0);
		if (!AthenaAircraft) return nullptr;

		if (AFortInventory* Inventory = AthenaAIBotController->Inventory)
		{
			Inventory::RemoveAllItemsFromInventory(Inventory, true);
			Inventory::UpdateInventory(Inventory);
		}

		AthenaAIBotController = SpawnBot(AthenaAircraft, false, AthenaAIBotController);

		AFortPlayerStateAthena* PlayerStateAthena = Cast<AFortPlayerStateAthena>(AthenaAIBotController->PlayerState);
		AFortPlayerPawnAthena* PlayerBotPawn = AthenaAIBotController->PlayerBotPawn;

		if (!PlayerStateAthena || !PlayerBotPawn)
			return nullptr;

		PlayerBotPawn->BeginSkydiving(true);

		PlayerStateAthena->bInAircraft = false;
		PlayerStateAthena->ServerSetInAircraft(false);

		AthenaAIBotController->CurrentTarget = GetClosestBuildingFoundation(AthenaAIBotController);

		return PlayerBotPawn;
	}

	void PlayRandomEmote(AFortAthenaAIBotController* AthenaAIBotController)
	{
		if (!AthenaAIBotController || !AthenaAIBotController->PlayerBotPawn)
			return;

		AFortPlayerState* PlayerState = Cast<AFortPlayerState>(AthenaAIBotController->PlayerState);
		if (!PlayerState) return;

		UFortAbilitySystemComponent* AbilitySystemComponent = PlayerState->AbilitySystemComponent;
		if (!AbilitySystemComponent) return;

		UFortGameplayAbility* GameplayAbility = UGAB_Emote_Generic_C::GetDefaultObj();
		
		if (GameplayAbility)
		{
			AthenaAIBotController->PlayerBotPawn->bIsPlayingEmote = true;

			FGameplayAbilitySpecHandle Handle{ rand() };

			FGameplayAbilitySpec Spec{ -1, -1, -1 };
			Spec.Ability = GameplayAbility;
			Spec.Level = 0;
			Spec.InputID = -1;
			Spec.Handle = Handle;
			Spec.SourceObject = ChooseRandomEmote();

			FGameplayAbilitySpecHandle OutHandle;
			Abilities::GiveAbilityAndActivateOnce(AbilitySystemComponent, &OutHandle, Spec);
		}
	}

	void ChooseRandomTimeToAircraftJump(AFortAthenaAIBotController* AthenaAIBotController)
	{
		AFortGameStateAthena* GameStateAthena = Globals::GetGameState();
		if (!GameStateAthena || !AthenaAIBotController) return;

		const float TimeSeconds = UGameplayStatics::GetTimeSeconds(GameStateAthena);

		FAircraftFlightInfo FlightInfo = GameStateAthena->MapInfo->FlightInfos[0];

		float TimeTillDropStart = FlightInfo.TimeTillDropStart + TimeSeconds;
		float TimeTillDropEnd = FlightInfo.TimeTillDropEnd + TimeSeconds;

		float TimeToDrop = UKismetMathLibrary::RandomFloatInRange(TimeTillDropStart, TimeTillDropEnd);
		float TimeToThanks = UKismetMathLibrary::RandomFloatInRange(TimeTillDropStart, TimeToDrop);

		FN_LOG(LogBots, Warning, "TimeToDrop: %.2f, TimeToThanks: %.2f", TimeToDrop, TimeToThanks);

		BotAircraftParamsArray.Add(FBotAircraftParams(AthenaAIBotController, TimeToThanks, TimeToDrop));
	}

	bool IsValidCurrentTarget(AActor* CurrentTarget)
	{
		if (!CurrentTarget)
			return false;

		if (CurrentTarget->bActorIsBeingDestroyed)
			return false;

		AFortPickup* Pickup = Cast<AFortPickup>(CurrentTarget);

		if (Pickup)
		{
			if (Pickup->bPickedUp)
				return false;
		}

		return true;
	}

	void DropAllInventory(AFortAthenaAIBotController* AthenaAIBotController)
	{
		if (!AthenaAIBotController)
			return;

		AFortPlayerPawnAthena* PlayerBotPawn = AthenaAIBotController->PlayerBotPawn;
		AFortInventory* Inventory = AthenaAIBotController->Inventory;

		if (!PlayerBotPawn || !Inventory)
			return;

		TArray<UFortWorldItem*> WorldItemToDrops;

		for (int32 i = 0; i < Inventory->Inventory.ItemInstances.Num(); i++)
		{
			UFortWorldItem* WorldItem = Inventory->Inventory.ItemInstances[i];
			if (!WorldItem) continue;

			FFortItemEntry* ItemEntry = &WorldItem->ItemEntry;
			if (!ItemEntry) continue;

			UFortWorldItemDefinition* ItemDefinition = Cast<UFortWorldItemDefinition>(ItemEntry->ItemDefinition);
			if (!ItemDefinition) continue;

			if (!ItemDefinition->bCanBeDropped)
				continue;

			WorldItemToDrops.Add(WorldItem);
		}

		const float LootSpawnLocationX = 0.0f;
		const float LootSpawnLocationY = 0.0f;
		const float LootSpawnLocationZ = 70.0f;

		const FVector& SpawnLocation = PlayerBotPawn->K2_GetActorLocation() +
			PlayerBotPawn->GetActorForwardVector() * LootSpawnLocationX +
			PlayerBotPawn->GetActorRightVector() * LootSpawnLocationY +
			PlayerBotPawn->GetActorUpVector() * LootSpawnLocationZ;

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

			AFortPickup* Pickup = Inventory::SpawnPickup(WorldItemToDrop->ItemEntry.ItemDefinition, PlayerBotPawn, &NewItemEntry, SpawnLocation, true, EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::PlayerElimination);

			if (Pickup)
			{
				Pickup->PawnWhoDroppedPickup = PlayerBotPawn;
				Pickup->bCombinePickupsWhenTossCompletes = true;
				Pickup->TossPickup(FinalLocation, PlayerBotPawn, 0, true, EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::PlayerElimination);
			}
		}

		Inventory::RemoveAllItemsFromInventory(Inventory, true);
		Inventory::UpdateInventory(Inventory);
	}

	void OnDeathServer(AFortAthenaAIBotController* AthenaAIBotController, AController* InstigatedBy, AActor* DamageCauser)
	{
		if (!AthenaAIBotController || !InstigatedBy || !DamageCauser)
			return;

		AFortGameStateAthena* GameStateAthena = Globals::GetGameState();
		AFortGameModeAthena* GameModeAthena = Globals::GetGameMode();

		if (!GameStateAthena || !GameModeAthena)
			return;

		AFortPlayerPawnAthena* PlayerBotPawn = AthenaAIBotController->PlayerBotPawn;
		if (!PlayerBotPawn) return;

		AFortPlayerStateAthena* KilledPlayerState = Cast<AFortPlayerStateAthena>(AthenaAIBotController->PlayerState);

		if (!KilledPlayerState)
		{
			FN_LOG(LogBots, Warning, "Bots::OnDeathServer - Invalid KilledPlayerState!");
			return;
		}

		AFortPlayerControllerAthena* KillerPlayerController = Cast<AFortPlayerControllerAthena>(InstigatedBy);
		AFortPlayerStateAthena* KillerPlayerState = Cast<AFortPlayerStateAthena>(InstigatedBy ? InstigatedBy->PlayerState : nullptr);
		AFortPlayerPawnAthena* KillerPlayerPawn = Cast<AFortPlayerPawnAthena>(InstigatedBy ? InstigatedBy->Pawn : nullptr);

		const FGameplayTagContainer& DeathTags = *(FGameplayTagContainer*)(__int64(PlayerBotPawn) + 0x1320);
		const EDeathCause DeathCause = AFortPlayerStateAthena::ToDeathCause(DeathTags, PlayerBotPawn->bIsDBNO);
		const float Distance = KillerPlayerPawn ? KillerPlayerPawn->GetDistanceTo(PlayerBotPawn) : 0.0f;

		KilledPlayerState->DeathInfo.FinisherOrDowner = KillerPlayerState ? KillerPlayerState : KilledPlayerState;
		KilledPlayerState->DeathInfo.bDBNO = PlayerBotPawn->bIsDBNO;
		KilledPlayerState->DeathInfo.DeathCause = DeathCause;
		KilledPlayerState->DeathInfo.Distance = (DeathCause == EDeathCause::FallDamage) ? PlayerBotPawn->LastFallDistance : Distance;
		KilledPlayerState->DeathInfo.DeathLocation = PlayerBotPawn->K2_GetActorLocation();
		KilledPlayerState->DeathInfo.DeathTags = DeathTags;

		KilledPlayerState->DeathInfo.bInitialized = true;
		KilledPlayerState->OnRep_DeathInfo();

		if (KillerPlayerState && KillerPlayerState != KilledPlayerState)
		{
			KillerPlayerState->KillScore++;
			KillerPlayerState->ClientReportKill(KilledPlayerState);
			KillerPlayerState->OnRep_Kills();

			AFortTeamInfo* KillerPlayerTeam = KillerPlayerState->PlayerTeam;

			if (KillerPlayerTeam)
			{
				for (int32 i = 0; i < KillerPlayerTeam->TeamMembers.Num(); i++)
				{
					AController* KillerTeamController = KillerPlayerTeam->TeamMembers[i];
					if (!KillerTeamController) continue;

					AFortPlayerStateAthena* KillerTeamPlayerState = Cast<AFortPlayerStateAthena>(KillerTeamController->PlayerState);
					if (!KillerTeamPlayerState) continue;

					KillerTeamPlayerState->TeamKillScore++;
					KillerTeamPlayerState->OnRep_TeamKillScore();

					KillerTeamPlayerState->ClientReportTeamKill(KillerTeamPlayerState->TeamKillScore);
				}
			}

			if (!PlayerBotPawn->bIsDBNO)
			{
				// Apply Siphon Effect + Heal/Shield
				Functions::GiveSiphonBonus(KillerPlayerController, KillerPlayerPawn, KilledPlayerState->DeathInfo.DeathLocation);
			}
		}

		DropAllInventory(AthenaAIBotController);

		GameStateAthena->PlayerBotsLeft--;
		GameStateAthena->OnRep_PlayersLeft();

		FN_LOG(LogBots, Log, "Bots::OnDeathServer finish!");
	}

	void MoveTo(AFortAthenaAIBotController* AthenaAIBotController, AActor* CurrentTarget)
	{
		if (!AthenaAIBotController || !IsValidCurrentTarget(CurrentTarget))
			return;

		AFortPlayerPawnAthena* PlayerBotPawn = AthenaAIBotController->PlayerBotPawn;
		if (!PlayerBotPawn) return;

		const FVector& FinalLocation = CurrentTarget->K2_GetActorLocation();

		FVector DirectionToPlayer = UKismetMathLibrary::Normal((FinalLocation - PlayerBotPawn->K2_GetActorLocation()), 1.0f);

		PlayerBotPawn->AddMovementInput(DirectionToPlayer, 1.0f, true);

		AFortPickup* Pickup = Cast<AFortPickup>(CurrentTarget);

		if (Pickup)
		{
			const float Distance = Pickup->GetDistanceTo(PlayerBotPawn);

			if (Distance < 100)
			{
				float (*GenFlyTime)(AFortPickup * Pickup) = decltype(GenFlyTime)(Offsets621::GenFlyTime + uintptr_t(GetModuleHandle(0))); // [GALAXIA PATCH] 0x1456e40 en 8.51
				float InFlyTime = GenFlyTime(Pickup);
				PlayerBotPawn->ServerHandlePickup(Pickup, InFlyTime, FVector(), true);
			}
		}
	}

	void Tick()
	{
		AFortGameStateAthena* GameStateAthena = Globals::GetGameState();
		if (!GameStateAthena) return;

		const float TimeSeconds = UGameplayStatics::GetTimeSeconds(GameStateAthena);
		const EAthenaGamePhase GamePhase = GameStateAthena->GamePhase;

		// Check bot tick game phase for deplacement
		if (GamePhase < EAthenaGamePhase::Warmup || GamePhase > EAthenaGamePhase::SafeZones)
			return;

		TArray<AActor*> Actors;
		UGameplayStatics::GetAllActorsOfClass(GameStateAthena, AFortAthenaAIBotController::StaticClass(), &Actors);

		for (int32 i = 0; i < Actors.Num(); i++)
		{
			AFortAthenaAIBotController* AthenaAIBotController = Cast<AFortAthenaAIBotController>(Actors[i]);
			if (!AthenaAIBotController) continue;

			AFortPlayerStateAthena* PlayerStateAthena = Cast<AFortPlayerStateAthena>(AthenaAIBotController->PlayerState);
			if (!PlayerStateAthena) continue;

			AFortPlayerPawnAthena* PlayerBotPawn = AthenaAIBotController->PlayerBotPawn;

			if (GamePhase == EAthenaGamePhase::Warmup)
			{
				// Faire le random 
			}
			else if (GamePhase == EAthenaGamePhase::Aircraft)
			{
				for (int32 j = 0; j < BotAircraftParamsArray.Num(); j++)
				{
					FBotAircraftParams& BotAircraftParams = BotAircraftParamsArray[j];

					if (BotAircraftParams.AthenaAIBotController != AthenaAIBotController)
						continue;

					if (!PlayerStateAthena->bInAircraft)
						continue;

					if (BotAircraftParams.TimeToThanks != 0.0f)
					{
						if (BotAircraftParams.TimeToThanks < TimeSeconds)
						{
							PlayerStateAthena->bThankedBusDriver = true;
							PlayerStateAthena->OnRep_ThankedBusDriver();

							BotAircraftParams.TimeToThanks = 0.0f;
							continue;
						}
					}

					if (BotAircraftParams.TimeToDrop != 0.0f)
					{
						if (BotAircraftParams.TimeToDrop < TimeSeconds)
						{
							SpawnAIPawnFromAircraft(AthenaAIBotController);
							BotAircraftParams.TimeToDrop = 0.0f;
							return;
						}
					}
				}
			}

			MoveTo(AthenaAIBotController, AthenaAIBotController->CurrentTarget);
		}

		if (Actors.IsValid())
			Actors.Free();
	}
#endif // BOTS
}