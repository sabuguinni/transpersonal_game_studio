#include "EnvArt_CretaceousEnvironmentPropLibrary.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"

UEnvArt_CretaceousEnvironmentPropLibrary::UEnvArt_CretaceousEnvironmentPropLibrary()
{
    InitializePropDatabase();
}

void UEnvArt_CretaceousEnvironmentPropLibrary::InitializePropDatabase()
{
    PropDatabase.Empty();

    // Fallen Log Props
    FEnvArt_CretaceousPropData FallenLogData;
    FallenLogData.PropType = EEnvArt_CretaceousPropType::FallenLog;
    FallenLogData.ScaleRange = FVector(0.8f, 1.5f, 1.0f);
    FallenLogData.SpawnWeight = 3.0f;
    FallenLogData.bRequiresGroundAlignment = true;
    FallenLogData.MaxSlopeAngle = 30.0f;
    PropDatabase.Add(FallenLogData);

    // Weathered Boulder Props
    FEnvArt_CretaceousPropData BoulderData;
    BoulderData.PropType = EEnvArt_CretaceousPropType::WeatheredBoulder;
    BoulderData.ScaleRange = FVector(0.7f, 2.0f, 1.0f);
    BoulderData.SpawnWeight = 2.5f;
    BoulderData.bRequiresGroundAlignment = true;
    BoulderData.MaxSlopeAngle = 60.0f;
    PropDatabase.Add(BoulderData);

    // Rock Formation Props
    FEnvArt_CretaceousPropData RockFormationData;
    RockFormationData.PropType = EEnvArt_CretaceousPropType::RockFormation;
    RockFormationData.ScaleRange = FVector(1.0f, 1.8f, 1.0f);
    RockFormationData.SpawnWeight = 1.5f;
    RockFormationData.bRequiresGroundAlignment = true;
    RockFormationData.MaxSlopeAngle = 45.0f;
    PropDatabase.Add(RockFormationData);

    // Dead Tree Props
    FEnvArt_CretaceousPropData DeadTreeData;
    DeadTreeData.PropType = EEnvArt_CretaceousPropType::DeadTree;
    DeadTreeData.ScaleRange = FVector(0.9f, 1.3f, 1.0f);
    DeadTreeData.SpawnWeight = 2.0f;
    DeadTreeData.bRequiresGroundAlignment = true;
    DeadTreeData.MaxSlopeAngle = 25.0f;
    PropDatabase.Add(DeadTreeData);

    // Fern Cluster Props
    FEnvArt_CretaceousPropData FernData;
    FernData.PropType = EEnvArt_CretaceousPropType::FernCluster;
    FernData.ScaleRange = FVector(0.6f, 1.4f, 1.0f);
    FernData.SpawnWeight = 4.0f;
    FernData.bRequiresGroundAlignment = true;
    FernData.MaxSlopeAngle = 35.0f;
    PropDatabase.Add(FernData);

    // Initialize biome prop type mappings
    BiomePropTypes.Empty();
    
    // Forest Biome
    TArray<EEnvArt_CretaceousPropType> ForestProps;
    ForestProps.Add(EEnvArt_CretaceousPropType::FallenLog);
    ForestProps.Add(EEnvArt_CretaceousPropType::DeadTree);
    ForestProps.Add(EEnvArt_CretaceousPropType::FernCluster);
    ForestProps.Add(EEnvArt_CretaceousPropType::MushroomGroup);
    BiomePropTypes.Add(TEXT("CretaceousForest"), ForestProps);

    // Plains Biome
    TArray<EEnvArt_CretaceousPropType> PlainsProps;
    PlainsProps.Add(EEnvArt_CretaceousPropType::WeatheredBoulder);
    PlainsProps.Add(EEnvArt_CretaceousPropType::StoneCluster);
    PlainsProps.Add(EEnvArt_CretaceousPropType::CycadStump);
    BiomePropTypes.Add(TEXT("CretaceousPlains"), PlainsProps);

    // Rocky Biome
    TArray<EEnvArt_CretaceousPropType> RockyProps;
    RockyProps.Add(EEnvArt_CretaceousPropType::RockFormation);
    RockyProps.Add(EEnvArt_CretaceousPropType::WeatheredBoulder);
    RockyProps.Add(EEnvArt_CretaceousPropType::FossilizedWood);
    BiomePropTypes.Add(TEXT("CretaceousRocky"), RockyProps);
}

FEnvArt_CretaceousPropData UEnvArt_CretaceousEnvironmentPropLibrary::GetRandomPropByType(EEnvArt_CretaceousPropType PropType)
{
    TArray<FEnvArt_CretaceousPropData> FilteredProps;
    
    for (const FEnvArt_CretaceousPropData& PropData : PropDatabase)
    {
        if (PropData.PropType == PropType)
        {
            FilteredProps.Add(PropData);
        }
    }

    if (FilteredProps.Num() > 0)
    {
        int32 RandomIndex = FMath::RandRange(0, FilteredProps.Num() - 1);
        return FilteredProps[RandomIndex];
    }

    // Return default if none found
    return FEnvArt_CretaceousPropData();
}

FEnvArt_CretaceousPropData UEnvArt_CretaceousEnvironmentPropLibrary::GetWeightedRandomProp()
{
    if (PropDatabase.Num() == 0)
    {
        return FEnvArt_CretaceousPropData();
    }

    float TotalWeight = 0.0f;
    for (const FEnvArt_CretaceousPropData& PropData : PropDatabase)
    {
        TotalWeight += PropData.SpawnWeight;
    }

    float RandomValue = FMath::FRandRange(0.0f, TotalWeight);
    float CurrentWeight = 0.0f;

    for (const FEnvArt_CretaceousPropData& PropData : PropDatabase)
    {
        CurrentWeight += PropData.SpawnWeight;
        if (RandomValue <= CurrentWeight)
        {
            return PropData;
        }
    }

    // Fallback to first prop
    return PropDatabase[0];
}

TArray<FEnvArt_CretaceousPropData> UEnvArt_CretaceousEnvironmentPropLibrary::GetPropsForBiome(const FString& BiomeName)
{
    TArray<FEnvArt_CretaceousPropData> BiomeProps;

    if (BiomePropTypes.Contains(BiomeName))
    {
        const TArray<EEnvArt_CretaceousPropType>& PropTypes = BiomePropTypes[BiomeName];
        
        for (EEnvArt_CretaceousPropType PropType : PropTypes)
        {
            for (const FEnvArt_CretaceousPropData& PropData : PropDatabase)
            {
                if (PropData.PropType == PropType)
                {
                    BiomeProps.Add(PropData);
                }
            }
        }
    }

    return BiomeProps;
}

bool UEnvArt_CretaceousEnvironmentPropLibrary::ValidateSpawnLocation(const FVector& Location, const FEnvArt_CretaceousPropData& PropData)
{
    // Basic validation - can be extended with line traces for ground detection
    if (!PropData.bRequiresGroundAlignment)
    {
        return true;
    }

    // For now, assume location is valid
    // In a full implementation, this would perform line traces to check ground slope
    return true;
}

FVector UEnvArt_CretaceousEnvironmentPropLibrary::CalculateAlignedRotation(const FVector& SurfaceNormal, const FEnvArt_CretaceousPropData& PropData)
{
    if (!PropData.bRequiresGroundAlignment)
    {
        return FVector::ZeroVector;
    }

    // Calculate rotation to align with surface normal
    FVector UpVector = FVector::UpVector;
    FVector ForwardVector = FVector::CrossProduct(SurfaceNormal, UpVector);
    ForwardVector.Normalize();

    FRotator AlignedRotation = UKismetMathLibrary::MakeRotFromZX(SurfaceNormal, ForwardVector);
    
    // Add random yaw rotation for natural variation
    AlignedRotation.Yaw += FMath::FRandRange(-180.0f, 180.0f);
    
    return AlignedRotation.Euler();
}