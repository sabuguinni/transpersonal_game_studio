#include "EnvArt_CretaceousPropAssetLibrary.h"
#include "Engine/Engine.h"

UEnvArt_CretaceousPropAssetLibrary::UEnvArt_CretaceousPropAssetLibrary()
{
    InitializeDefaultAssets();
}

void UEnvArt_CretaceousPropAssetLibrary::InitializeDefaultAssets()
{
    // Initialize Fallen Log Assets
    FEnvArt_CretaceousPropData LargeFallenLog;
    LargeFallenLog.PropType = EEnvArt_CretaceousPropType::FallenLog;
    LargeFallenLog.DefaultScale = FVector(2.5f, 1.0f, 1.0f);
    LargeFallenLog.SpawnWeight = 0.3f;
    LargeFallenLog.PropDescription = TEXT("Large weathered fallen log with moss coverage");
    FallenLogAssets.Add(LargeFallenLog);

    FEnvArt_CretaceousPropData MediumFallenLog;
    MediumFallenLog.PropType = EEnvArt_CretaceousPropType::FallenLog;
    MediumFallenLog.DefaultScale = FVector(1.8f, 0.8f, 0.8f);
    MediumFallenLog.SpawnWeight = 0.5f;
    MediumFallenLog.PropDescription = TEXT("Medium fallen log partially buried");
    FallenLogAssets.Add(MediumFallenLog);

    FEnvArt_CretaceousPropData SmallFallenLog;
    SmallFallenLog.PropType = EEnvArt_CretaceousPropType::FallenLog;
    SmallFallenLog.DefaultScale = FVector(1.2f, 0.6f, 0.6f);
    SmallFallenLog.SpawnWeight = 0.7f;
    SmallFallenLog.PropDescription = TEXT("Small fallen branch with bark texture");
    FallenLogAssets.Add(SmallFallenLog);

    // Initialize Boulder Assets
    FEnvArt_CretaceousPropData LargeBoulder;
    LargeBoulder.PropType = EEnvArt_CretaceousPropType::Boulder;
    LargeBoulder.DefaultScale = FVector(2.0f, 2.0f, 1.5f);
    LargeBoulder.SpawnWeight = 0.2f;
    LargeBoulder.PropDescription = TEXT("Large weathered boulder with lichen");
    BoulderAssets.Add(LargeBoulder);

    FEnvArt_CretaceousPropData MediumBoulder;
    MediumBoulder.PropType = EEnvArt_CretaceousPropType::Boulder;
    MediumBoulder.DefaultScale = FVector(1.5f, 1.5f, 1.2f);
    MediumBoulder.SpawnWeight = 0.4f;
    MediumBoulder.PropDescription = TEXT("Medium sized boulder cluster");
    BoulderAssets.Add(MediumBoulder);

    // Initialize Rock Formation Assets
    FEnvArt_CretaceousPropData RockOutcrop;
    RockOutcrop.PropType = EEnvArt_CretaceousPropType::RockFormation;
    RockOutcrop.DefaultScale = FVector(3.0f, 2.5f, 2.0f);
    RockOutcrop.SpawnWeight = 0.15f;
    RockOutcrop.PropDescription = TEXT("Large rock outcrop formation");
    RockFormationAssets.Add(RockOutcrop);

    // Initialize Dead Tree Assets
    FEnvArt_CretaceousPropData DeadTreeTrunk;
    DeadTreeTrunk.PropType = EEnvArt_CretaceousPropType::DeadTree;
    DeadTreeTrunk.DefaultScale = FVector(1.0f, 1.0f, 2.5f);
    DeadTreeTrunk.SpawnWeight = 0.25f;
    DeadTreeTrunk.PropDescription = TEXT("Standing dead tree trunk");
    DeadTreeAssets.Add(DeadTreeTrunk);

    // Initialize Small Props
    FEnvArt_CretaceousPropData FernCluster;
    FernCluster.PropType = EEnvArt_CretaceousPropType::Fern;
    FernCluster.DefaultScale = FVector(0.8f, 0.8f, 0.6f);
    FernCluster.SpawnWeight = 1.0f;
    FernCluster.PropDescription = TEXT("Prehistoric fern cluster");
    SmallPropAssets.Add(FernCluster);

    FEnvArt_CretaceousPropData MushroomCluster;
    MushroomCluster.PropType = EEnvArt_CretaceousPropType::Mushroom;
    MushroomCluster.DefaultScale = FVector(0.5f, 0.5f, 0.4f);
    MushroomCluster.SpawnWeight = 0.8f;
    MushroomCluster.PropDescription = TEXT("Ancient mushroom cluster on log");
    SmallPropAssets.Add(MushroomCluster);
}

FEnvArt_CretaceousPropData UEnvArt_CretaceousPropAssetLibrary::GetRandomPropOfType(EEnvArt_CretaceousPropType PropType) const
{
    TArray<FEnvArt_CretaceousPropData> PropsOfType = GetAllPropsOfType(PropType);
    
    if (PropsOfType.Num() == 0)
    {
        return FEnvArt_CretaceousPropData();
    }

    // Weighted random selection
    float TotalWeight = 0.0f;
    for (const FEnvArt_CretaceousPropData& PropData : PropsOfType)
    {
        TotalWeight += PropData.SpawnWeight;
    }

    float RandomValue = FMath::FRandRange(0.0f, TotalWeight);
    float CurrentWeight = 0.0f;

    for (const FEnvArt_CretaceousPropData& PropData : PropsOfType)
    {
        CurrentWeight += PropData.SpawnWeight;
        if (RandomValue <= CurrentWeight)
        {
            return PropData;
        }
    }

    return PropsOfType[0];
}

TArray<FEnvArt_CretaceousPropData> UEnvArt_CretaceousPropAssetLibrary::GetAllPropsOfType(EEnvArt_CretaceousPropType PropType) const
{
    switch (PropType)
    {
        case EEnvArt_CretaceousPropType::FallenLog:
            return FallenLogAssets;
        case EEnvArt_CretaceousPropType::Boulder:
            return BoulderAssets;
        case EEnvArt_CretaceousPropType::RockFormation:
            return RockFormationAssets;
        case EEnvArt_CretaceousPropType::DeadTree:
            return DeadTreeAssets;
        case EEnvArt_CretaceousPropType::Fern:
        case EEnvArt_CretaceousPropType::Mushroom:
        case EEnvArt_CretaceousPropType::BrokenBranch:
        case EEnvArt_CretaceousPropType::StoneCluster:
            return SmallPropAssets;
        default:
            return TArray<FEnvArt_CretaceousPropData>();
    }
}

int32 UEnvArt_CretaceousPropAssetLibrary::GetPropCountOfType(EEnvArt_CretaceousPropType PropType) const
{
    return GetAllPropsOfType(PropType).Num();
}