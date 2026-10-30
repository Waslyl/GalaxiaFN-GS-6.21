#pragma once
#include "util.h"
#include <array>
#include <atomic>
#include <type_traits>
#include <utility>

namespace MinHook
{
	static void HookVTable(UObject* Default, int Index, void* FuncHook, void** FuncOG, const std::string& FuncName)
	{
		if (!Default)
		{
			FN_LOG(LogHook, Log, "Default not found!");
			return;
		}

		uintptr_t Address = uintptr_t(Default->VTable[Index]);

		if (FuncOG)
			*FuncOG = Default->VTable[Index];

		DWORD dwProtection;
		VirtualProtect(&Default->VTable[Index], 8, PAGE_EXECUTE_READWRITE, &dwProtection);

        Default->VTable[Index] = FuncHook;

		DWORD dwTemp;
		VirtualProtect(&Default->VTable[Index], 8, dwProtection, &dwTemp);

		uintptr_t Offset = Address - InSDKUtils::GetImageBase();
		uintptr_t IdaAddress = Offset + InSDKUtils::GetIdaImageBase();

		FN_LOG(LogHook, Log, "Function VTable [%s] successfully hooked with Offset [0x%llx], IdaAddress [%p]", FuncName.c_str(), (unsigned long long)Offset, IdaAddress);
	}

	static void HookFunctionExec(UFunction* Function, void* FuncHook, void** FuncOG)
	{
		if (!Function)
		{
			FN_LOG(LogHook, Log, "Function not found!");
			return;
		}

		// [GALAXIA PATCH][OFFSET-TODO-6.21] Sans FFrame::Step, un hook exec appele depuis un Blueprint desynchroniserait la VM (crash) :
		// on ne l'installe pas tant que l'adresse de FFrame::Step n'est pas trouvee.
		if (!FFrame::CanStepBytecode())
		{
			FN_LOG(LogHook, Error, "[OFFSET-TODO-6.21] Hook exec [%s] non installe : FFrame::Step inconnu en 6.21", Function->GetName().c_str());
			return;
		}

		auto& ExecFunction = Function->ExecFunction;

		uintptr_t Offset = uintptr_t(ExecFunction) - InSDKUtils::GetImageBase();
		uintptr_t IdaAddress = Offset + InSDKUtils::GetIdaImageBase();

		FN_LOG(LogHook, Log, "Function Exec [%s] successfully hooked with Offset [0x%llx], IdaAddress [%p]", Function->GetName().c_str(), (unsigned long long)Offset, IdaAddress);

		if (FuncOG)
			*FuncOG = ExecFunction;

		ExecFunction = (UFunction::FNativeFuncPtr)FuncHook;
	}

    static uintptr_t FindPattern(const char* signature, bool bRelative = false, uint32_t offset = 0)
    {
        uintptr_t base_address = reinterpret_cast<uintptr_t>(GetModuleHandle(NULL));
        static auto patternToByte = [](const char* pattern)
            {
                auto bytes = std::vector<int>{};
                const auto start = const_cast<char*>(pattern);
                const auto end = const_cast<char*>(pattern) + strlen(pattern);

                for (auto current = start; current < end; ++current)
                {
                    if (*current == '?')
                    {
                        ++current;
                        if (*current == '?')
                            ++current;
                        bytes.push_back(-1);
                    }
                    else
                    {
                        bytes.push_back(strtoul(current, &current, 16));
                    }
                }
                return bytes;
            };

        const auto dosHeader = (PIMAGE_DOS_HEADER)base_address;
        const auto ntHeaders = (PIMAGE_NT_HEADERS)((std::uint8_t*)base_address + dosHeader->e_lfanew);

        const auto sizeOfImage = ntHeaders->OptionalHeader.SizeOfImage;
        auto patternBytes = patternToByte(signature);
        const auto scanBytes = reinterpret_cast<std::uint8_t*>(base_address);

        const auto s = patternBytes.size();
        const auto d = patternBytes.data();

        for (auto i = 0ul; i < sizeOfImage - s; ++i)
        {
            bool found = true;
            for (auto j = 0ul; j < s; ++j)
            {
                if (scanBytes[i + j] != d[j] && d[j] != -1)
                {
                    found = false;
                    break;
                }
            }
            if (found)
            {
                uintptr_t address = reinterpret_cast<uintptr_t>(&scanBytes[i]);
                if (bRelative)
                {
                    address = ((address + offset + 4) + *(int32_t*)(address + offset));
                    return address;
                }

                return address;
            }
        }

        return NULL;
    }

    // [GALAXIA PATCH] Comme FindPattern, mais refuse un pattern introuvable ou ambigu (plusieurs resultats) :
    // hooker la mauvaise fonction ferait crasher le serveur, alors on prefere ne rien hooker.
    // bFunctionStart = false pour un pattern qui vise volontairement le milieu d'une fonction (ex. CollectGarbageInternal)
    static uintptr_t FindPatternUnique(const char* signature, const char* Name, bool bFunctionStart = true)
    {
        uintptr_t base_address = reinterpret_cast<uintptr_t>(GetModuleHandle(NULL));

        std::vector<int> patternBytes;
        const char* end = signature + strlen(signature);

        for (char* current = const_cast<char*>(signature); current < end; ++current)
        {
            if (*current == ' ')
                continue;

            if (*current == '?')
            {
                if (*(current + 1) == '?')
                    ++current;

                patternBytes.push_back(-1);
            }
            else
            {
                patternBytes.push_back(strtoul(current, &current, 16));
                --current;
            }
        }

        const auto dosHeader = (PIMAGE_DOS_HEADER)base_address;
        const auto ntHeaders = (PIMAGE_NT_HEADERS)((std::uint8_t*)base_address + dosHeader->e_lfanew);
        const auto sizeOfImage = ntHeaders->OptionalHeader.SizeOfImage;
        const auto scanBytes = reinterpret_cast<std::uint8_t*>(base_address);

        const auto s = patternBytes.size();
        const auto d = patternBytes.data();

        uintptr_t FirstMatch = 0;
        int32 MatchCount = 0;

        for (auto i = 0ul; i < sizeOfImage - s && MatchCount < 2; ++i)
        {
            bool found = true;
            for (auto j = 0ul; j < s; ++j)
            {
                if (scanBytes[i + j] != d[j] && d[j] != -1)
                {
                    found = false;
                    break;
                }
            }

            if (found)
            {
                if (MatchCount == 0)
                    FirstMatch = reinterpret_cast<uintptr_t>(&scanBytes[i]);

                MatchCount++;
            }
        }

        if (MatchCount == 0)
        {
            FN_LOG(LogPattern, Error, "[OFFSET-TODO-6.21] Pattern [%s] introuvable en 6.21, fonctionnalite desactivee", Name);
            return 0;
        }

        if (MatchCount > 1)
        {
            FN_LOG(LogPattern, Error, "[OFFSET-TODO-6.21] Pattern [%s] ambigu (plusieurs resultats) en 6.21, fonctionnalite desactivee", Name);
            return 0;
        }

        // MSVC aligne le debut des fonctions x64 sur 16 octets : un resultat non aligne est tombe au milieu d'une autre fonction
        if (bFunctionStart && (FirstMatch & 0xF) != 0)
        {
            FN_LOG(LogPattern, Error, "[OFFSET-TODO-6.21] Pattern [%s] trouve a 0x%llx, pas un debut de fonction (non aligne sur 16) : mauvaise fonction, desactive", Name, (unsigned long long)(FirstMatch - base_address));
            return 0;
        }

        FN_LOG(LogPattern, Log, "Pattern [%s] trouve : Offset [0x%llx], IdaAddress [%p]", Name, (unsigned long long)(FirstMatch - base_address), (void*)(FirstMatch - base_address + InSDKUtils::GetIdaImageBase()));
        return FirstMatch;
    }

    // [GALAXIA PATCH] Hook d'une fonction native par adresse absolue (pattern). Ne fait rien si l'adresse est nulle.
    static bool HookAddress(uintptr_t Address, const char* Name, void* FuncHook, void** FuncOG)
    {
        if (!Address)
        {
            FN_LOG(LogHook, Error, "[OFFSET-TODO-6.21] Hook [%s] non installe : adresse inconnue", Name);
            return false;
        }

        if (MH_CreateHook((LPVOID)Address, FuncHook, (LPVOID*)FuncOG) != MH_OK || MH_EnableHook((LPVOID)Address) != MH_OK)
        {
            FN_LOG(LogHook, Error, "Hook [%s] : echec de MinHook", Name);
            return false;
        }

        FN_LOG(LogHook, Log, "Hook [%s] installe", Name);
        return true;
    }

    // [GALAXIA PATCH] Hook d'une fonction native par RVA (Offsets621). Ne fait rien si l'adresse vaut 0.
    static bool HookNative(uintptr_t Rva, const char* Name, void* FuncHook, void** FuncOG)
    {
        if (!Rva)
        {
            Offsets621::LogMissing(Name);
            return false;
        }

        return HookAddress(InSDKUtils::GetImageBase() + Rva, Name, FuncHook, FuncOG);
    }

    // ===================================================================================================
    // [GALAXIA PATCH] Hooks RPC / BlueprintNativeEvent par ProcessEvent (6.21)
    //
    // Les RPC recus du reseau (FObjectReplicator::ReceivedRPC) et les events natifs (ReadyToStartMatch,
    // SpawnDefaultPawnFor, HandleStartingNewPlayer...) passent tous par UObject::ProcessEvent, deja hooke dans Hooks.h.
    // On intercepte donc la UFunction elle-meme (par nom + classe) : aucun index de vtable ni aucune adresse.
    // (La lecture de l'index de vtable dans le thunk donnait des index faux sur ce build.)
    //
    // Les parametres sont lus dans le buffer Parms de ProcessEvent. Leur disposition est calculee depuis la
    // signature du hook, puis VERIFIEE au demarrage contre les UProperty de la UFunction 6.21 (offset et taille
    // de chaque parametre) : si ca ne correspond pas, le hook n'est pas installe.
    // ===================================================================================================

    // ProcessEvent d'origine (trampoline MinHook), rempli par Hooks::InitHook
    inline void (*ProcessEventOriginal)(UObject* Object, UFunction* Function, void* Parms) = nullptr;

    using FProcessEventHandler = void (*)(UObject* Object, UFunction* Function, void* Parms);

    struct FProcessEventEntry
    {
        FName FuncName;
        UClass* Class;
        FProcessEventHandler Handler;
    };

    // Rempli au demarrage, lu par le GameThread a chaque ProcessEvent : tableau fixe + compteur atomique (pas de reallocation)
    inline FProcessEventEntry ProcessEventEntries[128];
    inline std::atomic<int32> ProcessEventEntryCount = 0;

    // Appele au debut de Hooks::ProcessEventHook. Renvoie true si la fonction a ete prise en charge (ne pas appeler l'originale).
    inline bool DispatchProcessEvent(UObject* Object, UFunction* Function, void* Parms)
    {
        const int32 Count = ProcessEventEntryCount.load(std::memory_order_acquire);

        for (int32 i = 0; i < Count; i++)
        {
            const FProcessEventEntry& Entry = ProcessEventEntries[i];

            if (Entry.FuncName == Function->Name && Object->IsA(Entry.Class))
            {
                Entry.Handler(Object, Function, Parms);
                return true;
            }
        }

        return false;
    }

    constexpr size_t AlignUp(size_t Value, size_t Alignment)
    {
        return (Value + Alignment - 1) & ~(Alignment - 1);
    }

    // Disposition du buffer Parms (meme regle que UStruct::Link : chaque parametre aligne sur son alignement, ReturnValue en dernier)
    template<typename RetType, typename... ArgTypes>
    struct TParmsLayout
    {
        static constexpr size_t NumArgs = sizeof...(ArgTypes);

        static constexpr std::array<size_t, NumArgs> Sizes = { sizeof(ArgTypes)... };

        static constexpr std::array<size_t, NumArgs> Offsets = []()
        {
            std::array<size_t, NumArgs> Result{};
            const size_t Aligns[] = { alignof(ArgTypes)..., 1 };
            const size_t ArgSizes[] = { sizeof(ArgTypes)..., 0 };
            size_t Offset = 0;

            for (size_t i = 0; i < NumArgs; i++)
            {
                Offset = AlignUp(Offset, Aligns[i]);
                Result[i] = Offset;
                Offset += ArgSizes[i];
            }

            return Result;
        }();

        static constexpr size_t ArgsEnd = NumArgs == 0 ? 0 : Offsets[NumArgs == 0 ? 0 : NumArgs - 1] + Sizes[NumArgs == 0 ? 0 : NumArgs - 1];

        static constexpr bool bHasReturn = !std::is_void_v<RetType>;
        static constexpr size_t RetSize = []() { if constexpr (std::is_void_v<RetType>) return size_t(0); else return sizeof(RetType); }();
        static constexpr size_t RetAlign = []() { if constexpr (std::is_void_v<RetType>) return size_t(1); else return alignof(RetType); }();
        static constexpr size_t RetOffset = AlignUp(ArgsEnd, RetAlign);
        static constexpr size_t TotalSize = bHasReturn ? RetOffset + RetSize : ArgsEnd;
    };

    template<auto HookFn>
    struct TRPCBridge;

    template<typename Ret, typename Obj, typename... Args, Ret(*HookFn)(Obj*, Args...)>
    struct TRPCBridge<HookFn>
    {
        using Layout = TParmsLayout<Ret, std::remove_cvref_t<Args>...>;

        static inline UFunction* Function = nullptr;

        // UFunction reellement appelee (ex. override Blueprint de PlayerPawn_Athena_Generic_C) : l'OG la rappelle elle
        static inline thread_local UFunction* CurrentFunction = nullptr;

        template<size_t... I>
        static Ret CallHook(UObject* Object, uint8* Parms, std::index_sequence<I...>)
        {
            return HookFn(static_cast<Obj*>(Object), *reinterpret_cast<std::remove_cvref_t<Args>*>(Parms + Layout::Offsets[I])...);
        }

        // Appele par DispatchProcessEvent a la place de la fonction d'origine
        static void Handler(UObject* Object, UFunction* CalledFunction, void* Parms)
        {
            uint8* Buffer = static_cast<uint8*>(Parms);
            UFunction* PreviousFunction = CurrentFunction;
            CurrentFunction = CalledFunction;

            if constexpr (std::is_void_v<Ret>)
            {
                CallHook(Object, Buffer, std::index_sequence_for<Args...>{});
            }
            else
            {
                Ret Result = CallHook(Object, Buffer, std::index_sequence_for<Args...>{});
                memcpy(Buffer + Layout::RetOffset, &Result, sizeof(Ret));
            }

            CurrentFunction = PreviousFunction;
        }

        // "OG" du hook : rappelle la fonction d'origine par le ProcessEvent d'origine
        static Ret Original(Obj* Object, Args... InArgs)
        {
            alignas(16) uint8 Parms[Layout::TotalSize > 0 ? Layout::TotalSize : 1] = {};

            size_t Index = 0;
            ((memcpy(Parms + Layout::Offsets[Index++], std::addressof(InArgs), sizeof(std::remove_cvref_t<Args>))), ...);

            UFunction* FunctionToCall = CurrentFunction ? CurrentFunction : Function;

            if (ProcessEventOriginal && FunctionToCall)
                ProcessEventOriginal(Object, FunctionToCall, Parms);

            if constexpr (!std::is_void_v<Ret>)
            {
                Ret Result{};
                memcpy(&Result, Parms + Layout::RetOffset, sizeof(Ret));
                return Result;
            }
        }
    };

    // Verifie la disposition calculee contre les UProperty de la UFunction (offset + taille de chaque parametre)
    template<typename LayoutType>
    static bool CheckParmsLayout(UFunction* Function, const std::string& LogName)
    {
        constexpr uint64 CPF_Parm = 0x80;
        constexpr uint64 CPF_ReturnParm = 0x400;

        size_t ArgIndex = 0;
        bool bFoundReturn = false;

        for (UField* Field = Function->Children; Field; Field = Field->Next)
        {
            if (!Field->HasTypeFlag(EClassCastFlags::Property))
                continue;

            UProperty* Property = static_cast<UProperty*>(Field);

            if (!(Property->PropertyFlags & CPF_Parm))
                continue;

            const size_t PropertySize = size_t(Property->ElementSize) * Property->ArrayDim;

            if (Property->PropertyFlags & CPF_ReturnParm)
            {
                bFoundReturn = true;

                if (!LayoutType::bHasReturn || size_t(Property->Offset) != LayoutType::RetOffset || PropertySize != LayoutType::RetSize)
                {
                    FN_LOG(LogHook, Error, "HookRPC [%s] : valeur de retour differente en 6.21 (offset 0x%X taille 0x%X), hook non installe", LogName.c_str(), Property->Offset, (uint32)PropertySize);
                    return false;
                }

                continue;
            }

            if (ArgIndex >= LayoutType::NumArgs ||
                size_t(Property->Offset) != LayoutType::Offsets[ArgIndex] ||
                PropertySize != LayoutType::Sizes[ArgIndex])
            {
                FN_LOG(LogHook, Error, "HookRPC [%s] : parametre %d [%s] different en 6.21 (offset 0x%X taille 0x%X), hook non installe",
                    LogName.c_str(), (int32)ArgIndex, Property->GetName().c_str(), Property->Offset, (uint32)PropertySize);
                return false;
            }

            ArgIndex++;
        }

        if (ArgIndex != LayoutType::NumArgs || bFoundReturn != LayoutType::bHasReturn)
        {
            FN_LOG(LogHook, Error, "HookRPC [%s] : nombre de parametres different en 6.21 (%d au lieu de %d), hook non installe", LogName.c_str(), (int32)ArgIndex, (int32)LayoutType::NumArgs);
            return false;
        }

        return true;
    }

    // ClassName = classe qui DECLARE la UFunction (ex. "FortPlayerController"), cherchee dans la hierarchie de Default->Class.
    // FuncOG recoit une fonction qui rappelle l'originale (meme signature que le hook). Index851 n'est plus utilise (historique).
    template<auto HookFn>
    static bool HookRPC(UObject* Default, const std::string& ClassName, const std::string& FuncName, void** FuncOG = nullptr, int32 Index851 = -1)
    {
        using Bridge = TRPCBridge<HookFn>;
        const std::string LogName = ClassName + "::" + FuncName;

        if (!Default || !Default->Class)
        {
            FN_LOG(LogHook, Error, "HookRPC [%s] : Default/Class introuvable", LogName.c_str());
            return false;
        }

        UFunction* Function = Default->Class->GetFunction(ClassName.c_str(), FuncName.c_str());

        if (!Function)
        {
            FN_LOG(LogHook, Error, "HookRPC [%s] : UFunction introuvable en 6.21, hook non installe", LogName.c_str());
            return false;
        }

        if (!CheckParmsLayout<typename Bridge::Layout>(Function, LogName))
            return false;

        const int32 Index = ProcessEventEntryCount.load();

        if (Index >= int32(std::size(ProcessEventEntries)))
        {
            FN_LOG(LogHook, Error, "HookRPC [%s] : trop de hooks ProcessEvent", LogName.c_str());
            return false;
        }

        Bridge::Function = Function;

        if (FuncOG)
            *FuncOG = reinterpret_cast<void*>(&Bridge::Original);

        ProcessEventEntries[Index] = { Function->Name, Default->Class, &Bridge::Handler };
        ProcessEventEntryCount.store(Index + 1, std::memory_order_release);

        FN_LOG(LogHook, Log, "HookRPC [%s] installe via ProcessEvent (parametres verifies)", LogName.c_str());
        return true;
    }
}
