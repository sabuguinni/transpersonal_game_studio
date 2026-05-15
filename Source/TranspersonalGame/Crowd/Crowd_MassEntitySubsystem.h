#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MassEntityTypes.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerTypes.h"
#include "MassCommonFragments.h"
#include "Engine/World.h"
#include "Crowd_MassEntitySubsystem.generated.h"

// Forward declarations
class UMassEntitySubsystem;
class AMassSpawner;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_EntityConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config")
    int32 MaxEntities = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config")
    float SpawnRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config")
    float MovementSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config")
    float AvoidanceRadius = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config")
    float CombatFleeDistance = 2000.0f;

    FCrowd_EntityConfig()
    {
        MaxEntities = 1000;
        SpawnRadius = 5000.0f;
        MovementSpeed = 300.0f;
        AvoidanceRadius = 150.0f;
        CombatFleeDistance = 2000.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_BiomeSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector BiomeCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float BiomeRadius = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 TargetPopulation = 500;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName = TEXT("Unknown");

    FCrowd_BiomeSettings()
    {
        BiomeCenter = FVector::ZeroVector;
        BiomeRadius = 10000.0f;
        TargetPopulation = 500;
        BiomeName = TEXT("Unknown");
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UCrowd_MassEntitySubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCrowd_MassEntitySubsystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Mass Entity Management
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void InitializeCrowdSystem();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnCrowdEntitiesInBiome(const FCrowd_BiomeSettings& BiomeSettings);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateCrowdBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void HandleCombatDetection(const FVector& CombatLocation, float CombatRadius);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    int32 GetActiveCrowdEntityCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetCrowdConfiguration(const FCrowd_EntityConfig& NewConfig);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    FCrowd_EntityConfig GetCrowdConfiguration() const;

    // Biome Management
    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void RegisterBiome(const FCrowd_BiomeSettings& BiomeSettings);

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void PopulateAllBiomes();

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    TArray<FCrowd_BiomeSettings> GetRegisteredBiomes() const;

protected:
    // Mass Entity System
    UPROPERTY()
    UMassEntitySubsystem* MassEntitySubsystem;

    // Crowd Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config", meta = (AllowPrivateAccess = "true"))
    FCrowd_EntityConfig CrowdConfig;

    // Biome Registry
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes", meta = (AllowPrivateAccess = "true"))
    TArray<FCrowd_BiomeSettings> RegisteredBiomes;

    // Active Entities
    UPROPERTY()
    TArray<FMassEntityHandle> ActiveCrowdEntities;

    // Spawners
    UPROPERTY()
    TArray<AMassSpawner*> BiomeSpawners;

    // Internal Methods
    void CreateDefaultBiomes();
    void InitializeMassEntityArchetype();
    FMassEntityHandle SpawnCrowdEntity(const FVector& Location, const FCrowd_BiomeSettings& BiomeSettings);
    void UpdateEntityMovement(FMassEntityHandle EntityHandle, float DeltaTime);
    void ProcessCombatAvoidance(FMassEntityHandle EntityHandle, const FVector& CombatLocation, float CombatRadius);

private:
    // System State
    bool bIsInitialized = false;
    float LastUpdateTime = 0.0f;
    
    // Performance Tracking
    int32 MaxEntitiesPerFrame = 100;
    int32 CurrentFrameEntityCount = 0;
};