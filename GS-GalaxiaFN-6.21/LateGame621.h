#pragma once

#ifdef LATE_GAME
#include <cmath>
#include <limits>
#include <unordered_map>

// [GALAXIA PATCH] Lategame 6.21 : vol bas, tempete immediate et premier mouvement apres les atterrissages.
namespace LateGame621
{
    constexpr int32 InitialPhase = 3;
    constexpr float DropDelay = 2.0f;
    constexpr float DropDuration = 20.0f;
    constexpr float FlightDuration = 25.0f;
    constexpr float BusHeightAboveGround = 15000.0f;
    constexpr int32 GroundTraceSegments = 16;
    constexpr float FirstZoneDelay = 30.0f;

    bool bFlightConfigured = false;
    bool bZoneScheduled = false;
    bool bZoneReady = false;
    bool bActivatingZone = false;
    bool bHooksReady = false;
    float ZoneStartedAt = 0;
    float FirstMoveAt = 0;
    ULONGLONG NextLoadoutSweep = 0;
    std::unordered_map<AFortPlayerController*, FName> EquippedControllers;
    struct LandingState { FName Name; bool bExited = false; bool bLanded = false; };
    std::unordered_map<AFortPlayerController*, LandingState> Landings;

    void (*StartFlightOriginal)(AFortAthenaAircraft*, float) = nullptr;
    bool (*EndAircraftPhaseOriginal)(AFortGameModeAthena*, bool, bool) = nullptr;
    void (*StartSafeZoneOriginal)(AFortGameModeAthena*) = nullptr;
    void (*AdvanceSafeZoneOriginal)(AFortGameModeAthena*) = nullptr;
    void (*TickSafeZoneOriginal)(AFortGameModeAthena*, float) = nullptr;

    AFortGameStateAthena* GetCurrentState(AFortGameModeAthena* GameMode)
    {
        return GameMode && GameMode == Globals::GetGameMode() ? Cast<AFortGameStateAthena>(GameMode->GameState) : nullptr;
    }

    float ServerNow(AFortGameModeAthena* GameMode, AFortGameStateAthena* State)
    {
        return UGameplayStatics::GetTimeSeconds(GameMode) + State->ServerWorldTimeSecondsDelta;
    }

    bool IsPlayingPhase(AFortGameStateAthena* State)
    {
        return State && (State->GamePhase == EAthenaGamePhase::Aircraft || State->GamePhase == EAthenaGamePhase::SafeZones);
    }

    void Reset()
    {
        bFlightConfigured = false;
        bZoneScheduled = false;
        bZoneReady = false;
        bActivatingZone = false;
        ZoneStartedAt = 0;
        FirstMoveAt = 0;
        NextLoadoutSweep = 0;
        EquippedControllers.clear();
        Landings.clear();
    }

    bool FindFlightAltitude(AFortAthenaAircraft* Aircraft, const FVector& Start, const FVector& Forward,
        float Speed, float NativeHeight, float& OutHeight)
    {
        auto* TraceFunction = UKismetSystemLibrary::StaticClass()->GetFunction("KismetSystemLibrary", "LineTraceSingleForObjects");
        if (!TraceFunction) return false;
        TArray<EObjectTypeQuery> ObjectTypes;
        ObjectTypes.Add(EObjectTypeQuery::ObjectTypeQuery1);
        TArray<AActor*> Ignored;
        Ignored.Add(Aircraft);
        float HighestGround = -std::numeric_limits<float>::max();
        bool bFoundGround = false;
        const float TraceTop = std::max(NativeHeight + 20000.0f, 100000.0f);
        // Une altitude constante au-dessus du point le plus haut detecte sur tout le trajet.
        for (int32 i = 0; i <= GroundTraceSegments; ++i)
        {
            const float Time = FlightDuration * static_cast<float>(i) / GroundTraceSegments;
            FVector Top = Start;
            Top.X += Forward.X * Speed * Time;
            Top.Y += Forward.Y * Speed * Time;
            Top.Z = TraceTop;
            FVector Bottom = Top;
            Bottom.Z = -100000.0f;
            FHitResult Hit{};
            if (UKismetSystemLibrary::LineTraceSingleForObjects(Aircraft, Top, Bottom, ObjectTypes, false,
                Ignored, EDrawDebugTrace::None, &Hit, true, FLinearColor{}, FLinearColor{}, 0.0f) &&
                Hit.bBlockingHit && std::isfinite(Hit.ImpactPoint.Z))
            {
                HighestGround = std::max(HighestGround, Hit.ImpactPoint.Z);
                bFoundGround = true;
            }
        }
        ObjectTypes.Free();
        Ignored.Free();
        if (bFoundGround) OutHeight = HighestGround + BusHeightAboveGround;
        return bFoundGround;
    }

    void StartFlightHook(AFortAthenaAircraft* Aircraft, float ServerTime)
    {
        auto* GameMode = Globals::GetGameMode();
        auto* State = GetCurrentState(GameMode);
        auto* Map = State ? State->MapInfo : nullptr;
        if (bHooksReady && Aircraft && Aircraft->FlightStartTime < 0.0f && GameMode && Map &&
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
                Start.Z = Aircraft->FlightInfo.FlightStartLocation.Z;
                if (!FindFlightAltitude(Aircraft, Start, Forward, Flight.FlightSpeed, Start.Z, Start.Z))
                    FN_LOG(LogGameMode, Warning, "[LateGame621] Terrain non detecte : hauteur native du bus conservee.");
                Flight.FlightStartLocation = FVector_NetQuantize100(Start);
                Flight.TimeTillDropStart = DropDelay;
                Flight.TimeTillDropEnd = DropDelay + DropDuration;
                Flight.TimeTillFlightEnd = FlightDuration;
                if (Aircraft->K2_TeleportTo(Start, Flight.FlightStartRotation))
                {
                    Aircraft->FlightInfo = Flight;
                    Aircraft->ForceNetUpdate();
                    bFlightConfigured = true;
                    FN_LOG(LogGameMode, Log, "[LateGame621] Bus sur la zone %i, rayon %.0f m, altitude %.0f m, saut pendant %.0f s.",
                        InitialPhase, Radius / 100.0f, Start.Z / 100.0f, DropDuration);
                }
            }
        }
        StartFlightOriginal(Aircraft, ServerTime);
    }

    struct ZonePacing { float Hold; float Shrink; };

    ZonePacing GetZonePacing(int32 Phase)
    {
        // La phase initiale est deja fermee : sa transition technique reste non nulle.
        if (Phase <= InitialPhase) return { FirstZoneDelay, 0.1f };
        if (Phase == InitialPhase + 1) return { 0.0f, 35.0f };
        if (Phase == InitialPhase + 2) return { 10.0f, 30.0f };
        if (Phase == InitialPhase + 3) return { 8.0f, 25.0f };
        if (Phase == InitialPhase + 4) return { 5.0f, 20.0f };
        return { 3.0f, 15.0f };
    }

    void AdvanceSafeZoneHook(AFortGameModeAthena* GameMode)
    {
        auto* State = GetCurrentState(GameMode);
        // Le cercle de depart reste ferme jusqu'au dernier atterrissage + 30 s.
        if (bFlightConfigured && bZoneReady && IsPlayingPhase(State) && GameMode->SafeZonePhase == InitialPhase &&
            (!bZoneScheduled || ServerNow(GameMode, State) < FirstMoveAt)) return;
        AdvanceSafeZoneOriginal(GameMode);
        if (!bFlightConfigured || (!bZoneReady && !bActivatingZone) || !State ||
            !IsPlayingPhase(State) || !GameMode->SafeZoneIndicator || GameMode->SafeZonePhase < InitialPhase)
            return;
        const auto Timing = GetZonePacing(GameMode->SafeZonePhase);
        auto* Indicator = GameMode->SafeZoneIndicator;
        // Le tick natif 0xC97660 lit FinishShrinkTime (+0x364) pour passer a la phase suivante.
        // Appliquer apres le natif evite de modifier les definitions de zone et leurs caches.
        Indicator->SafeZoneStartShrinkTime = ServerNow(GameMode, State) + Timing.Hold;
        Indicator->SafeZoneFinishShrinkTime = Indicator->SafeZoneStartShrinkTime + Timing.Shrink;
        Indicator->ForceNetUpdate();
    }

    void StartSafeZoneHook(AFortGameModeAthena* GameMode)
    {
        auto* State = GetCurrentState(GameMode);
        if (bFlightConfigured && State)
        {
            // La zone est creee une seule fois au depart du bus.
            if (bZoneReady && IsPlayingPhase(State) && GameMode->SafeZoneIndicator)
                GameMode->bSafeZoneActive = true;
            return;
        }
        StartSafeZoneOriginal(GameMode);
    }

    void ActivateZone(AFortGameModeAthena* GameMode)
    {
        auto* State = GetCurrentState(GameMode);
        if (bZoneReady || !bHooksReady || !bFlightConfigured || !IsPlayingPhase(State) || !State->MapInfo ||
            State->MapInfo->SafeZoneDefinitions.Num() <= InitialPhase + 1 || GameMode->SafeZoneLocations.Num() <= InitialPhase + 1)
            return;
        StartSafeZoneOriginal(GameMode);
        auto* Indicator = GameMode->SafeZoneIndicator;
        if (!Indicator) return;
        GameMode->SafeZonePhase = InitialPhase - 1;
        bActivatingZone = true;
        AdvanceSafeZoneHook(GameMode);
        bActivatingZone = false;
        if (GameMode->SafeZonePhase != InitialPhase || Indicator->NextRadius <= 0) return;
        Indicator->LastCenter = Indicator->NextCenter;
        Indicator->LastRadius = Indicator->NextRadius;
        Indicator->Radius = Indicator->NextRadius;
        Indicator->ViewTargetLocation = Indicator->NextCenter;
        auto* SetCircle = Indicator->Class->GetFunction("FortSafeZoneIndicator", "SetSafeZoneRadiusAndCenter");
        if (SetCircle) Indicator->SetSafeZoneRadiusAndCenter(Indicator->NextRadius, Indicator->NextCenter);
        ZoneStartedAt = ServerNow(GameMode, State);
        State->SafeZonesStartTime = ZoneStartedAt;
        Indicator->bPausedForPreview = false;
        Indicator->ForceNetUpdate();
        State->ForceNetUpdate();
        bZoneReady = true;
        FN_LOG(LogGameMode, Log, "[LateGame621] Tempete active des le bus, phase %i, rayon %.0f m.", InitialPhase, Indicator->NextRadius / 100.0f);
    }

    void GiveLoadoutOnExit(AFortPlayerController* Controller)
    {
        // Le loot reste immediat au saut, independamment du premier mouvement de zone.
        if (!bFlightConfigured || !Controller || !Controller->WorldInventory || !Controller->MyFortPawn) return;
        auto* AthenaController = Cast<AFortPlayerControllerAthena>(Controller);
        auto* PlayerState = Controller->PlayerState;
        if (!AthenaController || !PlayerState || PlayerState->bIsSpectator || PlayerState->bOnlySpectator ||
            Controller->MyFortPawn->bIsDying || AthenaController->IsInAircraft()) return;
        const auto Existing = EquippedControllers.find(Controller);
        if (Existing != EquippedControllers.end() && Existing->second == Controller->Name) return;
        EquippedControllers[Controller] = Controller->Name;
        Functions::GiveLateGameInventory(Controller);
        Controller->MyFortPawn->SetMaxHealth(100);
        Controller->MyFortPawn->SetHealth(100);
        Controller->MyFortPawn->SetMaxShield(100);
        Controller->MyFortPawn->SetShield(100);
        Controller->MyFortPawn->bCanBeDamaged = true;
        FN_LOG(LogGameMode, Log, "[LateGame621] Equipement attribue a %s (500 bois, 300 pierre, 150 metal).", Controller->GetName().c_str());
    }

    LandingState& GetLanding(AFortPlayerController* Controller)
    {
        auto Entry = Landings.find(Controller);
        if (Entry == Landings.end() || Entry->second.Name != Controller->Name)
            return Landings.insert_or_assign(Controller, LandingState{ Controller->Name }).first->second;
        return Entry->second;
    }

    void NotifyAircraftExit(AFortPlayerControllerAthena* Controller)
    {
        // Le RPC appelle ceci seulement apres une vraie transition dans le bus -> hors du bus.
        if (bFlightConfigured && Controller && !Controller->IsInAircraft() && Controller->MyFortPawn)
            GetLanding(Controller).bExited = true;
    }

    void CheckLandings(AFortGameModeAthena* GameMode)
    {
        auto* State = GetCurrentState(GameMode);
        if (!bZoneReady || !IsPlayingPhase(State) || GameMode->SafeZonePhase != InitialPhase) return;
        int32 Participants = 0;
        int32 Waiting = 0;
        auto Controllers = UFortKismetLibrary::GetAllFortPlayerControllers(GameMode, true, false);
        for (int32 i = 0; i < Controllers.Num(); ++i)
        {
            auto* Controller = Cast<AFortPlayerControllerAthena>(Controllers[i]);
            auto* PlayerState = Controller ? Controller->PlayerState : nullptr;
            auto* Pawn = Controller ? Controller->MyFortPawn : nullptr;
            if (!PlayerState || PlayerState->bIsSpectator || PlayerState->bOnlySpectator || (Pawn && Pawn->bIsDying)) continue;
            ++Participants;
            auto& Landing = GetLanding(Controller);
            if (Controller->IsInAircraft())
            {
                ++Waiting;
                continue;
            }
            if (Pawn)
            {
                // Couvre aussi les sauts forces, qui ne passent pas par le RPC manuel.
                if (Pawn->bIsSkydiving || Pawn->bIsParachuteOpen || Pawn->bIsSkydivingFromBus) Landing.bExited = true;
                if (Landing.bExited) GiveLoadoutOnExit(Controller);
                auto* Movement = Pawn->CharacterMovement;
                if (Landing.bExited && !Landing.bLanded && !Pawn->bIsSkydiving && !Pawn->bIsParachuteOpen && Movement &&
                    Movement->Class->GetFunction("NavMovementComponent", "IsMovingOnGround") && Movement->IsMovingOnGround())
                    Landing.bLanded = true;
            }
            // Une fois pose, un joueur peut sauter de nouveau sans repousser le compte a rebours.
            if (!Landing.bLanded) ++Waiting;
        }
        Controllers.Free();
        if (Participants == 0 || Waiting != 0)
        {
            bZoneScheduled = false;
            FirstMoveAt = 0;
        }
        else if (!bZoneScheduled)
        {
            FirstMoveAt = ServerNow(GameMode, State) + FirstZoneDelay;
            bZoneScheduled = true;
            FN_LOG(LogGameMode, Log, "[LateGame621] Tous les joueurs vivants ont atterri : premier mouvement dans 30 secondes.");
        }
        // Tant qu'un joueur vole, le cercle reste ferme et le delai ne descend pas.
        auto* Indicator = GameMode->SafeZoneIndicator;
        if (!Indicator) return;
        Indicator->SafeZoneStartShrinkTime = bZoneScheduled ? FirstMoveAt : ServerNow(GameMode, State) + FirstZoneDelay;
        Indicator->SafeZoneFinishShrinkTime = Indicator->SafeZoneStartShrinkTime + 0.1f;
        Indicator->ForceNetUpdate();
    }

    void StartZone(AFortGameModeAthena* GameMode)
    {
        // Appele apres le depart natif : vraie tempete sur le cercle initial, pas un apercu.
        ActivateZone(GameMode);
        CheckLandings(GameMode);
    }

    void TickSafeZoneHook(AFortGameModeAthena* GameMode, float DeltaSeconds)
    {
        auto* State = GetCurrentState(GameMode);
        // 0xC97792..BF force bPausedForPreview pendant Aircraft : conserver notre tempete active.
        if (bFlightConfigured && bZoneReady && State && State->GamePhase == EAthenaGamePhase::Aircraft) return;
        TickSafeZoneOriginal(GameMode, DeltaSeconds);
    }

    bool EndAircraftPhaseHook(AFortGameModeAthena* GameMode, bool Arg1, bool Arg2)
    {
        const bool Result = EndAircraftPhaseOriginal(GameMode, Arg1, Arg2);
        auto* State = GetCurrentState(GameMode);
        if (bFlightConfigured && State && State->GamePhase == EAthenaGamePhase::SafeZones)
        {
            CheckLandings(GameMode);
            // EndAircraftPhase efface bSafeZoneActive et SafeZonesStartTime : garder la zone deja creee.
            if (bZoneReady && GameMode->SafeZoneIndicator && GameMode->SafeZonePhase >= InitialPhase)
            {
                GameMode->bSafeZoneActive = true;
                GameMode->SafeZoneIndicator->bPausedForPreview = false;
                State->SafeZonesStartTime = ZoneStartedAt;
                State->ForceNetUpdate();
            }
        }
        return Result;
    }

    void Tick(AFortGameModeAthena* GameMode)
    {
        auto* State = GetCurrentState(GameMode);
        if (!bFlightConfigured || !IsPlayingPhase(State)) return;
        if (!bZoneReady) ActivateZone(GameMode);
        const bool bDeadlineReached = bZoneScheduled && GameMode->SafeZonePhase == InitialPhase && ServerNow(GameMode, State) >= FirstMoveAt;
        if (GetTickCount64() >= NextLoadoutSweep || bDeadlineReached)
        {
            NextLoadoutSweep = GetTickCount64() + 500;
            CheckLandings(GameMode);
        }
        if (bZoneReady && bZoneScheduled && GameMode->SafeZonePhase == InitialPhase && ServerNow(GameMode, State) >= FirstMoveAt)
            AdvanceSafeZoneHook(GameMode);
    }

    void Init()
    {
        if (!Offsets621::IsExpectedBuild()) return;
        const bool FlightReady = MinHook::HookNative(Offsets621::StartAircraftFlight, "StartAircraftFlight", StartFlightHook, (void**)(&StartFlightOriginal));
        const bool EndReady = MinHook::HookNative(Offsets621::EndAircraftPhase, "EndAircraftPhase", EndAircraftPhaseHook, (void**)(&EndAircraftPhaseOriginal));
        const bool StartReady = MinHook::HookNative(Offsets621::StartSafeZone, "StartSafeZone", StartSafeZoneHook, (void**)(&StartSafeZoneOriginal));
        const bool AdvanceReady = MinHook::HookNative(Offsets621::AdvanceSafeZone, "AdvanceSafeZone", AdvanceSafeZoneHook, (void**)(&AdvanceSafeZoneOriginal));
        const bool TickReady = MinHook::HookNative(Offsets621::TickSafeZone, "TickSafeZone", TickSafeZoneHook, (void**)(&TickSafeZoneOriginal));
        bHooksReady = FlightReady && EndReady && StartReady && AdvanceReady && TickReady;
    }
}
#endif // LATE_GAME
