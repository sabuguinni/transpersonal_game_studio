#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MassEntityTypes.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerTypes.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassRepresentationFragments.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "SharedTypes.h"
#include "Crowd_MassEntityManager.generated.h"

USTRUCT(BlueprintType)
struct FCrowd_EntitySpawnData
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
    int32 GroupSize = 5;
};

USTRUCT(BlueprintType)
struct FCrowd_EntityGroup
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
    TArray<FMassEntityHandle> Entities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
    FVector GroupCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
    float GroupRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
    EBiomeType BiomeType = EBiomeType::Savana;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
    bool bIsActive = true;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCrowd_MassEntityManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UCrowd_MassEntityManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Mass Entity Management
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnCrowdGroup(const FCrowd_EntitySpawnData& SpawnData);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnCrowdInBiome(EBiomeType BiomeType, int32 EntityCount = 100);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void DespawnAllEntities();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateCrowdMovement(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    int32 GetActiveEntityCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetCrowdDensity(float Density);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetMovementSpeed(float Speed);

    // LOD Management
    UFUNCTION(BlueprintCallable, Category = "LOD")
    void UpdateLODForDistance(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "LOD")
    void SetLODDistances(float HighLOD, float MediumLOD, float LowLOD);

    // Debug Functions
    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void DebugSpawnTestGroup();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void PrintCrowdStats();

protected:
    // Mass Entity System
    UPROPERTY()
    class UMassEntitySubsystem* MassEntitySubsystem;

    UPROPERTY()
    TArray<FCrowd_EntityGroup> CrowdGroups;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    int32 MaxEntities = 50000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float CrowdDensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float DefaultMovementSpeed = 150.0f;

    // LOD Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float HighLODDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float MediumLODDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LowLODDistance = 15000.0f;

    // Biome Spawn Locations
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TMap<EBiomeType, FVector> BiomeSpawnCenters;

private:
    void InitializeBiomeSpawnCenters();
    void CreateEntityArchetype();
    FVector GetRandomLocationInBiome(EBiomeType BiomeType, float Radius = 10000.0f);
    void ApplyLODToEntity(FMassEntityHandle Entity, float Distance);
    
    // Entity tracking
    TArray<FMassEntityHandle> AllSpawnedEntities;
    int32 CurrentEntityCount = 0;
};