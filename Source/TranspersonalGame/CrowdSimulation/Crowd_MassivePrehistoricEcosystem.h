#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MassEntityTypes.h"
#include "MassProcessor.h"
#include "MassEntitySubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/TriggerVolume.h"
#include "../SharedTypes.h"
#include "Crowd_MassivePrehistoricEcosystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_PrehistoricSpeciesData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    FString SpeciesName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    int32 PopulationSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    float TerritoryRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    bool bIsHerbivore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    float AggressionLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    TArray<FVector> PreferredLocations;

    FCrowd_PrehistoricSpeciesData()
    {
        SpeciesName = TEXT("Unknown");
        PopulationSize = 100;
        TerritoryRadius = 1000.0f;
        bIsHerbivore = true;
        AggressionLevel = 0.3f;
        PreferredLocations.Empty();
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_EcosystemState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    int32 TotalActiveEntities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    float BiomeDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    float PredatorPreyRatio;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    bool bMigrationActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    float TerritorialTension;

    FCrowd_EcosystemState()
    {
        TotalActiveEntities = 0;
        BiomeDensity = 0.5f;
        PredatorPreyRatio = 0.1f;
        bMigrationActive = false;
        TerritorialTension = 0.0f;
    }
};

UENUM(BlueprintType)
enum class ECrowd_PrehistoricBehaviorState : uint8
{
    Grazing,
    Migrating,
    Hunting,
    Fleeing,
    Territorial,
    Mating,
    Nesting,
    Sleeping
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowd_MassivePrehistoricEcosystem : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_MassivePrehistoricEcosystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core ecosystem components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USphereComponent* EcosystemBounds;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* EcosystemVisualization;

    // Ecosystem configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem Configuration")
    int32 MaxEntityCapacity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem Configuration")
    float EcosystemRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem Configuration")
    TArray<FCrowd_PrehistoricSpeciesData> SpeciesConfiguration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem Configuration")
    bool bEnableMassiveScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem Configuration")
    float LODDistanceMultiplier;

    // Runtime state
    UPROPERTY(BlueprintReadOnly, Category = "Runtime State")
    FCrowd_EcosystemState CurrentEcosystemState;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime State")
    TArray<FMassEntityHandle> ActiveEntities;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime State")
    float LastUpdateTime;

    // Biome integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Integration")
    TMap<FString, FVector> BiomeLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Integration")
    TMap<FString, float> BiomeDensityMultipliers;

public:
    // Core ecosystem management
    UFUNCTION(BlueprintCallable, Category = "Ecosystem Management")
    void InitializeEcosystem();

    UFUNCTION(BlueprintCallable, Category = "Ecosystem Management")
    void SpawnSpeciesPopulation(const FCrowd_PrehistoricSpeciesData& SpeciesData);

    UFUNCTION(BlueprintCallable, Category = "Ecosystem Management")
    void UpdateEcosystemBalance();

    UFUNCTION(BlueprintCallable, Category = "Ecosystem Management")
    void TriggerMigrationEvent(const FString& FromBiome, const FString& ToBiome);

    // Massive scale management
    UFUNCTION(BlueprintCallable, Category = "Massive Scale")
    void EnableMassiveScaleSimulation();

    UFUNCTION(BlueprintCallable, Category = "Massive Scale")
    void OptimizeLODForPerformance();

    UFUNCTION(BlueprintCallable, Category = "Massive Scale")
    void ManageEntityCulling(float PlayerDistance);

    // Territorial behavior
    UFUNCTION(BlueprintCallable, Category = "Territorial Behavior")
    void EstablishTerritorialZones();

    UFUNCTION(BlueprintCallable, Category = "Territorial Behavior")
    void ProcessTerritorialConflicts();

    UFUNCTION(BlueprintCallable, Category = "Territorial Behavior")
    bool CheckTerritorialOverlap(const FVector& Location, float Radius);

    // Combat integration
    UFUNCTION(BlueprintCallable, Category = "Combat Integration")
    void IntegrateWithCombatSystem();

    UFUNCTION(BlueprintCallable, Category = "Combat Integration")
    void TriggerPredatorResponse(const FVector& ThreatLocation);

    UFUNCTION(BlueprintCallable, Category = "Combat Integration")
    void ActivateHerdDefenseBehavior(const FVector& HerdCenter);

    // Quest system integration
    UFUNCTION(BlueprintCallable, Category = "Quest Integration")
    void RegisterQuestTriggers();

    UFUNCTION(BlueprintCallable, Category = "Quest Integration")
    void OnPlayerEnterTerritory(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Quest Integration")
    void GenerateDynamicEncounters();

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCurrentFrameImpact() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetActiveEntityCount() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void AdjustSimulationQuality(float TargetFrameRate);

    // Ecosystem queries
    UFUNCTION(BlueprintCallable, Category = "Ecosystem Queries")
    TArray<FVector> GetNearbyHerdLocations(const FVector& QueryLocation, float SearchRadius) const;

    UFUNCTION(BlueprintCallable, Category = "Ecosystem Queries")
    bool IsLocationSafeForPlayer(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Ecosystem Queries")
    FString GetDominantSpeciesInArea(const FVector& Location, float Radius) const;

private:
    // Internal management
    void UpdateSpeciesPopulations();
    void ProcessMigrationPatterns();
    void HandleTerritorialDisputes();
    void OptimizePerformance();
    void UpdateBiomeIntegration();

    // Mass Entity helpers
    void CreateMassEntityArchetype();
    void SpawnMassEntities(int32 Count, const FVector& Location);
    void UpdateMassEntityBehavior();

    // Performance optimization
    void CullDistantEntities(const FVector& PlayerLocation);
    void AdjustLODLevels();
    void ManageMemoryUsage();

    // Integration helpers
    void SetupCombatIntegration();
    void SetupQuestIntegration();
    void SetupBiomeIntegration();
};