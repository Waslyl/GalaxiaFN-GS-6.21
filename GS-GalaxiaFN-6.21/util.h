#pragma once
#include <mutex>
#include <unordered_map>

inline bool bDebugLog = false;

enum LogLevel {
    Log,
    Debug,
    Warning,
    Error
};

void WriteLog(const char* category, LogLevel level, const char* format, ...)
{
    const char* levelStr;

    if (!bDebugLog && level == LogLevel::Debug)
        return;

    switch (level) {
        case LogLevel::Log: levelStr = "Info"; break;
        case LogLevel::Debug: levelStr = "Debug"; break;
        case LogLevel::Warning: levelStr = "Warning"; break;
        case LogLevel::Error: levelStr = "Error"; break;
        default: levelStr = "Unknown"; break;
    }

    printf("Log: %s: %s: ", category, levelStr);

    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    printf("\n");
}

#define FN_LOG(Category, Level, ...) WriteLog(#Category, Level, __VA_ARGS__)

// [OFFSET-TODO-6.21] StaticFindObject (8.51 = 0x1E825F0) : remplace par une recherche dans GObjects (sans adresse)
// template <typename T>
// static T* FindObjectFast(std::string ObjectName, UClass* ObjectClass = UObject::StaticClass())
// {
//     auto OrigInName = std::wstring(ObjectName.begin(), ObjectName.end()).c_str();
//
//     auto StaticFindObject = (T * (*)(UClass*, UObject * Package, const wchar_t* OrigInName, bool ExactClass))((uintptr_t)GetModuleHandleA(0) + 0x1E825F0);
//     return StaticFindObject(ObjectClass, nullptr, OrigInName, false);
// }

// [GALAXIA PATCH] Meme comportement que StaticFindObject (objets deja charges uniquement), sans adresse native.
// IMPORTANT : ne doit JAMAIS passer par ProcessEvent (pas de UFunction comme Conv_StringToName) : FindObjectFast est utilise
// dans des initialisations "static" a l'interieur de Hooks::ProcessEventHook, et y revenir bloquait le jeu (interblocage).
// On filtre par type (T et ObjectClass), on compare le dernier segment du nom, puis le chemin complet.
// Les resultats sont gardes en cache (index GObjects) et reverifies a chaque utilisation.
inline std::string GetObjectPathName(UObject* Object)
{
    // GetRawString : GetName()/GetFullName() du SDK coupent tout ce qui precede le dernier '/'
    // ("/Game/Athena/Items/Weapons/WID_X" -> "WID_X"), ce qui empecherait de comparer avec un chemin "/Game/...".
    std::string PathName = Object->Name.GetRawString();

    for (UObject* Outer = Object->Outer; Outer; Outer = Outer->Outer)
        PathName = Outer->Name.GetRawString() + "." + PathName;

    return PathName;
}

template <typename T>
static T* FindObjectFast(std::string ObjectName, UClass* ObjectClass = UObject::StaticClass())
{
    static std::mutex CacheMutex;
    static std::unordered_map<std::string, int32> Cache;

    for (char& c : ObjectName)
    {
        if (c == ':')
            c = '.';
    }

    {
        std::lock_guard<std::mutex> Lock(CacheMutex);
        auto It = Cache.find(ObjectName);

        if (It != Cache.end())
        {
            UObject* Cached = UObject::GObjects->GetByIndex(It->second);

            if (Cached && _stricmp(GetObjectPathName(Cached).c_str(), ObjectName.c_str()) == 0)
                return static_cast<T*>(Cached);

            Cache.erase(It);
        }
    }

    // Filtre de type : la classe demandee, ou T s'il s'agit d'un UObject (les appels du GS passent rarement la classe)
    UClass* FilterClass = ObjectClass;

    if constexpr (requires { T::StaticClass(); })
    {
        if (!FilterClass || FilterClass == UObject::StaticClass())
            FilterClass = T::StaticClass();
    }

    const size_t LastSeparator = ObjectName.find_last_of('.');
    const std::string ShortName = LastSeparator == std::string::npos ? ObjectName : ObjectName.substr(LastSeparator + 1);

    for (int32 i = 0; i < UObject::GObjects->Num(); i++)
    {
        UObject* Object = UObject::GObjects->GetByIndex(i);

        if (!Object)
            continue;

        if (FilterClass && !Object->IsA(FilterClass))
            continue;

        if (_stricmp(Object->Name.GetRawString().c_str(), ShortName.c_str()) != 0)
            continue;

        if (_stricmp(GetObjectPathName(Object).c_str(), ObjectName.c_str()) != 0)
            continue;

        std::lock_guard<std::mutex> Lock(CacheMutex);
        Cache[ObjectName] = i;

        return static_cast<T*>(Object);
    }

    return nullptr;
}

// [OFFSET-TODO-6.21] StaticLoadObject (8.51 = 0x1E838D0)
// static auto StaticLoadObjectInternal = reinterpret_cast<UObject * (*)(UClass * InClass, UObject * InOuter, const TCHAR * Name, const TCHAR * FileName, uint32_t LoadFlags, UPackageMap * Sandbox, bool bAllowObjectReconciliation)>(uintptr_t(GetModuleHandle(NULL)) + 0x1E838D0);
static auto StaticLoadObjectInternal = Offsets621::Native<UObject*(UClass* InClass, UObject* InOuter, const TCHAR* Name, const TCHAR* FileName, uint32_t LoadFlags, UPackageMap* Sandbox, bool bAllowObjectReconciliation)>(Offsets621::StaticLoadObject, "StaticLoadObject");

template <typename T>
static T* StaticLoadObject(const TCHAR* InPath)
{
    if (StaticLoadObjectInternal.Func)
        return (T*)StaticLoadObjectInternal(T::StaticClass(), nullptr, InPath, nullptr, 0, nullptr, false);

    // [GALAXIA PATCH] Repli sans adresse : l'objet n'est pas charge, on le cherche seulement s'il est deja en memoire
    Offsets621::LogMissing("StaticLoadObject (repli : objets deja charges uniquement)");

    const std::wstring WidePath(InPath);
    return FindObjectFast<T>(std::string(WidePath.begin(), WidePath.end()), T::StaticClass());
}

#define RESULT_PARAM Z_Param__Result
#define RESULT_DECL void*const RESULT_PARAM

#pragma pack(push, 0x1)
class alignas(0x08) FOutputDevice
{
public:
    uint8 Pad_01[0x10];
};
#pragma pack(pop)

struct FFrame : public FOutputDevice
{
public:
    UFunction* Node; // 0x10
    UObject* Object; // 0x18
    uint8* Code; // 0x20
    uint8* Locals; // 0x28

    UProperty* MostRecentProperty; // 0x30
    uint8* MostRecentPropertyAddress; // 0x38

    uint8 Pad_01[0x58];

public:
    // [GALAXIA PATCH][OFFSET-TODO-6.21] FFrame::Step (8.51 = 0x1e83f60) : necessaire quand la fonction est appelee depuis un Blueprint (Code != nullptr)
    static bool CanStepBytecode()
    {
        return Offsets621::FFrame_Step != 0 && Offsets621::IsExpectedBuild();
    }

    void Step(UObject* Context, RESULT_DECL)
    {
        // 7FF66FE7FCE0
        // void (*Step)(FFrame * Frame, UObject * Context, RESULT_DECL) = decltype(Step)(0x1e83f60 + uintptr_t(GetModuleHandle(0)));
        auto Step = Offsets621::Native<void(FFrame* Frame, UObject* Context, void* const Result)>(Offsets621::FFrame_Step, "FFrame::Step");
        Step(this, Context, RESULT_PARAM);
    }

    // [GALAXIA PATCH] FFrame::StepExplicitProperty (8.51 = 0x1e83f90) reecrit sans adresse native, d'apres UE 4.21 :
    // parametre de sortie -> adresse dans OutParms, sinon copie de la valeur depuis Locals.
    // OutParms est a 0x78 dans FFrame (UE 4.21, juste avant PropertyChainForCompiledIn a 0x80).
    void StepExplicitProperty(void* const Result, UProperty* Property)
    {
        // Version native 6.21 si l'adresse est la bonne (copie correcte des types complexes)
        static auto NativeStepExplicitProperty = Offsets621::Native<void(FFrame* Frame, void* const Result, UProperty* Property)>(Offsets621::FFrame_StepExplicitProperty, "FFrame::StepExplicitProperty");

        if (NativeStepExplicitProperty.Func)
        {
            NativeStepExplicitProperty(this, Result, Property);
            return;
        }

        constexpr uint64 CPF_OutParm = 0x100;

        struct FOutParmRec
        {
            UProperty* Property;
            uint8* PropAddr;
            FOutParmRec* NextOutParm;
        };

        if (Property->PropertyFlags & CPF_OutParm)
        {
            FOutParmRec* Out = *(FOutParmRec**)(__int64(this) + 0x78);

            while (Out && Out->Property != Property)
                Out = Out->NextOutParm;

            MostRecentPropertyAddress = Out ? Out->PropAddr : nullptr;
        }
        else
        {
            MostRecentPropertyAddress = Locals + Property->Offset;

            if (Result)
                memcpy(Result, MostRecentPropertyAddress, size_t(Property->ElementSize) * Property->ArrayDim);
        }
    }

    void StepCompiledIn(void* const Result)
    {
        // https://imgur.com/q5efUyh

        if (Code)
        {
            Step(Object, Result);
        }
        else
        {
            // https://imgur.com/a/CvmkuCy
            UProperty* Property = (UProperty*)(*(UField**)(__int64(this) + 0x80));
            *(UField**)(__int64(this) + 0x80) = Property->Next;

            StepExplicitProperty(Result, Property);
        }
    }

    template<typename TNativeType>
    TNativeType& StepCompiledInRef(void* const TemporaryBuffer)
    {
        MostRecentPropertyAddress = NULL;

        if (Code)
        {
            Step(Object, TemporaryBuffer);
        }
        else
        {
            // https://imgur.com/a/CvmkuCy
            UProperty* Property = (UProperty*)(*(UField**)(__int64(this) + 0x80));
            *(UField**)(__int64(this) + 0x80) = Property->Next;

            StepExplicitProperty(TemporaryBuffer, Property);
        }

        return (MostRecentPropertyAddress != NULL) ? *(TNativeType*)(MostRecentPropertyAddress) : *(TNativeType*)TemporaryBuffer;
    }
};

struct FActorSpawnParameters
{
    /* A name to assign as the Name of the Actor being spawned. If no value is specified, the name of the spawned Actor will be automatically generated using the form [Class]_[Number]. */
    FName Name = FName(0);

    /* An Actor to use as a template when spawning the new Actor. The spawned Actor will be initialized using the property values of the template Actor. If left NULL the class default object (CDO) will be used to initialize the spawned Actor. */
    AActor* Template = nullptr;

    /* The Actor that spawned this Actor. (Can be left as NULL). */
    AActor* Owner = nullptr;

    /* The APawn that is responsible for damage done by the spawned Actor. (Can be left as NULL). */
    APawn* Instigator = nullptr;

    /* The ULevel to spawn the Actor in, i.e. the Outer of the Actor. If left as NULL the Outer of the Owner is used. If the Owner is NULL the persistent level is used. */
    class	ULevel* OverrideLevel = nullptr;

    /** Method for resolving collisions at the spawn point. Undefined means no override, use the actor's setting. */
    ESpawnActorCollisionHandlingMethod SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::Undefined;

private:

    friend class UPackageMapClient;

    /* Is the actor remotely owned. This should only be set true by the package map when it is creating an actor on a client that was replicated from the server. */
    uint16	bRemoteOwned : 1;

public:

    bool IsRemoteOwned() const { return bRemoteOwned; }

    /* Determines whether spawning will not fail if certain conditions are not met. If true, spawning will not fail because the class being spawned is `bStatic=true` or because the class of the template Actor is not the same as the class of the Actor being spawned. */
    uint16	bNoFail : 1;

    /* Determines whether the construction script will be run. If true, the construction script will not be run on the spawned Actor. Only applicable if the Actor is being spawned from a Blueprint. */
    uint16	bDeferConstruction : 1;

    /* Determines whether or not the actor may be spawned when running a construction script. If true spawning will fail if a construction script is being run. */
    uint16	bAllowDuringConstructionScript : 1;

#if WITH_EDITOR
    /** Determines whether the begin play cycle will run on the spawned actor when in the editor. */
    uint16 bTemporaryEditorActor : 1;
#endif

    /* Flags used to describe the spawned actor/object instance. */
    EObjectFlags ObjectFlags;
};

class Util
{
public:
    template <typename ActorType>
    static ActorType* SpawnActor(UClass* Class, FVector Location = FVector(), FRotator Rotation = FRotator())
    {
        /*FQuat Quat;
        FTransform Transform;

        auto DEG_TO_RAD = 3.14159 / 180;
        auto DIVIDE_BY_2 = DEG_TO_RAD / 2;

        auto SP = sin(Rotation.Pitch * DIVIDE_BY_2);
        auto CP = cos(Rotation.Pitch * DIVIDE_BY_2);

        auto SY = sin(Rotation.Yaw * DIVIDE_BY_2);
        auto CY = cos(Rotation.Yaw * DIVIDE_BY_2);

        auto SR = sin(Rotation.Roll * DIVIDE_BY_2);
        auto CR = cos(Rotation.Roll * DIVIDE_BY_2);

        Quat.X = CR * SP * SY - SR * CP * CY;
        Quat.Y = -CR * SP * CY - SR * CP * SY;
        Quat.Z = CR * CP * SY - SR * SP * CY;
        Quat.W = CR * CP * CY + SR * SP * SY;

        Transform.Rotation = Quat;
        Transform.Scale3D = Scale3D;
        Transform.Translation = Location;

        auto Actor = Globals::GetGameplayStatics()->BeginDeferredActorSpawnFromClass(Globals::GetWorld(), Class, Transform, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn, nullptr);
        Globals::GetGameplayStatics()->FinishSpawningActor(Actor, Transform);*/

        // [OFFSET-TODO-6.21] UWorld::SpawnActor (8.51 = 0x2CAAAE0) : remplace par BeginDeferredActorSpawnFromClass + FinishSpawningActor (sans adresse)
        // AActor* (*RealSpawnActor)(UWorld* World, UClass* Class, FVector const* Location, FRotator const* Rotation, const FActorSpawnParameters& SpawnParameters) = decltype(RealSpawnActor)(0x2CAAAE0 + uintptr_t(GetModuleHandle(0)));
        //
        // auto SpawnParameters = (FActorSpawnParameters*)VirtualAlloc(0, sizeof(FActorSpawnParameters), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
        // SpawnParameters->SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
        // SpawnParameters->bDeferConstruction = false;
        //
        // AActor* Actor = RealSpawnActor(UWorld::GetWorld(), Class, &Location, &Rotation, *SpawnParameters);
        //
        // VirtualFree(SpawnParameters, 0, MEM_RELEASE);
        //
        // return (ActorType*)Actor;

        const FTransform Transform = UKismetMathLibrary::MakeTransform(Location, Rotation, FVector{ 1.0f, 1.0f, 1.0f });
        return SpawnActorTransfrom<ActorType>(Class, Transform);
    }

    template <typename ActorType>
    static ActorType* SpawnActorTransfrom(UClass* Class, FTransform Transform)
    {
        // [OFFSET-TODO-6.21] UWorld::SpawnActor (8.51 = 0x2ca9b50) : remplace par BeginDeferredActorSpawnFromClass + FinishSpawningActor (sans adresse)
        // AActor* (*RealSpawnActor)(UWorld * World, UClass * Class, FTransform Transform, const FActorSpawnParameters & SpawnParameters) = decltype(RealSpawnActor)(0x2ca9b50 + uintptr_t(GetModuleHandle(0)));
        //
        // auto SpawnParameters = (FActorSpawnParameters*)VirtualAlloc(0, sizeof(FActorSpawnParameters), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
        // SpawnParameters->SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
        // SpawnParameters->bDeferConstruction = false;
        //
        // AActor* Actor = RealSpawnActor(UWorld::GetWorld(), Class, Transform, *SpawnParameters);
        //
        // VirtualFree(SpawnParameters, 0, MEM_RELEASE);
        //
        // return (ActorType*)Actor;

        if (!Class)
            return nullptr;

        AActor* Actor = UGameplayStatics::BeginDeferredActorSpawnFromClass(UWorld::GetWorld(), Class, Transform, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn, nullptr);

        if (Actor)
            Actor = UGameplayStatics::FinishSpawningActor(Actor, Transform);

        return (ActorType*)Actor;
    }
};