#pragma once
#include <cstdio>
#include <string>
#include <type_traits>
#include <unordered_set>
#include <utility>

// [GALAXIA PATCH] Adresses natives de la 6.21 (RVA depuis l'image base).
// 0 = pas encore trouvee : la fonctionnalite est desactivee (ou remplacee par un repli sans adresse) au lieu de crasher.
// L'adresse 8.51 est en commentaire pour aider a la retrouver dans IDA sur la 6.21.
// L'exe 6.21 sur disque est chiffre (.text) : rien n'a pu etre verifie statiquement, d'ou les 0.

namespace Offsets621
{
	// ===== Donnees globales =====
	// Les adresses trouvees dans le dump ne sont valables que pour CE binaire : verifie au demarrage (IsExpectedBuild)
	constexpr uint32_t ExpectedTimeDateStamp = 0x5BD92536;              // FortniteClient-Win64-Shipping.exe 6.21 (CL 4526925)

	constexpr uintptr_t GIsClient = 0x556C63B;                          // bool GIsClient (8.51 = 0x5940A13). Lu par UNetDriver::GetNetMode (0x27668f0)
	constexpr uintptr_t GIsServer = 0x556C63C;                          // bool GIsServer (8.51 = 0x5940A14). Ecrit juste apres GIsClient a l'init du moteur (meme disposition qu'en 8.51)

	// ===== CoreUObject / Engine =====
	constexpr uintptr_t StaticLoadObject = 0x19786C0;                   // StaticLoadObjectInternal (8.51 = 0x1E838D0) : log "Return an object still needing load from StaticLoadObjectInternal %s", 7 parametres (UE 4.21)
	constexpr uintptr_t FFrame_Step = 0x1951bb0;                        // FFrame::Step (8.51 = 0x1e83f60) : GNatives[*Code++](Context, *this, Result), GNatives = 0x5672100
	constexpr uintptr_t FFrame_StepExplicitProperty = 0x1951be0;        // FFrame::StepExplicitProperty (8.51 = 0x1e83f90), juste apres Step
	constexpr uintptr_t GetInterfaceAddress = 0x19692B0;                      // UObject::GetInterfaceAddress (GS 6.21 de reference (Heliato), verifie dans le dump : debut de fonction). Avant : UObject::GetInterfaceAddress (8.51 = 0x1E73E00)
	constexpr uintptr_t NetConnection_Close = 0x0;                      // [OFFSET-TODO-6.21] UNetConnection::Close (8.51 = 0x2cfef80)
	constexpr uintptr_t NetSerialize = 0x0;                             // [OFFSET-TODO-6.21] FGameplayAbilityTargetDataHandle::NetSerialize (8.51 = 0x887f90)
	constexpr uintptr_t SpawnContextCheck = 0x191D2E0;                  // CanCreateInCurrentContext (8.51 = 0x1e2ae20) : test juste avant "Unable to spawn class '%s' due to client/server context." dans UWorld::SpawnActor (0x2702010), seul appelant
	constexpr uintptr_t BugItGo = 0x0;                                  // [OFFSET-TODO-6.21] UCheatManager::BugItGo (8.51 = 0x2a77100)

	// [GALAXIA PATCH] Retour de l'appel au retrait dans la transaction native du collecteur.
	// Filtre du hook de paiement : aucun autre appelant de K2_RemoveItemFromPlayer n'est modifie.
	constexpr uintptr_t ItemCollector_RemoveCostReturn = 0xD56E6C;

	// ===== Session / GameMode =====
	// [GALAXIA PATCH] Deux appels FParse::Param(..., L"NoAuth") du controle de session 6.21.
	// Adresses de retour (CALL + 5), verifiees dans le dump : filtre strict du hook SessionNoAuthParam.
	constexpr uintptr_t Session_NoAuthValidationReturn = 0x110301E;
	constexpr uintptr_t Session_NoAuthReservationReturn = 0x11038B5;
	constexpr uintptr_t GetGameSessionClass = 0x0;                      // [OFFSET-TODO-6.21] AFortGameMode::GetGameSessionClass (8.51 = 0x12d2bc0)
	constexpr uintptr_t DestroyPreviousGameSession1 = 0x0;              // [OFFSET-TODO-6.21] "Failed to destroy previous game session %s" (8.51 = 0x1524e90)
	constexpr uintptr_t DestroyPreviousGameSession2 = 0x0;              // [OFFSET-TODO-6.21] "Failed to destroy previous game session %s" (8.51 = 0x1524660)
	constexpr uintptr_t BotCrashFix = 0x0;                              // [OFFSET-TODO-6.21] sub_7FF6B9034770, crash a1 invalide (bots) (8.51 = 0xdc4770)
	constexpr uintptr_t MemoryLeakFix = 0x0;                            // [OFFSET-TODO-6.21] sub_7FF6B8E55480, fuite memoire (8.51 = 0xbe5480)
	constexpr uintptr_t StartAircraftPhase = 0xC91F80;                  // AFortGameModeAthena::StartAircraftPhase (8.51 = 0xfb8070) : logs "AFortGameModeAthena::StartAircraftPhase: ...", (GameMode, octet)
	constexpr uintptr_t StartAircraftFlight = 0xC345B0;                 // AFortAthenaAircraft : FlightInfo -> horaires +0x3E0..0x3F4, (Aircraft, float ServerTime)
	constexpr uintptr_t EndAircraftPhase = 0xC93F50;                    // (GameMode, octet DL, octet R8B) -> bool ; remet bSafeZoneActive a 0 a la fin du bus
	constexpr uintptr_t StartSafeZone = 0xC911E0;                       // AFortGameModeAthena : cree l'indicateur, phase 0, bSafeZoneActive ; (GameMode) seul
	constexpr uintptr_t AdvanceSafeZone = 0xC93420;                     // AFortGameModeAthena : incremente SafeZonePhase, applique centres/definition ; (GameMode) seul
	constexpr uintptr_t HandlePostSafeZonePhaseChanged = 0x0;           // [OFFSET-TODO-6.21] AFortGameModeAthena::HandlePostSafeZonePhaseChanged (8.51 = 0xfb9830)
	constexpr uintptr_t PickTeam = 0xC862C0;                            // AFortGameModeAthena::PickTeam (8.51 = 0xfa9b20) : logs "PickTeam for [%s] ...", (GameMode, EFortTeam, PC)
	constexpr uintptr_t Restart = 0x10FD9B0;                            // AFortGameSessionDedicatedAthena::Restart, vtable 0x3C29E00 + 0x688 ; tailcall base 0x10FD6E0 (8.51 = 0x153cda0)
	constexpr uintptr_t PreLogin = 0x0;                                 // [OFFSET-TODO-6.21] AFortGameModeAthena::PreLogin (8.51 = 0xFAABC0)
	constexpr uintptr_t RemoveFromAlivePlayers = 0x0;                   // [OFFSET-TODO-6.21] AFortGameModeAthena::RemoveFromAlivePlayers (8.51 = 0xfae8c0, pattern Patterns::RemoveFromAlivePlayers)
	constexpr uintptr_t PickSupplyDropLocation = 0x0;                   // [OFFSET-TODO-6.21] AFortAthenaMapInfo::PickSupplyDropLocation (8.51 = 0xf4fe80)
	constexpr uintptr_t SpawnSupplyDrop = 0x0;                          // [OFFSET-TODO-6.21] AFortAthenaMapInfo::SpawnSupplyDrop (8.51 = 0xf55880)

	// ===== Loot =====
	constexpr uintptr_t LoadDataTable = 0xC67C80;                       // Charge un TSoftObjectPtr<UDataTable> (8.51 = 0xe25400) : appele par GetPlaylistLootTierDatas sur Playlist+0x198, avec a2 = 1
	constexpr uintptr_t GetPlaylistLootTierDatas = 0xC7C2A0;                 // GetLootTierDataTables (GS 6.21 de reference (Heliato), verifie dans le dump : debut de fonction). Avant : (8.51 = 0xf9fce0)
	constexpr uintptr_t GetPlaylistLootPackageDatas = 0xC7C1C0;              // GetLootPackageDataTables (GS 6.21 de reference (Heliato), verifie dans le dump : debut de fonction). Avant : (8.51 = 0xf9fc20)
	constexpr uintptr_t PickLootTierGroupAthena = 0xC8A130;                  // RedirectLootTierGroup ; 3e parametre FName par valeur (compare directement a des FName) (GS 6.21 de reference (Heliato), verifie dans le dump : debut de fonction). Avant : (8.51 = 0xfad620)

	// ===== Inventaire / items =====
	constexpr uintptr_t AddItemEntryStateValue = 0x1018A10;                   // FFortItemEntry::SetStateValue (GS 6.21 de reference (Heliato), verifie dans le dump : debut de fonction). Avant : "TestJsp" / PeutEtreAddStateValueJsp (8.51 = 0x1416E40)
	constexpr uintptr_t CreateItemEntryNative = 0x0;                    // [OFFSET-TODO-6.21] "JspTestCreate" (8.51 = 0x144B200)
	constexpr uintptr_t ResetItemEntry = 0x1048F10;                           // constructeur FFortItemEntry (GS 6.21 de reference (Heliato), verifie dans le dump : debut de fonction). Avant : FFortItemEntry::Reset (8.51 = 0x144B580)
	constexpr uintptr_t CopyItemEntry2 = 0x104AB00;                           // FFortItemEntry::operator= (GS 6.21 de reference (Heliato), verifie dans le dump : debut de fonction). Avant : (8.51 = 0x144B1D0)
	constexpr uintptr_t CopyItemEntry = 0x104AB00;                            // FFortItemEntry::operator= (GS 6.21 de reference (Heliato), verifie dans le dump : debut de fonction). Avant : FFortItemEntry::operator= (8.51 = 0x144D770)
	constexpr uintptr_t GetWeaponStats = 0x0;                           // [OFFSET-TODO-6.21] UFortWeaponItemDefinition::GetWeaponStats (8.51 = 0xF355D0)
	constexpr uintptr_t GetItemInstances = 0x0;                         // [OFFSET-TODO-6.21] IFortInventoryOwnerInterface (8.51 = 0x16DAC30)
	constexpr uintptr_t GetItemInstance = 0x0;                          // [OFFSET-TODO-6.21] IFortInventoryOwnerInterface (8.51 = 0x1004CE0)
	constexpr uintptr_t InitializeExistingItemInternal = 0x1036640;           // AFortInventory::InitializeExistingItem (GS 6.21 de reference (Heliato), verifie dans le dump : debut de fonction). Avant : AFortInventory (8.51 = 0x145fdf0)
	constexpr uintptr_t SetItemEntryDirty = 0x10667D0;                        // FFortItemEntry::SetToDirty (GS 6.21 de reference (Heliato), verifie dans le dump : debut de fonction). Avant : FFortItemEntry "SetToDirty" (8.51 = 0x1467a10)
	constexpr uintptr_t UpdateItemInstances = 0x0;                      // [OFFSET-TODO-6.21] AFortInventory (8.51 = 0x146c170)
	constexpr uintptr_t ApplyGadgetData = 0x0;                          // [OFFSET-TODO-6.21] UFortGadgetItemDefinition (8.51 = 0x1462180)
	constexpr uintptr_t K2_RemoveItemFromPlayer = 0x0;                  // [OFFSET-TODO-6.21] UFortKismetLibrary (8.51 = 0x134bb90)
	constexpr uintptr_t K2_RemoveItemFromPlayerByGuid = 0x0;            // [OFFSET-TODO-6.21] UFortKismetLibrary (8.51 = 0x134bbb0)

	// ===== Pickups =====
	constexpr uintptr_t PickupInitialize = 0x0;                         // [OFFSET-TODO-6.21] AFortPickup::Initialize (8.51 = 0xFECD10)
	constexpr uintptr_t SetPickupTarget = 0x1065FE0;                    // AFortPickup::SetPickupTarget (8.51 = 0x1467300) : pattern du GS 6.21 de reference, (Pickup, Pawn, float FlyTime, const FVector& Dir, bool bPlaySound) verifie dans le dump
	constexpr uintptr_t GenFlyTime = 0x1058950;                         // AFortPickup "GenFlyTime" (8.51 = 0x1456e40) : appelee par SetPickupTarget quand FlyTime <= 0, renvoie un float

	// ===== Construction / batiments =====
	constexpr uintptr_t GetCreateResourceAmount = 0x12644D0;            // AFortPlayerController (8.51 = 0x16cfcc0) : pattern du GS 6.21 de reference, (PC, const FBuildingClassData&)
	constexpr uintptr_t CanPlaceBuildableClassInStructuralGrid = 0x1265190; // AFortPlayerController (8.51 = 0x16d0aa0) : (PC, Class, Loc, Rot, bMirrored, Existing), renvoie EFortStructuralGridQueryResults
	constexpr uintptr_t DetermineMaxResourcesToSpawn = 0xD58310;             // ABuildingSMActor::DetermineMaxResourcesToSpawn (GS 6.21 de reference (Heliato), verifie dans le dump : debut de fonction). Avant : ABuildingSMActor (8.51 = 0x11114a0)
	constexpr uintptr_t ABuildingSMActor_PostUpdate = 0x0;              // [OFFSET-TODO-6.21] (8.51 = 0x1123260)
	constexpr uintptr_t OnDamageServer = 0x0;                           // [OFFSET-TODO-6.21] ABuildingActor::OnDamageServer (8.51 = 0x18f0570)

	// ===== Pawn / vehicules / abilities =====
	constexpr uintptr_t CanReviveFromDBNO = 0x0;                        // [OFFSET-TODO-6.21] AFortPlayerPawnAthena (8.51 = 0x10277e0)
	constexpr uintptr_t OnEnteredVehicle = 0x0;                         // [OFFSET-TODO-6.21] AFortPlayerPawn (8.51 = 0x1af3570)
	constexpr uintptr_t OnDeathServer = 0x0;                            // [OFFSET-TODO-6.21] AFortPlayerPawnAthena::OnDeathServer (8.51 = 0x1ab3df0)
	constexpr uintptr_t CreateDefaultAbilitySpec = 0x679760;            // FGameplayAbilitySpec::FGameplayAbilitySpec(Ability, Level, InputID, SourceObject) (8.51 = 0x868290) : pattern du GS 6.21 de reference, verifie dans le dump (9 appelants, ecrit jusqu'a +0xC4, taille SDK 0xC8)

	// ===== Fonctions natives appelees depuis le SDK =====
	constexpr uintptr_t CheckBeginEditBuildingActor = 0x0;              // [OFFSET-TODO-6.21] ABuildingSMActor (8.51 = 0x110e010)
	constexpr uintptr_t SelectMeshSet = 0xD76040;                            // ABuildingSMActor::SelectMeshSetByLootTier (GS 6.21 de reference (Heliato), verifie dans le dump : debut de fonction). Avant : ABuildingSMActor (8.51 = 0x1126e00)
	constexpr uintptr_t SetMeshSet = 0x0;                               // [OFFSET-TODO-6.21] ABuildingSMActor (8.51 = 0x110b850)
	constexpr uintptr_t EquipWeaponDefinition = 0x0;                    // [OFFSET-TODO-6.21] UFortWeaponItemDefinition (8.51 = 0x186e3e0)
	constexpr uintptr_t EquipDecoDefinition = 0x0;                      // [OFFSET-TODO-6.21] UFortDecoItemDefinition (8.51 = 0x143db70)
	constexpr uintptr_t EquipContextTrapDefinition = 0x0;               // [OFFSET-TODO-6.21] UFortContextTrapItemDefinition (8.51 = 0x143da40)
	constexpr uintptr_t GetValueAtLevel = 0x0;                          // FScalableFloat (8.51 = 0x87fe30). Pas besoin : repli exact via UFortScalableFloatUtils::GetValueAtLevel (Fallbacks621.h)

	// ===== Offsets de champs =====
	// [GALAXIA PATCH] Contexte interne de SendRequestNow : le switch a 0x949374 lit ce champ.
	// 1 = dedicated_server (WITH_SERVER_CODE absent du client), 3 = branche client. Verifie dans le dump 6.21.
	constexpr uintptr_t McpRequestContext_CommandType = 0x28;
	// Interface IFortInventoryOwnerInterface dans AFortPlayerController.
	// Deduit du SDK : elle suit APlayerController et IFortTeamActorInterface (meme ordre d'heritage en 8.51 et 6.21).
	//   8.51 : sizeof(APlayerController) 0x590 + 0x8 = 0x598 (valeur en dur d'origine, ce qui confirme la regle)
	//   6.21 : sizeof(APlayerController) 0x678 + 0x8 = 0x680 (premier champ d'AFortPlayerController a 0x688)
	constexpr uintptr_t AFortPlayerController_InventoryOwnerInterface = 0x680;
	constexpr uintptr_t AFortPickup_LastDropPickupTime = 0x0;           // [OFFSET-TODO-6.21] float (8.51 = +0x4FC, en 6.21 +0x4FC est un padding)
	constexpr uintptr_t AFortGameModeAthena_bMatchHasStarted = 0x0;     // [OFFSET-TODO-6.21] bool "Match Has Started" (8.51 = +0xDC8, champ natif non expose)
	constexpr uintptr_t AFortPlayerPawnAthena_DeathTags = 0x0;          // [OFFSET-TODO-6.21] FGameplayTagContainer (8.51 = +0x1320, champ natif non expose)

	// Horodatage du binaire charge (lu directement dans l'en-tete PE, sans type Windows)
	inline bool IsExpectedBuild()
	{
		static int State = -1;

		if (State < 0)
		{
			const uintptr_t Base = SDK::InSDKUtils::GetImageBase();
			const uint32_t PeOffset = *reinterpret_cast<const uint32_t*>(Base + 0x3C);
			const uint32_t TimeDateStamp = *reinterpret_cast<const uint32_t*>(Base + PeOffset + 8);

			State = TimeDateStamp == ExpectedTimeDateStamp ? 1 : 0;

			if (!State)
				printf("[OFFSET-TODO-6.21] Binaire inattendu (TimeDateStamp 0x%X au lieu de 0x%X) : adresses 6.21 ignorees\n", TimeDateStamp, ExpectedTimeDateStamp);
		}

		return State == 1;
	}

	inline void LogMissing(const char* Name)
	{
		static std::unordered_set<std::string> AlreadyLogged;

		if (AlreadyLogged.insert(Name).second)
			printf("[OFFSET-TODO-6.21] %s n'a pas d'adresse 6.21, fonctionnalite desactivee\n", Name);
	}

	template<typename FuncType>
	inline FuncType Resolve(uintptr_t Rva, const char* Name)
	{
		if (!Rva || !IsExpectedBuild())
		{
			LogMissing(Name);
			return nullptr;
		}

		return reinterpret_cast<FuncType>(SDK::InSDKUtils::GetImageBase() + Rva);
	}

	// Fonction native appelable comme un pointeur de fonction.
	// Sans adresse : appelle le repli s'il y en a un, sinon logue et renvoie une valeur neutre (0 / nullptr / false / {}).
	template<typename Sig> struct NativeFunc;

	template<typename Ret, typename... Args>
	struct NativeFunc<Ret(Args...)>
	{
		using FuncPtr = Ret(*)(Args...);

		FuncPtr Func = nullptr;
		FuncPtr Fallback = nullptr;
		const char* Name = "";

		explicit operator bool() const { return Func != nullptr || Fallback != nullptr; }

		Ret operator()(Args... InArgs) const
		{
			if (Func)
				return Func(std::forward<Args>(InArgs)...);

			if (Fallback)
				return Fallback(std::forward<Args>(InArgs)...);

			LogMissing(Name);

			if constexpr (!std::is_void_v<Ret>)
				return Ret{};
		}
	};

	template<typename Sig>
	inline NativeFunc<Sig> Native(uintptr_t Rva, const char* Name, typename NativeFunc<Sig>::FuncPtr Fallback = nullptr)
	{
		NativeFunc<Sig> Result;
		Result.Name = Name;
		Result.Fallback = Fallback;

		if (Rva && IsExpectedBuild())
			Result.Func = reinterpret_cast<typename NativeFunc<Sig>::FuncPtr>(SDK::InSDKUtils::GetImageBase() + Rva);
		else if (Fallback)
		{
			static std::unordered_set<std::string> AlreadyLogged;

			if (AlreadyLogged.insert(Name).second)
				printf("[OFFSET-TODO-6.21] %s n'a pas d'adresse 6.21, repli sans adresse utilise\n", Name);
		}

		return Result;
	}

	// Adresse d'une donnee globale (nullptr si inconnue)
	template<typename T>
	inline T* Global(uintptr_t Rva, const char* Name)
	{
		if (!Rva || !IsExpectedBuild())
		{
			LogMissing(Name);
			return nullptr;
		}

		return reinterpret_cast<T*>(SDK::InSDKUtils::GetImageBase() + Rva);
	}
}
