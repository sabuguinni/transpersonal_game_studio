#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "MassEntityTypes.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerTypes.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "SharedTypes.h"
#include "Crowd_MassSimulationSubsystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_EntityConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Entity")
    FVector SpawnLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Entity")
    float MovementSpeed = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Entity")
    float WanderRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Entity")
    EBiomeType BiomeType = EBiomeType::Savana;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Entity")
    bool bIsActive = true;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_BiomeConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType = EBiomeType::Savana;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector BiomeCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float BiomeRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 MaxCrowdEntities = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float EntityDensity = 0.1f;
};

/**
 * Mass Entity-based crowd simulation subsystem for prehistoric world
 * Manages up to 50,000 crowd entities across multiple biomes
 * Integrates with UE5 Mass Entity framework for performance
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UCrowd_MassSimulationSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCrowd_MassSimulationSubsystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Mass Entity management
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void InitializeMassEntitySystem();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnCrowdEntitiesInBiome(EBiomeType BiomeType, int32 EntityCount);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateCrowdSimulation(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetBiomeConfiguration(const TArray<FCrowd_BiomeConfig>& BiomeConfigs);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    int32 GetActiveCrowdEntityCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void EnableCrowdSimulation(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void ClearAllCrowdEntities();

    // Biome-specific crowd management
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void ConfigureSavanaCrowd();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void ConfigureForestCrowd();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void ConfigureDesertCrowd();

protected:
    // Mass Entity system reference
    UPROPERTY()
    class UMassEntitySubsystem* MassEntitySubsystem;

    // Crowd entity configurations
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Configuration")
    TArray<FCrowd_BiomeConfig> BiomeConfigurations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Configuration")
    TArray<FCrowd_EntityConfig> ActiveCrowdEntities;

    // Simulation parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation")
    bool bCrowdSimulationEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation")
    float SimulationUpdateRate = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation")
    int32 MaxTotalCrowdEntities = 50000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation")
    float CrowdEntityLifespan = 300.0f;

private:
    // Internal crowd management
    void CreateMassEntityArchetype();
    void SpawnEntityAtLocation(const FVector& Location, EBiomeType BiomeType);
    void UpdateEntityMovement(float DeltaTime);
    void CleanupExpiredEntities();
    
    // Mass Entity archetype
    FMassArchetypeHandle CrowdArchetype;
    
    // Performance tracking
    float LastUpdateTime = 0.0f;
    int32 CurrentEntityCount = 0;
    
    // Biome coordinates
    FVector GetBiomeCenter(EBiomeType BiomeType) const;
};