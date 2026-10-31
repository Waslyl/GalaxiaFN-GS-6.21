#pragma once

#ifdef LATE_GAME
#include <cmath>
#include <unordered_map>

// [GALAXIA PATCH] Lategame 6.21 : bus natif sur la zone de phase 3, equipement a la sortie.
namespace LateGame621
{
    constexpr int32 InitialPhase = 3;
    constexpr float DropDelay = 2.0f;
    constexpr float DropDuration = 20.0f;
    constexpr float FlightDuration = 25.0f;
    bool bFlightConfigured = false;
    bool bZoneReady = false;
    bool bFlightHookReady = false;
    ULONGLONG NextLoadoutSweep = 0;
    std::unordered_map<AFortPlayerController*, FName> EquippedControllers;

    void (*StartFlightOriginal)(AFortAthenaAircraft*, float) = nullptr;
    bool (*EndAircraftPhaseOriginal)(AFortGameModeAthena*, bool, bool) = nullptr;

    void Reset()
    {
        bFlightConfigured = false;
        bZoneReady = false;
        NextLoadoutSweep = 0;
        EquippedControllers.clear();
    }

    bool EndAircraftPhaseHook(AFortGameModeAthena* GameMode, bool Arg1, bool Arg2)
    {
        const bool Result = EndAircraftPhaseOriginal(GameMode, Arg1, Arg2);
        auto* State = GameMode ? Cast<AFortGameStateAthena>(GameMode->GameState) : nullptr;
        if (bZoneReady && GameMode == Globals::GetGameMode() && State &&
            State->GamePhase == EAthenaGamePhase::SafeZones && GameMode->SafeZoneIndicator &&
            GameMode->SafeZonePhase >= InitialPhase)
        {
            // 0xC93F50 remet bSafeZoneActive a false et retarde SafeZonesStartTime a la fin du bus.
            // Notre indicateur de phase avancee existe deja : le conserver sans recreer la phase 0.
            GameMode->bSafeZoneActive = true;
            State->SafeZonesStartTime = UGameplayStatics::GetTimeSeconds(GameMode) + State->ServerWorldTimeSecondsDelta;
            State->ForceNetUpdate();
            FN_LOG(LogGameMode, Log, "[LateGame621] Fin du bus : zone avancee conservee.");
        }
        return Result;
    }

    void StartFlightHook(AFortAthenaAircraft* Aircraft, float ServerTime)
    {
        auto* GameMode = Globals::GetGameMode();
        auto* State = GameMode ? Cast<AFortGameStateAthena>(GameMode->GameState) : nullptr;
        auto* Map = State ? State->MapInfo : nullptr;
        if (bFlightHookReady && Aircraft && Aircraft->FlightStartTime < 0.0f && GameMode && Map &&
            GameMode->SafeZoneLocations.Num() > InitialPhase && Map->SafeZoneDefinitions.Num() > InitialPhase + 1)
        {
            const float Radius = Map->SafeZoneDefinitions[InitialPhase].Radius.GetValueAtLevel(0);
            if (std::isfinite(Radius) && Radius > 0.0f)
            {
                const FVector Center = GameMode->SafeZoneLocations[InitialPhase];
                auto Flight = Aircraft->FlightInfo;
                Flight.FlightStartRotation.Pitch = 0;
                Flight.FlightStartRotation.Roll = 0;
                const FVector Forward = UKismetMathLibrary::GetForwardVector(Flight.FlightStartRotation);
                Flight.FlightSpeed = Radius * 1.6f / DropDuration;
                FVector Start = Center;
                Start.X -= Forward.X * Flight.FlightSpeed * (DropDelay + DropDuration * 0.5f);
                Start.Y -= Forward.Y * Flight.FlightSpeed * (DropDelay + DropDuration * 0.5f);
                Start.Z = std::max(Aircraft->FlightInfo.FlightStartLocation.Z, Center.Z + 20000.0f);
                Flight.FlightStartLocation = FVector_NetQuantize100(Start);
                Flight.TimeTillDropStart = DropDelay;
                Flight.TimeTillDropEnd = DropDelay + DropDuration;
                Flight.TimeTillFlightEnd = FlightDuration;
                if (Aircraft->K2_TeleportTo(Start, Flight.FlightStartRotation))
                {
                    Aircraft->FlightInfo = Flight;
                    Aircraft->ForceNetUpdate();
                    bFlightConfigured = true;
                    FN_LOG(LogGameMode, Log, "[LateGame621] Bus sur la zone %i, rayon %.0f m, saut pendant %.0f s.", InitialPhase, Radius / 100.0f, DropDuration);
                }
            }
        }
        // 0xC345B0 : derive les quatre horaires du vol depuis FlightInfo et ServerTime (xmm1).
        StartFlightOriginal(Aircraft, ServerTime);
    }

    void StartZone(AFortGameModeAthena* GameMode)
    {
        if (bZoneReady || !bFlightConfigured || !GameMode) return;
        auto* State = Cast<AFortGameStateAthena>(GameMode->GameState);
        if (!State || !State->MapInfo || State->MapInfo->SafeZoneDefinitions.Num() <= InitialPhase + 1 ||
            GameMode->SafeZoneLocations.Num() <= InitialPhase + 1)
            return;

        auto Start = Offsets621::Native<void(AFortGameModeAthena*)>(Offsets621::StartSafeZone, "StartSafeZone");
        auto Advance = Offsets621::Native<void(AFortGameModeAthena*)>(Offsets621::AdvanceSafeZone, "AdvanceSafeZone");
        if (!Start || !Advance) return;
        if (!GameMode->bSafeZoneActive) Start(GameMode);
        auto* Indicator = GameMode->SafeZoneIndicator;
        if (!Indicator) return;

        // AdvanceSafeZone incremente AVANT de lire le centre et les reglages : 2 -> 3.
        GameMode->SafeZonePhase = InitialPhase - 1;
        Advance(GameMode);
        if (GameMode->SafeZonePhase != InitialPhase || Indicator->NextRadius <= 0) return;

        // Le premier cercle est deja ferme sur la zone avancee ; les phases suivantes restent natives.
        Indicator->LastCenter = Indicator->NextCenter;
        Indicator->LastRadius = Indicator->NextRadius;
        Indicator->Radius = Indicator->NextRadius;
        Indicator->ViewTargetLocation = Indicator->NextCenter;
        auto* SetCircle = Indicator->Class->GetFunction("FortSafeZoneIndicator", "SetSafeZoneRadiusAndCenter");
        if (SetCircle) Indicator->SetSafeZoneRadiusAndCenter(Indicator->NextRadius, Indicator->NextCenter);
        State->SafeZonesStartTime = UGameplayStatics::GetTimeSeconds(GameMode) + State->ServerWorldTimeSecondsDelta;
        Indicator->ForceNetUpdate();
        State->ForceNetUpdate();
        bZoneReady = true;
        FN_LOG(LogGameMode, Log, "[LateGame621] Zone native prete, phase %i, rayon %.0f m.", InitialPhase, Indicator->NextRadius / 100.0f);
    }

    void GiveLoadoutOnExit(AFortPlayerController* Controller)
    {
        if (!bZoneReady || !Controller || !Controller->WorldInventory || !Controller->MyFortPawn) return;
        auto* AthenaController = Cast<AFortPlayerControllerAthena>(Controller);
        auto* PlayerState = Controller->PlayerState;
        if (!AthenaController || !PlayerState || PlayerState->bIsSpectator || PlayerState->bOnlySpectator ||
            Controller->MyFortPawn->bIsDying || AthenaController->IsInAircraft()) return;
        const auto Existing = EquippedControllers.find(Controller);
        if (Existing != EquippedControllers.end() && Existing->second == Controller->Name) return;

        // Le nom d'instance distingue aussi un nouveau controleur qui reutiliserait la meme adresse.
        EquippedControllers[Controller] = Controller->Name;
        Functions::GiveLateGameInventory(Controller);
        Controller->MyFortPawn->SetMaxHealth(100);
        Controller->MyFortPawn->SetHealth(100);
        Controller->MyFortPawn->SetMaxShield(100);
        Controller->MyFortPawn->SetShield(100);
        Controller->MyFortPawn->bCanBeDamaged = true;
        FN_LOG(LogGameMode, Log, "[LateGame621] Equipement attribue a %s (500 bois, 300 pierre, 150 metal).", Controller->GetName().c_str());
    }

    void Tick(AFortGameModeAthena* GameMode)
    {
        if (!bZoneReady || !GameMode || GetTickCount64() < NextLoadoutSweep) return;
        NextLoadoutSweep = GetTickCount64() + 500;
        // Couvre aussi l'ejection automatique native, qui peut contourner le RPC ProcessEvent.
        auto Controllers = UFortKismetLibrary::GetAllFortPlayerControllers(GameMode, true, false);
        for (int32 i = 0; i < Controllers.Num(); ++i) GiveLoadoutOnExit(Controllers[i]);
        Controllers.Free();
    }

    void Init()
    {
        if (!Offsets621::IsExpectedBuild()) return;
        const bool FlightReady = MinHook::HookNative(Offsets621::StartAircraftFlight, "StartAircraftFlight", StartFlightHook, (void**)(&StartFlightOriginal));
        const bool EndReady = MinHook::HookNative(Offsets621::EndAircraftPhase, "EndAircraftPhase", EndAircraftPhaseHook, (void**)(&EndAircraftPhaseOriginal));
        bFlightHookReady = FlightReady && EndReady;
    }
}
#endif // LATE_GAME
