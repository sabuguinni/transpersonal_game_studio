#include "EnvArt_CretaceousTerrainProps.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

AEnvArt_CretaceousTerrainProps::AEnvArt_CretaceousTerrainProps()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize default settings
    MinDistanceBetweenProps = 500.0f;
    MaxDistanceBetweenProps = 2000.0f;
    MaxPropsPerBiome = 50;
    bUseRandomSeed = true;
    RandomSeed = 12345;

    // Initialize default prop configurations
    FEnvArt_TerrainPropData DefaultLogProp;
    DefaultLogProp.PropName = TEXT("FallenLog");
    DefaultLogProp.BaseScale = FVector(3.0f, 3.0f, 0.8f);
    DefaultLogProp.ScaleVariation = 0.4f;
    DefaultLogProp.RotationVariation = 360.0f;
    DefaultLogProp.MaxInstancesPerBiome = 25;
    DefaultLogProp.SpawnProbability = 0.8f;
    PropConfigurations.Add(DefaultLogProp);

    FEnvArt_TerrainPropData DefaultRockProp;
    DefaultRockProp.PropName = TEXT("WeatheredRock");
    DefaultRockProp.BaseScale = FVector(2.0f, 2.0f, 1.4f);
    DefaultRockProp.ScaleVariation = 0.6f;
    DefaultRockProp.RotationVariation = 360.0f;
    DefaultRockProp.MaxInstancesPerBiome = 30;
    DefaultRockProp.SpawnProbability = 0.7f;
    PropConfigurations.Add(DefaultRockProp);

    // Initialize default fallen log configurations
    for (int32 i = 0; i < 5; ++i)
    {
        FEnvArt_FallenLogData LogData;
        LogData.LogLength = FMath::RandRange(600.0f, 1200.0f);
        LogData.LogDiameter = FMath::RandRange(80.0f, 160.0f);
        LogData.DecayLevel = FMath::RandRange(0.2f, 0.8f);
        LogData.bHasMoss = FMath::RandBool();
        LogData.bHasFerns = FMath::RandBool();
        FallenLogConfigurations.Add(LogData);
    }

    // Initialize default weathered rock configurations
    for (int32 i = 0; i < 5; ++i)
    {
        FEnvArt_WeatheredRockData RockData;
        RockData.RockSize = FMath::RandRange(150.0f, 400.0f);
        RockData.WeatheringLevel = FMath::RandRange(0.3f, 0.9f);
        RockData.bHasLichens = FMath::RandBool();
        RockData.bHasCracks = FMath::RandBool();
        RockData.RockColor = FLinearColor(
            FMath::RandRange(0.3f, 0.5f),
            FMath::RandRange(0.25f, 0.4f),
            FMath::RandRange(0.2f, 0.35f),
            1.0f
        );
        WeatheredRockConfigurations.Add(RockData);
    }
}

void AEnvArt_CretaceousTerrainProps::BeginPlay()
{
    Super::BeginPlay();

    if (bUseRandomSeed)
    {
        FMath::RandInit(RandomSeed);
    }

    // Validate initial configuration
    ValidatePropCounts();
}

void AEnvArt_CretaceousTerrainProps::SpawnFallenLogsInBiome(EBiomeType BiomeType, int32 Count)
{
    if (Count <= 0 || FallenLogConfigurations.Num() == 0)
    {
        return;
    }

    int32 CurrentLogsInBiome = 0;
    for (AActor* LogActor : SpawnedFallenLogs)
    {
        if (IsValid(LogActor))
        {
            CurrentLogsInBiome++;
        }
    }

    if (CurrentLogsInBiome >= MaxPropsPerBiome)
    {
        UE_LOG(LogTemp, Warning, TEXT("Maximum fallen logs reached for biome"));
        return;
    }

    int32 ActualSpawnCount = FMath::Min(Count, MaxPropsPerBiome - CurrentLogsInBiome);

    for (int32 i = 0; i < ActualSpawnCount; ++i)
    {
        FVector SpawnLocation = GetRandomLocationInBiome(BiomeType);
        if (IsValidPropLocation(SpawnLocation, MinDistanceBetweenProps))
        {
            int32 ConfigIndex = FMath::RandRange(0, FallenLogConfigurations.Num() - 1);
            FEnvArt_FallenLogData LogData = FallenLogConfigurations[ConfigIndex];
            LogData.SpawnLocation = SpawnLocation;

            AActor* NewLog = SpawnFallenLogActor(LogData);
            if (IsValid(NewLog))
            {
                SpawnedFallenLogs.Add(NewLog);
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Spawned %d fallen logs in biome %d"), ActualSpawnCount, (int32)BiomeType);
}

void AEnvArt_CretaceousTerrainProps::SpawnWeatheredRocksInBiome(EBiomeType BiomeType, int32 Count)
{
    if (Count <= 0 || WeatheredRockConfigurations.Num() == 0)
    {
        return;
    }

    int32 CurrentRocksInBiome = 0;
    for (AActor* RockActor : SpawnedWeatheredRocks)
    {
        if (IsValid(RockActor))
        {
            CurrentRocksInBiome++;
        }
    }

    if (CurrentRocksInBiome >= MaxPropsPerBiome)
    {
        UE_LOG(LogTemp, Warning, TEXT("Maximum weathered rocks reached for biome"));
        return;
    }

    int32 ActualSpawnCount = FMath::Min(Count, MaxPropsPerBiome - CurrentRocksInBiome);

    for (int32 i = 0; i < ActualSpawnCount; ++i)
    {
        FVector SpawnLocation = GetRandomLocationInBiome(BiomeType);
        if (IsValidPropLocation(SpawnLocation, MinDistanceBetweenProps))
        {
            int32 ConfigIndex = FMath::RandRange(0, WeatheredRockConfigurations.Num() - 1);
            FEnvArt_WeatheredRockData RockData = WeatheredRockConfigurations[ConfigIndex];
            RockData.SpawnLocation = SpawnLocation;

            AActor* NewRock = SpawnWeatheredRockActor(RockData);
            if (IsValid(NewRock))
            {
                SpawnedWeatheredRocks.Add(NewRock);
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Spawned %d weathered rocks in biome %d"), ActualSpawnCount, (int32)BiomeType);
}

void AEnvArt_CretaceousTerrainProps::SpawnGenericPropsInBiome(EBiomeType BiomeType, const FString& PropType, int32 Count)
{
    FEnvArt_TerrainPropData* PropConfig = PropConfigurations.FindByPredicate([&PropType](const FEnvArt_TerrainPropData& Config)
    {
        return Config.PropName == PropType;
    });

    if (!PropConfig || Count <= 0)
    {
        return;
    }

    int32 CurrentGenericPropsInBiome = 0;
    for (AActor* PropActor : SpawnedGenericProps)
    {
        if (IsValid(PropActor))
        {
            CurrentGenericPropsInBiome++;
        }
    }

    if (CurrentGenericPropsInBiome >= MaxPropsPerBiome)
    {
        UE_LOG(LogTemp, Warning, TEXT("Maximum generic props reached for biome"));
        return;
    }

    int32 ActualSpawnCount = FMath::Min(Count, MaxPropsPerBiome - CurrentGenericPropsInBiome);

    for (int32 i = 0; i < ActualSpawnCount; ++i)
    {
        FVector SpawnLocation = GetRandomLocationInBiome(BiomeType);
        if (IsValidPropLocation(SpawnLocation, MinDistanceBetweenProps))
        {
            AActor* NewProp = SpawnGenericPropActor(*PropConfig, SpawnLocation);
            if (IsValid(NewProp))
            {
                SpawnedGenericProps.Add(NewProp);
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Spawned %d %s props in biome %d"), ActualSpawnCount, *PropType, (int32)BiomeType);
}

void AEnvArt_CretaceousTerrainProps::ClearAllPropsInBiome(EBiomeType BiomeType)
{
    // Clear fallen logs
    for (int32 i = SpawnedFallenLogs.Num() - 1; i >= 0; --i)
    {
        if (IsValid(SpawnedFallenLogs[i]))
        {
            SpawnedFallenLogs[i]->Destroy();
        }
        SpawnedFallenLogs.RemoveAt(i);
    }

    // Clear weathered rocks
    for (int32 i = SpawnedWeatheredRocks.Num() - 1; i >= 0; --i)
    {
        if (IsValid(SpawnedWeatheredRocks[i]))
        {
            SpawnedWeatheredRocks[i]->Destroy();
        }
        SpawnedWeatheredRocks.RemoveAt(i);
    }

    // Clear generic props
    for (int32 i = SpawnedGenericProps.Num() - 1; i >= 0; --i)
    {
        if (IsValid(SpawnedGenericProps[i]))
        {
            SpawnedGenericProps[i]->Destroy();
        }
        SpawnedGenericProps.RemoveAt(i);
    }

    UE_LOG(LogTemp, Log, TEXT("Cleared all props in biome %d"), (int32)BiomeType);
}

void AEnvArt_CretaceousTerrainProps::RegeneratePropsForBiome(EBiomeType BiomeType)
{
    ClearAllPropsInBiome(BiomeType);

    // Respawn with default counts
    SpawnFallenLogsInBiome(BiomeType, 15);
    SpawnWeatheredRocksInBiome(BiomeType, 20);
    SpawnGenericPropsInBiome(BiomeType, TEXT("FallenLog"), 10);
}

AActor* AEnvArt_CretaceousTerrainProps::SpawnFallenLogActor(const FEnvArt_FallenLogData& LogData)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = nullptr;

    FRotator SpawnRotation = FRotator(
        FMath::RandRange(-15.0f, 15.0f),
        FMath::RandRange(0.0f, 360.0f),
        FMath::RandRange(-10.0f, 10.0f)
    );

    AStaticMeshActor* LogActor = World->SpawnActor<AStaticMeshActor>(
        AStaticMeshActor::StaticClass(),
        LogData.SpawnLocation,
        SpawnRotation,
        SpawnParams
    );

    if (LogActor)
    {
        LogActor->SetActorLabel(FString::Printf(TEXT("CretaceousFallenLog_%d"), SpawnedFallenLogs.Num() + 1));

        UStaticMeshComponent* MeshComp = LogActor->GetStaticMeshComponent();
        if (MeshComp)
        {
            // Use cylinder mesh as placeholder
            UStaticMesh* CylinderMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder"));
            if (CylinderMesh)
            {
                MeshComp->SetStaticMesh(CylinderMesh);
                
                float ScaleVariation = FMath::RandRange(0.7f, 1.3f);
                FVector LogScale = FVector(
                    LogData.LogLength / 100.0f * ScaleVariation,
                    LogData.LogDiameter / 100.0f * ScaleVariation,
                    LogData.LogDiameter / 100.0f * ScaleVariation
                );
                MeshComp->SetWorldScale3D(LogScale);
            }
        }
    }

    return LogActor;
}

AActor* AEnvArt_CretaceousTerrainProps::SpawnWeatheredRockActor(const FEnvArt_WeatheredRockData& RockData)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = nullptr;

    FRotator SpawnRotation = FRotator(
        FMath::RandRange(-5.0f, 5.0f),
        FMath::RandRange(0.0f, 360.0f),
        0.0f
    );

    AStaticMeshActor* RockActor = World->SpawnActor<AStaticMeshActor>(
        AStaticMeshActor::StaticClass(),
        RockData.SpawnLocation,
        SpawnRotation,
        SpawnParams
    );

    if (RockActor)
    {
        RockActor->SetActorLabel(FString::Printf(TEXT("CretaceousWeatheredRock_%d"), SpawnedWeatheredRocks.Num() + 1));

        UStaticMeshComponent* MeshComp = RockActor->GetStaticMeshComponent();
        if (MeshComp)
        {
            // Use cube mesh as placeholder
            UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube"));
            if (CubeMesh)
            {
                MeshComp->SetStaticMesh(CubeMesh);
                
                float ScaleVariation = FMath::RandRange(0.8f, 1.4f);
                FVector RockScale = FVector(
                    RockData.RockSize / 100.0f * ScaleVariation,
                    RockData.RockSize / 100.0f * ScaleVariation,
                    RockData.RockSize / 100.0f * ScaleVariation * 0.7f
                );
                MeshComp->SetWorldScale3D(RockScale);
            }
        }
    }

    return RockActor;
}

AActor* AEnvArt_CretaceousTerrainProps::SpawnGenericPropActor(const FEnvArt_TerrainPropData& PropData, const FVector& Location)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = nullptr;

    FRotator SpawnRotation = FRotator(
        0.0f,
        FMath::RandRange(0.0f, PropData.RotationVariation),
        0.0f
    );

    AStaticMeshActor* PropActor = World->SpawnActor<AStaticMeshActor>(
        AStaticMeshActor::StaticClass(),
        Location,
        SpawnRotation,
        SpawnParams
    );

    if (PropActor)
    {
        PropActor->SetActorLabel(FString::Printf(TEXT("CretaceousProp_%s_%d"), *PropData.PropName, SpawnedGenericProps.Num() + 1));

        UStaticMeshComponent* MeshComp = PropActor->GetStaticMeshComponent();
        if (MeshComp)
        {
            // Use sphere mesh as placeholder for generic props
            UStaticMesh* SphereMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere"));
            if (SphereMesh)
            {
                MeshComp->SetStaticMesh(SphereMesh);
                
                float ScaleVariation = FMath::RandRange(1.0f - PropData.ScaleVariation, 1.0f + PropData.ScaleVariation);
                FVector PropScale = PropData.BaseScale * ScaleVariation;
                MeshComp->SetWorldScale3D(PropScale);
            }
        }
    }

    return PropActor;
}

FVector AEnvArt_CretaceousTerrainProps::GetRandomLocationInBiome(EBiomeType BiomeType) const
{
    FVector BiomeBounds = GetBiomeBounds(BiomeType);
    
    float X = FMath::RandRange(BiomeBounds.X - 10000.0f, BiomeBounds.X + 10000.0f);
    float Y = FMath::RandRange(BiomeBounds.Y - 10000.0f, BiomeBounds.Y + 10000.0f);
    float Z = BiomeBounds.Z;

    return FVector(X, Y, Z);
}

bool AEnvArt_CretaceousTerrainProps::IsValidPropLocation(const FVector& Location, float MinDistance) const
{
    // Check distance from existing props
    for (const AActor* ExistingProp : SpawnedFallenLogs)
    {
        if (IsValid(ExistingProp))
        {
            float Distance = FVector::Dist(Location, ExistingProp->GetActorLocation());
            if (Distance < MinDistance)
            {
                return false;
            }
        }
    }

    for (const AActor* ExistingProp : SpawnedWeatheredRocks)
    {
        if (IsValid(ExistingProp))
        {
            float Distance = FVector::Dist(Location, ExistingProp->GetActorLocation());
            if (Distance < MinDistance)
            {
                return false;
            }
        }
    }

    for (const AActor* ExistingProp : SpawnedGenericProps)
    {
        if (IsValid(ExistingProp))
        {
            float Distance = FVector::Dist(Location, ExistingProp->GetActorLocation());
            if (Distance < MinDistance)
            {
                return false;
            }
        }
    }

    return true;
}

int32 AEnvArt_CretaceousTerrainProps::GetTotalPropsInBiome(EBiomeType BiomeType) const
{
    int32 TotalProps = 0;
    
    // Count valid props
    for (const AActor* Prop : SpawnedFallenLogs)
    {
        if (IsValid(Prop))
        {
            TotalProps++;
        }
    }

    for (const AActor* Prop : SpawnedWeatheredRocks)
    {
        if (IsValid(Prop))
        {
            TotalProps++;
        }
    }

    for (const AActor* Prop : SpawnedGenericProps)
    {
        if (IsValid(Prop))
        {
            TotalProps++;
        }
    }

    return TotalProps;
}

void AEnvArt_CretaceousTerrainProps::ValidatePropCounts()
{
    CleanupInvalidProps();
    
    int32 TotalProps = GetTotalPropsInBiome(EBiomeType::Forest);
    if (TotalProps > MaxPropsPerBiome * 5) // 5 biomes
    {
        UE_LOG(LogTemp, Warning, TEXT("Total props (%d) exceeds maximum allowed"), TotalProps);
        ReducePropDensity(0.8f);
    }
}

void AEnvArt_CretaceousTerrainProps::CleanupInvalidProps()
{
    // Remove invalid fallen logs
    for (int32 i = SpawnedFallenLogs.Num() - 1; i >= 0; --i)
    {
        if (!IsValid(SpawnedFallenLogs[i]))
        {
            SpawnedFallenLogs.RemoveAt(i);
        }
    }

    // Remove invalid weathered rocks
    for (int32 i = SpawnedWeatheredRocks.Num() - 1; i >= 0; --i)
    {
        if (!IsValid(SpawnedWeatheredRocks[i]))
        {
            SpawnedWeatheredRocks.RemoveAt(i);
        }
    }

    // Remove invalid generic props
    for (int32 i = SpawnedGenericProps.Num() - 1; i >= 0; --i)
    {
        if (!IsValid(SpawnedGenericProps[i]))
        {
            SpawnedGenericProps.RemoveAt(i);
        }
    }
}

FVector AEnvArt_CretaceousTerrainProps::GetBiomeBounds(EBiomeType BiomeType) const
{
    switch (BiomeType)
    {
        case EBiomeType::Forest:
            return FVector(20000.0f, 30000.0f, 100.0f);
        case EBiomeType::Plains:
            return FVector(40000.0f, 40000.0f, 120.0f);
        case EBiomeType::Mountains:
            return FVector(60000.0f, 20000.0f, 200.0f);
        case EBiomeType::Wetlands:
            return FVector(30000.0f, 60000.0f, 80.0f);
        case EBiomeType::Desert:
            return FVector(50000.0f, 50000.0f, 150.0f);
        default:
            return FVector(30000.0f, 30000.0f, 100.0f);
    }
}

void AEnvArt_CretaceousTerrainProps::SetPropConfiguration(const FString& PropName, const FEnvArt_TerrainPropData& Configuration)
{
    FEnvArt_TerrainPropData* ExistingConfig = PropConfigurations.FindByPredicate([&PropName](const FEnvArt_TerrainPropData& Config)
    {
        return Config.PropName == PropName;
    });

    if (ExistingConfig)
    {
        *ExistingConfig = Configuration;
    }
    else
    {
        PropConfigurations.Add(Configuration);
    }
}

FEnvArt_TerrainPropData AEnvArt_CretaceousTerrainProps::GetPropConfiguration(const FString& PropName) const
{
    const FEnvArt_TerrainPropData* Config = PropConfigurations.FindByPredicate([&PropName](const FEnvArt_TerrainPropData& Config)
    {
        return Config.PropName == PropName;
    });

    return Config ? *Config : FEnvArt_TerrainPropData();
}

void AEnvArt_CretaceousTerrainProps::OptimizePropDistribution()
{
    // Remove props that are too close together
    TArray<AActor*> AllProps;
    AllProps.Append(SpawnedFallenLogs);
    AllProps.Append(SpawnedWeatheredRocks);
    AllProps.Append(SpawnedGenericProps);

    for (int32 i = AllProps.Num() - 1; i >= 0; --i)
    {
        if (!IsValid(AllProps[i]))
        {
            continue;
        }

        for (int32 j = i - 1; j >= 0; --j)
        {
            if (!IsValid(AllProps[j]))
            {
                continue;
            }

            float Distance = FVector::Dist(AllProps[i]->GetActorLocation(), AllProps[j]->GetActorLocation());
            if (Distance < MinDistanceBetweenProps * 0.5f)
            {
                // Remove the prop with lower priority (generic props first)
                if (SpawnedGenericProps.Contains(AllProps[i]))
                {
                    AllProps[i]->Destroy();
                    SpawnedGenericProps.Remove(AllProps[i]);
                    break;
                }
                else if (SpawnedGenericProps.Contains(AllProps[j]))
                {
                    AllProps[j]->Destroy();
                    SpawnedGenericProps.Remove(AllProps[j]);
                }
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Optimized prop distribution"));
}

void AEnvArt_CretaceousTerrainProps::CheckPropPerformance()
{
    int32 TotalProps = GetTotalPropsInBiome(EBiomeType::Forest);
    
    UE_LOG(LogTemp, Log, TEXT("Performance Check - Total Props: %d"), TotalProps);
    UE_LOG(LogTemp, Log, TEXT("Fallen Logs: %d"), SpawnedFallenLogs.Num());
    UE_LOG(LogTemp, Log, TEXT("Weathered Rocks: %d"), SpawnedWeatheredRocks.Num());
    UE_LOG(LogTemp, Log, TEXT("Generic Props: %d"), SpawnedGenericProps.Num());

    if (TotalProps > MaxPropsPerBiome * 4)
    {
        UE_LOG(LogTemp, Warning, TEXT("Performance warning: High prop count detected"));
    }
}

void AEnvArt_CretaceousTerrainProps::ReducePropDensity(float ReductionFactor)
{
    int32 PropsToRemove = FMath::RoundToInt((1.0f - ReductionFactor) * SpawnedGenericProps.Num());
    
    for (int32 i = 0; i < PropsToRemove && SpawnedGenericProps.Num() > 0; ++i)
    {
        int32 IndexToRemove = FMath::RandRange(0, SpawnedGenericProps.Num() - 1);
        if (IsValid(SpawnedGenericProps[IndexToRemove]))
        {
            SpawnedGenericProps[IndexToRemove]->Destroy();
        }
        SpawnedGenericProps.RemoveAt(IndexToRemove);
    }

    UE_LOG(LogTemp, Log, TEXT("Reduced prop density by removing %d props"), PropsToRemove);
}