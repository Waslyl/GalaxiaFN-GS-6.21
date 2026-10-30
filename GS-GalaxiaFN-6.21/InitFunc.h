#pragma once

namespace Init
{
	// [GALAXIA PATCH] Copie du module 6.21 tel qu'il est en memoire (dechiffre), dans l'ordre des RVA.
	// Fichier : <dossier de l'exe>\FortniteClient-Win64-Shipping.exe.dump621.bin
	// Sert uniquement a l'analyse hors ligne (patterns, adresses [OFFSET-TODO-6.21]). Active avec #define DUMP_621.
	void DumpModule621()
	{
		const uintptr_t Base = InSDKUtils::GetImageBase();
		const auto NtHeaders = (PIMAGE_NT_HEADERS)(Base + ((PIMAGE_DOS_HEADER)Base)->e_lfanew);
		const size_t ImageSize = NtHeaders->OptionalHeader.SizeOfImage;

		std::vector<uint8> Buffer(ImageSize, 0);

		for (size_t Offset = 0; Offset < ImageSize;)
		{
			MEMORY_BASIC_INFORMATION Mbi{};

			if (!VirtualQuery((void*)(Base + Offset), &Mbi, sizeof(Mbi)))
				break;

			const size_t RegionEnd = std::min(size_t((uintptr_t)Mbi.BaseAddress + Mbi.RegionSize - Base), ImageSize);
			const bool bReadable = Mbi.State == MEM_COMMIT && !(Mbi.Protect & (PAGE_NOACCESS | PAGE_GUARD));

			if (bReadable)
				memcpy(&Buffer[Offset], (void*)(Base + Offset), RegionEnd - Offset);

			Offset = RegionEnd;
		}

		char ExePath[MAX_PATH] = {};
		GetModuleFileNameA(nullptr, ExePath, MAX_PATH);
		const std::string OutPath = std::string(ExePath) + ".dump621.bin";

		std::ofstream File(OutPath, std::ios::binary);

		if (!File)
		{
			FN_LOG(LogInit, Error, "[DUMP_621] Impossible d'ecrire %s", OutPath.c_str());
			return;
		}

		File.write((const char*)Buffer.data(), Buffer.size());
		FN_LOG(LogInit, Log, "[DUMP_621] Module 6.21 ecrit : %s (%llu octets)", OutPath.c_str(), (unsigned long long)Buffer.size());
	}

	bool InitializeAll()
	{
#ifdef DUMP_621
		DumpModule621();
#endif // DUMP_621

		//uintptr_t AddressFMemoryMalloc = MinHook::FindPattern(Patterns::FMemoryMalloc);
		uintptr_t AddressFMemoryRealloc = MinHook::FindPatternUnique(Patterns::FMemoryRealloc, "FMemoryRealloc");
		uintptr_t AddressFMemoryFree = MinHook::FindPatternUnique(Patterns::FMemoryFree, "FMemoryFree");

		//UC::FMemory_Malloc = decltype(UC::FMemory_Malloc)(AddressFMemoryMalloc);
		UC::FMemory_Realloc = decltype(UC::FMemory_Realloc)(AddressFMemoryRealloc);
		UC::FMemory_Free = decltype(UC::FMemory_Free)(AddressFMemoryFree);

		// [GALAXIA PATCH] Sans FMemory, TArray::Add ne peut pas agrandir les tableaux (inventaire, loot...) : tres visible dans la console
		if (!UC::FMemory_Realloc || !UC::FMemory_Free)
		{
			HANDLE Console = GetStdHandle(STD_OUTPUT_HANDLE);
			SetConsoleTextAttribute(Console, FOREGROUND_RED | FOREGROUND_INTENSITY);
			FN_LOG(LogInit, Error, "==============================================================");
			FN_LOG(LogInit, Error, "[OFFSET-TODO-6.21] FMemory introuvable (Realloc: %p, Free: %p) !", UC::FMemory_Realloc, UC::FMemory_Free);
			FN_LOG(LogInit, Error, "TArray::Add ne pourra pas agrandir les tableaux : inventaire et loot casses.");
			FN_LOG(LogInit, Error, "==============================================================");
			SetConsoleTextAttribute(Console, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		}

		// [GALAXIA PATCH] Un log avant/apres chaque Init : si le serveur crashe, la derniere ligne "..." indique ou
#define GALAXIA_INIT_STEP(Step) \
		FN_LOG(LogInit, Log, "%s...", #Step); \
		Step(); \
		FN_LOG(LogInit, Log, "%s OK", #Step);

		GALAXIA_INIT_STEP(Hooks::InitHook);
		GALAXIA_INIT_STEP(GameMode::InitGameMode);
		GALAXIA_INIT_STEP(Beacon::InitBeacon);
		GALAXIA_INIT_STEP(PlayerController::InitPlayerController);
		GALAXIA_INIT_STEP(Pawn::InitPawn);
		GALAXIA_INIT_STEP(Abilities::InitAbilities);
		GALAXIA_INIT_STEP(FortWeapon::InitFortWeapon);
		GALAXIA_INIT_STEP(BuildingActor::InitBuildingActor);
		GALAXIA_INIT_STEP(Vehicle::InitVehicle);
		GALAXIA_INIT_STEP(CheatManager::InitCheatManager);
		GALAXIA_INIT_STEP(FortKismetLibrary::InitFortKismetLibrary);

#undef GALAXIA_INIT_STEP

		return true;
	}
}
