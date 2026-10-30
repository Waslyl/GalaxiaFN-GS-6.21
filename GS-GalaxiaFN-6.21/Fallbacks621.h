#pragma once

// [GALAXIA PATCH] Replis sans adresse native pour la 6.21.
// Ils sont utilises par Offsets621::Native<...>(..., Repli) tant que l'adresse 6.21 de la fonction native n'est pas trouvee.
// Ils n'utilisent que le SDK 6.21 (champs + UFunctions) : aucun offset ni aucune adresse en dur.
// Quand l'adresse native est renseignee dans Offsets621.h, c'est la fonction native qui est appelee a la place.

// FScalableFloat::GetValueAtLevel (8.51 = 0x87fe30) : declaree dans SDK\GameplayAbilities_structs.hpp.
// Repli = UFortScalableFloatUtils::GetValueAtLevel, UFunction du jeu 6.21 qui fait le meme calcul (sans adresse).
inline float SDK::FScalableFloat::GetValueAtLevel(float Level, const class FString* ContextString)
{
	if (Offsets621::GetValueAtLevel)
	{
		auto Native = Offsets621::Resolve<float(*)(FScalableFloat*, float, const class FString*)>(Offsets621::GetValueAtLevel, "FScalableFloat::GetValueAtLevel");
		return Native(this, Level, ContextString);
	}

	return UFortScalableFloatUtils::GetValueAtLevel(*this, Level);
}

// Equipement (8.51 = 0x186e3e0 / 0x143db70 / 0x143da40) : declares dans SDK\FortniteGame_classes.hpp.
// Repli = AFortPawn::EquipWeaponDefinition(WeaponData, ItemGuid), UFunction du jeu 6.21 (decos et pieges sont des UFortWeaponItemDefinition).
namespace Fallbacks621
{
	inline bool EquipWeaponDefinitionSDK(UFortWeaponItemDefinition* WeaponItemDefinition, UFortWorldItem* WorldItem, AFortPlayerController* PlayerController)
	{
		if (!WeaponItemDefinition || !WorldItem || !PlayerController || !PlayerController->MyFortPawn)
			return false;

		return PlayerController->MyFortPawn->EquipWeaponDefinition(WeaponItemDefinition, WorldItem->ItemEntry.ItemGuid) != nullptr;
	}
}

inline bool SDK::UFortWeaponItemDefinition::EquipWeaponDefinition(class UFortWorldItem* WorldItem, class AFortPlayerController* PlayerController)
{
	if (auto Native = Offsets621::Resolve<bool(*)(UFortWeaponItemDefinition*, UFortWorldItem*, AFortPlayerController*)>(Offsets621::EquipWeaponDefinition, "UFortWeaponItemDefinition::EquipWeaponDefinition (repli AFortPawn::EquipWeaponDefinition)"))
		return Native(this, WorldItem, PlayerController);

	return Fallbacks621::EquipWeaponDefinitionSDK(this, WorldItem, PlayerController);
}

inline bool SDK::UFortDecoItemDefinition::EquipDecoDefinition(class UFortWorldItem* WorldItem, class AFortPlayerController* PlayerController)
{
	if (auto Native = Offsets621::Resolve<bool(*)(UFortDecoItemDefinition*, UFortWorldItem*, AFortPlayerController*)>(Offsets621::EquipDecoDefinition, "UFortDecoItemDefinition::EquipDecoDefinition (repli AFortPawn::EquipWeaponDefinition)"))
		return Native(this, WorldItem, PlayerController);

	return Fallbacks621::EquipWeaponDefinitionSDK(this, WorldItem, PlayerController);
}

inline bool SDK::UFortContextTrapItemDefinition::EquipContextTrapDefinition(class UFortWorldItem* WorldItem, class AFortPlayerController* PlayerController)
{
	if (auto Native = Offsets621::Resolve<bool(*)(UFortContextTrapItemDefinition*, UFortWorldItem*, AFortPlayerController*)>(Offsets621::EquipContextTrapDefinition, "UFortContextTrapItemDefinition::EquipContextTrapDefinition (repli AFortPawn::EquipWeaponDefinition)"))
		return Native(this, WorldItem, PlayerController);

	return Fallbacks621::EquipWeaponDefinitionSDK(this, WorldItem, PlayerController);
}

// CheckBeginEditBuildingActor (8.51 = 0x110e010) : declare dans SDK\FortniteGame_classes.hpp.
// Repli : edition autorisee si personne d'autre n'edite deja ce batiment et s'il appartient a l'equipe du joueur
// (memes conditions que celles que le GS verifie ensuite dans ServerEditBuildingActorHook).
inline bool SDK::ABuildingSMActor::CheckBeginEditBuildingActor(class AFortPlayerController* PlayerController)
{
	if (auto Native = Offsets621::Resolve<bool(*)(ABuildingSMActor*, AFortPlayerController*)>(Offsets621::CheckBeginEditBuildingActor, "ABuildingSMActor::CheckBeginEditBuildingActor (repli equipe + editeur)"))
		return Native(this, PlayerController);

	if (!PlayerController)
		return false;

	AFortPlayerStateAthena* PlayerState = Cast<AFortPlayerStateAthena>(PlayerController->PlayerState);

	if (!PlayerState)
		return false;

	if (EditingPlayer && EditingPlayer != PlayerState)
		return false;

	return Team == PlayerState->TeamIndex;
}

namespace Fallbacks621
{
	// L'interface IFortInventoryOwnerInterface pointe dans le PlayerController (voir Offsets621::AFortPlayerController_InventoryOwnerInterface)
	inline AFortPlayerController* PlayerControllerFromInventoryOwner(void* InventoryOwner)
	{
		if (!InventoryOwner)
			return nullptr;

		return reinterpret_cast<AFortPlayerController*>(__int64(InventoryOwner) - Offsets621::AFortPlayerController_InventoryOwnerInterface);
	}

	// GetItemInstances (8.51 = 0x16DAC30) : toutes les instances d'une definition dans l'inventaire du joueur
	inline bool GetItemInstances(void* InventoryOwner, UFortWorldItemDefinition* ItemDefinition, TArray<UFortWorldItem*>* OutWorldItems)
	{
		AFortPlayerController* PlayerController = PlayerControllerFromInventoryOwner(InventoryOwner);

		if (!PlayerController || !PlayerController->WorldInventory || !ItemDefinition || !OutWorldItems)
			return false;

		TArray<UFortWorldItem*>& ItemInstances = PlayerController->WorldInventory->Inventory.ItemInstances;

		for (int32 i = 0; i < ItemInstances.Num(); i++)
		{
			UFortWorldItem* ItemInstance = ItemInstances[i];

			if (ItemInstance && ItemInstance->ItemEntry.ItemDefinition == ItemDefinition)
				OutWorldItems->Add(ItemInstance);
		}

		return OutWorldItems->Num() > 0;
	}

	// GetItemInstance (8.51 = 0x1004CE0) : instance de l'inventaire du joueur par GUID
	inline UFortWorldItem* GetItemInstance(void* InventoryOwner, const FGuid& ItemGuid)
	{
		AFortPlayerController* PlayerController = PlayerControllerFromInventoryOwner(InventoryOwner);

		if (!PlayerController || !PlayerController->WorldInventory)
			return nullptr;

		TArray<UFortWorldItem*>& ItemInstances = PlayerController->WorldInventory->Inventory.ItemInstances;

		for (int32 i = 0; i < ItemInstances.Num(); i++)
		{
			UFortWorldItem* ItemInstance = ItemInstances[i];

			if (ItemInstance && ItemInstance->ItemEntry.ItemGuid == ItemGuid)
				return ItemInstance;
		}

		return nullptr;
	}

	// ResetItemEntry (8.51 = 0x144B580) : entree vide, identifiants de replication a INDEX_NONE
	inline FFortItemEntry* ResetItemEntry(FFortItemEntry* ItemEntry)
	{
		if (!ItemEntry)
			return nullptr;

		memset(ItemEntry, 0, sizeof(FFortItemEntry));
		ItemEntry->ReplicationID = -1;
		ItemEntry->ReplicationKey = -1;
		ItemEntry->MostRecentArrayReplicationKey = -1;

		return ItemEntry;
	}

	// CopyItemEntry (8.51 = 0x144D770) : copie des valeurs simples uniquement.
	// Les TArray/FString (StateValues, etc.) ne sont pas copies : les partager entre deux entrees ferait liberer la meme memoire deux fois.
	inline FFortItemEntry* CopyItemEntry(FFortItemEntry* PrimaryItemEntry, FFortItemEntry* ItemEntry)
	{
		if (!PrimaryItemEntry || !ItemEntry)
			return PrimaryItemEntry;

		PrimaryItemEntry->ItemDefinition = ItemEntry->ItemDefinition;
		PrimaryItemEntry->Count = ItemEntry->Count;
		PrimaryItemEntry->PreviousCount = ItemEntry->PreviousCount;
		PrimaryItemEntry->Durability = ItemEntry->Durability;
		PrimaryItemEntry->Level = ItemEntry->Level;
		PrimaryItemEntry->LoadedAmmo = ItemEntry->LoadedAmmo;
		PrimaryItemEntry->ItemGuid = ItemEntry->ItemGuid;

		return PrimaryItemEntry;
	}

	// InitializeExistingItemInternal (8.51 = 0x145fdf0) : meme logique que le code commente d'origine dans Inventory::AddItem
	inline void InitializeExistingItem(AFortInventory* WorldInventory, UFortWorldItem* WorldItem)
	{
		if (!WorldInventory || !WorldItem)
			return;

		FFortItemEntry& ItemEntry = WorldItem->ItemEntry;

		// Chaque objet doit avoir son propre GUID (le GS retrouve les objets par GUID)
		if (!UKismetGuidLibrary::IsValid_Guid(ItemEntry.ItemGuid))
			ItemEntry.ItemGuid = UKismetGuidLibrary::NewGuid();

		WorldInventory->Inventory.ReplicatedEntries.Add(ItemEntry);
		WorldInventory->Inventory.ItemInstances.Add(WorldItem);

		const int32 LastIndex = WorldInventory->Inventory.ReplicatedEntries.Num() - 1;

		if (LastIndex >= 0)
			WorldInventory->Inventory.MarkItemDirty(WorldInventory->Inventory.ReplicatedEntries[LastIndex]);
	}

	// GetWeaponStats (8.51 = 0xF355D0) : ligne WeaponStatHandle de la DataTable des stats d'armes
	inline FFortBaseWeaponStats* GetWeaponStats(UFortWeaponItemDefinition* ItemDefinition)
	{
		if (!ItemDefinition)
			return nullptr;

		return Globals::GetDataTableRowFromName<FFortBaseWeaponStats>(ItemDefinition->WeaponStatHandle.DataTable, ItemDefinition->WeaponStatHandle.RowName);
	}

	// SetToDirty (8.51 = 0x1467a10)
	inline void SetItemEntryDirty(FFortItemEntry* ItemEntry)
	{
		if (ItemEntry)
			ItemEntry->bIsDirty = true;
	}

	// LoadDataTable (8.51 = 0xe25400) : ne charge pas depuis le disque, renvoie la DataTable si elle est deja en memoire
	// (les tables de loot de la playlist le sont une fois la partie chargee)
	inline UDataTable* LoadDataTable(TSoftObjectPtr<UDataTable> DataTableToLoad, char a2)
	{
		UObject* Object = UKismetSystemLibrary::Conv_SoftObjectReferenceToObject(*reinterpret_cast<TSoftObjectPtr<UObject>*>(&DataTableToLoad));
		return Cast<UDataTable>(Object);
	}

	// AFortPickup::Initialize (8.51 = 0xFECD10) : entree principale du pickup + OnRep (valeurs simples, voir CopyItemEntry)
	inline void PickupInitialize(AFortPickupAthena* Pickup, FFortItemEntry* ItemEntry, TArray<FFortItemEntry> MultiItemEntries, bool bSplitOnPickup)
	{
		if (!Pickup || !ItemEntry)
			return;

		CopyItemEntry(&Pickup->PrimaryPickupItemEntry, ItemEntry);
		Pickup->OnRep_PrimaryPickupItemEntry();
	}
}
