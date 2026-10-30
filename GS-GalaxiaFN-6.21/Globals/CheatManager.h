#pragma once

namespace CheatManager
{
	/*
		📌 Liste des Cheats Disponibles 

		Cheats pour Joueur:

		Cheat <Texte> : Exécuté par le client pour appliquer un cheat au joueur appelant.
		CheatAll <Texte> : Exécuté par le client/serveur pour appliquer un cheat à tous les joueurs du serveur.
		CheatScript : Exécute les commandes listées dans la section CheatScript.ScriptName du fichier defaultgame.ini.

		Badass : Active une série de cheats rendant le joueur surpuissant (God, GiveGoodWeapons, BuildFree, ToggleInfiniteStamina, ToggleInfiniteAmmo, ToggleNoCooldown, ToggleInfiniteDurability). Attention : si certains sont déjà activés, ils seront désactivés.
		God : Active/désactive l’invincibilité du joueur.
		ToggleUnlimitedHealth : Active/désactive l'impossibilité de mourir (le joueur prend des dégâts mais ne peut pas descendre à 0 PV).
		GiveGoodWeapons : Donne au joueur des armes puissantes (actuellement un marteau tier 3, un bon fusil à pompe et un revolver six-coups).
		FreeBuild : Active/désactive la suppression des ressources requises pour construire.
		ToggleInfiniteStamina : Active/désactive l'endurance infinie.
		ToggleNoCoolDown : Supprime les délais de rechargement des compétences.
		ToggleShotLogging : Active/désactive la journalisation avancée de la précision des tirs.
		PrintShotReport : Affiche un rapport détaillé des tirs (à exécuter côté client).
		SetRespawnTimerOverride <Temps> : Définit le temps de réapparition en secondes. Mettre une valeur négative (ex: -1) pour annuler.
		Suicide : Tue instantanément le personnage du joueur (respawn selon les règles du jeu).
		TestStun : Applique un effet d'étourdissement au joueur.
		TestKnockback : Applique un effet de recul au joueur.
		SwitchTeam : Change l'équipe PvP du joueur.
		TogglePickers : Active/désactive les roues de sélection d'armes et de pièges.
		SwapPickerThumbstick : Inverse la commande de sélection entre le stick gauche et droit.
		Voir les cheats d'inventaire pour les munitions, la durabilité, etc.

		Cheats sur les Capacités:

		DebugAbilityTags : Affiche les balises de gameplay d’un acteur directement à l’écran.
		DebugAttribute <Attribut> : Affiche la valeur d’un attribut pour tous les acteurs proches.
		DebugAttributeModifiers : Active/désactive l’affichage des modificateurs d’attributs.

		Cheats sur la Caméra et l'Interface:

		ToggleDebugCamera : Active une caméra libre pour explorer le monde (appuyer sur Backspace pour masquer l’interface).
		ShowHUD : Active/désactive l’affichage de l’interface utilisateur et des menus associés.
		BeautyMode : Améliore la résolution, désactive l’interface, augmente la distance de rendu et active la caméra de debug (à activer uniquement en contrôlant le joueur).
		Touches spéciales : Insert, Delete, Home, End, Page Up, et Page Down changent les modes de caméra.
		SetBuildingCullDistance <Distance> : Définit la distance d'affichage des structures.

		Cheats Jour/Nuit:

		SetDayPhase <Phase> : Définit la phase de la journée (nom ou index).
		SetTimeOfDay <Heure> : Définit l'heure (ex: 1p, 1:15pm, 1800, 12a, etc.).
		SetTimeOfDaySpeed <Vitesse> : Modifie la vitesse de passage du temps (valeur par défaut : 60, soit 1 min IRL = 1h en jeu).
		FFToTimeOfDay <Heure> <Vitesse> : Avance rapidement le temps jusqu'à une heure donnée (⚠️ Des valeurs trop élevées peuvent causer des erreurs).
		ToggleSkipNight : Active/désactive le saut automatique de la nuit vers l’aube.
		SetTimeOfDayInHours <Heure> : Définit l’heure de la journée avec une valeur décimale entre 0 et 24.
		SetTimeOfDayAbsoluteSpeed <Vitesse> : Définit la vitesse de défilement du temps (par défaut 60).

		Cheats de Construction:

		BuildFree : Active/désactive la construction gratuite.
		BuildWith <Matériau> <Niveau> : Change le matériau et/ou le niveau de construction (wood, stone, metal, w, s, m, niveaux 1, 2, 3).
		GiveResources <Quantité> : Donne <Quantité> de chaque ressource.
		GiveWood <Quantité> : Donne <Quantité> de bois.
		GiveStone <Quantité> : Donne <Quantité> de pierre.
		GiveMetal <Quantité> : Donne <Quantité> de métal.
		DebugStructuralSupport : Active/désactive l'affichage du support structurel en mode debug.

		Cheats de Personnalisation:

		OpenClassSelectScreen : Ouvre le menu de sélection du héros/classe.
		RandomizeCharacter <Genre> : Randomise l’apparence du personnage (genre optionnel : M, Male, F, Female).
		SwitchToNextPart <Partie> : Change la partie du corps suivante (Legs, Torso, Head).
		SwitchToPreviousPart <Partie> : Change la partie du corps précédente (Legs, Torso, Head).
		ListCharacterParts <Type> <Genre> : Liste les parties du personnage disponibles (ex: M Legs pour voir les jambes masculines). All liste tout.
		ToggleGender : Alterne entre personnage masculin et féminin.



		PS: 

		Alors faut savoir que sur les anciennes version du jeu y'a littéralement une doc sur les cheats du jeu,
		j'ai tout traduit avec chat gpt et j'ai mais ça ici, J'ai tester sur la 1.1 et en 8.51 elle n'existe plus,
		aussi y'a moyen que des commandes ont changer de noms ou simplement n'existe plus. En plus faut savoir
		que fortnite à littéralement tous les cheat sur le client (Faut tout refaire).
	*/

	void BuildFreeHook(UFortCheatManager* CheatManager, FFrame& Stack, void* Ret)
	{
		Stack.Code += Stack.Code != nullptr;

		AFortPlayerController* PlayerController = Cast<AFortPlayerController>(CheatManager->Outer);
		if (!PlayerController) return;

		PlayerController->bBuildFree = ~PlayerController->bBuildFree;

		std::string BuildFreeString = "BuildFree is " + std::to_string(PlayerController->bBuildFree);
		PlayerController->ClientMessage(std::wstring(BuildFreeString.begin(), BuildFreeString.end()).c_str(), FName(), 1);
	}

	void ReviveFromDBNOHook(UFortCheatManager* CheatManager, FFrame& Stack, void* Ret)
	{
		Stack.Code += Stack.Code != nullptr;

		AFortPlayerController* PlayerController = Cast<AFortPlayerController>(CheatManager->Outer);
		if (!PlayerController) return;

		AFortPlayerPawn* MyFortPawn = PlayerController->MyFortPawn;

		if (MyFortPawn && MyFortPawn->bIsDBNO)
			MyFortPawn->ServerReviveFromDBNO(nullptr);
	}

	void SimulateLootHook(UFortCheatManager* CheatManager, FFrame& Stack, void* Ret)
	{
		FName TierGroup; 
		int32 LootTier; 
		int32 LootLevel;

		Stack.StepCompiledIn(&TierGroup);
		Stack.StepCompiledIn(&LootTier);
		Stack.StepCompiledIn(&LootLevel);

		Stack.Code += Stack.Code != nullptr;

		AFortPlayerController* PlayerController = Cast<AFortPlayerController>(CheatManager->Outer);
		if (!PlayerController) return;

		FName LootTierKey;
		Loots::PickLootTierKeyAndLootTierFromTierGroup(&LootTierKey, &LootTier, TierGroup, LootLevel, 0, LootTier, FGameplayTagContainer());

		FN_LOG(LogCheatManager, Log, "Simulating loot roll on Table %s, Level %d", TierGroup.ToString().c_str(), LootTier);

		std::string SimulateLootString = "Simulating loot roll on Table " + TierGroup.ToString() + ", Level " + std::to_string(LootTier);
		PlayerController->ClientMessage(std::wstring(SimulateLootString.begin(), SimulateLootString.end()).c_str(), FName(), 1);

		TArray<FFortItemEntry> LootToDrops;
		Loots::PickLootDrops(&LootToDrops, 0, LootTierKey, 0, 0, FGameplayTagContainer(), true, false);

		if (LootToDrops.Num() <= 0)
		{
			FN_LOG(LogCheatManager, Warning, "No loot picked.");
			PlayerController->ClientMessage(L"No loot picked.", FName(), 1);
		}
		else
		{
			for (int32 i = 0; i < LootToDrops.Num(); i++)
			{
				FFortItemEntry ItemEntry = LootToDrops[i];

				UFortItemDefinition* ItemDefinition = ItemEntry.ItemDefinition;
				if (!ItemDefinition) continue;

				FN_LOG(LogCheatManager, Log, "%d %s", ItemEntry.Count, ItemDefinition->GetName().c_str());

				std::string SimulateLootString = std::to_string(ItemEntry.Count) + " " + ItemDefinition->GetName();
				PlayerController->ClientMessage(std::wstring(SimulateLootString.begin(), SimulateLootString.end()).c_str(), FName(), 1);
			}
		}
	}

	void GiveSpecificItemHook(UFortCheatManager* CheatManager, FFrame& Stack, void* Ret)
	{
		FString ItemPath;
		int32 ItemAmount;
		int32 ItemLevel;

		Stack.StepCompiledIn(&ItemPath);
		Stack.StepCompiledIn(&ItemAmount);
		Stack.StepCompiledIn(&ItemLevel);

		Stack.Code += Stack.Code != nullptr;

		AFortPlayerController* PlayerController = Cast<AFortPlayerController>(CheatManager->Outer);
		if (!PlayerController) return;

		UFortWorldItemDefinition* WorldItemDefinition = FindObjectFast<UFortWorldItemDefinition>(ItemPath.ToString());
		if (!WorldItemDefinition) return;

		TScriptInterface<IFortInventoryOwnerInterface> InventoryOwner{};
		InventoryOwner.InterfacePointer = (void*)(__int64(PlayerController) + Offsets621::AFortPlayerController_InventoryOwnerInterface); // 0x598 en 8.51
		InventoryOwner.ObjectPointer = PlayerController;

		// [8.51-ONLY] GiveItemToInventoryOwner n'a pas de parametre ItemLevel en 6.21
		// UFortKismetLibrary::GiveItemToInventoryOwner(InventoryOwner, WorldItemDefinition, ItemAmount, false, ItemLevel);
		UFortKismetLibrary::GiveItemToInventoryOwner(InventoryOwner, WorldItemDefinition, ItemAmount, false);
	}

	void GiveWeaponHook(UFortCheatManager* CheatManager, FFrame& Stack, void* Ret)
	{
		FString WeaponName;
		int32 RequestedLevel;
		int32 Count;

		Stack.StepCompiledIn(&WeaponName);
		Stack.StepCompiledIn(&RequestedLevel);
		Stack.StepCompiledIn(&Count);

		Stack.Code += Stack.Code != nullptr;

		AFortPlayerController* PlayerController = Cast<AFortPlayerController>(CheatManager->Outer);
		if (!PlayerController) return;

		TArray<UFortItemDefinition*> AllItems = Functions::GetAllItems();

		for (int32 i = 0; i < AllItems.Num(); i++)
		{
			UFortWorldItemDefinition* WorldItemDefinition = Cast<UFortWorldItemDefinition>(AllItems[i]);

			if (!WorldItemDefinition)
				continue;

			std::string ItemDefinitionName2 = WorldItemDefinition->GetName();

			std::transform(ItemDefinitionName2.begin(), ItemDefinitionName2.end(), ItemDefinitionName2.begin(),
				[](unsigned char c) { return std::tolower(c); });

			if (ItemDefinitionName2 != WeaponName.ToString())
				continue;

			TScriptInterface<IFortInventoryOwnerInterface> InventoryOwner{};
			InventoryOwner.InterfacePointer = (void*)(__int64(PlayerController) + Offsets621::AFortPlayerController_InventoryOwnerInterface); // 0x598 en 8.51
			InventoryOwner.ObjectPointer = PlayerController;

			// [8.51-ONLY] GiveItemToInventoryOwner n'a pas de parametre ItemLevel en 6.21
			// UFortKismetLibrary::GiveItemToInventoryOwner(InventoryOwner, WorldItemDefinition, Count, false, RequestedLevel);
			UFortKismetLibrary::GiveItemToInventoryOwner(InventoryOwner, WorldItemDefinition, Count, false);
			break;
		}
	}

	void GiveWoodHook(UFortCheatManager* CheatManager, FFrame& Stack, void* Ret)
	{
		int32 Amount;

		Stack.StepCompiledIn(&Amount);

		Stack.Code += Stack.Code != nullptr;

		AFortPlayerController* PlayerController = Cast<AFortPlayerController>(CheatManager->Outer);
		if (!PlayerController) return;

		UFortKismetLibrary::K2_GiveBuildingResource(PlayerController, EFortResourceType::Wood, Amount);
	}
	
	void GiveStoneHook(UFortCheatManager* CheatManager, FFrame& Stack, void* Ret)
	{
		int32 Amount;

		Stack.StepCompiledIn(&Amount);

		Stack.Code += Stack.Code != nullptr;

		AFortPlayerController* PlayerController = Cast<AFortPlayerController>(CheatManager->Outer);
		if (!PlayerController) return;

		UFortKismetLibrary::K2_GiveBuildingResource(PlayerController, EFortResourceType::Stone, Amount);
	}
	
	void GiveMetalHook(UFortCheatManager* CheatManager, FFrame& Stack, void* Ret)
	{
		int32 Amount;

		Stack.StepCompiledIn(&Amount);

		Stack.Code += Stack.Code != nullptr;

		AFortPlayerController* PlayerController = Cast<AFortPlayerController>(CheatManager->Outer);
		if (!PlayerController) return;

		UFortKismetLibrary::K2_GiveBuildingResource(PlayerController, EFortResourceType::Metal, Amount);
	}
	
	void GiveResourcesHook(UFortCheatManager* CheatManager, FFrame& Stack, void* Ret)
	{
		int32 Amount;

		Stack.StepCompiledIn(&Amount);

		Stack.Code += Stack.Code != nullptr;

		CheatManager->GiveWood(Amount);
		CheatManager->GiveStone(Amount);
		CheatManager->GiveMetal(Amount);
	}

	void SendTwitchToastNotificationHook(UFortCheatManager* CheatManager, FFrame& Stack, void* Ret)
	{
		FString Title; 
		FString Description;

		Stack.StepCompiledIn(&Title);
		Stack.StepCompiledIn(&Description);

		Stack.Code += Stack.Code != nullptr;

		AFortPlayerController* PlayerController = Cast<AFortPlayerController>(CheatManager->Outer);
		if (!PlayerController) return;

		PlayerController->ClientMessage(L"Send Twitch Toast Notification", FName(), 1);

		UFortGameData* GameData = Globals::GetGameData();
		GameData->TwitchNotificationClass;

		UWorld* World = UWorld::GetWorld();

		UFortGameInstance* GameInstance = Cast<UFortGameInstance>(World->OwningGameInstance);

		GameInstance->UIManagerInterface;
	}

	void BugItGoHook(UFortCheatManager* CheatManager, FFrame& Stack, void* Ret)
	{
		float X; 
		float Y; 
		float Z; 
		float Pitch; 
		float Yaw; 
		float Roll;

		Stack.StepCompiledIn(&X);
		Stack.StepCompiledIn(&Y);
		Stack.StepCompiledIn(&Z);
		Stack.StepCompiledIn(&Pitch);
		Stack.StepCompiledIn(&Yaw);
		Stack.StepCompiledIn(&Roll);

		Stack.Code += Stack.Code != nullptr;

		/*float* a2{};
		*a2 = X;
		a2[1] = Y;
		a2[2] = Z;
		
		float* a3{};
		*a3 = Pitch;
		a3[1] = Yaw;
		a3[2] = Roll;

		// [OFFSET-TODO-6.21] BugItGo (8.51 = 0x2a77100)
		// void (*BugItGo)(UCheatManager* CheatManager, float* a2, float* a3) = decltype(BugItGo)(0x2a77100 + uintptr_t(GetModuleHandle(0)));
		auto BugItGo = Offsets621::Native<void(UCheatManager* CheatManager, float* a2, float* a3)>(Offsets621::BugItGo, "BugItGo");
		BugItGo(CheatManager, a2, a3);*/
	}

	void InitCheatManager()
	{
		UFortCheatManager* FortCheatManagerDefault = UFortCheatManager::GetDefaultObj();
		UClass* FortCheatManagerClass = UFortCheatManager::StaticClass();

		UFunction* BuildFreeFunc = FortCheatManagerClass->GetFunction("FortCheatManager", "BuildFree");
		MinHook::HookFunctionExec(BuildFreeFunc, BuildFreeHook, nullptr);
		
		UFunction* ReviveFromDBNOFunc = FortCheatManagerClass->GetFunction("FortCheatManager", "ReviveFromDBNO");
		MinHook::HookFunctionExec(ReviveFromDBNOFunc, ReviveFromDBNOHook, nullptr);

		UFunction* SimulateLootFunc = FortCheatManagerClass->GetFunction("FortCheatManager", "SimulateLoot");
		MinHook::HookFunctionExec(SimulateLootFunc, SimulateLootHook, nullptr);

		UFunction* GiveSpecificItemFunc = FortCheatManagerClass->GetFunction("FortCheatManager", "GiveSpecificItem");
		MinHook::HookFunctionExec(GiveSpecificItemFunc, GiveSpecificItemHook, nullptr);

		UFunction* GiveWeaponFunc = FortCheatManagerClass->GetFunction("FortCheatManager", "GiveWeapon");
		MinHook::HookFunctionExec(GiveWeaponFunc, GiveWeaponHook, nullptr);

		UFunction* GiveWoodFunc = FortCheatManagerClass->GetFunction("FortCheatManager", "GiveWood");
		MinHook::HookFunctionExec(GiveWoodFunc, GiveWoodHook, nullptr);

		UFunction* GiveStoneFunc = FortCheatManagerClass->GetFunction("FortCheatManager", "GiveStone");
		MinHook::HookFunctionExec(GiveStoneFunc, GiveStoneHook, nullptr);

		UFunction* GiveMetalFunc = FortCheatManagerClass->GetFunction("FortCheatManager", "GiveMetal");
		MinHook::HookFunctionExec(GiveMetalFunc, GiveMetalHook, nullptr);

		UFunction* GiveResourcesFunc = FortCheatManagerClass->GetFunction("FortCheatManager", "GiveResources");
		MinHook::HookFunctionExec(GiveResourcesFunc, GiveResourcesHook, nullptr);

		UFunction* SendTwitchToastNotificationFunc = FortCheatManagerClass->GetFunction("FortCheatManager", "SendTwitchToastNotification");
		MinHook::HookFunctionExec(SendTwitchToastNotificationFunc, SendTwitchToastNotificationHook, nullptr);

		UFunction* BugItGoFunc = FortCheatManagerClass->GetFunction("CheatManager", "BugItGo");
		MinHook::HookFunctionExec(BugItGoFunc, BugItGoHook, nullptr);

		FN_LOG(LogInit, Log, "InitCheatManager Success!");
	}
}