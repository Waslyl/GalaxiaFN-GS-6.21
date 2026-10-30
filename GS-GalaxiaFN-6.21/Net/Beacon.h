#pragma once

namespace Beacon
{
	// [GALAXIA PATCH] NativeFunc : rempli par pattern, l'appel est ignore (avec log) si le pattern n'est pas trouve en 6.21
	// bool (*InitHost)(AOnlineBeaconHost* OnlineBeaconHost);
	Offsets621::NativeFunc<bool(AOnlineBeaconHost* OnlineBeaconHost)> InitHost;
	// [GALAXIA PATCH] NativeFunc : rempli par pattern, l'appel est ignore (avec log) si le pattern n'est pas trouve en 6.21
	// char (*InitListen)(UNetDriver* NetDriver, void* InNotify, FURL& LocalURL, bool bReuseAddressAndPort, FString& Error);
	Offsets621::NativeFunc<char(UNetDriver* NetDriver, void* InNotify, FURL& LocalURL, bool bReuseAddressAndPort, FString& Error)> InitListen;
	// [GALAXIA PATCH] NativeFunc : rempli par pattern, l'appel est ignore (avec log) si le pattern n'est pas trouve en 6.21
	// void (*SetWorld)(UNetDriver* NetDriver, UWorld* World);
	Offsets621::NativeFunc<void(UNetDriver* NetDriver, UWorld* World)> SetWorld;
	void (*TickFlush)(UNetDriver* NetDriver, float DeltaSeconds);
	// [GALAXIA PATCH] 6.21 : cible verifiee dans les deux vtables, receveur = graphe et non NetDriver.
	Offsets621::NativeFunc<int32(UFortReplicationGraph* ReplicationGraph, float DeltaSeconds)> ServerReplicateActors;

	AOnlineBeaconHost* Beacon = nullptr;
#if defined(SERVER1)
	int32 RandomPort = 7001;
#elif defined(SERVER2)
	int32 RandomPort = 7002;
#elif defined(SERVER3)
	int32 RandomPort = 7003;
#elif defined(SERVER4)
	int32 RandomPort = 7004;
#elif defined(SERVER5)
	int32 RandomPort = 7777;
#endif

	void TickFlushHook(UNetDriver* NetDriver, float DeltaSeconds)
	{
		if (NetDriver)
		{
			if (NetDriver->IsA(UIpNetDriver::StaticClass()) &&
				NetDriver->ClientConnections.Num() > 0 &&
				NetDriver->ClientConnections[0] &&
				!NetDriver->ClientConnections[0]->InternalAck &&
				NetDriver->ReplicationDriver &&
				NetDriver->ReplicationDriver->IsA(UFortReplicationGraph::StaticClass()) &&
				ServerReplicateActors)
			{
				auto* Graph = static_cast<UFortReplicationGraph*>(NetDriver->ReplicationDriver);
				ServerReplicateActors(Graph, DeltaSeconds);
			}
		}

		TickFlush(NetDriver, DeltaSeconds);
	}

	void Listen()
	{
		Beacon = Util::SpawnActor<AOnlineBeaconHost>(AOnlineBeaconHost::StaticClass(), {}, {});
		UWorld* World = Globals::GetWorld();
		
		if (!Beacon || !World)
		{
			FN_LOG(LogBeacon, Error, "Failed to create Beacon or to get World!");
			return;
		}

		Beacon->ListenPort = RandomPort - 1;

		if (World->NetDriver)
		{
			FN_LOG(LogBeacon, Warning, "Beacon already created!");
			return;
		}

		if (InitHost(Beacon))
		{
			World->NetDriver = Beacon->NetDriver;
			FN_LOG(LogBeacon, Log, "Beacon created successful!");
		}
		else
		{
			FN_LOG(LogBeacon, Error, "Failed to InitHost!");
			return;
		}

		if (World->NetDriver)
		{
			World->NetDriver->World = World;
			World->NetDriver->NetDriverName = UKismetStringLibrary::Conv_StringToName(L"GameNetDriver");

			FString Error;
			auto URL = FURL();
			URL.Port = RandomPort;

			InitListen(World->NetDriver, World, URL, true, Error);
			SetWorld(World->NetDriver, World);

			World->LevelCollections[0].NetDriver = World->NetDriver;
			World->LevelCollections[1].NetDriver = World->NetDriver;

			FN_LOG(LogBeacon, Log, "Beacon listen on port %i", URL.Port);

			uintptr_t PatternTickFlush = MinHook::FindPatternUnique(Patterns::TickFlush, "TickFlush");

			MinHook::HookAddress(PatternTickFlush, "TickFlush", TickFlushHook, (void**)(&TickFlush)); // [GALAXIA PATCH] rien n'est installe si le pattern n'est pas trouve
		}
		else
		{
			FN_LOG(LogBeacon, Error, "Failed to listen!");
		}
	}

	void InitBeacon()
	{
		uintptr_t PatternInitHost = MinHook::FindPatternUnique(Patterns::InitHost, "InitHost");
		uintptr_t PatternInitListen = MinHook::FindPatternUnique(Patterns::InitListen, "InitListen");
		uintptr_t PatternSetWorld = MinHook::FindPatternUnique(Patterns::SetWorld, "SetWorld");
		uintptr_t PatternServerReplicateActors = MinHook::FindPatternUnique(Patterns::ServerReplicateActors, "ServerReplicateActors");

		InitHost.Func = decltype(InitHost.Func)(PatternInitHost);
		InitHost.Name = "InitHost";
		InitListen.Func = decltype(InitListen.Func)(PatternInitListen);
		InitListen.Name = "InitListen";
		SetWorld.Func = decltype(SetWorld.Func)(PatternSetWorld);
		SetWorld.Name = "SetWorld";
		ServerReplicateActors.Func = Offsets621::IsExpectedBuild() ? decltype(ServerReplicateActors.Func)(PatternServerReplicateActors) : nullptr;
		ServerReplicateActors.Name = "ServerReplicateActors";

		FN_LOG(LogInit, Log, "InitBeacon Success!");
	}
}
