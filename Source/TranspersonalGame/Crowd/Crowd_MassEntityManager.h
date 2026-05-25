#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MassEntityTypes.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerTypes.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassRepresentationFragments.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "Crowd_MassEntityManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_EntitySpawnData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    EBiomeType BiomeType = EBiomeType::Savana;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    float MovementSpeed = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    float WanderRadius = 1000.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_BiomeCrowdConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType = EBiomeType::Savana;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 MaxEntities = 10000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float SpawnRadius = 50000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector BiomeCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<UStaticMesh*> CrowdMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float LODDistance1 = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float LODDistance2 = 15000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float CullDistance = 30000.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCrowd_MassEntityManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCrowd_MassEntityManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Mass Entity Management
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void InitializeMassEntitySystem();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnCrowdEntitiesInBiome(EBiomeType BiomeType, int32 EntityCount);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnCrowdEntity(const FCrowd_EntitySpawnData& SpawnData);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void ClearAllCrowdEntities();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetBiomeCrowdConfig(const FCrowd_BiomeCrowdConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    int32 GetTotalEntityCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    int32 GetBiomeEntityCount(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateCrowdLOD(const FVector& ViewerLocation);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetCrowdDensityMultiplier(float Multiplier);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation", CallInEditor = true)
    void DebugSpawnTestCrowd();

protected:
    // Mass Entity System
    UPROPERTY()
    class UMassEntitySubsystem* MassEntitySubsystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config")
    TArray<FCrowd_BiomeCrowdConfig> BiomeConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config")
    float CrowdDensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config")
    int32 MaxTotalEntities = 50000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config")
    bool bEnableLODSystem = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config")
    float LODUpdateFrequency = 0.5f;

    // Entity tracking
    UPROPERTY()
    TMap<EBiomeType, TArray<FMassEntityHandle>> BiomeEntities;

    UPROPERTY()
    TArray<FMassEntityHandle> AllCrowdEntities;

    // Internal methods
    void InitializeBiomeConfigs();
    FVector GetBiomeSpawnLocation(EBiomeType BiomeType, float Radius) const;
    void SetupEntityFragments(FMassEntityHandle EntityHandle, const FCrowd_EntitySpawnData& SpawnData);
    void UpdateEntityLOD(FMassEntityHandle EntityHandle, float DistanceToViewer);

    // Timer handles
    FTimerHandle LODUpdateTimer;
    void OnLODUpdateTick();
};