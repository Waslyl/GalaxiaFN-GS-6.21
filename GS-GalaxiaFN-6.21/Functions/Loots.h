#pragma once
#include <random>

struct FFortLootTierDataCheck
{
public:
    FName                                         TierGroup;                                         // 0x0000(0x0008)(100% c'est ça)
    int32                                         LootTier;                                          // 0x0008(0x0004)(100% c'est ça)
    uint8                                         Pad_1[0x4];                                        // 0x000C(0x0004)(jsp ça commence à me faire peur)
    ELootQuotaLevel                               QuotaLevel;                                        // 0x0010(0x0001)(inshallah)
    FGameplayTagContainer                         GameplayTags;                                      // 0x0018(0x0008)(inshallah)
    int32                                         WorldLevel;                                        // 0x0020(0x0004)(inshallah)
};

struct FFortLootPackageDataCheck
{
public:
    FName                                         LootPackageName;                                   // 0x0000(0x0008)(100% c'est ça)
    int32                                         LootPackageCategory;                               // 0x0008(0x0004)(100% c'est ça)
    uint8                                         Pad_1[0x4];                                        // 0x000C(0x0004)(jsp ça commence à me faire peur)
    FGameplayTagContainer                         GameplayTags;                                      // 0x0010(0x0001)(inshallah)
    int32                                         WorldLevel;                                        // 0x0018(0x0008)(inshallah)
};

namespace Loots
{
    UDataTable* LoadDataTable(TSoftObjectPtr<UDataTable> DataTableToLoad)
    {
        // [OFFSET-TODO-6.21] LoadDataTable (8.51 = 0xe25400)
        // UDataTable* (*LoadDataTable)(TSoftObjectPtr<UDataTable> DataTableToLoad, char a2) = decltype(LoadDataTable)(0xe25400 + uintptr_t(GetModuleHandle(0)));
        auto LoadDataTable = Offsets621::Native<UDataTable*(TSoftObjectPtr<UDataTable> DataTableToLoad, char a2)>(Offsets621::LoadDataTable, "LoadDataTable", &Fallbacks621::LoadDataTable);
        return LoadDataTable(DataTableToLoad, 1);
    }

    UFortItemDefinition* LoadItemDefinition(TSoftObjectPtr<UFortItemDefinition> SoftItemDefinition)
    {
        UFortItemDefinition* ItemDefinition = SoftItemDefinition.Get();

        if (!ItemDefinition)
        {
            const FString& AssetPathName = UKismetStringLibrary::Conv_NameToString(SoftItemDefinition.ObjectID.AssetPathName);
            ItemDefinition = StaticLoadObject<UFortItemDefinition>(AssetPathName.CStr());
        }

        return ItemDefinition;
    }

    TArray<UDataTable*> GetPlaylistLootTierDataTables()
    {
        AFortGameModeAthena* GameModeAthena = Globals::GetGameMode();
        TArray<UDataTable*> LootTierDataTables;

        if (GameModeAthena)
        {
            // [OFFSET-TODO-6.21] GetPlaylistLootTierDatas (8.51 = 0xf9fce0)
            // void (*GetPlaylistLootTierDatas)(AFortGameModeAthena* GameModeAthena, TArray<UDataTable*>* OutLootTierDatas) = decltype(GetPlaylistLootTierDatas)(0xf9fce0 + uintptr_t(GetModuleHandle(0)));
            auto GetPlaylistLootTierDatas = Offsets621::Native<void(AFortGameModeAthena* GameModeAthena, TArray<UDataTable*>* OutLootTierDatas)>(Offsets621::GetPlaylistLootTierDatas, "GetPlaylistLootTierDatas");
            GetPlaylistLootTierDatas(GameModeAthena, &LootTierDataTables);
        }

        for (int32 i = 0; i < LootTierDataTables.Num(); i++)
        {
            UCompositeDataTable* CompositeDataTable = Cast<UCompositeDataTable>(LootTierDataTables[i]);
            if (!CompositeDataTable) continue;

            break;
            for (int32 j = 0; j < CompositeDataTable->ParentTables.Num(); j++)
            {
                UDataTable* ParentTable = CompositeDataTable->ParentTables[j];
                if (!ParentTable) continue;

                LootTierDataTables.Add(ParentTable);
            }
        }

        return LootTierDataTables;
    }
    
    TArray<UDataTable*> GetPlaylistLootPackageDatas()
    {
        AFortGameModeAthena* GameModeAthena = Globals::GetGameMode();
        TArray<UDataTable*> LootPackageDatas;

        if (GameModeAthena)
        {
            // [OFFSET-TODO-6.21] GetPlaylistLootPackageDatas (8.51 = 0xf9fc20)
            // void (*GetPlaylistLootPackageDatas)(AFortGameModeAthena* GameModeAthena, TArray<UDataTable*>* OutLootPackageDatas) = decltype(GetPlaylistLootPackageDatas)(0xf9fc20 + uintptr_t(GetModuleHandle(0)));
            auto GetPlaylistLootPackageDatas = Offsets621::Native<void(AFortGameModeAthena* GameModeAthena, TArray<UDataTable*>* OutLootPackageDatas)>(Offsets621::GetPlaylistLootPackageDatas, "GetPlaylistLootPackageDatas");
            GetPlaylistLootPackageDatas(GameModeAthena, &LootPackageDatas);
        }

        for (int32 i = 0; i < LootPackageDatas.Num(); i++)
        {
            UCompositeDataTable* CompositeDataTable = Cast<UCompositeDataTable>(LootPackageDatas[i]);
            if (!CompositeDataTable) continue;

            break;
            for (int32 j = 0; j < CompositeDataTable->ParentTables.Num(); j++)
            {
                UDataTable* ParentTable = CompositeDataTable->ParentTables[j];
                if (!ParentTable) continue;

                LootPackageDatas.Add(ParentTable);
            }
        }

        return LootPackageDatas;
    }

    // [GALAXIA PATCH] 6.21 : le repli du jeu vise LootTierData_Client / LootPackages_Client, introuvables ("Failed attempt to load").
    // Si la table de la playlist ne se charge pas, on la pointe vers la table Athena : le code natif (coffres, butin au sol) et celui du GS l'utilisent ensuite.
    UDataTable* EnsureLootTable(TSoftObjectPtr<UDataTable>& SoftTable, const wchar_t* AthenaTablePath)
    {
        UDataTable* DataTable = LoadDataTable(SoftTable);

        if (!DataTable)
        {
            SoftTable.WeakPtr.ObjectIndex = 0;
            SoftTable.WeakPtr.ObjectSerialNumber = 0;
            SoftTable.TagAtLastTest = -1;
            SoftTable.ObjectID.AssetPathName = UKismetStringLibrary::Conv_StringToName(AthenaTablePath);

            DataTable = LoadDataTable(SoftTable);
        }

        return DataTable;
    }

    void EnsurePlaylistLootTables(UFortPlaylistAthena* PlaylistAthena)
    {
        if (!PlaylistAthena)
            return;

        UDataTable* LootTierData = EnsureLootTable(PlaylistAthena->LootTierData, L"/Game/Items/Datatables/AthenaLootTierData_Client.AthenaLootTierData_Client");
        UDataTable* LootPackages = EnsureLootTable(PlaylistAthena->LootPackages, L"/Game/Items/Datatables/AthenaLootPackages_Client.AthenaLootPackages_Client");

        if (!LootTierData || !LootPackages)
            FN_LOG(LogLoot, Error, "Tables de loot Athena introuvables : LootTierData=%p, LootPackages=%p", LootTierData, LootPackages);
    }

    FFortLootTierData* FindLootTierDataRow(UFortGameData* GameData, FName LootTierKey, FString ContextString)
    {
        TArray<UDataTable*> LootTierDataTables = GetPlaylistLootTierDataTables();

        if (LootTierDataTables.Num() <= 0)
        {
            for (int32 i = 0; i < GameData->LootTierDataTables.Num(); i++)
            {
                TSoftObjectPtr<UDataTable> LootTierDataTable = GameData->LootTierDataTables[i];

                UDataTable* DataTable = LoadDataTable(LootTierDataTable);

                if (DataTable)
                    LootTierDataTables.Add(DataTable);
            }
        }

        FFortLootTierData* SelectLootTierData = nullptr;

        for (int32 i = 0; i < LootTierDataTables.Num(); i++)
        {
            UDataTable* LootTierDataTable = LootTierDataTables[i];
            if (!LootTierDataTable) continue;

            FFortLootTierData* LootTierData = Globals::GetDataTableRowFromName<FFortLootTierData>(LootTierDataTable, LootTierKey);

            if (LootTierData)
            {
                SelectLootTierData = LootTierData;
                break;
            }
        }

        if (!SelectLootTierData)
            FN_LOG(LogLoot, Warning, "UFortGameData::FindLootTierDataRow : TierGroupName %s with context %s not found!", LootTierKey.ToString().c_str(), ContextString.ToString().c_str());

        return SelectLootTierData;
    }

    bool IsLootDropPossible(FName LootTierKey, FString ContextString, __int64 a3)
    {
        if (!LootTierKey.IsValid())
            return false;

        UFortGameData* GameData = Globals::GetGameData();
        FFortLootTierData* LootTierData = FindLootTierDataRow(GameData, LootTierKey, ContextString);

        if (!LootTierData)
            return false;

        float v9 = (float)rand() * 0.000030518509f;

        float NumLootPackageDrops = LootTierData->NumLootPackageDrops;
        return NumLootPackageDrops > 0.0f && v9 <= NumLootPackageDrops;
    }

    int32 FindLootTierFromKey(FName LootTierKey)
    {
        UFortGameData* GameData = Globals::GetGameData();
        FFortLootTierData* LootTierData = FindLootTierDataRow(GameData, LootTierKey, L"UFortLootTier::FindLootTierFromKey");

        if (LootTierData)
            return LootTierData->LootTier;

        return -1;
    }

    bool IsValidLootTier(FFortLootTierDataCheck LootTierDataCheck, FFortLootTierData LootTierData)
    {
        if (LootTierDataCheck.TierGroup != LootTierData.TierGroup)
            return false;

        int32 LootTier = LootTierDataCheck.LootTier;

        if (LootTier != -1 && LootTier != LootTierData.LootTier)
            return false;

        ELootQuotaLevel QuotaLevel = LootTierDataCheck.QuotaLevel;

        if (QuotaLevel < ELootQuotaLevel::NumLevels && QuotaLevel != LootTierData.QuotaLevel)
            return false;

        if (LootTierData.RequiredGameplayTags.GameplayTags.IsValid())
        {
            bool bFindTagName = false;

            for (int32 i = 0; i < LootTierData.RequiredGameplayTags.GameplayTags.Num(); i++)
            {
                FGameplayTag GameplayTag = LootTierData.RequiredGameplayTags.GameplayTags[i];
                if (!GameplayTag.TagName.IsValid()) continue;

                const FName TagName = GameplayTag.TagName;

                for (int32 j = 0; j < LootTierDataCheck.GameplayTags.GameplayTags.Num(); j++)
                {
                    FGameplayTag GameplayTagCheck = LootTierDataCheck.GameplayTags.GameplayTags[j];

                    if (GameplayTagCheck.TagName == TagName)
                    {
                        bFindTagName = true;
                        break;
                    }
                }
            }

            if (!bFindTagName)
                return false;
        }

        int32 WorldLevel = LootTierDataCheck.WorldLevel;

        if (WorldLevel >= 0)
        {
            int32 MaxWorldLevel = LootTierData.MaxWorldLevel;

            if (MaxWorldLevel >= 0 && WorldLevel > MaxWorldLevel)
                return false;

            int32 MinWorldLevel = LootTierData.MinWorldLevel;

            if (MinWorldLevel >= 0 && WorldLevel < MinWorldLevel)
                return false;
        }

        return true;
    }

    bool IsValidLootPackage(FFortLootPackageData LootPackageData, FName LootPackageName, int32 LootPackageCategory, FGameplayTagContainer GameplayTags, int32 WorldLevel)
    {
        if (LootPackageName != LootPackageData.LootPackageID || LootPackageCategory != -1 && LootPackageData.LootPackageCategory != LootPackageCategory)
            return false;

        if (LootPackageData.RequiredTag.IsValid())
        {
            FName RequiredTag = LootPackageData.RequiredTag;
            bool bFindTagName = false;

            for (int32 i = 0; i < GameplayTags.GameplayTags.Num(); i++)
            {
                FGameplayTag GameplayTag = GameplayTags.GameplayTags[i];

                if (GameplayTag.TagName == RequiredTag)
                {
                    bFindTagName = true;
                    break;
                }
            }

            for (int32 i = 0; i < GameplayTags.ParentTags.Num(); i++)
            {
                FGameplayTag ParentTag = GameplayTags.ParentTags[i];

                if (ParentTag.TagName == RequiredTag)
                {
                    bFindTagName = true;
                    break;
                }
            }

            if (!bFindTagName)
                return false;
        }

        if (WorldLevel >= 0)
        {
            int32 MaxWorldLevel = LootPackageData.MaxWorldLevel;

            if (MaxWorldLevel >= 0 && WorldLevel > MaxWorldLevel)
                return false;

            int32 MinWorldLevel = LootPackageData.MinWorldLevel;

            if (MinWorldLevel >= 0 && WorldLevel < MinWorldLevel)
                return false;
        }

        return true;
    }

    void CollectLootTierDatas(std::map<FName, FFortLootTierData*>* OutLootTierDatas, float* OutWeight, TArray<UDataTable*> LootTierDataTables, FFortLootTierDataCheck LootTierDataCheck)
    {
        for (int32 i = 0; i < LootTierDataTables.Num(); i++)
        {
            UDataTable* LootTierDataTable = LootTierDataTables[i];
            if (!LootTierDataTable) continue;

            TArray<FName> RowNames;
            UDataTableFunctionLibrary::GetDataTableRowNames(LootTierDataTable, &RowNames);

            for (int32 j = 0; j < RowNames.Num(); j++)
            {
                FName RowName = RowNames[j];
                if (!RowName.IsValid()) continue;

                FFortLootTierData* LootTierData = Globals::GetDataTableRowFromName<FFortLootTierData>(LootTierDataTable, RowName);

                if (LootTierData && IsValidLootTier(LootTierDataCheck, *LootTierData))
                {
                    auto ExistingData = OutLootTierDatas->find(RowName);
                    if (ExistingData != OutLootTierDatas->end())
                    {
                        ExistingData->second = LootTierData;
                    }
                    else
                    {
                        OutLootTierDatas->emplace(RowName, LootTierData);
                        *OutWeight = LootTierData->Weight + *OutWeight;
                    }
                }
            }
        }

        if (LootTierDataTables.IsValid())
            LootTierDataTables.Free();
    }

    void CollectLootPackageDatas(TArray<FFortLootPackageData*>* OutLootPackageDatas, float* OutWeight, TArray<UDataTable*> LootTierPackageTables, FFortLootPackageDataCheck LootTierPackageCheck)
    {
        for (int32 i = 0; i < LootTierPackageTables.Num(); i++)
        {
            UDataTable* LootTierPackageTable = LootTierPackageTables[i];
            if (!LootTierPackageTable) continue;

            TArray<FName> RowNames;
            UDataTableFunctionLibrary::GetDataTableRowNames(LootTierPackageTable, &RowNames);

            for (int32 j = 0; j < RowNames.Num(); j++)
            {
                FName RowName = RowNames[j];
                if (!RowName.IsValid()) continue;

                FFortLootPackageData* LootPackageData = Globals::GetDataTableRowFromName<FFortLootPackageData>(LootTierPackageTable, RowName);

                if (LootPackageData && IsValidLootPackage(*LootPackageData, LootTierPackageCheck.LootPackageName, LootTierPackageCheck.LootPackageCategory, LootTierPackageCheck.GameplayTags, LootTierPackageCheck.WorldLevel))
                {
                    bool bRemove = false;
                    for (int32 x = 0; x < OutLootPackageDatas->Num(); x++)
                    {
                        FFortLootPackageData* TempLootPackageData = (*OutLootPackageDatas)[x];
                        if (!TempLootPackageData) continue;

                        if (TempLootPackageData == LootPackageData)
                        {
                            OutLootPackageDatas->Remove(x);
                            bRemove = true;
                        }
                    }

                    OutLootPackageDatas->Add(LootPackageData);

                    if (!bRemove)
                        *OutWeight = LootPackageData->Weight + *OutWeight;
                }
            }
        }

        if (LootTierPackageTables.IsValid())
            LootTierPackageTables.Free();
    }

    void ChooseLootTierDatas(UFortGameData* GameData, std::map<FName, FFortLootTierData*>* OutLootTierDatas, float* OutWeight, FFortLootTierDataCheck LootTierDataCheck)
    {
        TArray<UDataTable*> LootTierDataTables = GetPlaylistLootTierDataTables();

        if (LootTierDataTables.Num() <= 0)
        {
            for (int32 i = 0; i < GameData->LootTierDataTables.Num(); i++)
            {
                TSoftObjectPtr<UDataTable> LootTierDataTable = GameData->LootTierDataTables[i];

                UDataTable* DataTable = LoadDataTable(LootTierDataTable);

                if (DataTable)
                    LootTierDataTables.Add(DataTable);
            }
        }

        CollectLootTierDatas(OutLootTierDatas, OutWeight, LootTierDataTables, LootTierDataCheck);
    }

    void ChooseLootPackageDatas(UFortGameData* GameData, TArray<FFortLootPackageData*>* OutLootPackageDatas, float* OutWeight, FFortLootPackageDataCheck LootPackageDataCheck)
    {
        TArray<UDataTable*> LootPackageDataTables = GetPlaylistLootPackageDatas();

        if (LootPackageDataTables.Num() <= 0)
        {
            for (int32 i = 0; i < GameData->LootPackageDataTables.Num(); i++)
            {
                TSoftObjectPtr<UDataTable> LootPackageDataTable = GameData->LootPackageDataTables[i];

                UDataTable* DataTable = LoadDataTable(LootPackageDataTable);

                if (DataTable)
                    LootPackageDataTables.Add(DataTable);
            }
        }

        CollectLootPackageDatas(OutLootPackageDatas, OutWeight, LootPackageDataTables, LootPackageDataCheck);
    }

    bool ChooseLootTierKeyAndLootTierByWeight(FName* OutLootTierKey, int32* OutLootTier, float TotalWeight, std::map<FName, FFortLootTierData*> LootTierDatas, __int64 a5)
    {
        if (TotalWeight <= 0.0000000099999999f)
            return false;

        float SelectWeight = (float)rand() * 0.000030518509f;
        float TargetWeight = SelectWeight * TotalWeight;

        if (LootTierDatas.empty())
            return false;

        for (auto& LootTierDataMap : LootTierDatas)
        {
            FFortLootTierData* LootTierData = LootTierDataMap.second;
            FName LootTierKey = LootTierDataMap.first;

            if (!LootTierData || !LootTierKey.IsValid())
                continue;

            float CurrentWeight = LootTierData->Weight;

            if (TargetWeight <= CurrentWeight)
            {
                *OutLootTier = LootTierData->LootTier;
                *OutLootTierKey = LootTierKey;
                return true;
            }

            TargetWeight -= CurrentWeight;
        }

        return false;
    }

    bool PickLootTierKeyAndLootTierFromTierGroup(FName* OutLootTierKey, int32* OutLootTier, FName TierGroupName, int32 WorldLevel, __int64 a5, int32 ForcedLootTier, FGameplayTagContainer StaticGameplayTags)
    {
        FFortLootTierDataCheck LootTierDataCheck;
        LootTierDataCheck.TierGroup = TierGroupName;
        LootTierDataCheck.LootTier = -1;
        LootTierDataCheck.QuotaLevel = ELootQuotaLevel::Unlimited;
        LootTierDataCheck.GameplayTags = StaticGameplayTags;
        LootTierDataCheck.WorldLevel = WorldLevel;

        float TotalWeight = 0.0f;
        float FinalWeight = 0.0f;

        std::map<FName, FFortLootTierData*> LootTierDatas;

        UFortGameData* GameData = Globals::GetGameData();
        ChooseLootTierDatas(GameData, &LootTierDatas, &TotalWeight, LootTierDataCheck);

        if (ForcedLootTier == -1)
        {
            FinalWeight = TotalWeight;
        }
        else
        {
            std::map<FName, FFortLootTierData*> NewLootTierDatas;
            float NewWeight = 0.0f;

            for (auto& LootTierDataMap : LootTierDatas)
            {
                FFortLootTierData* LootTierData = LootTierDataMap.second;
                FName LootTierKey = LootTierDataMap.first;

                if (LootTierData && LootTierKey.IsValid())
                {
                    if (LootTierData->LootTier == ForcedLootTier && LootTierData->Weight > 0.0f)
                    {
                        NewLootTierDatas[LootTierKey] = LootTierData;
                        NewWeight = LootTierData->Weight + NewWeight;
                    }
                }
            }

            LootTierDatas = NewLootTierDatas;
            FinalWeight = NewWeight;
        }

        return ChooseLootTierKeyAndLootTierByWeight(OutLootTierKey, OutLootTier, FinalWeight, LootTierDatas, a5);
    }

    bool PickLootDropsFromLootPackage(TArray<FFortItemEntry>* OutLootToDrops, FName LootPackageName, int32 WorldLevel, int32 LootPackageCategory, __int64 a5, __int64 a6, FGameplayTagContainer StaticGameplayTags, bool bAllowBonusDrops)
    {
        FFortLootPackageDataCheck LootTierPackageCheck;
        LootTierPackageCheck.LootPackageName = LootPackageName;
        LootTierPackageCheck.LootPackageCategory = LootPackageCategory;
        LootTierPackageCheck.GameplayTags = StaticGameplayTags;
        LootTierPackageCheck.WorldLevel = WorldLevel;

        float TotalWeight = 0.0f;

        TArray<FFortLootPackageData*> LootPackageDatas;

        UFortGameData* GameData = Globals::GetGameData();
        ChooseLootPackageDatas(GameData, &LootPackageDatas, &TotalWeight, LootTierPackageCheck);

        if (TotalWeight == 0.0f)
        {
            FN_LOG(LogLoot, Warning, "Loot Package %s has no valid weights with the following data:\nLootPackageCategory: %d\nWorldLevel: %d\nTags: ",
                LootPackageName.ToString().c_str(), LootPackageCategory, WorldLevel);

            for (int32 i = 0; i < StaticGameplayTags.GameplayTags.Num(); i++)
            {
                FGameplayTag GameplayTag = StaticGameplayTags.GameplayTags[i];
                if (!GameplayTag.TagName.IsValid()) continue;

                FN_LOG(LogLoot, Warning, "%s", GameplayTag.TagName.ToString().c_str());
            }

            for (int32 i = 0; i < StaticGameplayTags.ParentTags.Num(); i++)
            {
                FGameplayTag ParentTag = StaticGameplayTags.ParentTags[i];
                if (!ParentTag.TagName.IsValid()) continue;

                FN_LOG(LogLoot, Warning, "%s", ParentTag.TagName.ToString().c_str());
            }

            return false;
        }

        float RandomWeight = ((float)rand() * 0.000030518509f) * TotalWeight;
        FFortLootPackageData* SelectLootPackageData = nullptr;

        for (int32 i = 0; i < LootPackageDatas.Num(); i++)
        {
            FFortLootPackageData* LootPackageData = LootPackageDatas[i];
            if (!LootPackageData) continue;

            if (RandomWeight <= LootPackageData->Weight)
            {
                SelectLootPackageData = LootPackageData;
                break;
            }

            RandomWeight -= LootPackageData->Weight;
        }

        if (!SelectLootPackageData)
            return false;

        FN_LOG(LogLoot, Debug, "PickLootDropsFromLootPackage selected package %s", SelectLootPackageData->LootPackageID.ToString().c_str());

        if (SelectLootPackageData->LootPackageCall.IsValid())
        {
            if (SelectLootPackageData->Count > 0)
            {
                FName LootPackageCallName = UKismetStringLibrary::Conv_StringToName(SelectLootPackageData->LootPackageCall);

                for (int32 i = 0; i < SelectLootPackageData->Count; ++i)
                {
                    PickLootDropsFromLootPackage(
                        OutLootToDrops,
                        LootPackageCallName,
                        WorldLevel,
                        -1, // -1
                        a5,
                        a6,
                        StaticGameplayTags,
                        false);
                }
            }

            return false;
        }

        if (!SelectLootPackageData->ItemDefinition.Get())
        {
            FName AssetPathName = SelectLootPackageData->ItemDefinition.ObjectID.AssetPathName;

            if (!AssetPathName.IsValid())
            {
                FN_LOG(LogLoot, Warning, "Loot Package %s does not contain a LootPackageCall or ItemDefinition.", SelectLootPackageData->LootPackageID.ToString().c_str());
                return false;
            }
        }

        UFortItemDefinition* ItemDefinition = LoadItemDefinition(SelectLootPackageData->ItemDefinition);

        if (!ItemDefinition)
        {
            FN_LOG(LogLoot, Warning, "Failed to load loot package %s.", SelectLootPackageData->LootPackageID.ToString().c_str());
            return false;
        }

        UFortWorldItemDefinition* WorldItemDefinition = Cast<UFortWorldItemDefinition>(ItemDefinition);
        int32 ItemLevel = 0;

        if (WorldItemDefinition)
        {
            ItemLevel = Inventory::GetItemLevel(WorldItemDefinition->LootLevelData);
        }

        int32 CountMultiplier = 1;

        FN_LOG(LogLoot, Debug, "Dropping %s (Count: %i, CountMultiplier: %i, ItemLevel: %i",
            ItemDefinition->DisplayName.ToString().c_str(), SelectLootPackageData->Count, CountMultiplier, ItemLevel);

        int32 TotalCount = CountMultiplier * SelectLootPackageData->Count;

        if (TotalCount > 0)
        {
            int32 Level = 0;

            if (ItemLevel >= 0)
                Level = ItemLevel;

            for (int32 i = 0; i < TotalCount; i++)
            {
                int32 MaxStackSize = ItemDefinition->MaxStackSize;
                int32 Count = MaxStackSize;

                if (TotalCount <= MaxStackSize)
                    Count = TotalCount;

                if (Count <= 0)
                    Count = 0;

                FFortItemEntry ItemEntry = Inventory::CreateItemEntry(WorldItemDefinition, Count, 0.f, -1, -1);
                OutLootToDrops->Add(ItemEntry);
                TotalCount -= Count;
            }
        }

        if (LootPackageDatas.IsValid())
            LootPackageDatas.Free();

        return true;
    }

    bool CheckLootPackageDataTables(UFortGameData* GameData)
    {
        // [GALAXIA PATCH] 6.21 : la table de GameData (LootPackages_Client) n'existe pas, celle de la playlist suffit
        TArray<UDataTable*> PlaylistLootPackageDatas = GetPlaylistLootPackageDatas();
        const bool bHasPlaylistTable = PlaylistLootPackageDatas.Num() > 0;

        if (PlaylistLootPackageDatas.IsValid())
            PlaylistLootPackageDatas.Free();

        if (bHasPlaylistTable)
            return true;

        TArray<TSoftObjectPtr<UDataTable>> LootPackageDataTables = GameData->LootPackageDataTables;
        TArray<UDataTable*> ValidDataTables;

        for (int32 i = 0; i < LootPackageDataTables.Num(); i++)
        {
            TSoftObjectPtr<UDataTable> LootPackageDataTable = LootPackageDataTables[i];

            UDataTable* DataTable = LoadDataTable(LootPackageDataTable);

            if (DataTable)
                ValidDataTables.Add(DataTable);
        }

        return (ValidDataTables.Num() > 0);
    }

    bool ProcessLootDrops(FFortLootTierData* LootTierData, int32 NumberOfLootDrops, std::map<int32, int32>* LootMaps, __int64 a4)
    {
        if (!LootTierData)
            return false;

        LootMaps->clear();

        int32 NumberOfLootPackageCategoryMin = LootTierData->LootPackageCategoryMinArray.Num();

        if (NumberOfLootPackageCategoryMin != LootTierData->LootPackageCategoryWeightArray.Num() || NumberOfLootPackageCategoryMin != LootTierData->LootPackageCategoryMaxArray.Num())
            return false;

        int32 MinimumOfLootDrops = 0;

        for (int32 i = 0; i < NumberOfLootPackageCategoryMin; i++)
        {
            int32 LootPackageCategoryMin = LootTierData->LootPackageCategoryMinArray[i];

            if (LootPackageCategoryMin > 0)
            {
                LootMaps->emplace(i, LootPackageCategoryMin);
                MinimumOfLootDrops += LootPackageCategoryMin;
            }
        }

        if (MinimumOfLootDrops > NumberOfLootDrops)
        {
            FN_LOG(LogLoot, Warning, "Requested %i loot drops but minimum drops is %i for loot package %s", NumberOfLootDrops, MinimumOfLootDrops, LootTierData->LootPackage.ToString().c_str());
            LootMaps->clear();
            return false;
        }

        int32 TotalCategoryWeight = 0;
        std::map<int32, int32> CategoryWeightMap;

        for (int32 i = 0; i < LootTierData->LootPackageCategoryWeightArray.Num(); i++)
        {
            int32 LootPackageCategoryWeight = LootTierData->LootPackageCategoryWeightArray[i];
            int32 LootPackageCategoryMax = LootTierData->LootPackageCategoryMaxArray[i];

            if (LootPackageCategoryWeight > 0)
            {
                int32 CurrentDropsForCategory = LootMaps->contains(i) ? LootMaps->at(i) : 0;

                if (LootPackageCategoryMax < 0 || CurrentDropsForCategory < LootPackageCategoryMax)
                {
                    TotalCategoryWeight += LootPackageCategoryWeight;
                    CategoryWeightMap.emplace(i, LootPackageCategoryWeight);
                }
            }
        }

        if (MinimumOfLootDrops < NumberOfLootDrops)
        {
            while (TotalCategoryWeight > 0)
            {
                float RandomValue = (float)rand() * 0.000030518509f;

                int32 WeightedRandomSelection = (int32)((TotalCategoryWeight * RandomValue * 2) + 0.5f) >> 1;
                int32 SelectedDropCategory = 0;

                for (auto& [DropCategory, Weight] : CategoryWeightMap)
                {
                    if (WeightedRandomSelection <= Weight)
                    {
                        SelectedDropCategory = DropCategory;
                        break;
                    }

                    WeightedRandomSelection -= Weight;
                }

                int32& NumDropsForCategory = (*LootMaps)[SelectedDropCategory];
                NumDropsForCategory++;
                MinimumOfLootDrops++;

                int32 MaxDropsForCategory = LootTierData->LootPackageCategoryMaxArray[SelectedDropCategory];

                if (MaxDropsForCategory >= 0 && NumDropsForCategory >= MaxDropsForCategory)
                {
                    FN_LOG(LogLoot, Warning, "We have dropped %i items of category %i but the max is %i for tier group %s at tier %i.",
                        NumDropsForCategory, SelectedDropCategory, MaxDropsForCategory, LootTierData->TierGroup.ToString().c_str(), LootTierData->LootTier);

                    CategoryWeightMap.erase(SelectedDropCategory);
                }

                TotalCategoryWeight -= LootTierData->LootPackageCategoryWeightArray[SelectedDropCategory];

                if (MinimumOfLootDrops >= NumberOfLootDrops)
                    break;
            }

            if (MinimumOfLootDrops < NumberOfLootDrops)
            {
                FN_LOG(LogLoot, Warning, "Requested %i loot drops but maximum drops is %i for loot package %s", NumberOfLootDrops, MinimumOfLootDrops, LootTierData->LootPackage.ToString().c_str());
                LootMaps->clear();
                return false;
            }
        }

        return true;
    }

    bool PickLootDrops(TArray<FFortItemEntry>* OutLootToDrops, int32 WorldLevel, FName LootTierKey, __int64 a4, __int64 a5, FGameplayTagContainer StaticGameplayTags, bool bAllowBonusDrops, bool a8)
    {
        if (!LootTierKey.IsValid())
            return false;

        OutLootToDrops->Clear();

        UFortGameData* GameData = Globals::GetGameData();
        FFortLootTierData* LootTierData = FindLootTierDataRow(GameData, LootTierKey, L"UFortLootPackage::PickLootDrops");

        int32 NumberOfLootDrops = 0;

        if (LootTierData && CheckLootPackageDataTables(GameData))
        {
            FName LootPackageName = a8 ? LootTierData->LootPreviewPackage : LootTierData->LootPackage;

            float NumLootPackageDrops = LootTierData->NumLootPackageDrops;

            if (NumLootPackageDrops > 0.0f)
            {
                if (a8 || NumLootPackageDrops < 1.0f)
                {
                    NumberOfLootDrops = 1;
                }
                else
                {
                    NumberOfLootDrops = (int32)((NumLootPackageDrops * 2) - 0.5f) / 2;
                    float RemainingFraction = NumLootPackageDrops - (float)NumberOfLootDrops;

                    if (RemainingFraction > 0.0000099999997f)
                    {
                        if (a5)
                            NumberOfLootDrops += RemainingFraction >= 0.0f; // sub_7FF71A068EB0(a5);
                        else
                            NumberOfLootDrops += RemainingFraction >= (float)rand() * 0.000030518509f;
                    }
                }

                FN_LOG(LogLoot, Debug, "Actual number of loot drops is: %i", NumberOfLootDrops);

                std::map<int32, int32> LootMap;

                if (!a8)
                {
                    ProcessLootDrops(LootTierData, NumberOfLootDrops, &LootMap, a5);
                }

                for (const auto& [LootPackageCategory, DropCount] : LootMap)
                {
                    FN_LOG(LogLoot, Debug, "Dropping %i loot packages from drop category %i", DropCount, LootPackageCategory);

                    if (DropCount > 0)
                    {
                        bool v33 = !a8 && bAllowBonusDrops && LootTierData->bAllowBonusLootDrops;

                        for (int32 i = 0; i < DropCount; ++i)
                        {
                            PickLootDropsFromLootPackage(
                                OutLootToDrops,
                                LootPackageName,
                                WorldLevel,
                                LootPackageCategory,
                                a4,
                                0,
                                StaticGameplayTags,
                                v33);
                        }
                    }
                }
            }

            int32 NumberOfLootToDrops = OutLootToDrops->Num();

            if (NumberOfLootToDrops > 0)
            {
                for (int32 i = 0; i < NumberOfLootToDrops; ++i)
                {
                    FFortItemEntry* ItemEntry = &(*OutLootToDrops)[i];
                    UFortItemDefinition* ItemDefinition = ItemEntry->ItemDefinition;

                    if (!ItemDefinition || ItemDefinition->bAllowMultipleStacks)
                        continue;

                    for (int32 j = i + 1; j < NumberOfLootToDrops; ++j)
                    {
                        FFortItemEntry* NextItemEntry = &(*OutLootToDrops)[j];

                        if (ItemEntry->ItemDefinition != NextItemEntry->ItemDefinition)
                            continue;

                        int32 NewCount = ItemEntry->Count + NextItemEntry->Count;

                        ItemEntry->Count = NewCount;

                        OutLootToDrops->Remove(j--);
                        NumberOfLootToDrops--;
                    }
                }
            }
        }

        return (NumberOfLootDrops > 0);
    }
}