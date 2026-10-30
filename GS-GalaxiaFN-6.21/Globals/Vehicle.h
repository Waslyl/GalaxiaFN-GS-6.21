#pragma once

namespace Vehicle
{
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

	void (*ServerRequestSeatChangeOG)(AFortPlayerControllerAthena* PlayerControllerAthena, int32 TargetSeatIndex);
	void ServerRequestSeatChangeHook(AFortPlayerControllerAthena* PlayerControllerAthena, int32 TargetSeatIndex)
	{
		if (AFortPlayerPawn* PlayerPawn = PlayerControllerAthena->MyFortPawn)
		{
			AFortAthenaVehicle* Vehicle = PlayerPawn->VehicleStateLocal.Vehicle;
			if (!Vehicle) return ServerRequestSeatChangeOG(PlayerControllerAthena, TargetSeatIndex);

			int32 SeatIndex = PlayerPawn->VehicleStateLocal.SeatIndex;

			if (SeatIndex != TargetSeatIndex)
			{
				UFortWeaponItemDefinition* VehicleWeaponItemDefinition = Functions::GetVehicleWeaponItemDefinition(Vehicle->Class, SeatIndex);

				if (VehicleWeaponItemDefinition)
					Functions::UnEquipVehicleWeapon(PlayerControllerAthena, VehicleWeaponItemDefinition);
			}

			UFortWeaponItemDefinition* VehicleWeaponItemDefinition = Functions::GetVehicleWeaponItemDefinition(Vehicle->Class, PlayerPawn->VehicleStateLocal.SeatIndex);

			if (VehicleWeaponItemDefinition)
				Functions::EquipVehicleWeapon(PlayerControllerAthena, VehicleWeaponItemDefinition);
		}

		ServerRequestSeatChangeOG(PlayerControllerAthena, TargetSeatIndex);
	}

	void (*ServerAttemptExitVehicleOG)(AFortPlayerControllerAthena* PlayerControllerAthena);
	void ServerAttemptExitVehicleHook(AFortPlayerControllerAthena* PlayerControllerAthena)
	{
		if (AFortPlayerPawn* PlayerPawn = PlayerControllerAthena->MyFortPawn)
		{
			AFortAthenaVehicle* Vehicle = PlayerPawn->VehicleStateLocal.Vehicle;
			if (!Vehicle) return ServerAttemptExitVehicleOG(PlayerControllerAthena);

			int32 SeatIndex = PlayerPawn->VehicleStateLocal.SeatIndex;
			UFortWeaponItemDefinition* VehicleWeaponItemDefinition = Functions::GetVehicleWeaponItemDefinition(Vehicle->Class, PlayerPawn->VehicleStateLocal.SeatIndex);

			if (VehicleWeaponItemDefinition)
				Functions::UnEquipVehicleWeapon(PlayerControllerAthena, VehicleWeaponItemDefinition);
		}

		ServerAttemptExitVehicleOG(PlayerControllerAthena);
	}

	// [8.51-ONLY] AFortOctopusVehicle (Baller) n'existe pas en 6.21
	// void ServerUpdateTowhookHook(AFortOctopusVehicle* OctopusVehicle, const FVector_NetQuantizeNormal& InNetTowhookAimDir)
	// {
		// bool bIsTowhookAttached = OctopusVehicle->IsTowhookAttached();
		// bool bIsTowhookContracting = OctopusVehicle->IsTowhookContracting();
		// bool bIsTowhookExtending = OctopusVehicle->IsTowhookExtending();

		// OctopusVehicle->NetTowhookAimDir = InNetTowhookAimDir;
		// OctopusVehicle->OnRep_NetTowhookAimDir();

		// FN_LOG(LogInit, Log, "ServerUpdateTowhookHook called, bIsTowhookAttached: %i, bIsTowhookContracting: %i, bIsTowhookExtending: %i", bIsTowhookAttached, bIsTowhookContracting, bIsTowhookExtending);
	// }

	// [8.51-ONLY] Canon pirate (AFortWeaponRangedMountedCannon, AFortMountedCannon, AFortAthenaSKPushCannon) n'existe pas en 6.21
	// void ServerFireActorInCannonHook(AFortWeaponRangedMountedCannon* RangedMountedCannonn, const FVector& LaunchDir)
	// {
		// AFortPlayerPawn* InstigatorPlayerPawn = Cast<AFortPlayerPawn>(RangedMountedCannonn->Instigator);
		// if (!InstigatorPlayerPawn) return;

		// AFortAthenaSKPushCannon* AthenaSKPushCannon = Cast<AFortAthenaSKPushCannon>(InstigatorPlayerPawn->VehicleStateLocal.Vehicle);

		// if (!AthenaSKPushCannon)
		// {
			// AFortMountedCannon* MountedCannon = Cast<AFortMountedCannon>(InstigatorPlayerPawn->VehicleStateLocal.Vehicle);
			// if (!MountedCannon) return;

			// AFortPlayerPawn* PlayerPawn = Cast<AFortPlayerPawn>(MountedCannon->GetPawnAtSeat(0));
			// if (!PlayerPawn) return;

			// AFortPlayerControllerAthena* PlayerControllerAthena = Cast<AFortPlayerControllerAthena>(PlayerPawn->Controller);
			// if (!PlayerControllerAthena) return;

			// PlayerControllerAthena->ServerAttemptExitVehicle();
			// MountedCannon->OnLaunchPawn(PlayerPawn);
			// return;
		// }

		// AFortPlayerPawn* PlayerPawn = Cast<AFortPlayerPawn>(AthenaSKPushCannon->GetPawnAtSeat(1));
		// if (!PlayerPawn) return;

		// AFortPlayerControllerAthena* PlayerControllerAthena = Cast<AFortPlayerControllerAthena>(PlayerPawn->Controller);
		// if (!PlayerControllerAthena) return;

		// AthenaSKPushCannon->OnPreLaunchPawn(PlayerPawn, LaunchDir);
		// PlayerControllerAthena->ServerAttemptExitVehicle();
		// //PlayerPawn->ServerOnExitVehicle();
		// AthenaSKPushCannon->OnLaunchPawn(PlayerPawn, LaunchDir);
		// AthenaSKPushCannon->MultiCastPushCannonLaunchedPlayer();
		// /*
			// - 1250
		
			// "ImpulsePower": {
				// "Value": 1.0,
				// "Curve": {
				  // "CurveTable": {
					// "ObjectName": "CurveTable'AthenaGameData'",
					// "ObjectPath": "FortniteGame/Content/Athena/Balance/DataTables/AthenaGameData.0"
				  // },
				  // "RowName": "Default.PirateCannon.ImpulsePower"
				// }
			// }
		// */

		// /*
			// - 3800
		
			// "PlayerLaunchSpeed": {
				// "Value": 1.0,
				// "Curve": {
				  // "CurveTable": {
					// "ObjectName": "CurveTable'AthenaGameData'",
					// "ObjectPath": "FortniteGame/Content/Athena/Balance/DataTables/AthenaGameData.0"
				  // },
				  // "RowName": "Default.PirateCannon.LaunchVelocity"
				// }
			// }
		// */

		// /*
			// Log: LogInit: Info: ServerFireActorInCannonHook called - LaunchDir: X: 0.73, Y: -0.38, Z: 0.56
			// Log: LogInit: Info: ServerFireActorInCannonHook called - LaunchDir: X: 0.86, Y: -0.48, Z: 0.15
			// Log: LogInit: Info: ServerFireActorInCannonHook called - LaunchDir: X: 0.87, Y: -0.50, Z: 0.04
			// Log: LogInit: Info: ServerFireActorInCannonHook called - LaunchDir: X: 0.86, Y: -0.50, Z: -0.06
			// Log: LogInit: Info: ServerFireActorInCannonHook called - LaunchDir: X: 0.87, Y: -0.50, Z: -0.05
			// Log: LogInit: Info: ServerFireActorInCannonHook called - LaunchDir: X: 0.87, Y: -0.50, Z: 0.00
			// Log: LogInit: Info: ServerFireActorInCannonHook called - LaunchDir: X: 0.87, Y: -0.50, Z: -0.02
			// Log: LogInit: Info: ServerFireActorInCannonHook called - LaunchDir: X: 0.93, Y: -0.34, Z: 0.10
		// */

		// FN_LOG(LogInit, Log, "ServerFireActorInCannonHook called - LaunchDir: X: %.2f, Y: %.2f, Z: %.2f", LaunchDir.X, LaunchDir.Y, LaunchDir.Z);
	// }

	void InitVehicle()
	{
		AFortAthenaVehicle* AFortPhysicsPawnDefault = AFortAthenaVehicle::GetDefaultObj(); // AFortPhysicsPawn en 8.51
		UClass* AFortPhysicsPawnClass = AFortAthenaVehicle::StaticClass(); // AFortPhysicsPawn en 8.51

		UFunction* ServerUpdatePhysicsParamsFunc = AFortPhysicsPawnClass->GetFunction("FortAthenaVehicle", "ServerUpdatePhysicsParams"); // "FortPhysicsPawn" en 8.51
		MinHook::HookFunctionExec(ServerUpdatePhysicsParamsFunc, ServerUpdatePhysicsParamsHook, nullptr);

		// [OFFSET-TODO-6.21] OnEnteredVehicle (8.51 = 0x1af3570)
		// MH_CreateHook((LPVOID)(InSDKUtils::GetImageBase() + 0x1af3570), OnEnteredVehicleHook, (LPVOID*)(&OnEnteredVehicleOG));
		// MH_EnableHook((LPVOID)(InSDKUtils::GetImageBase() + 0x1af3570));
		MinHook::HookNative(Offsets621::OnEnteredVehicle, "OnEnteredVehicle", OnEnteredVehicleHook, (void**)(&OnEnteredVehicleOG));

		AFortPlayerControllerAthena* FortPlayerControllerAthenaDefault = AFortPlayerControllerAthena::GetDefaultObj();
		UClass* FortPlayerControllerAthenaClass = AFortPlayerControllerAthena::StaticClass();

		// Hook par ProcessEvent (parametres verifies au demarrage) (8.51 = 0x1DE0 / 8)
		// MinHook::HookVTable(FortPlayerControllerAthenaDefault, 0x1DE0 / 8, ServerRequestSeatChangeHook, (LPVOID*)(&ServerRequestSeatChangeOG), "AFortPlayerControllerAthena::ServerRequestSeatChange");
		MinHook::HookRPC<ServerRequestSeatChangeHook>(FortPlayerControllerAthenaDefault, "FortPlayerControllerZone", "ServerRequestSeatChange", (void**)(&ServerRequestSeatChangeOG), 0x1DE0 / 8);
		// Hook par ProcessEvent (parametres verifies au demarrage) (8.51 = 0x1E78 / 8)
		// MinHook::HookVTable(FortPlayerControllerAthenaDefault, 0x1E78 / 8, ServerAttemptExitVehicleHook, (LPVOID*)(&ServerAttemptExitVehicleOG), "AFortPlayerControllerAthena::ServerAttemptExitVehicle");
		MinHook::HookRPC<ServerAttemptExitVehicleHook>(FortPlayerControllerAthenaDefault, "FortPlayerControllerZone", "ServerAttemptExitVehicle", (void**)(&ServerAttemptExitVehicleOG), 0x1E78 / 8);

		// [8.51-ONLY] AFortOctopusVehicle (Baller) n'existe pas en 6.21
		// AFortOctopusVehicle* FortOctopusVehicleDefault = AFortOctopusVehicle::GetDefaultObj();
		// UClass* FortOctopusVehicleClass = AFortOctopusVehicle::StaticClass();

		// MinHook::HookVTable(FortOctopusVehicleDefault, 0xE40 / 8, ServerUpdateTowhookHook, nullptr, "AFortOctopusVehicle::ServerUpdateTowhook");
		
		// [8.51-ONLY] Canon pirate (AFortWeaponRangedMountedCannon) n'existe pas en 6.21
		// AFortWeaponRangedMountedCannon* FortWeaponRangedMountedCannonDefault = AFortWeaponRangedMountedCannon::GetDefaultObj();
		// UClass* FortWeaponRangedMountedCannonClass = AFortWeaponRangedMountedCannon::StaticClass();

		// MinHook::HookVTable(FortWeaponRangedMountedCannonDefault, 0xA88 / 8, ServerFireActorInCannonHook, nullptr, "AFortWeaponRangedMountedCannon::ServerFireActorInCannon");

		FN_LOG(LogInit, Log, "InitVehicle Success!");
	}
}