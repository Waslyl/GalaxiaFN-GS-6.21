#pragma once

namespace FortWeapon
{
	void (*PickupDelay)(AFortPickup* Pickup);


	void PickupDelayHook(AFortPickup* Pickup)
	{
		if (!Pickup)
		{
			FN_LOG(LogFortWeapon, Error, "[AFortPickup::PickupDelay] Failed to get Pickup.");
			return PickupDelay(Pickup);
		}

		FFortPickupLocationData* PickupLocationData = &Pickup->PickupLocationData;
		FFortItemEntry ItemEntry = Pickup->PrimaryPickupItemEntry;
		FGuid CurrentWepGuid = PickupLocationData->PickupGuid;

		if (!PickupLocationData)
		{
			FN_LOG(LogFortWeapon, Error, "[AFortPickup::PickupDelay] Failed to get PickupLocationData.");
			return PickupDelay(Pickup);
		}

		UFortWorldItemDefinition* ItemDefinition = (UFortWorldItemDefinition*)ItemEntry.ItemDefinition;

		if (!ItemDefinition)
		{
			FN_LOG(LogFortWeapon, Error, "[AFortPickup::PickupDelay] Failed to get ItemDefinition.");
			return PickupDelay(Pickup);
		}

		AFortPlayerPawn* PlayerPawn = Cast<AFortPlayerPawn>(PickupLocationData->PickupTarget);

		if (!PlayerPawn)
		{
			FN_LOG(LogFortWeapon, Error, "[AFortPickup::PickupDelay] Failed to get PlayerPawn.");
			return PickupDelay(Pickup);
		}

		AFortPlayerController* PlayerController = Cast<AFortPlayerController>(PlayerPawn->Controller);

		if (!PlayerController)
		{
#ifdef BOTS
			if (AFortAthenaAIBotController* AthenaAIBotController = Cast<AFortAthenaAIBotController>(PlayerPawn->Controller))
			{
				Bots::AddAndEquipItem(AthenaAIBotController->Inventory, AthenaAIBotController->PlayerBotPawn, ItemDefinition, ItemEntry.Count);
				return PickupDelay(Pickup);
			}
#endif // BOTS

			FN_LOG(LogFortWeapon, Error, "[AFortPickup::PickupDelay] Failed to get PlayerController.");
			return PickupDelay(Pickup);
		}

		for (int32 i = 0; i < PlayerPawn->QueuedAutoPickups.Num(); i++)
		{
			AFortPickup* QueuedAutoPickup = PlayerPawn->QueuedAutoPickups[i];
			if (!QueuedAutoPickup) continue;

			if (QueuedAutoPickup == Pickup)
			{
				PlayerPawn->QueuedAutoPickups.Remove(i);
				break;
			}
		}

		Inventory::AddInventoryItem(PlayerController, ItemDefinition, &ItemEntry, CurrentWepGuid);
		Inventory::UpdateInventory(PlayerController->WorldInventory);

		PickupDelay(Pickup);
	}

	void InitFortWeapon()
	{
		static auto FortWeaponDefault = FindObjectFast<AFortWeapon>("/Script/FortniteGame.Default__FortWeapon");

		uintptr_t AddressPickupDelay = MinHook::FindPatternUnique(Patterns::PickupDelay, "PickupDelay");

		MinHook::HookAddress(AddressPickupDelay, "PickupDelay", PickupDelayHook, (void**)(&PickupDelay)); // [GALAXIA PATCH] rien n'est installe si le pattern n'est pas trouve

		FN_LOG(LogInit, Log, "InitFortWeapon Success!");
	}
}