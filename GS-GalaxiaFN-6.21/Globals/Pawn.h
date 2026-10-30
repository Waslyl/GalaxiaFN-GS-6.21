#pragma once

namespace Pawn
{
	void ServerHandlePickupHook(AFortPlayerPawn* PlayerPawn, AFortPickup* Pickup, float InFlyTime, const FVector& InStartDirection, bool bPlayPickupSound)
	{
		if (!Pickup)
			return;

		float FlyTime = InFlyTime / PlayerPawn->PickupSpeedMultiplier;

		FGuid LastEquippedWeaponGUID = PlayerPawn->CurrentWeapon ? PlayerPawn->CurrentWeapon->ItemEntryGuid : FGuid();

		Pickup->PickupLocationData.PickupGuid = LastEquippedWeaponGUID;

		// [OFFSET-TODO-6.21] SetPickupTarget (8.51 = 0x1467300)
		// void (*SetPickupTarget)(AFortPickup* Pickup, AFortPlayerPawn* PlayerPawn, float InFlyTime, const FVector& InStartDirection, bool bPlayPickupSound) = decltype(SetPickupTarget)(0x1467300 + uintptr_t(GetModuleHandle(0)));
		auto SetPickupTarget = Offsets621::Native<void(AFortPickup* Pickup, AFortPlayerPawn* PlayerPawn, float InFlyTime, const FVector& InStartDirection, bool bPlayPickupSound)>(Offsets621::SetPickupTarget, "SetPickupTarget", &Inventory::SetPickupTargetSDK);
		SetPickupTarget(Pickup, PlayerPawn, FlyTime, InStartDirection, bPlayPickupSound);
	}

	// [8.51-ONLY] ServerHandlePickupWithRequestedSwap n'existe pas en 6.21
	// void ServerHandlePickupWithRequestedSwapHook(AFortPlayerPawn* PlayerPawn, AFortPickup* Pickup, const FGuid& Swap, float InFlyTime, const FVector& InStartDirection, bool bPlayPickupSound)
	// {
		// if (!Pickup)
			// return;

		// AFortPlayerControllerAthena* PlayerControllerAthena = Cast<AFortPlayerControllerAthena>(PlayerPawn->Controller);
		// if (!PlayerControllerAthena) return;

		// // [8.51-ONLY] bTryPickupSwap n'existe pas en 6.21
		// // PlayerControllerAthena->bTryPickupSwap = true;

		// float FlyTime = InFlyTime / PlayerPawn->PickupSpeedMultiplier;

		// Pickup->PickupLocationData.PickupGuid = Swap;

		// void (*SetPickupTarget)(AFortPickup* Pickup, AFortPlayerPawn* PlayerPawn, float InFlyTime, const FVector& InStartDirection, bool bPlayPickupSound) = decltype(SetPickupTarget)(0x1467300 + uintptr_t(GetModuleHandle(0)));
		// SetPickupTarget(Pickup, PlayerPawn, FlyTime, InStartDirection, bPlayPickupSound);
	// }

	// [8.51-ONLY] ServerSendZiplineState et les champs de tyrolienne 8.51 (AuthoritativeValue, bJumped, ZiplineJump*) n'existent pas en 6.21
	// void ServerSendZiplineStateHook(AFortPlayerPawn* PlayerPawn, const FZiplinePawnState& InZiplineState)
	// {
		// if (InZiplineState.AuthoritativeValue > PlayerPawn->ZiplineState.AuthoritativeValue)
		// {
			// PlayerPawn->ZiplineState = InZiplineState;

			// bool bCancelJump = false;
			// if (AFortPlayerController* PlayerController = Cast<AFortPlayerController>(PlayerPawn->Controller))
			// {
				// UAntiCheatGalaxia* AntiCheatGalaxia = FindOrCreateAntiCheatGalaxia((AFortPlayerController*)PlayerPawn->Controller);

				// if (AntiCheatGalaxia)
				// {
					// if (AntiCheatGalaxia->AntiCheatZipline.bOldJumped && InZiplineState.bJumped)
					// {
						// AntiCheatGalaxia->AddDetection(30);
						// bCancelJump = true;
					// }

					// AntiCheatGalaxia->AntiCheatZipline.bOldJumped = InZiplineState.bJumped;
				// }
			// }

			// if (bCancelJump) // Annuler le jump si c'est détecter
				// return;

			// if (!PlayerPawn->ZiplineState.bIsZiplining && PlayerPawn->ZiplineState.bJumped && PlayerPawn->ZiplineState.Zipline)
			// {
				// float ZiplineJumpActivateDelay = PlayerPawn->ZiplineJumpActivateDelay.GetValueAtLevel(0);

				// if ((float)(UGameplayStatics::GetTimeSeconds(PlayerPawn) - PlayerPawn->ZiplineState.TimeZipliningBegan) < ZiplineJumpActivateDelay)
					// return;

				// EEvaluateCurveTableResult Result;

				// float ZiplineJumpDampening;
				// UDataTableFunctionLibrary::EvaluateCurveTableRow(PlayerPawn->ZiplineJumpDampening.CurveTable, PlayerPawn->ZiplineJumpDampening.RowName, 0, &Result, &ZiplineJumpDampening, FString());

				// float ZiplineJumpStrength;
				// UDataTableFunctionLibrary::EvaluateCurveTableRow(PlayerPawn->ZiplineJumpStrength.CurveTable, PlayerPawn->ZiplineJumpStrength.RowName, 0, &Result, &ZiplineJumpStrength, FString());

				// const FVector& Velocity = PlayerPawn->GetVelocity();

				// FVector LaunchVelocity = FVector({ -750.0f, -750.0f, ZiplineJumpStrength });

				// if ((float)(ZiplineJumpDampening * Velocity.X) >= -750.0f)
					// LaunchVelocity.X = fminf(ZiplineJumpDampening * Velocity.X, 750.0f);

				// if ((float)(ZiplineJumpDampening * Velocity.Y) >= -750.0f)
					// LaunchVelocity.Y = fminf(ZiplineJumpDampening * Velocity.Y, 750.0f);

				// PlayerPawn->LaunchCharacter(LaunchVelocity, false, false);
			// }

			// void (*OnRep_ZiplineState)(AFortPlayerPawn* PlayerPawn) = decltype(OnRep_ZiplineState)(0x16A2800 + uintptr_t(GetModuleHandle(0)));
			// OnRep_ZiplineState(PlayerPawn);
		// }
	// }

	AFortAthenaVehicle* ServerOnExitVehicleHook(AFortPlayerPawn* PlayerPawn)
	{
		
	}

	void ServerReviveFromDBNOHook(AFortPlayerPawnAthena* PlayerPawnAthena, AController* EventInstigator)
	{
		AFortPlayerStateAthena* PlayerStateAthena = Cast<AFortPlayerStateAthena>(PlayerPawnAthena->PlayerState);
		if (!PlayerStateAthena) return;

		UFortAbilitySystemComponent* AbilitySystemComponent = PlayerStateAthena->AbilitySystemComponent;

		// [OFFSET-TODO-6.21] CanReviveFromDBNO (8.51 = 0x10277e0)
		// bool (*CanReviveFromDBNO)(AFortPlayerPawnAthena* PlayerPawnAthena, AController* EventInstigator) = decltype(CanReviveFromDBNO)(0x10277e0 + uintptr_t(GetModuleHandle(0)));
		auto CanReviveFromDBNO = Offsets621::Native<bool(AFortPlayerPawnAthena* PlayerPawnAthena, AController* EventInstigator)>(Offsets621::CanReviveFromDBNO, "CanReviveFromDBNO");

		if (CanReviveFromDBNO(PlayerPawnAthena, EventInstigator) && AbilitySystemComponent)
		{
			FGameplayAbilitySpec* AbilitySpec = Abilities::FindAbilitySpecFromClass(AbilitySystemComponent, UGAB_AthenaDBNO_C::StaticClass());
			if (!AbilitySpec) return;

			AbilitySystemComponent->ServerEndAbility(AbilitySpec->Handle, AbilitySpec->ActivationInfo, AbilitySpec->ActivationInfo.PredictionKeyWhenActivated);
			AbilitySystemComponent->ServerCancelAbility(AbilitySpec->Handle, AbilitySpec->ActivationInfo);
			AbilitySystemComponent->ClientCancelAbility(AbilitySpec->Handle, AbilitySpec->ActivationInfo);

			PlayerStateAthena->DeathInfo.bDBNO = false;
			PlayerStateAthena->DeathInfo.bInitialized = true;
			PlayerStateAthena->OnRep_DeathInfo();

			bool bIsReviveSelf = EventInstigator == PlayerPawnAthena->Controller;
			TSubclassOf<UGameplayEffect> ReviveGameplayEffect{};

			if (bIsReviveSelf)
				ReviveGameplayEffect = PlayerPawnAthena->SelfReviveGameplayEffect;
			else
				ReviveGameplayEffect = PlayerPawnAthena->TeammateReviveGameplayEffect;

			FGameplayEffectContextHandle EffectContextHandle = AbilitySystemComponent->MakeEffectContext();
			FGameplayEffectSpecHandle EffectSpecHandle = AbilitySystemComponent->MakeOutgoingSpec(ReviveGameplayEffect, 0, EffectContextHandle);

			PlayerPawnAthena->bIsDBNO = false;
			PlayerPawnAthena->bPlayedDying = false;
			PlayerPawnAthena->bIsDying = false;

			float ReviveHealth = PlayerPawnAthena->SetByCallerReviveHealth.GetValueAtLevel(0);

			if (ReviveHealth > 0.0f)
			{
				PlayerPawnAthena->SetHealth(ReviveHealth);
			}

			PlayerPawnAthena->OnRep_IsDBNO();

			AbilitySystemComponent->BP_ApplyGameplayEffectSpecToSelf(EffectSpecHandle);

			AFortPlayerControllerZone* PlayerControllerZone = Cast<AFortPlayerControllerZone>(PlayerPawnAthena->Controller);

			if (PlayerControllerZone)
			{
				PlayerControllerZone->ClientOnPawnRevived(EventInstigator);
			}

			PlayerPawnAthena->DBNORevivalStacking = 0;
		}
	}

	// 1:1 auto pickup frerot
	void OnCapsuleBeginOverlapHook(AFortPlayerPawn* PlayerPawn, FFrame& Stack, void* Ret)
	{
		UPrimitiveComponent* OverlappedComp = nullptr;
		AActor* OtherActor = nullptr;
		UPrimitiveComponent* OtherComp = nullptr;
		int32 OtherBodyIndex = -1;
		bool bFromSweep = true;
		FHitResult SweepResult = FHitResult();

		Stack.StepCompiledIn(&OverlappedComp);
		Stack.StepCompiledIn(&OtherActor);
		Stack.StepCompiledIn(&OtherComp);
		Stack.StepCompiledIn(&OtherBodyIndex);
		Stack.StepCompiledIn(&bFromSweep);
		Stack.StepCompiledIn(&SweepResult);

		Stack.Code += Stack.Code != nullptr;

		AFortPlayerController* PlayerController = Cast<AFortPlayerController>(PlayerPawn->Controller);
		if (!PlayerController) return;

		AFortPickup* Pickup = Cast<AFortPickup>(OtherActor);
		if (!Pickup) return;

		float RepickupDelay = PlayerPawn->AutoPickupDropRepickupDelay.GetValueAtLevel(0);

		if (!PlayerPawn->bIsDBNO ||
			!PlayerPawn->bIsSkydiving)
		{
			if (Pickup->bPickedUp || !Pickup->bWeaponsCanBeAutoPickups)
				return;

			if (!Pickup->bServerStoppedSimulation && (Pickup->PawnWhoDroppedPickup == PlayerPawn))
				return;

			if (Pickup->PawnWhoDroppedPickup == PlayerPawn)
			{
				const float LastDropPickupTime = Pickup->LastDropPickupTime();
				const float TimeSeconds = UGameplayStatics::GetTimeSeconds(PlayerPawn);

				if ((TimeSeconds - LastDropPickupTime) <= RepickupDelay)
					return;
			}

			UFortWorldItemDefinition* WorldItemDefinition = Cast<UFortWorldItemDefinition>(Pickup->PrimaryPickupItemEntry.ItemDefinition);
			if (!WorldItemDefinition) return;

			int32 ItemQuantity = UFortKismetLibrary::K2_GetItemQuantityOnPlayer(PlayerController, WorldItemDefinition);

			for (int32 i = 0; i < PlayerPawn->QueuedAutoPickups.Num(); i++)
			{
				AFortPickup* QueuedAutoPickup = PlayerPawn->QueuedAutoPickups[i];
				if (!QueuedAutoPickup) continue;

				UFortWorldItemDefinition* QueuedWorldItemDefinition = Cast<UFortWorldItemDefinition>(QueuedAutoPickup->PrimaryPickupItemEntry.ItemDefinition);
				if (!QueuedWorldItemDefinition) continue;

				if (QueuedWorldItemDefinition != WorldItemDefinition)
					continue;

				ItemQuantity += QueuedAutoPickup->PrimaryPickupItemEntry.Count;
			}

			if (ItemQuantity >= WorldItemDefinition->MaxStackSize && !WorldItemDefinition->bAllowMultipleStacks)
				return;

			PlayerPawn->QueuedAutoPickups.Add(Pickup);

			// [OFFSET-TODO-6.21] GenFlyTime (8.51 = 0x1456e40)
			// float (*GenFlyTime)(AFortPickup* Pickup) = decltype(GenFlyTime)(0x1456e40 + uintptr_t(GetModuleHandle(0)));
			auto GenFlyTime = Offsets621::Native<float(AFortPickup* Pickup)>(Offsets621::GenFlyTime, "GenFlyTime");
			float InFlyTime = GenFlyTime(Pickup);
			PlayerPawn->ServerHandlePickup(Pickup, InFlyTime, FVector(), true);
		}
	}

	void (*OnDeathServerOG)(AFortPlayerPawnAthena* PlayerPawnAthena, float Damage, const FGameplayTagContainer& DamageTags, const FVector& Momentum, const FHitResult& HitInfo, AController* InstigatedBy, AActor* DamageCauser, const FGameplayEffectContextHandle& EffectContext);
	void OnDeathServer(AFortPlayerPawnAthena* PlayerPawnAthena, float Damage, const FGameplayTagContainer& DamageTags, const FVector& Momentum, const FHitResult& HitInfo, AController* InstigatedBy, AActor* DamageCauser, const FGameplayEffectContextHandle& EffectContext)
	{
		AFortGameStateAthena* GameStateAthena = Globals::GetGameState();
		AFortGameModeAthena* GameModeAthena = Globals::GetGameMode();

		if (!GameStateAthena || !GameModeAthena)
			return OnDeathServerOG(PlayerPawnAthena, Damage, DamageTags, Momentum, HitInfo, InstigatedBy, DamageCauser, EffectContext);

#ifdef BOTS
		if (AFortAthenaAIBotController* AthenaAIBotController = Cast<AFortAthenaAIBotController>(PlayerPawnAthena->Controller))
		{
			Bots::OnDeathServer(AthenaAIBotController, InstigatedBy, DamageCauser);
			return OnDeathServerOG(PlayerPawnAthena, Damage, DamageTags, Momentum, HitInfo, InstigatedBy, DamageCauser, EffectContext);
		}
#endif // BOTS

		AFortPlayerControllerAthena* KilledPlayerController = Cast<AFortPlayerControllerAthena>(PlayerPawnAthena->Controller);
		AFortPlayerStateAthena* KilledPlayerState = Cast<AFortPlayerStateAthena>(PlayerPawnAthena->PlayerState);

		if (!KilledPlayerController || !KilledPlayerState)
		{
			FN_LOG(LogPawn, Log, "OnDeathServer - Invalid KilledPlayerController or KilledPlayerState");
			return OnDeathServerOG(PlayerPawnAthena, Damage, DamageTags, Momentum, HitInfo, InstigatedBy, DamageCauser, EffectContext);
		}

		AFortPlayerControllerAthena* KillerPlayerController = Cast<AFortPlayerControllerAthena>(InstigatedBy);
		AFortPlayerStateAthena* KillerPlayerState = Cast<AFortPlayerStateAthena>(InstigatedBy ? InstigatedBy->PlayerState : nullptr);
		AFortPlayerPawnAthena* KillerPlayerPawn = Cast<AFortPlayerPawnAthena>(InstigatedBy ? InstigatedBy->Pawn : nullptr);

		// [OFFSET-TODO-6.21] DeathTags (8.51 = +0x1320, champ natif non expose par le dump) : conteneur vide tant que l'offset n'est pas trouve
		// const FGameplayTagContainer& DeathTags = *(FGameplayTagContainer*)(__int64(PlayerPawnAthena) + 0x1320);
		static FGameplayTagContainer EmptyDeathTags{};
		const FGameplayTagContainer& DeathTags = Offsets621::AFortPlayerPawnAthena_DeathTags ? *(FGameplayTagContainer*)(__int64(PlayerPawnAthena) + Offsets621::AFortPlayerPawnAthena_DeathTags) : EmptyDeathTags;
		const EDeathCause DeathCause = AFortPlayerStateAthena::ToDeathCause(DeathTags, PlayerPawnAthena->bIsDBNO);
		const float Distance = KillerPlayerPawn ? KillerPlayerPawn->GetDistanceTo(PlayerPawnAthena) : 0.0f;

		KilledPlayerState->DeathInfo.FinisherOrDowner = KillerPlayerState ? KillerPlayerState : KilledPlayerState;
		KilledPlayerState->DeathInfo.bDBNO = PlayerPawnAthena->bIsDBNO;
		KilledPlayerState->DeathInfo.DeathCause = DeathCause;
		KilledPlayerState->DeathInfo.Distance = (DeathCause == EDeathCause::FallDamage) ? PlayerPawnAthena->LastFallDistance : Distance;
		KilledPlayerState->DeathInfo.DeathLocation = PlayerPawnAthena->K2_GetActorLocation();
		// [8.51-ONLY] FDeathInfo::DeathTags n'existe pas en 6.21
		// KilledPlayerState->DeathInfo.DeathTags = DeathTags;

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
					// [8.51-ONLY] OnRep_TeamKillScore n'existe pas en 6.21
					// KillerTeamPlayerState->OnRep_TeamKillScore();

					KillerTeamPlayerState->ClientReportTeamKill(KillerTeamPlayerState->TeamKillScore);
				}
			}

			if (!PlayerPawnAthena->bIsDBNO)
			{
				// Apply Siphon Effect + Heal/Shield
				Functions::GiveSiphonBonus(KillerPlayerController, KillerPlayerPawn, KilledPlayerState->DeathInfo.DeathLocation);
			}
		}

		// Quand le respawn est off et qu'il n'est pas en dbno
		// [8.51-ONLY] IsRespawningAllowed n'existe pas en 6.21 -> bAllowRespawning
		// if (!GameStateAthena->IsRespawningAllowed(KilledPlayerState) && !PlayerPawnAthena->bIsDBNO)
		if (!GameStateAthena->bAllowRespawning && !PlayerPawnAthena->bIsDBNO)
		{
			AFortPlayerStateAthena* CorrectKillerPlayerState = (KillerPlayerState && KillerPlayerState == KilledPlayerState) ? nullptr : KillerPlayerState;
			UFortWeaponItemDefinition* KillerWeaponItemDefinition = nullptr;

			if (DamageCauser)
			{
				AFortProjectileBase* ProjectileBase = Cast<AFortProjectileBase>(DamageCauser);
				AFortWeapon* Weapon = Cast<AFortWeapon>(DamageCauser);

				if (ProjectileBase)
				{
					AFortWeapon* ProjectileBaseWeapon = Cast<AFortWeapon>(ProjectileBase->Owner);

					if (ProjectileBaseWeapon)
						KillerWeaponItemDefinition = ProjectileBaseWeapon->WeaponData;
				}
				else if (Weapon)
					KillerWeaponItemDefinition = Weapon->WeaponData;
			}

			bool bMatchEnded = GameModeAthena->HasMatchEnded();
			int32 OldPlayersLeft = GameStateAthena->PlayersLeft;

			// RemoveFromAlivePlayers (8.51 = 0xfae8c0) : trouve en 6.21 par pattern, voir GameMode::RemoveFromAlivePlayers
			// void (*RemoveFromAlivePlayers)(AFortGameModeAthena* GameModeAthena, AFortPlayerControllerAthena* PlayerControllerAthena, AFortPlayerStateAthena* PlayerStateAthena, AFortPlayerPawnAthena* PlayerPawnAthena, UFortWeaponItemDefinition* WeaponItemDefinition, EDeathCause DeathCause, char a7) = decltype(RemoveFromAlivePlayers)(0xfae8c0 + uintptr_t(GetModuleHandle(0)));
			// auto RemoveFromAlivePlayers = Offsets621::Native<void(AFortGameModeAthena* GameModeAthena, AFortPlayerControllerAthena* PlayerControllerAthena, AFortPlayerStateAthena* PlayerStateAthena, AFortPlayerPawnAthena* PlayerPawnAthena, UFortWeaponItemDefinition* WeaponItemDefinition, EDeathCause DeathCause, char a7)>(Offsets621::RemoveFromAlivePlayers, "RemoveFromAlivePlayers");
			auto& RemoveFromAlivePlayers = GameMode::RemoveFromAlivePlayers; // [GALAXIA PATCH] 6.21 : version trouvee par pattern (RVA 0xC8ADC0)
			RemoveFromAlivePlayers(GameModeAthena, KilledPlayerController, CorrectKillerPlayerState, KillerPlayerPawn, KillerWeaponItemDefinition, DeathCause, 0);

			// Si c'est après le Warmup (Lobby)
			if (GameStateAthena->GamePhase > EAthenaGamePhase::Warmup)
			{
				auto SendMatchReport = [&](AFortPlayerControllerAthena* PlayerControllerAthena, int32 Place) -> void
					{
						UAthenaPlayerMatchReport* PlayerMatchReport = PlayerControllerAthena->MatchReport;

						if (PlayerMatchReport )
						{
							if (PlayerMatchReport->bHasTeamStats)
							{
								FAthenaMatchTeamStats& TeamStats = PlayerMatchReport->TeamStats;
								TeamStats.Place = Place;
								TeamStats.TotalPlayers = GameStateAthena->TotalPlayers;

								PlayerControllerAthena->ClientSendTeamStatsForPlayer(TeamStats);
							}

							if (PlayerMatchReport->bHasMatchStats)
							{
								FAthenaMatchStats& MatchStats = PlayerMatchReport->MatchStats;

								PlayerControllerAthena->ClientSendMatchStatsForPlayer(MatchStats);
							}

							if (PlayerMatchReport->bHasRewards)
							{
								FAthenaRewardResult& EndOfMatchResults = PlayerMatchReport->EndOfMatchResults;
								EndOfMatchResults.LevelsGained = 5;
								EndOfMatchResults.BookLevelsGained = 10;
								EndOfMatchResults.TotalSeasonXpGained = 15; // ça fontionne
								EndOfMatchResults.TotalBookXpGained = 20;
								EndOfMatchResults.PrePenaltySeasonXpGained = 25;

								PlayerControllerAthena->ClientSendEndBattleRoyaleMatchForPlayer(true, EndOfMatchResults);
							}

							PlayerControllerAthena->ClientSendEndMatchReportHeartbeat();
						}
					};

				if (KillerPlayerState) // Verif que c'est bien la personne qui c'est fait tuer
				{
					AFortTeamInfo* KilledPlayerTeam = KilledPlayerState->PlayerTeam;

					if (KilledPlayerTeam && KilledPlayerState->IsSquadDead()) // Verif que sa team est bien morte pour envoyer le match report
					{
						for (int32 i = 0; i < KilledPlayerTeam->TeamMembers.Num(); i++)
						{
							AFortPlayerControllerAthena* KilledTeamController = Cast<AFortPlayerControllerAthena>(KilledPlayerTeam->TeamMembers[i]);
							if (!KilledTeamController) continue;

							AFortPlayerStateAthena* KilledTeamPlayerState = Cast<AFortPlayerStateAthena>(KilledTeamController->PlayerState);
							if (!KilledTeamPlayerState) continue;

							SendMatchReport(KilledTeamController, OldPlayersLeft);
						}
					}
				}

				// A Check je suis vrm pas sûr du tout (bah ça marche pas)
				// [8.51-ONLY] ABuildingGameplayActorSpawnChip (cartes de Reboot) n'existe pas en 6.21
				// if (!KilledPlayerState->IsSquadDead())
				// {
					// UClass* SCMachinePickupClass = FindObjectFast<UClass>("/Game/Athena/Items/EnvironmentalItems/SCMachine/BGA_Athena_SCMachine_Pickup.BGA_Athena_SCMachine_Pickup_C");
					// ABuildingGameplayActorSpawnChip* GameplayActorSpawnChip = Util::SpawnActor<ABuildingGameplayActorSpawnChip>(SCMachinePickupClass, KilledPlayerState->DeathInfo.DeathLocation);

					// if (GameplayActorSpawnChip)
					// {
						// GameplayActorSpawnChip->OwnerPlayerController = KilledPlayerController;
						// GameplayActorSpawnChip->OwnerPlayerId = KilledPlayerState->UniqueId;
						// GameplayActorSpawnChip->OwnerPlayerState = KilledPlayerState;

						// GameplayActorSpawnChip->RebootCardReplicatedState.ChipExpirationServerStartTime = UGameplayStatics::GetTimeSeconds(PlayerPawnAthena);
						// GameplayActorSpawnChip->RebootCardReplicatedState.PlayerState = KilledPlayerState;
						// GameplayActorSpawnChip->OnRep_RebootCardReplicatedState();

						// GameplayActorSpawnChip->SquadId = KilledPlayerState->SquadId;

						// FN_LOG(LogPawn, Log, "Spawning GameplayActorSpawnChip [%s], SquadId: %i", GameplayActorSpawnChip->GetName().c_str(), KilledPlayerState->SquadId);
					// }
				// }

				if (GameStateAthena->TeamsLeft <= 1 && !bMatchEnded) // Verif que c'est bien la dernière team pour ensuite le faire le top 1
				{
					AFortTeamInfo* PlayerTeam = KillerPlayerState ? KillerPlayerState->PlayerTeam : KilledPlayerState->PlayerTeam;

					if (PlayerTeam)
					{
						for (int32 i = 0; i < PlayerTeam->TeamMembers.Num(); i++)
						{
							AFortPlayerControllerAthena* TeamController = Cast<AFortPlayerControllerAthena>(PlayerTeam->TeamMembers[i]);
							if (!TeamController) continue;

							AFortPlayerStateAthena* TeamPlayerState = Cast<AFortPlayerStateAthena>(TeamController->PlayerState);
							if (!TeamPlayerState) continue;

							TeamPlayerState->Place = GameStateAthena->TeamsLeft;
							TeamPlayerState->OnRep_Place();

							APawn* FinisherPawn = KillerPlayerPawn ? KillerPlayerPawn : PlayerPawnAthena;

							TeamController->ClientNotifyWon(FinisherPawn, KillerWeaponItemDefinition, DeathCause);
							TeamController->ClientNotifyTeamWon(FinisherPawn, KillerWeaponItemDefinition, DeathCause);

							int32 PlayersLeft = KillerPlayerPawn ? GameStateAthena->PlayersLeft : OldPlayersLeft;

							SendMatchReport(TeamController, PlayersLeft);
						}
					}
				}
			}

			KilledPlayerController->ServerDropAllItems(nullptr);
			Inventory::RemoveAllItemsFromInventory(KilledPlayerController->WorldInventory);
			Inventory::UpdateInventory(KilledPlayerController->WorldInventory);
		}

		OnDeathServerOG(PlayerPawnAthena, Damage, DamageTags, Momentum, HitInfo, InstigatedBy, DamageCauser, EffectContext);
	}

	void ServerUpdatePhysicsParamsHook(AFortAthenaVehicle* PhysicsPawn, FFrame& Stack, void* Ret) // AFortPhysicsPawn n'existe pas en 6.21 : la fonction est sur AFortAthenaVehicle
	{
		FReplicatedAthenaVehiclePhysicsState InState; // FReplicatedPhysicsPawnState en 8.51

		Stack.StepCompiledIn(&InState);

		Stack.Code += Stack.Code != nullptr;

		UPrimitiveComponent* RootComponent = static_cast<UPrimitiveComponent*>(PhysicsPawn->RootComponent);

		if (RootComponent)
		{
			// From Reboot
			InState.Rotation.X -= 2.5f;
			InState.Rotation.Y /= 0.3f;
			InState.Rotation.Z -= -2.0f;
			InState.Rotation.W /= -1.2f;

			FTransform Transform{};
			Transform.Translation = InState.Translation;
			Transform.Rotation = InState.Rotation;
			Transform.Scale3D = FVector{ 1, 1, 1 };

			RootComponent->K2_SetWorldTransform(Transform, false, nullptr, true);
			RootComponent->SetPhysicsLinearVelocity(InState.LinearVelocity, false, FName());
			RootComponent->SetPhysicsAngularVelocity(InState.AngularVelocity, false, FName());
		}
	}

	void MovingEmoteStoppedHook(AFortPawn* Pawn, FFrame& Stack, void* Ret)
	{
		Stack.Code += Stack.Code != nullptr;

		Pawn->bMovingEmote = false;
		// [8.51-ONLY] bMovingEmoteForwardOnly n'existe pas en 6.21
		// Pawn->bMovingEmoteForwardOnly = false;
	}

	void (*OnEnteredVehicleOG)(AFortPlayerPawn* PlayerPawn);
	void OnEnteredVehicleHook(AFortPlayerPawn* PlayerPawn)
	{
		OnEnteredVehicleOG(PlayerPawn);

		AFortPlayerControllerAthena* PlayerControllerAthena = Cast<AFortPlayerControllerAthena>(PlayerPawn->Controller);
		AFortAthenaVehicle* Vehicle = PlayerPawn->VehicleStateLocal.Vehicle;

		if (!PlayerControllerAthena || !Vehicle) 
			return;

		int32 SeatIndex = PlayerPawn->VehicleStateLocal.SeatIndex;
		UFortWeaponItemDefinition* VehicleWeaponItemDefinition = Functions::GetVehicleWeaponItemDefinition(Vehicle->Class, SeatIndex);

		if (VehicleWeaponItemDefinition)
			Functions::EquipVehicleWeapon(PlayerControllerAthena, VehicleWeaponItemDefinition);
	}

	void InitPawn()
	{
		AFortPlayerPawnAthena* FortPlayerPawnAthenaDefault = AFortPlayerPawnAthena::GetDefaultObj();
		UClass* FortPlayerPawnAthenaClass = AFortPlayerPawnAthena::StaticClass();

		// Hook par ProcessEvent (parametres verifies au demarrage) (8.51 = 0xDD0 / 8)
		// MinHook::HookVTable(FortPlayerPawnAthenaDefault, 0xDD0 / 8, ServerHandlePickupHook, nullptr, "AFortPlayerPawn::ServerHandlePickup");
		MinHook::HookRPC<ServerHandlePickupHook>(FortPlayerPawnAthenaDefault, "FortPlayerPawn", "ServerHandlePickup", nullptr, 0xDD0 / 8);
		// [8.51-ONLY] ServerHandlePickupWithRequestedSwap n'existe pas en 6.21
		// MinHook::HookVTable(FortPlayerPawnAthenaDefault, 0xDC0 / 8, ServerHandlePickupWithRequestedSwapHook, nullptr, "AFortPlayerPawn::ServerHandlePickupWithRequestedSwap");
		// [8.51-ONLY] ServerSendZiplineState n'existe pas en 6.21
		// MinHook::HookVTable(FortPlayerPawnAthenaDefault, 0xE28 / 8, ServerSendZiplineStateHook, nullptr, "AFortPlayerPawn::ServerSendZiplineState");
		// Hook par ProcessEvent (parametres verifies au demarrage) (8.51 = 0xD30 / 8)
		// MinHook::HookVTable(FortPlayerPawnAthenaDefault, 0xD30 / 8, ServerReviveFromDBNOHook, nullptr, "AFortPlayerPawn::ServerReviveFromDBNO");
		MinHook::HookRPC<ServerReviveFromDBNOHook>(FortPlayerPawnAthenaDefault, "FortPlayerPawn", "ServerReviveFromDBNO", nullptr, 0xD30 / 8);

		UFunction* OnCapsuleBeginOverlapFunc = FortPlayerPawnAthenaClass->GetFunction("FortPlayerPawn", "OnCapsuleBeginOverlap");
		MinHook::HookFunctionExec(OnCapsuleBeginOverlapFunc, OnCapsuleBeginOverlapHook, nullptr);

		// [OFFSET-TODO-6.21] OnEnteredVehicle (8.51 = 0x1af3570)
		// MH_CreateHook((LPVOID)(InSDKUtils::GetImageBase() + 0x1af3570), OnEnteredVehicleHook, (LPVOID*)(&OnEnteredVehicleOG));
		// MH_EnableHook((LPVOID)(InSDKUtils::GetImageBase() + 0x1af3570));
		MinHook::HookNative(Offsets621::OnEnteredVehicle, "OnEnteredVehicle", OnEnteredVehicleHook, (void**)(&OnEnteredVehicleOG));

		UFunction* MovingEmoteStoppedFunc = FortPlayerPawnAthenaClass->GetFunction("FortPawn", "MovingEmoteStopped");
		MinHook::HookFunctionExec(MovingEmoteStoppedFunc, MovingEmoteStoppedHook, nullptr);

		// [OFFSET-TODO-6.21] OnDeathServer (8.51 = 0x1ab3df0)
		// MH_CreateHook((LPVOID)(InSDKUtils::GetImageBase() + 0x1ab3df0), OnDeathServer, (LPVOID*)(&OnDeathServerOG));
		// MH_EnableHook((LPVOID)(InSDKUtils::GetImageBase() + 0x1ab3df0));
		// MinHook::HookNative(Offsets621::OnDeathServer, "OnDeathServer", OnDeathServer, (void**)(&OnDeathServerOG));
		// [GALAXIA PATCH] 6.21 : AFortPawn::OnDeathServer est un event Blueprint (appele par ProcessEvent a la mort du pawn) :
		// hook par ProcessEvent, sans adresse. L'OG rappelle l'event (override Blueprint du pawn).
		MinHook::HookRPC<OnDeathServer>(FortPlayerPawnAthenaDefault, "FortPawn", "OnDeathServer", (void**)(&OnDeathServerOG));

		AFortAthenaVehicle* AFortPhysicsPawnDefault = AFortAthenaVehicle::GetDefaultObj(); // AFortPhysicsPawn en 8.51
		UClass* AFortPhysicsPawnClass = AFortAthenaVehicle::StaticClass(); // AFortPhysicsPawn en 8.51

		UFunction* ServerUpdatePhysicsParamsFunc = AFortPhysicsPawnClass->GetFunction("FortAthenaVehicle", "ServerUpdatePhysicsParams"); // "FortPhysicsPawn" en 8.51
		MinHook::HookFunctionExec(ServerUpdatePhysicsParamsFunc, ServerUpdatePhysicsParamsHook, nullptr);

		FN_LOG(LogInit, Log, "InitPawn Success!");
	}
}