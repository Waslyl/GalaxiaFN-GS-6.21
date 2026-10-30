#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <Windows.h>
#include <thread>
#include <detours.h>
#include <intrin.h>
#include <wincrypt.h>
#include <vadefs.h>
#include <xtree>

#include "Minhook/Minhook.h"
#pragma comment(lib,"Minhook.lib")

// SDK
#include "SDK.hpp"

#include "MinHook.h"
#include "FMath.h"

#include "Util.h"
#include "Globals.h"
#include "Patterns.h"
#include "Fallbacks621.h" // [GALAXIA PATCH] replis sans adresse native (6.21)

#include "Functions/Inventory.h"
#include "Functions/Loots.h"
#include "Functions/Functions.h"
#include "Functions/Abilities.h"
#include "Functions/Anticheat.h"
#include "Functions/Bots.h"

#include "Net/Beacon.h"

#include "Globals/CheatManager.h"
#include "Globals/BuildingActor.h"
#include "Globals/FortKismetLibrary.h"
#include "Globals/FortWeapon.h"
#include "Globals/GameMode.h"
#include "Globals/Pawn.h"
#include "Globals/PlayerController.h"
#include "Globals/PlayerState.h"
#include "Globals/Vehicle.h"

#include <fstream>
#include "Hooks.h"
#include "InitFunc.h"



DWORD WINAPI MainThread(LPVOID Module)
{
    FILE* fDummy;
    AllocConsole();
    freopen_s(&fDummy, "CONIN$", "r", stdin);
    freopen_s(&fDummy, "CONOUT$", "w", stderr);
    freopen_s(&fDummy, "CONOUT$", "w", stdout);

    MH_Initialize();

    SetConsoleTitleA("6.21 Galaxia");
    // [GALAXIA PATCH] Identifier la DLL reellement chargee (toujours bin\Release\GS-GalaxiaFN-6.21.dll) grace a sa date de compilation.
    char DllPath[MAX_PATH] = {};
    GetModuleFileNameA(static_cast<HMODULE>(Module), DllPath, MAX_PATH);
    FN_LOG(LogInitialize, Log, "DLL chargee : %s (compilee le %s a %s)", DllPath, __DATE__, __TIME__);
    srand(time(0));

    // Setup
    bool bSuccess = true;

    if (bSuccess)
    {
        UFortEngine* FortEngine = Globals::GetFortEngine();
        APlayerController* PlayerController = Globals::GetServerPlayerController();

        if (FortEngine && PlayerController)
        {
            UFortConsole* NewConsole = (UFortConsole*)UGameplayStatics::SpawnObject(UFortConsole::StaticClass(), FortEngine->GameViewport);
            UCheatManager* NewCheatManager = (UCheatManager*)UGameplayStatics::SpawnObject(UCheatManager::StaticClass(), PlayerController);

            if (NewConsole && NewCheatManager)
            {
                FortEngine->GameViewport->ViewportConsole = NewConsole;
                //PlayerController->CheatManager = NewCheatManager; //Activate cheat mananager (dois je faire pour Playground)

                
                PlayerController->SwitchLevel(L"Athena_Terrain");
                // PlayerController->SwitchLevel(L"DS_GalaxiaTest");

                Globals::GetWorld()->OwningGameInstance->LocalPlayers.Remove(0);
            }
        }

        Init::InitializeAll();

        // [OFFSET-TODO-6.21] On n'ecrit que si l'adresse 6.21 est connue (ecrire a une adresse 8.51 corromprait la memoire)
        // *(bool*)Hooks::GIsClient() = false;
        // *(bool*)Hooks::GIsServer() = true;
        if (bool* GIsClient = (bool*)Hooks::GIsClient())
            *GIsClient = false;

        if (bool* GIsServer = (bool*)Hooks::GIsServer())
            *GIsServer = true;

        // [8.51-ONLY] Patch d'un octet a une adresse 8.51 : ecrire a cet endroit en 6.21 corromprait le code du jeu
        // uint8_t* BYTE = (uint8_t*)(0x3012AEA + uintptr_t(GetModuleHandle(0)));
        // *BYTE = 0x74;

        FN_LOG(LogInitialize, Log, "Image Base: [%p]", InSDKUtils::GetImageBase());
        FN_LOG(LogInitialize, Log, "Init Success!");
    }
    else
    {
        FN_LOG(LogInitialize, Error, "Init Failed!");
    }

    return 0;
}

BOOL APIENTRY DllMain(HMODULE mod, DWORD reason, LPVOID res)
{
    if (reason == 1)
        CreateThread(0, 0, MainThread, mod, 0, 0);

    return TRUE;
}
