#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "MassProcessor.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassDinosaurFragments.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "EcosystemMassManager.generated.h"

/**
 * Advanced ecosystem management for Mass Entity simulation
 * Coordinates up to 50,000 dinosaur agents across multiple biomes
 * Implements emergent ecosystem behaviors and population dynamics
 */

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEcosystemZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Center = FVector::ZeroVector;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Radius = 10000.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<EDinosaurSpecies, int32> SpeciesPopulation;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FoodAvailability = 1.0f; // 0-1
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WaterAvailability = 1.0f; // 0-1
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ThreatLevel = 0.0f; // 0-1
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bPlayerPresent = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PlayerInfluence = 0.0f; // How much player affects this zone
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> WaterSources;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> FeedingAreas;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> NestingSites;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FMigrationRoute
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> Waypoints;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDinosaurSpecies Species = EDinosaurSpecies::Triceratops;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SeasonalTrigger = 0.5f; // Time of year 0-1
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ExpectedHerdSize = 20;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MigrationSpeed = 300.0f; // cm/s
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsActive = false;
};

UCLASS()
class TRANSPERSONALGAME_API UEcosystemMassManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }

    // Ecosystem management
    UFUNCTION(BlueprintCallable, Category = "Ecosystem")
    void RegisterEcosystemZone(const FEcosystemZone& Zone);
    
    UFUNCTION(BlueprintCallable, Category = "Ecosystem")
    void UpdateZonePopulation(int32 ZoneIndex, EDinosaurSpecies Species, int32 PopulationChange);
    
    UFUNCTION(BlueprintCallable, Category = "Ecosystem")
    FEcosystemZone GetNearestZone(const FVector& Location) const;
    
    UFUNCTION(BlueprintCallable, Category = "Ecosystem")
    void TriggerMigration(EDinosaurSpecies Species, const FVector& FromLocation, const FVector& ToLocation);
    
    // Population dynamics
    UFUNCTION(BlueprintCallable, Category = "Population")
    void SpawnHerd(EDinosaurSpecies Species, const FVector& Location, int32 HerdSize);
    
    UFUNCTION(BlueprintCallable, Category = "Population")
    void DespawnEntitiesInRadius(const FVector& Location, float Radius);
    
    UFUNCTION(BlueprintCallable, Category = "Population")
    int32 GetTotalPopulation() const;
    
    UFUNCTION(BlueprintCallable, Category = "Population")
    int32 GetSpeciesPopulation(EDinosaurSpecies Species) const;
    
    // Player interaction
    UFUNCTION(BlueprintCallable, Category = "Player")
    void NotifyPlayerLocation(const FVector& PlayerLocation);
    
    UFUNCTION(BlueprintCallable, Category = "Player")
    void NotifyPlayerAction(const FVector& Location, float ImpactRadius, float ThreatLevel);
    
    // Environmental events
    UFUNCTION(BlueprintCallable, Category = "Environment")
    void TriggerEnvironmentalEvent(const FVector& Location, float Radius, float Severity);
    
    UFUNCTION(BlueprintCallable, Category = "Environment")
    void UpdateSeasonalConditions(float SeasonProgress); // 0-1 through the year

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ecosystem")
    TArray<FEcosystemZone> EcosystemZones;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Migration")
    TArray<FMigrationRoute> MigrationRoutes;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Population")
    TMap<EDinosaurSpecies, int32> GlobalPopulationLimits;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Population")
    int32 MaxTotalPopulation = 50000;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player")
    FVector LastKnownPlayerLocation = FVector::ZeroVector;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player")
    float PlayerInfluenceRadius = 5000.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment")
    float CurrentSeason = 0.0f; // 0-1 through the year
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    float UpdateFrequency = 1.0f; // Updates per second
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 MaxEntitiesPerFrame = 1000; // LOD system

private:
    float TimeSinceLastUpdate = 0.0f;
    int32 CurrentPopulation = 0;
    
    // Internal ecosystem simulation
    void UpdateEcosystemZones(float DeltaTime);
    void ProcessMigrations(float DeltaTime);
    void BalancePopulations();
    void UpdatePlayerInfluence();
    void ProcessEnvironmentalPressures();
    
    // Performance optimization
    void OptimizeSimulation();
    void UpdateLODLevels();
    bool ShouldSpawnInZone(const FEcosystemZone& Zone) const;
    bool ShouldDespawnFromZone(const FEcosystemZone& Zone) const;
};

UCLASS()
class TRANSPERSONALGAME_API UMassEcosystemProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UMassEcosystemProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery EcosystemQuery;
    
    UPROPERTY(EditAnywhere, Category = "Ecosystem Behavior")
    float ResourceCompetitionRadius = 2000.0f;
    
    UPROPERTY(EditAnywhere, Category = "Ecosystem Behavior")
    float TerritorialBehaviorStrength = 1.0f;
    
    UPROPERTY(EditAnywhere, Category = "Ecosystem Behavior")
    float SeasonalMigrationTrigger = 0.8f;
    
    UPROPERTY(EditAnywhere, Category = "Performance")
    int32 MaxProcessedEntitiesPerFrame = 500;
    
    void ProcessResourceCompetition(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    void ProcessTerritorialBehavior(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    void ProcessSeasonalBehavior(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    void ProcessPlayerInteraction(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
};

UCLASS()
class TRANSPERSONALGAME_API UMassPopulationDynamicsProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UMassPopulationDynamicsProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery PopulationQuery;
    
    UPROPERTY(EditAnywhere, Category = "Population")
    float NaturalDeathRate = 0.001f; // Per second
    
    UPROPERTY(EditAnywhere, Category = "Population")
    float ReproductionRate = 0.0001f; // Per second
    
    UPROPERTY(EditAnywhere, Category = "Population")
    float CarryingCapacityPressure = 0.8f; // When to start population control
    
    UPROPERTY(EditAnywhere, Category = "Performance")
    int32 MaxBirthsPerFrame = 10;
    
    UPROPERTY(EditAnywhere, Category = "Performance")
    int32 MaxDeathsPerFrame = 50;
    
    void ProcessNaturalLifecycle(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    void ProcessReproduction(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    void ProcessPopulationPressure(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    void ProcessAging(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
};