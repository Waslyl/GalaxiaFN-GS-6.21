#pragma once
#include <iostream>
#include <string>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

extern std::string storedSessionId;

namespace Hooks
{
	size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* user_data) {
		size_t total_size = size * nmemb;
		user_data->append(static_cast<char*>(contents), total_size);
		return total_size;
	}

	void (*ProcessEvent)(UObject* Object, UFunction* Function, void* Parms);
	void (*DispatchRequest)(__int64 a1, __int64* a2, int a3);
	bool (*KickPlayerOG)(AGameSession* GameSession, APlayerController* PlayerController, const FText& Reason);
	void (*McpSendRequestNowOG)(UMcpProfileGroup* ProfileGroup, void* RequestContext);
	bool (*SessionNoAuthParamOG)(const wchar_t* CommandLine, const wchar_t* ParamName);
	bool (*PoiPawnOverlapOG)(AFortPoiVolume* Volume, AFortPawn* Pawn);
	void (*GetPlayerViewPoint)(AFortPlayerController* PlayerController, FVector& out_Location, FRotator& out_Rotation);
	void (*CollectGarbageInternal)(unsigned int a1, unsigned __int8 a2);

	// Webhook Stuff
    void SendJoinableGameWebhook(const std::string& serverName, const std::string& description) {
        std::string title = "Serveur Galaxia " + serverName;
        
    #if defined(SERVER1)
        title += " n°1";
    #elif defined(SERVER2)
        title += " n°2";
    #elif defined(SERVER3)
        title += " n°3";
    #elif defined(SERVER4)
        title += " n°4";
    #elif defined(SERVER5)
        title += " n°5";
    #endif

		std::string fr = "Vous pouvez rejoindre la partie en " + description + ", elle va commencer dans 1m30s! <:llama46:1224085002384445570>";
		std::string eng = "You can join the game in " + description + ", it will start in 1m30s! <:llama46:1224085002384445570>";

        std::cout << "Webhook message: " << fr << std::endl;
		std::cout << "Webhook message: " << eng << std::endl;

        UptimeWebHook.send_embed(title, fr, eng, 65280);

        Globals::wasWebhookSent = true;
    }

	void GetPlayerViewPointHook(AFortPlayerController* PlayerController, FVector& out_Location, FRotator& out_Rotation)
	{
		APawn* Pawn = PlayerController->Pawn;
		ASpectatorPawn* SpectatorPawn = PlayerController->GetSpectatorPawn();

		if (Pawn)
		{
			out_Location = Pawn->K2_GetActorLocation();
			out_Rotation = PlayerController->GetControlRotation();
			return;
		}
		else if (SpectatorPawn && PlayerController->HasAuthority())
		{
			out_Location = SpectatorPawn->K2_GetActorLocation();
			out_Rotation = ((APlayerController*)SpectatorPawn->Owner)->GetControlRotation();
			return;
		}
		else if (!SpectatorPawn && !Pawn)
		{
			out_Location = PlayerController->LastSpectatorSyncLocation;
			out_Rotation = PlayerController->LastSpectatorSyncRotation;
			return;
		}

		GetPlayerViewPoint(PlayerController, out_Location, out_Rotation);
	}

	static ENetMode GetNetModeHook()
	{
		return ENetMode::NM_DedicatedServer;
	}

	// [OFFSET-TODO-6.21] GIsClient / GIsServer (8.51 = 0x5940A13 / 0x5940A14) : renvoie 0 tant que l'adresse 6.21 n'est pas trouvee
	inline int64_t GIsClient()
	{
		// return __int64(GetModuleHandleW(0)) + 0x5940A13;
		return __int64(Offsets621::Global<bool>(Offsets621::GIsClient, "GIsClient"));
	}

	inline int64_t GIsServer()
	{
		// return __int64(GetModuleHandleW(0)) + 0x5940A14;
		return __int64(Offsets621::Global<bool>(Offsets621::GIsServer, "GIsServer"));
	}

	__int64* ChangingGameSessionIdHook(__int64 a1, __int64 a2)
	{
		// [GALAXIA PATCH] Conserve le contournement de session native du GS 8.51.
		// En 6.21, l'originale accede a un service absent du client pendant l'initialisation du GameState.
		static bool bLogged = false;
		if (!bLogged)
		{
			FN_LOG(LogHook, Log, "ChangingGameSessionId : mise a jour de session native ignoree (serveur prive)");
			bLogged = true;
		}
		return 0;
	}

	// [GALAXIA PATCH] La presence MCP est une fonction du client, inutile au serveur prive.
	// Le timer peut encore l'appeler apres le changement de carte avec un controleur invalide.
	// Ne pas lire le controleur ici : le crash 6.21 est justement sa lecture de vtable (0x12AE0F9).
	void UpdateRichPresenceHook(AFortPlayerController*, ERichPresenceStateChange, bool)
	{
		static bool bLogged = false;
		if (!bLogged)
		{
			FN_LOG(LogHook, Log, "UpdateRichPresence : presence MCP du client ignoree (serveur prive)");
			bLogged = true;
		}
	}

	// [GALAXIA PATCH] 6.21 : le helper natif lit BrushComponent->Bounds sans verifier le composant.
	// Les volumes dont PostInitializeComponents a echoue n'ont pas de geometrie exploitable.
	bool PoiPawnOverlapHook(AFortPoiVolume* Volume, AFortPawn* Pawn)
	{
		if (!Volume || !Pawn || !Volume->BrushComponent || !Volume->PoiCollisionComp || !Pawn->CapsuleComponent)
		{
			static bool bLogged = false;
			if (!bLogged)
			{
				FN_LOG(LogHook, Warning, "PoiPawnOverlap : volume/pawn sans composant de collision, test ignore");
				bLogged = true;
			}
			return false;
		}

		return PoiPawnOverlapOG(Volume, Pawn);
	}

	void DispatchRequestHook(__int64 a1, __int64* a2, int a3)
	{
		DispatchRequest(a1, a2, 3);
	}

	bool CanActivateAbilityHook()
	{
		return 1;
	}

	// [GALAXIA PATCH] Utilise le chemin NoAuth deja prevu par les deux validations de session 6.21.
	// Bloquer seulement AGameSession::KickPlayer arrive trop tard : l'override Fortnite a deja mis
	// bIsDisconnecting a true via SetIsDisconnecting (0x12A0390), ce qui bloque le chargement du joueur.
	bool SessionNoAuthParamHook(const wchar_t* CommandLine, const wchar_t* ParamName)
	{
		if (ParamName && _wcsicmp(ParamName, L"NoAuth") == 0)
		{
			const uintptr_t CallerRva = reinterpret_cast<uintptr_t>(_ReturnAddress()) - InSDKUtils::GetImageBase();
			if (CallerRva == Offsets621::Session_NoAuthValidationReturn ||
				CallerRva == Offsets621::Session_NoAuthReservationReturn)
			{
				static bool bLogged = false;
				if (!bLogged)
				{
					FN_LOG(LogHook, Log, "SessionNoAuth : validation de session privee sans reservation Epic activee");
					bLogged = true;
				}
				return true;
			}
		}
		return SessionNoAuthParamOG(CommandLine, ParamName);
	}

	// [GALAXIA PATCH] Le serveur prive n'a pas les reservations du matchmaking Epic.
	// Ne bloque que ce rejet : les autres motifs continuent vers le KickPlayer d'origine.
	bool KickPlayerHook(AGameSession* GameSession, APlayerController* PlayerController, const FText& Reason)
	{
		if (Reason.TextData && Reason.GetStringRef().Num() > 0 && Reason.ToString() == "No reservation in game.")
		{
			static bool bLogged = false;
			if (!bLogged)
			{
				FN_LOG(LogHook, Log, "KickPlayer : rejet No reservation in game ignore (serveur prive)");
				bLogged = true;
			}
			return false;
		}

		return KickPlayerOG(GameSession, PlayerController, Reason);
	}

	// [GALAXIA PATCH] Repli 6.21 du routage de DispatchRequest(..., 3).
	// Ne convertit que les commandes dedicated_server : les autres types sont conserves.
	void McpSendRequestNowHook(UMcpProfileGroup* ProfileGroup, void* RequestContext)
	{
		if (RequestContext)
		{
			auto& CommandType = *reinterpret_cast<int32*>(static_cast<uint8*>(RequestContext) + Offsets621::McpRequestContext_CommandType);
			if (CommandType == 1)
			{
				CommandType = 3;
				static bool bLogged = false;
				if (!bLogged)
				{
					FN_LOG(LogHook, Log, "McpSendRequestNow : commande dedicated_server routee via le client (1 -> 3)");
					bLogged = true;
				}
			}
		}
		McpSendRequestNowOG(ProfileGroup, RequestContext);
	}

	void CollectGarbageInternalHook(unsigned int a1, unsigned __int8 a2)
	{
		//FN_LOG(LogHook, Log, "Function Call CollectGarbageInternalHook");
		return CollectGarbageInternal(a1, a2);
	}

	bool bLogs = false;
	
	void ProcessEventHook(UObject* Object, UFunction* Function, void* Parms)
	{
		if (!Object || !Function)
		{
			ProcessEvent(Object, Function, Parms);
			return;
		}

		// [GALAXIA PATCH] Hooks RPC / events 6.21 (MinHook::HookRPC) : la fonction d'origine n'est pas appelee si un hook la prend en charge
		if (MinHook::DispatchProcessEvent(Object, Function, Parms))
			return;

		static auto ReceiveTickFunc = FindObjectFast<UFunction>("/Script/Engine.Actor.ReceiveTick");

		if (Function == ReceiveTickFunc)
		{
#ifdef BOTS
			Bots::Tick();

			if (GetAsyncKeyState(VK_F4) & 0x1)
			{
				AFortPlayerController* PlayerController = Cast<AFortPlayerController>(UGameplayStatics::GetPlayerController(Globals::GetWorld(), 0));

				if (!PlayerController)
					return;

				AFortPlayerPawn* PlayerPawn = PlayerController->MyFortPawn;

				if (!PlayerPawn)
					return;

				Bots::SpawnBot(PlayerPawn);
			}
#endif // BOTS

			if (GetAsyncKeyState(VK_F5) & 0x1)
			{

			}

			if (GetAsyncKeyState(VK_F6) & 0x1)
			{

			}

			if (GetAsyncKeyState(VK_F7) & 0x1)
			{
				bLogs = bLogs ? false : true;
			}

			return;
		}

		const std::string& FunctionName = Function->GetName();

		if (FunctionName.contains("OnWorldReady"))
		{
			if (!Globals::GetGameMode()->bWorldIsReady)
			{
#ifdef SESSION

#ifdef NORMAL
				SendJoinableGameWebhook("Solo", "Solo");
#endif

#ifdef ARENA
				SendJoinableGameWebhook("Lategame Arena", "Lategame Arena");
#endif

#ifdef AVENGERS
				SendJoinableGameWebhook("Avengers", "Avengers");
#endif

#ifdef LATE_GAME
				SendJoinableGameWebhook("Lategame", "Lategame");
#endif

#ifdef DODGE_ROCKET
				SendJoinableGameWebhook("Esquive Rocket", "Esquive Rocket");
#endif

#ifdef ONESHOT
				SendJoinableGameWebhook("Lategame Oneshot", "Lategame Oneshot");
#endif

#ifdef PLAYGROUND
				SendJoinableGameWebhook("Terrain de Jeu", "Terrain de Jeu");
#endif

				int enableCreatingAuto = 1;

				if (enableCreatingAuto == 1) {
					std::string ip = ""; // faut pas déconner
					int port = Beacon::RandomPort;
					std::string port_str = std::to_string(port);

#ifdef ARENA
					std::string url = "https://galaxia-secure-backend.mega-hosting.fr:2083/fortnite/game/privaterelay/creatematch/Playlist_ShowdownAlt_Solo/" + ip + "/" + port_str; // Arena
#endif ARENA

#ifdef PLAYGROUND
					std::string url = "https://galaxia-secure-backend.mega-hosting.fr:2083/fortnite/game/privaterelay/creatematch/Playlist_Playground/" + ip + "/" + port_str; // Terrain de Jeu
#endif PLAYGROUND

#ifdef NORMAL
					std::string url = "https://galaxia-secure-backend.mega-hosting.fr:2083/fortnite/game/privaterelay/creatematch/Playlist_DefaultSolo/" + ip + "/" + port_str; // Solo
#endif NORMAL

#ifdef ONESHOT
					std::string url = "https://galaxia-secure-backend.mega-hosting.fr:2083/fortnite/game/privaterelay/creatematch/Playlist_Snipers_Solo/" + ip + "/" + port_str; // Oneshot
#endif ONESHOT

#ifdef DODGE_ROCKET
					std::string url = "https://galaxia-secure-backend.mega-hosting.fr:2083/fortnite/game/privaterelay/creatematch/Playlist_HighExplosives_Duos/" + ip + "/" + port_str; // Esquive rocket
#endif DODGE_ROCKET

#ifdef LATE_GAME
					std::string url = "https://galaxia-secure-backend.mega-hosting.fr:2083/fortnite/game/privaterelay/creatematch/Playlist_DefaultSquad/" + ip + "/" + port_str; // solo Lategame
#endif LATE_GAME

#ifdef AVENGERS
					std::string url = "https://galaxia-secure-backend.mega-hosting.fr:2083/fortnite/game/privaterelay/creatematch/Playlist_Ashton_Sm/" + ip + "/" + port_str; // Endgame
#endif AVENGERS

					// On utilise curl (logique)
					CURL* curl = curl_easy_init();
					if (curl) {
						std::string response_string; // stock la réponse JSON

						curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
						curl_easy_setopt(curl, CURLOPT_USERAGENT, "GalaxiaMatchDeleter/1.0");

						// recupérer la réponse
						curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
						curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
						CURLcode res = curl_easy_perform(curl);

						if (res != CURLE_OK) {
							std::cerr << "Erreur lors de la requête: " << curl_easy_strerror(res) << std::endl;
						}
						else {
							std::cout << "Réponse JSON reçue: " << std::endl;
							std::cout << response_string << std::endl;

							try {
								auto json_response = nlohmann::json::parse(response_string);

								std::string sessionId = json_response["session"]["sessionId"];
								std::cout << "Le sessionId est: " << sessionId << std::endl;

								storedSessionId = sessionId;

							}
							catch (const std::exception& e) {
								std::cerr << "Erreur lors du parsing JSON: " << e.what() << std::endl;
							}
						}
						curl_easy_cleanup(curl);
					}
					else {
						std::cerr << "Erreur lors de l'initialisation de Curl." << std::endl;
					}
				}

				// PARTIE GAME START
				CURL* curl = curl_easy_init();

				std::string url2 = "https://galaxia-secure-backend.mega-hosting.fr:2083/fortnite/game/privaterelay/startmatch/" + storedSessionId;

				curl_easy_setopt(curl, CURLOPT_URL, url2.c_str());
				curl_easy_setopt(curl, CURLOPT_USERAGENT, "GalaxiaMatchDeleter/1.0");
				CURLcode res = curl_easy_perform(curl);

				long http_code = 0;
				curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

				if (res != CURLE_OK) {
					std::cerr << "Erreur de requête: " << curl_easy_strerror(res) << std::endl;
				}
				else {
					if (http_code == 200) {
						std::cout << "Réponse 200 OK reçue." << std::endl;
					}
					else {
						std::cout << "Code de réponse HTTP: " << http_code << std::endl;
					}
				}
				curl_easy_cleanup(curl);

#endif // SESSION

				auto GameModeAthena = Globals::GetGameMode();
				auto GameStateAthena = Globals::GetGameState();
				
				if (GameModeAthena && GameStateAthena && false)
				{
					float ServerWorldTimeSeconds = UGameplayStatics::GetTimeSeconds(GameModeAthena) + GameStateAthena->ServerWorldTimeSecondsDelta;
					float Duration = 900.0f; // Seconds

					GameStateAthena->WarmupCountdownStartTime = Duration;
					GameStateAthena->WarmupCountdownEndTime = ServerWorldTimeSeconds + Duration;

					GameModeAthena->WarmupEarlyCountdownDuration = ServerWorldTimeSeconds;
					GameModeAthena->WarmupCountdownDuration = Duration;
				}

				FN_LOG(LogHooks, Log, "OnWorldReady!");
				Globals::GetGameMode()->bWorldIsReady = true;
			}
		}

		if (bLogs)
		{
			if (!FunctionName.contains("Tick") &&
				!FunctionName.contains("Visual") &&
				!FunctionName.contains("Clown Spinner") &&
				!FunctionName.contains("CustomStateChanged") &&
				!FunctionName.contains("ReceiveBeginPlay") &&
				!FunctionName.contains("OnAttachToBuilding") &&
				!FunctionName.contains("OnWorldReady") &&
				!FunctionName.contains("K2_GetActorLocation") &&
				!FunctionName.contains("ReceiveDrawHUD") &&
				!FunctionName.contains("ServerUpdateCamera") &&
				!FunctionName.contains("ServerMove") &&
				!FunctionName.contains("ContrailCheck") &&
				!FunctionName.contains("GetViewTarget") &&
				!FunctionName.contains("GetAllActorsOfClass") &&
				!FunctionName.contains("ClientAckGoodMove") &&
				!FunctionName.contains("ReadyToEndMatch") &&
				!FunctionName.contains("Check Closest Point") &&
				!FunctionName.contains("ServerTriggerCombatEvent") &&
				!FunctionName.contains("UpdateTime") &&
				!FunctionName.contains("OnUpdateMusic") &&
				!FunctionName.contains("UpdateStateEvent") &&
				!FunctionName.contains("ServerTouchActiveTime") &&
				!FunctionName.contains("OnCheckIfSurrounded") &&
				!FunctionName.contains("ServerFireAIDirectorEventBatch") &&
				!FunctionName.contains("ServerTriggerCombatEventBatch") &&
				!FunctionName.contains("UserConstructionScript") &&
				!FunctionName.contains("K2_OnReset") &&
				!FunctionName.contains("K2_OnEndViewTarget") &&
				!FunctionName.contains("K2_OnBecomeViewTarget") &&
				!FunctionName.contains("ReceiveUnpossessed") &&
				!FunctionName.contains("ClientGotoState") &&
				!FunctionName.contains("K2_OnEndViewTarget") &&
				!FunctionName.contains("K2_OnBecomeViewTarget") &&
				!FunctionName.contains("ClientSetViewTarget") &&
				!FunctionName.contains("ServerClientPawnLoaded") &&
				!FunctionName.contains("ReceiveEndPlay") &&
				!FunctionName.contains("OnPerceptionStimuliSourceEndPlay") &&
				!FunctionName.contains("HandleOnHUDElementVisibilityChanged") &&
				!FunctionName.contains("OnHUDElementVisibilityChanged") &&
				!FunctionName.contains("HandleInteractionChanged") &&
				!FunctionName.contains("BlueprintModifyCamera") &&
				!FunctionName.contains("BlueprintModifyPostProcess") &&
				!FunctionName.contains("ServerSetSpectatorLocation") &&
				!FunctionName.contains("ServerFireAIDirectorEvent") &&
				!FunctionName.contains("ServerTryActivateAbility") &&
				!FunctionName.contains("ClientActivateAbilitySucceed") &&
				!FunctionName.contains("K2_CommitExecute") &&
				!FunctionName.contains("ServerSetSpectatorLocation") &&
				!FunctionName.contains("CanJumpInternal") &&
				!FunctionName.contains("K2_OnMovementModeChanged") &&
				!FunctionName.contains("OnJumped") &&
				!FunctionName.contains("ServerModifyStat") &&
				!FunctionName.contains("OnLanded") &&
				!FunctionName.contains("ReceiveHit") &&
				!FunctionName.contains("OnWalkingOffLedge") &&
				!FunctionName.contains("ServerEndAbility") &&
				!FunctionName.contains("Execute") &&
				!FunctionName.contains("NetMulticast_InvokeGameplayCueExecuted_FromSpec") &&
				!FunctionName.contains("OnDamagePlayEffects") &&
				!FunctionName.contains("OnMontageStarted") &&
				!FunctionName.contains("OnNewDamageNumber") &&
				!FunctionName.contains("BP_GetTokenizedDescriptionText") &&
				!FunctionName.contains("NetMulticast_InvokeGameplayCueExecuted_WithParams") &&
				!FunctionName.contains("GameplayCue_InstantDeath") &&
				!FunctionName.contains("K2_GetActorRotation") &&
				!FunctionName.contains("K2_DestroyActor") &&
				!FunctionName.contains("OnDetachFromBuilding") &&
				!FunctionName.contains("OnRep_bAlreadySearched") &&
				!FunctionName.contains("OnSetSearched") &&
				!FunctionName.contains("GetAircraft") &&
				!FunctionName.contains("BeginSpawningActorFromClass") &&
				!FunctionName.contains("BlueprintInitializeAnimation") &&
				!FunctionName.contains("BlueprintUpdateAnimation") &&
				!FunctionName.contains("BlueprintPostEvaluateAnimation") &&
				!FunctionName.contains("FinishSpawningActor") &&
				!FunctionName.contains("PawnUniqueIDSet") &&
				!FunctionName.contains("OnRep_Owner") &&
				!FunctionName.contains("OnRep_Pawn") &&
				!FunctionName.contains("Possess") &&
				!FunctionName.contains("ReceivePossessed") &&
				!FunctionName.contains("ClientRestart") &&
				!FunctionName.contains("SetControlRotation") &&
				!FunctionName.contains("ClientRetryClientRestart") &&
				!FunctionName.contains("ExecuteUbergraph_PlayerPawn_Athena_Generic") &&
				!FunctionName.contains("ExecuteUbergraph_PlayerPawn_Athena") &&
				!FunctionName.contains("ServerAcknowledgePossession") &&
				!FunctionName.contains("IsInAircraft") &&
				!FunctionName.contains("FindPlayerStart") &&
				!FunctionName.contains("SpawnDefaultPawnFor") &&
				!FunctionName.contains("MustSpectate") &&
				!FunctionName.contains("GetDefaultPawnClassForController") &&
				!FunctionName.contains("On Game Phase Change") &&
				!FunctionName.contains("ClientAdjustPosition") &&
				!FunctionName.contains("Movement Audio Crossfader__UpdateFunc") &&
				!FunctionName.contains("Holding Audio Crossfader__UpdateFunc") &&
				!FunctionName.contains("OnUpdateDirectionalLightForTimeOfDay") &&
				!FunctionName.contains("OnMontageEnded") &&
				!FunctionName.contains("ServerCancelAbility") &&
				!FunctionName.contains("K2_ActivateAbility") &&
				!FunctionName.contains("ServerHandleMissionEvent_ToggledCursorMode") &&
				!FunctionName.contains("OnBlendOut_") &&
				!FunctionName.contains("ClientEndAbility") &&
				!FunctionName.contains("OnSafeZoneStateChange") &&
				!FunctionName.contains("ClientVeryShortAdjustPosition") &&
				!FunctionName.contains("OnDayPhaseChange") &&
				!FunctionName.contains("On Day Phase Change") &&
				!FunctionName.contains("K2_OnStartCrouch") &&
				!FunctionName.contains("K2_OnEndCrouch") &&
				!FunctionName.contains("On Player Won") &&
				!FunctionName.contains("ClientFinishedInteractionInZone") &&
				!FunctionName.contains("ClientReceiveKillNotification") &&
				!FunctionName.contains("ReceiveCopyProperties") &&
				!FunctionName.contains("K2_OnLogout") &&
				!FunctionName.contains("ClientReceiveLocalizedMessage") &&
				!FunctionName.contains("ClientCancelAbility") &&
				!FunctionName.contains("ServerFinishedInteractionInZoneReport") &&
				!FunctionName.contains("FallingTimeline__UpdateFunc") &&
				!FunctionName.contains("BndEvt__InterceptCollision_K2Node_ComponentBoundEvent_5_ComponentBeginOverlapSignature__DelegateSignature") &&
				!FunctionName.contains("ReceiveActorBeginOverlap") &&
				!FunctionName.contains("Conv_StringToName") &&
				!FunctionName.contains("OnRep_GamePhase") &&
				!FunctionName.contains("K2_OnSetMatchState") &&
				!FunctionName.contains("StartPlay") &&
				!FunctionName.contains("StartMatch") &&
				!FunctionName.contains("OnAircraftEnteredDropZone") &&
				!FunctionName.contains("ServerShortTimeout") &&
				!FunctionName.contains("UpdateStateWidgetContent") &&
				!FunctionName.contains("PreConstruct") &&
				!FunctionName.contains("Construct") &&
				!FunctionName.contains("OnCurrentTextStyleChanged") &&
				!FunctionName.contains("UpdateButtonState") &&
				!FunctionName.contains("OnBangStateChanged") &&
				!FunctionName.contains("OnPlayerInfoChanged") &&
				!FunctionName.contains("Update") &&
				!FunctionName.contains("OnBeginIntro") &&
				!FunctionName.contains("HandleQuickBarChangedBP") &&
				!FunctionName.contains("HandleInventoryUpdatedEvent") &&
				!FunctionName.contains("OnActivated") &&
				!FunctionName.contains("OnBeginOutro") &&
				!FunctionName.contains("HandleActiveWidgetDeactivated") &&
				!FunctionName.contains("OnDeactivated") &&
				!FunctionName.contains("OnStateStarted") &&
				!FunctionName.contains("SetRenderTransform") &&
				!FunctionName.contains("OnAnimationFinished") &&
				!FunctionName.contains("ReadyToStartMatch") &&
				!FunctionName.contains("SetWidthOverride") &&
				!FunctionName.contains("SetHeightOverride") &&
				!FunctionName.contains("HandleMinimizeFinished") &&
				!FunctionName.contains("ServerUpdateLevelVisibility") &&
				!FunctionName.contains("OnDayPhaseChanged") &&
				!FunctionName.contains("ServerLoadingScreenDropped") &&
				!FunctionName.contains("On Game Phase Step Changed") &&
				!FunctionName.contains("HandleGamePhaseStepChanged") &&
				!FunctionName.contains("GamePhaseStepChanged") &&
				!FunctionName.contains("SetColorAndOpacity") &&
				!FunctionName.contains("OnAnimationStarted") &&
				!FunctionName.contains("UpdateMessaging") &&
				!FunctionName.contains("ServerSendLoadoutConfig") &&
				!FunctionName.contains("CalculateBaseMagnitude") &&
				!FunctionName.contains("ClientRegisterWithParty") &&
				!FunctionName.contains("InitializeHUDForPlayer") &&
				!FunctionName.contains("ClientSetHUD") &&
				!FunctionName.contains("ClientEnableNetworkVoice") &&
				!FunctionName.contains("ClientUpdateMultipleLevelsStreamingStatus") &&
				!FunctionName.contains("ClientFlushLevelStreaming") &&
				!FunctionName.contains("ClientOnGenericPlayerInitialization") &&
				!FunctionName.contains("ClientCapBandwidth") &&
				!FunctionName.contains("K2_PostLogin") &&
				!FunctionName.contains("OnRep_bHasStartedPlaying") &&
				!FunctionName.contains("ServerChoosePart") &&
				!FunctionName.contains("SetOwner") &&
				!FunctionName.contains("OnRep_QuickBar") &&
				!FunctionName.contains("HandleStartingNewPlayer") &&
				!FunctionName.contains("ServerUpdateMultipleLevelsVisibility") &&
				!FunctionName.contains("ServerSetPartyOwner") &&
				!FunctionName.contains("PlayerCanRestart") &&
				!FunctionName.contains("ServerCreateCombatManager") &&
				!FunctionName.contains("ServerCreateAIDirectorDataManager") &&
				!FunctionName.contains("EnableSlot") &&
				!FunctionName.contains("DisableSlot") &&
				!FunctionName.contains("ServerSetShowHeroBackpack") &&
				!FunctionName.contains("ServerSetShowHeroHeadAccessories") &&
				!FunctionName.contains("ServerSetClientHasFinishedLoading") &&
				!FunctionName.contains("ServerReadyToStartMatch") &&
				!FunctionName.contains("Received_Notify") &&
				!FunctionName.contains("Received_NotifyBegin") &&
				!FunctionName.contains("AnimNotify_LeftFootStep") &&
				!FunctionName.contains("AnimNotify_RightFootStep") &&
				!FunctionName.contains("Completed_") &&
				!FunctionName.contains("InputActionHoldStopped") &&
				!FunctionName.contains("ServerCurrentMontageSetPlayRate") &&
				!FunctionName.contains("ServerSetReplicatedTargetData") &&
				!FunctionName.contains("Triggered_") &&
				!FunctionName.contains("ActorHasTag") &&
				!FunctionName.contains("RandomIntegerInRange") &&
				!FunctionName.contains("GetItemDefinitionBP") &&
				!FunctionName.contains("CreateTemporaryItemInstanceBP") &&
				!FunctionName.contains("SetOwningControllerForTemporaryItem") &&
				!FunctionName.contains("OnRep_PrimaryQuickBar") &&
				!FunctionName.contains("OnRep_SecondaryQuickBar") &&
				!FunctionName.contains("ServerSetupWeakSpotsOnBuildingActor") &&
				!FunctionName.contains("OnStartDirectionEffect") &&
				!FunctionName.contains("SetReplicates") &&
				!FunctionName.contains("ServerCurrentMontageSetNextSectionName") &&
				!FunctionName.contains("NetFadeOut") &&
				!FunctionName.contains("OnFadeOut") &&
				!FunctionName.contains("NetOnHitCrack") &&
				!FunctionName.contains("OnHitCrack") &&
				!FunctionName.contains("NetMulticast_InvokeGameplayCueAdded_WithParams") &&
				!FunctionName.contains("GameplayCue") &&
				!FunctionName.contains("ReceiveActorEndOverlap") &&
				!FunctionName.contains("PhysicsVolumeChanged") &&
				!FunctionName.contains("ServerAddItemInternal") &&
				!FunctionName.contains("FortClientPlaySound") &&
				!FunctionName.contains("OnCapsuleBeginOverlap") &&
				!FunctionName.contains("GetPlayerController") &&
				!FunctionName.contains("TossPickup") &&
				!FunctionName.contains("OnRep_PrimaryPickupItemEntry") &&
				!FunctionName.contains("ServerActivateSlotInternal") &&
				!FunctionName.contains("EquipWeaponDefinition") &&
				!FunctionName.contains("OnInitAlteration") &&
				!FunctionName.contains("OnInitCosmeticAlterations") &&
				!FunctionName.contains("ClientGivenTo") &&
				!FunctionName.contains("K2_OnUnEquip") &&
				!FunctionName.contains("ClientInternalEquipWeapon") &&
				!FunctionName.contains("GetItemGuid") &&
				!FunctionName.contains("InternalServerSetTargeting") &&
				!FunctionName.contains("ServerReleaseInventoryItemKey") &&
				!FunctionName.contains("OnPawnMontageBlendingOut") &&
				!FunctionName.contains("OnSetTargeting") &&
				!FunctionName.contains("OnRep_DefaultMetadata") &&
				!FunctionName.contains("GetDataTableRowNames") &&
				!FunctionName.contains("GetMaxDurability") &&
				!FunctionName.contains("BeginDeferredActorSpawnFromClass") &&
				!FunctionName.contains("OnRep_PickupLocationData") &&
				!FunctionName.contains("GetControlRotation") &&
				!FunctionName.contains("OnVisibilitySetEvent") &&
				!FunctionName.contains("ShouldShowSoundIndicator") &&
				!FunctionName.contains("WaitForPawn") &&
				!FunctionName.contains("OnProjectileStopDelegate") &&
				!FunctionName.contains("Handle Parachute Audio State") &&
				!FunctionName.contains("GetSpectatorPawn") &&
				!FunctionName.contains("OnHitCrack") &&
				!FunctionName.contains("OnHitCrack") &&
				!FunctionName.contains("OnHitCrack") &&
				!FunctionName.contains("OnHitCrack") &&
				!FunctionName.contains("OnHitCrack") &&
				!FunctionName.contains("OnHitCrack") &&
				!FunctionName.contains("OnHitCrack") &&
				!FunctionName.contains("OnHitCrack") &&
				!FunctionName.contains("OnHitCrack") &&
				!FunctionName.contains("OnHitCrack") &&
				!FunctionName.contains("OnHitCrack") &&
				!FunctionName.contains("EvaluateGraphExposedInputs_ExecuteUbergraph_Fortnite_M_Avg_Player_AnimBlueprint_AnimGraphNode_"))
			{
				FN_LOG(Logs, Log, "FunctionName: [%s]", FunctionName.c_str()); 
			}
		}

		ProcessEvent(Object, Function, Parms);
	}

	bool (*NetSerializeOG)(FGameplayAbilityTargetDataHandle* AbilityTargetDataHandle, __int64& a2, UPackageMap* Map, bool& bOutSuccess);
	bool NetSerializeHook(FGameplayAbilityTargetDataHandle* AbilityTargetDataHandle, __int64& a2, UPackageMap* Map, bool& bOutSuccess)
	{
		bool bResult = NetSerializeOG(AbilityTargetDataHandle, a2, Map, bOutSuccess);

		if (bResult)
		{
			UIpConnection* IpConnection = Cast<UIpConnection>(Map->Outer);

			if (!IpConnection)
				return bResult;

			UAntiCheatGalaxia* AntiCheatGalaxia = FindOrCreateAntiCheatGalaxia(Cast<AFortPlayerController>(IpConnection->PlayerController));

			if (AntiCheatGalaxia)
			{
				uint8 DataNum = 0;

				// [OFFSET-TODO-6.21] +0x20 = nombre de donnees en 8.51 : a reverifier en 6.21 avant de reactiver ce hook (adresse NetSerialize inconnue)
				if (*(int32*)(__int64(AbilityTargetDataHandle) + 0x20) <= 255)
					DataNum = *(int32*)(__int64(AbilityTargetDataHandle) + 0x20);

				if (DataNum > AntiCheatGalaxia->GetMaxDataNum())
				{
					AntiCheatGalaxia->KickByAntiCheat("Damage Multiplier Detected!");
				}
			}
		}

		return bResult;
	}

	bool RetTrue()
	{
		return true;
	}

	UClass** GetGameSessionClassHook(AFortGameMode* GameMode, UClass** GameSessionClass)
	{
		*GameSessionClass = AFortGameSessionDedicatedAthena::StaticClass();
		return GameSessionClass;
	}

	void (*sub_7FF6B9034770OG)(UObject* a1);
	void sub_7FF6B9034770Hook(UObject* a1) // Check CurrentTarget and others
	{
		if (a1) // Check if a1 is valid!!!
			sub_7FF6B9034770OG(a1);
	}

	__int64 sub_7FF6B8E55480() // Memory Leak
	{
		return 0;
	}

	ABuildingSMActor* sub_7FF6B858A000Hook(AFortPlayerController* PlayerController, FBuildingClassData BuildingClassData, FVector Location, FRotator Rotation, bool a5, float TimeSeconds)
	{
		uintptr_t FuncCallAddress = (uintptr_t)_ReturnAddress();
		uintptr_t Offset = uintptr_t(FuncCallAddress) - InSDKUtils::GetImageBase();

		if (!PlayerController || Offset != 0x189C03E)
			return nullptr;

		FN_LOG(Logs, Log, "PlayerController: %s", PlayerController->GetName().c_str());
		FN_LOG(Logs, Log, "BuildingClassData.BuildingClass: %s", BuildingClassData.BuildingClass->GetName().c_str());
		FN_LOG(Logs, Log, "sub_7FF6B858A000Hook called!");

		return nullptr;
	}

	void InitHook()
	{
		uintptr_t AddressGetNetMode = MinHook::FindPatternUnique(Patterns::GetNetMode, "GetNetMode");
		uintptr_t AddressInternalGetNetMode = MinHook::FindPatternUnique(Patterns::InternalGetNetMode, "InternalGetNetMode");
		uintptr_t AddressActorInternalGetNetMode = MinHook::FindPatternUnique(Patterns::ActorInternalGetNetMode, "ActorInternalGetNetMode");
		uintptr_t AddressKickPlayer = MinHook::FindPatternUnique(Patterns::KickPlayer, "KickPlayer");
		uintptr_t AddressGetPlayerViewPoint = MinHook::FindPatternUnique(Patterns::GetPlayerViewPoint, "GetPlayerViewPoint");
		uintptr_t AddressDispatchRequest = MinHook::FindPatternUnique(Patterns::DispatchRequest, "DispatchRequest");
		uintptr_t AddressCanActivateAbility = MinHook::FindPatternUnique(Patterns::CanActivateAbility, "CanActivateAbility");
		uintptr_t AddressChangingGameSessionId = MinHook::FindPatternUnique(Patterns::ChangingGameSessionId, "ChangingGameSessionId");
		uintptr_t AddressUpdateRichPresence = MinHook::FindPatternUnique(Patterns::UpdateRichPresence, "UpdateRichPresence");
		uintptr_t AddressCollectGarbageInternal = MinHook::FindPatternUnique(Patterns::CollectGarbageInternal, "CollectGarbageInternal", false); // vise un jmp au milieu d'une fonction

		MinHook::HookAddress(AddressGetNetMode, "GetNetMode", GetNetModeHook, nullptr); // [GALAXIA PATCH] rien n'est installe si le pattern n'est pas trouve
		MinHook::HookAddress(AddressInternalGetNetMode, "InternalGetNetMode", GetNetModeHook, nullptr); // [GALAXIA PATCH] rien n'est installe si le pattern n'est pas trouve
		MinHook::HookAddress(AddressActorInternalGetNetMode, "ActorInternalGetNetMode", GetNetModeHook, nullptr); // [GALAXIA PATCH] rien n'est installe si le pattern n'est pas trouve
		MinHook::HookAddress(AddressKickPlayer, "KickPlayer", KickPlayerHook, (void**)(&KickPlayerOG)); // [GALAXIA PATCH] conserve l'original pour les autres motifs
		if (Offsets621::IsExpectedBuild())
		{
			uintptr_t AddressSessionNoAuthParam = MinHook::FindPatternUnique(Patterns::SessionNoAuthParam, "SessionNoAuthParam");
			MinHook::HookAddress(AddressSessionNoAuthParam, "SessionNoAuthParam", SessionNoAuthParamHook, (void**)(&SessionNoAuthParamOG));
			uintptr_t AddressPoiPawnOverlap = MinHook::FindPatternUnique(Patterns::PoiPawnOverlap, "PoiPawnOverlap");
			MinHook::HookAddress(AddressPoiPawnOverlap, "PoiPawnOverlap", PoiPawnOverlapHook, (void**)(&PoiPawnOverlapOG));
		}
		MinHook::HookAddress(AddressGetPlayerViewPoint, "GetPlayerViewPoint", GetPlayerViewPointHook, (void**)(&GetPlayerViewPoint)); // [GALAXIA PATCH] rien n'est installe si le pattern n'est pas trouve
		MinHook::HookAddress(AddressDispatchRequest, "DispatchRequest", DispatchRequestHook, (void**)(&DispatchRequest)); // [GALAXIA PATCH] rien n'est installe si le pattern n'est pas trouve
		if (!AddressDispatchRequest && Offsets621::IsExpectedBuild())
		{
			uintptr_t AddressMcpSendRequestNow = MinHook::FindPatternUnique(Patterns::McpSendRequestNow, "McpSendRequestNow");
			MinHook::HookAddress(AddressMcpSendRequestNow, "McpSendRequestNow", McpSendRequestNowHook, (void**)(&McpSendRequestNowOG));
		}
		MinHook::HookAddress(AddressChangingGameSessionId, "ChangingGameSessionId", ChangingGameSessionIdHook, nullptr); // [GALAXIA PATCH] rien n'est installe si le pattern n'est pas trouve
		MinHook::HookAddress(AddressUpdateRichPresence, "UpdateRichPresence", UpdateRichPresenceHook, nullptr); // [GALAXIA PATCH] neutralise aussi les appels directs depuis les timers
		MinHook::HookAddress(AddressCollectGarbageInternal, "CollectGarbageInternal", CollectGarbageInternalHook, (void**)(&CollectGarbageInternal)); // [GALAXIA PATCH] rien n'est installe si le pattern n'est pas trouve
		MH_CreateHook((LPVOID)(InSDKUtils::GetImageBase() + Offsets::ProcessEvent), ProcessEventHook, (LPVOID*)(&ProcessEvent));
		MinHook::ProcessEventOriginal = ProcessEvent; // [GALAXIA PATCH] utilise par les "OG" des hooks RPC
		MH_EnableHook((LPVOID)(InSDKUtils::GetImageBase() + Offsets::ProcessEvent));

		// [OFFSET-TODO-6.21] NetSerialize (8.51 = 0x887f90)
		// MH_CreateHook((LPVOID)(InSDKUtils::GetImageBase() + 0x887f90), NetSerializeHook, (LPVOID*)(&NetSerializeOG));
		// MH_EnableHook((LPVOID)(InSDKUtils::GetImageBase() + 0x887f90));
		MinHook::HookNative(Offsets621::NetSerialize, "NetSerialize", NetSerializeHook, (void**)(&NetSerializeOG));

		// [OFFSET-TODO-6.21] SpawnContextCheck (8.51 = 0x1e2ae20)
		// MH_CreateHook((LPVOID)(InSDKUtils::GetImageBase() + 0x1e2ae20), RetTrue, nullptr); // Fix (Unable to spawn class '%s' due to client/server context.)
		// MH_EnableHook((LPVOID)(InSDKUtils::GetImageBase() + 0x1e2ae20));
		MinHook::HookNative(Offsets621::SpawnContextCheck, "SpawnContextCheck", RetTrue, nullptr); // Fix (Unable to spawn class '%s' due to client/server context.)
		
		// [OFFSET-TODO-6.21] GetGameSessionClass (8.51 = 0x12d2bc0)
		// MH_CreateHook((LPVOID)(InSDKUtils::GetImageBase() + 0x12d2bc0), GetGameSessionClassHook, nullptr);
		// MH_EnableHook((LPVOID)(InSDKUtils::GetImageBase() + 0x12d2bc0));
		MinHook::HookNative(Offsets621::GetGameSessionClass, "GetGameSessionClass", GetGameSessionClassHook, nullptr); 
		
		// [OFFSET-TODO-6.21] DestroyPreviousGameSession1 (8.51 = 0x1524e90)
		// MH_CreateHook((LPVOID)(InSDKUtils::GetImageBase() + 0x1524e90), RetTrue, nullptr); // Failed to destroy previous game session %s
		// MH_EnableHook((LPVOID)(InSDKUtils::GetImageBase() + 0x1524e90));
		MinHook::HookNative(Offsets621::DestroyPreviousGameSession1, "DestroyPreviousGameSession1", RetTrue, nullptr); // Failed to destroy previous game session %s
		
		// [OFFSET-TODO-6.21] DestroyPreviousGameSession2 (8.51 = 0x1524660)
		// MH_CreateHook((LPVOID)(InSDKUtils::GetImageBase() + 0x1524660), RetTrue, nullptr); // Failed to destroy previous game session %s
		// MH_EnableHook((LPVOID)(InSDKUtils::GetImageBase() + 0x1524660));
		MinHook::HookNative(Offsets621::DestroyPreviousGameSession2, "DestroyPreviousGameSession2", RetTrue, nullptr); // Failed to destroy previous game session %s

		// [OFFSET-TODO-6.21] BotCrashFix (8.51 = 0xdc4770)
		// MH_CreateHook((LPVOID)(InSDKUtils::GetImageBase() + 0xdc4770), sub_7FF6B9034770Hook, (LPVOID*)(&sub_7FF6B9034770OG)); // Crash cause a1 invalid (Cause Bot)
		// MH_EnableHook((LPVOID)(InSDKUtils::GetImageBase() + 0xdc4770));
		MinHook::HookNative(Offsets621::BotCrashFix, "BotCrashFix", sub_7FF6B9034770Hook, (void**)(&sub_7FF6B9034770OG)); // Crash cause a1 invalid (Cause Bot)

		// [OFFSET-TODO-6.21] MemoryLeakFix (8.51 = 0xbe5480)
		// MH_CreateHook((LPVOID)(InSDKUtils::GetImageBase() + 0xbe5480), sub_7FF6B8E55480, nullptr); // Memory Leak De merde
		// MH_EnableHook((LPVOID)(InSDKUtils::GetImageBase() + 0xbe5480));
		MinHook::HookNative(Offsets621::MemoryLeakFix, "MemoryLeakFix", sub_7FF6B8E55480, nullptr); // Memory Leak De merde

		//MH_CreateHook((LPVOID)(InSDKUtils::GetImageBase() + 0x31a000), sub_7FF6B858A000Hook, nullptr); // OG mais pas trop quand même ta capter
		//MH_EnableHook((LPVOID)(InSDKUtils::GetImageBase() + 0x31a000));

		FN_LOG(LogInit, Log, "InitHook Success!");
	}
}
