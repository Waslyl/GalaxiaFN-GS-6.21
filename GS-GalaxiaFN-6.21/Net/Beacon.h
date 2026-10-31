#pragma once

namespace GameMode { void TickRestart(UNetDriver* NetDriver); }

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

	// [GALAXIA PATCH] Ne limiter le bind que pendant l'ouverture de notre propre beacon de jeu.
	bool bInitializingGameBeacon = false;
	bool bFixedPortListenReady = false;

	char InitListenHook(UNetDriver* NetDriver, void* InNotify, FURL& LocalURL, bool bReuseAddressAndPort, FString& ListenError)
	{
		const bool bGameListen = bInitializingGameBeacon && Beacon && NetDriver && Beacon->NetDriver == NetDriver;
		if (bGameListen)
		{
			auto* IpDriver = Cast<UIpNetDriver>(NetDriver);
			if (!IpDriver) return false;

			// 6.21 : InitBase 0x442140 passe MaxPortCountToTry (+0x704) + 1 a BindNextPort.
			// Une seule tentative sur le port configure, sans repli silencieux sur 7778/7779.
			// Meme contexte GameNetDriver que l'ancien second appel, des l'unique initialisation.
			NetDriver->World = UWorld::GetWorld();
			NetDriver->NetDriverName = UKismetStringLibrary::Conv_StringToName(L"GameNetDriver");
			IpDriver->MaxPortCountToTry = 0;
			LocalURL.Port = RandomPort;
		}

		const char Result = InitListen(NetDriver, InNotify, LocalURL, bReuseAddressAndPort, ListenError);
		if (bGameListen && Result && LocalURL.Port != RandomPort)
		{
			FN_LOG(LogBeacon, Error, "Port d'ecoute inattendu : %i au lieu de %i.", LocalURL.Port, RandomPort);
			return false;
		}
		return Result;
	}

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
		GameMode::TickRestart(NetDriver);
	}

	void Listen()
	{
		UWorld* World = UWorld::GetWorld();
		if (!World)
		{
			FN_LOG(LogBeacon, Error, "Failed to get World!");
			return;
		}
		if (World->NetDriver) return;
		if (!bFixedPortListenReady)
		{
			FN_LOG(LogBeacon, Error, "InitListen non hooke : ouverture du port fixe annulee.");
			return;
		}

		Beacon = Util::SpawnActor<AOnlineBeaconHost>(AOnlineBeaconHost::StaticClass(), {}, {});
		if (!Beacon)
		{
			FN_LOG(LogBeacon, Error, "Failed to create Beacon!");
			return;
		}

		// [8.51-ONLY] L'ancien premier bind sur un port auxiliaire fuyait au second InitListen.
		// Beacon->ListenPort = RandomPort - 1;
		Beacon->ListenPort = RandomPort;
		bInitializingGameBeacon = true;
		const bool bHostReady = InitHost(Beacon);
		bInitializingGameBeacon = false;
		if (!bHostReady || !Beacon->NetDriver)
		{
			FN_LOG(LogBeacon, Error, "InitHost a echoue sur le port fixe %i.", RandomPort);
			return;
		}

		World->NetDriver = Beacon->NetDriver;
		World->NetDriver->NetDriverName = UKismetStringLibrary::Conv_StringToName(L"GameNetDriver");

		// [8.51-ONLY] InitHost a deja appele InitListen : le refaire ecrasait le socket sans le fermer.
		// InitListen(World->NetDriver, World, URL, true, Error);
		// SetWorld remplace aussi le NetworkNotify du beacon par celui du monde (+0x230).
		SetWorld(World->NetDriver, World);
		World->LevelCollections[0].NetDriver = World->NetDriver;
		World->LevelCollections[1].NetDriver = World->NetDriver;
		FN_LOG(LogBeacon, Log, "Beacon listen on port %i", Beacon->ListenPort);

		// [GALAXIA PATCH] Le detour survit au voyage serveur : ne pas le reinstaller a chaque carte.
		if (!TickFlush)
		{
			uintptr_t PatternTickFlush = MinHook::FindPatternUnique(Patterns::TickFlush, "TickFlush");
			MinHook::HookAddress(PatternTickFlush, "TickFlush", TickFlushHook, (void**)(&TickFlush));
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
		if (Offsets621::IsExpectedBuild())
			bFixedPortListenReady = MinHook::HookAddress(PatternInitListen, "InitListen", InitListenHook, (void**)(&InitListen.Func));
		SetWorld.Func = decltype(SetWorld.Func)(PatternSetWorld);
		SetWorld.Name = "SetWorld";
		ServerReplicateActors.Func = Offsets621::IsExpectedBuild() ? decltype(ServerReplicateActors.Func)(PatternServerReplicateActors) : nullptr;
		ServerReplicateActors.Name = "ServerReplicateActors";

		FN_LOG(LogInit, Log, "InitBeacon Success!");
	}
}
