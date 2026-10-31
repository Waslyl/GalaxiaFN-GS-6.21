#pragma once

namespace Patterns
{
	// Base
	constexpr const char* FMemoryFree = "48 85 C9 74 2E 53 48 83 EC 20 48 8B D9"; // Fait
	constexpr const char* FMemoryMalloc = "48 89 5C 24 ? 57 48 83 EC 20 48 8B F9 8B DA 48 8B 0D ? ? ? ? 48 85 C9 75 0C";
	constexpr const char* FMemoryRealloc = "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 48 8B F1 41 8B D8 48 8B 0D ? ? ? ? 48 8B FA"; // Fait

	// FortWeapon
	// [8.51-ONLY] Pattern 8.51 introuvable en 6.21
	// constexpr const char* PickupDelay = "40 53 56 48 83 EC 38 4C 89 6C 24 ? 48 8B F1 4C 8B A9 ? ? ? ? 4D 85 ED 0F 84 ? ? ? ? 49 63 8D ? ? ? ? 85 C9"; // Fait
	// [GALAXIA PATCH] AFortPickup::PickupDelay 6.21 (RVA 0x1056960), appelee a la fin du vol : lit la cible (+0x440) puis retire le pickup de la liste du pawn (+0x860), unique dans le dump
	constexpr const char* PickupDelay = "40 53 56 57 48 83 EC 30 4C 89 6C 24 60 48 8B F1";

	// GameMode
	constexpr const char* AddFromAlivePlayers = "48 85 D2 0F 84 ? ? ? ? 56 57 48 83 EC 78"; // Fait
	// [8.51-ONLY] Pattern 8.51 : en 6.21 il tombe au milieu d'une autre fonction (0x14139ba, non aligne)
	// constexpr const char* RemoveFromAlivePlayers = "4C 89 4C 24 ? 4C 89 44 24 ? 55 41 54 41 55 41 56 41 57 48 8D 6C 24 ?"; // Fait
	// [GALAXIA PATCH] AFortGameModeAthena::RemoveFromAlivePlayers 6.21 (RVA 0xC8ADC0) : contient les logs
	// "RemoveFromAlivePlayers called for Player [%s] who wasn't in the AlivePlayers list." ; 7 parametres comme en 8.51 (verifie sur 3 appelants)
	constexpr const char* RemoveFromAlivePlayers = "4C 89 4C 24 20 4C 89 44 24 18 55 41 55 41 56 41 57 48 8D 6C 24 D9 48 81";
	constexpr const char* RestartSession = "40 55 53 57 41 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 45 27 48 8B 01"; // Fait

	// PlayerController
	constexpr const char* RemoveInventoryItem = "48 83 EC 48 80 B9 ? ? ? ? ? 74 22 0F 10 02 0F B6 44 24 ?"; // Fait
	constexpr const char* ModifyLoadedAmmo = "40 53 48 83 EC 30 80 B9 ? ? ? ? ? 41 8B D8 75 3F"; // Fait (1.7.2 - 12.10)
	// [8.51-ONLY] Patterns 8.51 introuvables en 6.21
	// constexpr const char* PayRepairCosts = "40 57 41 54 41 55 41 57 48 83 EC 78 4C 8B FA 48 8B F9 45 33 E4"; // Fait
	// constexpr const char* PayBuildingCosts = "48 89 5C 24 ? 48 89 6C 24 ? 56 57 41 56 48 83 EC 50 48 8B 1A 33 F6"; // Fait
	// [GALAXIA PATCH] Patterns du GS 6.21 de reference, uniques dans le dump (RVA 0x12926A0 et 0x1292490)
	constexpr const char* PayRepairCosts = "40 57 41 54 41 55 41 57 48 83 EC 78";
	constexpr const char* PayBuildingCosts = "48 89 5C 24 18 56 57 41 56 48 83 EC 50 48 83 3A 00";
	constexpr const char* CantBuild = "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 41 56 48 83 EC 60 49 8B E9"; // Fait
	// [8.51-ONLY] En 6.21, ce pattern trouve une fonction de chargement de package (RVA 0x196D6B0) : crash LoadPackageAsync a l'edition
	// constexpr const char* ReplaceBuildingActor = "4C 8B DC 55 57 49 8D AB ? ? ? ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 48 8B 85 ? ? ? ? 33 FF 40 38 3D ? ? ? ?"; // Fait
	// [GALAXIA PATCH] ABuildingSMActor::ReplaceBuildingActor 6.21 (RVA 0xD74D00) : pattern du GS 6.21 de reference, unique dans le dump, arguments verifies
	constexpr const char* ReplaceBuildingActor = "48 8B C4 44 89 48 20 55 57 48 8D A8 ? ? ? ? 48 81 EC ? ? ? ? 48 89 70 E8 33 FF 40 38 3D ? ? ? ? 48 8B F1";

	// Beacon
	constexpr const char* InitHost = "48 8B C4 48 81 EC ? ? ? ? 48 89 58 18 4C 8D 05 ? ? ? ? 48 8B D9"; // Fait
	constexpr const char* InitListen = "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 50 48 8B BC 24 ? ? ? ? 49 8B F0"; // Fait
	// [8.51-ONLY] Patterns 8.51 introuvables en 6.21 (remplaces par les patterns 6.21 ci-dessous)
	// constexpr const char* SetWorld = "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 48 8B 99 ? ? ? ? 48 8B F2 48 8B F9 48 85 DB 0F 84 ? ? ? ? 48 8B 97 ? ? ? ? 48 8D 8B ? ? ? ? E8 ? ? ? ?"; // Fait
	// constexpr const char* TickFlush = "4C 8B DC 55 49 8D AB ? ? ? ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 49 89 5B 18 49 89 73 F0 48 8B F1 49 89 7B E8 48 8D 0D ? ? ? ? 4D 89 73 D0"; // Fait
	// constexpr const char* ServerReplicateActors = "4C 8B DC 55 41 55 49 8D AB ? ? ? ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 48 8B 41 30 45 33 C0"; // Fait

	// [GALAXIA PATCH] Patterns 6.21, tires du dump dechiffre et verifies uniques (FortniteClient-Win64-Shipping.exe 6.21, CL 4526925)
	constexpr const char* SetWorld = "48 89 5C 24 08 57 48 83 EC 20 48 8B FA 48 8B D9 48 8B 91 28 01 00 00"; // UNetDriver::SetWorld (RVA 0x2779740), appelee par AOnlineBeaconHost::InitHost
	constexpr const char* TickFlush = "4C 8B DC 55 49 8D AB 98 FE FF FF 48 81 EC 60 02 00 00 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 00 01 00 00 49 89 5B 18 48 8D 05 ? ? ? ? 49 89 73 F0"; // UNetDriver::TickFlush (RVA 0x277ac20), vtable UIpNetDriver +0x2C0
	// 6.21 : UFortReplicationGraph::ServerReplicateActors, slot +0x2B0 ; transmet ensuite a UReplicationGraph (0x74FB30).
	// L'ancienne cible 0x2765330 est UNetDriver::ForceNetUpdate(AActor*), pas une fonction de replication par frame.
	constexpr const char* ServerReplicateActors = "48 8B C4 41 56 48 81 EC 80 00 00 00 83 3D ? ? ? ? 00 4C 8B F1 0F 29 70 B8 0F 28 F1 0F 8E ? ? ? ?"; // RVA 0x11C7F40

	// Abilities
	constexpr const char* InternalTryActivateAbility = "4C 89 4C 24 ? 4C 89 44 24 ? 89 54 24 10 55 53 56 57 41 54 41 56"; // Fait
	constexpr const char* GiveAbility = "48 89 5C 24 ? 48 89 6C 24 ? 48 89 7C 24 ? 41 56 48 83 EC 20 83 B9 ? ? ? ? ? 49 8B E8 4C 8B F2 48 8B F9 7E 56"; // Fait
	constexpr const char* CanActivateAbility = "48 89 5C 24 ? 4C 89 4C 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 49 8B F0"; // Fait
	// [8.51-ONLY] Pattern 8.51 introuvable en 6.21
	// constexpr const char* CreateDefaultObject = "4C 8B DC 55 57 49 8D AB ? ? ? ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 48 83 B9 ? ? ? ? ? 48 8B F9"; // Fait
	// [GALAXIA PATCH] UClass::CreateDefaultObject 6.21 (RVA 0x182DC30) : pattern du GS 6.21 de reference, unique dans le dump (teste ClassDefaultObject a +0xF8)
	constexpr const char* CreateDefaultObject = "4C 8B DC 57 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 84 24 ? ? ? ? 48 83 B9 ? ? ? ? ? 48 8B F9";
	constexpr const char* GiveAbilityAndActivateOnce = "48 89 5C 24 ? 55 56 57 48 8B EC 48 83 EC 70 49 8B 40 10 49 8B D8"; // Fait

	// Others
	constexpr const char* GetNetMode = "48 83 EC 28 48 8B 01 FF 90 ? ? ? ? 84 C0 74 12 33 C0"; // Fait
	constexpr const char* InternalGetNetMode = "40 53 48 81 EC ? ? ? ? 48 83 79 ? ? 48 8B D9 74 0E"; // Fait
	constexpr const char* ActorInternalGetNetMode = "48 89 5C 24 ? 57 48 83 EC 20 48 8B 01 48 8B D9 FF 90 ? ? ? ? 4C 8B 83 ? ? ? ? 48 8B F8"; // Fait
	// [8.51-ONLY] Pattern 8.51 introuvable en 6.21
	// constexpr const char* GetPlayerViewPoint = "48 89 74 24 ? 55 57 41 56 48 8B EC 48 83 EC 40 48 8B 81 ? ? ? ? 4D 8B F0"; // Fait
	// [GALAXIA PATCH] APlayerController::GetPlayerViewPoint 6.21 (RVA 0x2888760) : logs "APlayerController::GetPlayerViewPoint: out_Location, ViewTarget=%s"
	constexpr const char* GetPlayerViewPoint = "48 89 5C 24 10 48 89 74 24 18 55 41 56 41 57 48 8B EC 48 83 EC 40 48 8B F2 48 C7 45 38 42 01 00";
	constexpr const char* DispatchRequest = "48 89 5C 24 ? 48 89 74 24 ? 55 57 41 56 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 80 3D ? ? ? ? ? 48 8D 35 ? ? ? ? 41 8B D8 4C 8B F2 48 8B F9 72 5D"; // Fait
	constexpr const char* ApplyCharacterCustomization = "48 8B C4 48 89 50 10 55 57 48 8D 68 A1 48 81 EC ? ? ? ? 80 B9 ? ? ? ? ? 48 8B F9"; // Fait
	// [8.51-ONLY] En 6.21 le saut de KickPlayer est un je long (0F 84), pas le je court 74 6F de la 8.51.
	// constexpr const char* KickPlayer = "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 49 8B F0 48 8B DA 48 85 D2 74 6F"; // Fait
	// [GALAXIA PATCH] AGameSession::KickPlayer 6.21 (RVA 0x263A770), appelee par AFortGameSession::KickPlayer a 0x10F0EE5.
	constexpr const char* KickPlayer = "48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 20 49 8B F0 48 8B DA 48 85 D2 0F 84 82 00 00 00 48 8B BA A8 03 00 00";
	// [GALAXIA PATCH] UMcpProfileGroup::SendRequestNow 6.21 (RVA 0x9492B0) : repli pour DispatchRequest introuvable.
	// Reference "MCP-Profile: Dispatching request to %s" et le message WITH_SERVER_CODE observe dans le log.
	constexpr const char* McpSendRequestNow = "48 89 5C 24 20 55 56 57 41 56 41 57 48 8B EC 48 83 EC 70 48 8B F1 48 8B FA 48 8B 0A 48 85 C9 74 28";
	// [GALAXIA PATCH] FParse::Param 6.21 (RVA 0x17880D0), cible des deux CALL NoAuth a 0x1103019 / 0x11038B0.
	// Le hook ne change que ces deux lectures de NoAuth ; tous les autres parametres/appelants passent a l'original.
	constexpr const char* SessionNoAuthParam = "48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 41 54 41 55 41 56 41 57 48 83 EC 20 66 83 39 00";
	// [8.51-ONLY] Ancien pattern introuvable en 6.21 : le hook de session restait desactive et le chargement de carte crashait.
	// constexpr const char* ChangingGameSessionId = "40 55 56 41 54 48 8B EC 48 81 EC ? ? ? ? 48 8B 01 4C 8B E2 48 8B F1"; // Fait
	// [GALAXIA PATCH] 6.21, RVA 0xF474B0 : chaine "Changing GameSessionId from '%s' to '%s'", champ AFortGameState::GameSessionId (+0x4A8).
	// Le call a +0x216 finit a 0xF476CC (pile du crash), puis le service de session nul est dereference a 0xBD3ED4.
	constexpr const char* ChangingGameSessionId = "40 55 56 41 57 48 8B EC 48 81 EC 80 00 00 00 48 8B 01 4C 8B FA 48 8B F1 FF 90 40 01 00 00";
	// [GALAXIA PATCH] 6.21, RVA 0x12AD580 : chaine "AFortPlayerController::UpdateRichPresence - Executed MCP presence update".
	// Appelee par un timer dans le log du crash : retour 0x12AD61B, helper 0x12AE0C0, acces invalide a 0x12AE0F9.
	constexpr const char* UpdateRichPresence = "48 89 5C 24 08 48 89 74 24 10 48 89 7C 24 18 41 56 48 83 EC 30 41 0F B6 F0";
	// [GALAXIA PATCH] Helper POI/pawn 6.21 a 0xCEAD50, appele par CheckPawnOverlap (retour 0xCD4DF0).
	// Le crash a 0xCEAE04 lit les Bounds du BrushComponent (+0x340) nul, apres ajout de +0x140.
	constexpr const char* PoiPawnOverlap = "48 8B C4 48 89 58 08 48 89 70 10 57 48 81 EC F0 00 00 00 48 8B BA 98 03 00 00 48 8B DA 0F 29 70 E8 48 8B F1";
	// [GALAXIA PATCH] K2_RemoveItemFromPlayer 6.21, wrapper 0xF770D0 : le corps client saute vers return 0.
	// Signature confirmee par l'exec UFunction 0x155366F et l'appel du collecteur a 0xD56E67.
	constexpr const char* RemoveVendingCost = "48 85 C9 74 09 48 85 D2 0F 85 82 54 33 FF 33 C0 C3";
	constexpr const char* CollectGarbageInternal = "E9 ? ? ? ? 48 BA ? ? ? ? ? ? ? ? 48 0F 45 CA E9 ? ? ? ? 48 8B 8C 24 ? ? ? ? 48 33 CC E9 ? ? ? ?"; // Fait
}
