#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "Engine/World.h"
#include "CrowdSimulationSubsystem.generated.h"

class UMassEntitySubsystem;
class UMassSpawnerSubsystem;

UENUM(BlueprintType)
enum class EDinosaurHerdType : uint8
{
    SmallHerbivores,    // Compsognathus, Dryosaurus - 50-200 individuals
    MediumHerbivores,   // Triceratops, Parasaurolophus - 20-80 individuals
    LargeHerbivores,    // Brontosaurus, Diplodocus - 5-20 individuals
    Carnivores,         // Velociraptors, Allosaurus - 3-12 individuals
    Scavengers,         // Pteranodons, small theropods - 10-50 individuals
    Aquatic             // Plesiosaurs, marine reptiles - 5-30 individuals
};

UENUM(BlueprintType)
enum class EHerdBehaviorState : uint8
{
    Grazing,            // Peaceful feeding
    Migrating,          // Moving to new area
    Fleeing,            // Running from predator
    Drinking,           // At water source
    Resting,            // Sleeping/resting
    Alerting,           // Suspicious, looking around
    Panicking           // Chaos state
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FHerdConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDinosaurHerdType HerdType = EDinosaurHerdType::SmallHerbivores;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MinHerdSize = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxHerdSize = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MovementSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FleeSpeed = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AlertRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CohesionRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SeparationRadius = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AlignmentRadius = 300.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FMigrationRoute
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> Waypoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SeasonalTrigger = 0.0f; // 0-1 seasonal cycle

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDinosaurHerdType TargetHerdType = EDinosaurHerdType::SmallHerbivores;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MigrationDuration = 3600.0f; // seconds
};

/**
 * Crowd Simulation Subsystem
 * Manages large-scale dinosaur herd behavior using Mass Entity framework
 * Handles up to 50,000 simultaneous agents with emergent group behaviors
 */
UCLASS()
class TRANSPERSONALGAME_API UCrowdSimulationSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Core crowd simulation functions
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnHerd(const FHerdConfiguration& Config, const FVector& SpawnLocation);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void TriggerPanicInRadius(const FVector& Location, float Radius, float Duration = 30.0f);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void StartMigration(EDinosaurHerdType HerdType, const FMigrationRoute& Route);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetGlobalHerdBehavior(EHerdBehaviorState NewState);

    // Query functions
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    int32 GetTotalActiveAgents() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    TArray<FVector> GetHerdLocations(EDinosaurHerdType HerdType) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    bool IsAreaCrowded(const FVector& Location, float Radius, int32 Threshold = 100) const;

    // Environmental response
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void OnPredatorDetected(const FVector& PredatorLocation, float ThreatRadius);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void OnPlayerDetected(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void OnEnvironmentalChange(const FVector& Location, float Radius, bool bIsPositive);

protected:
    // Mass Entity integration
    UPROPERTY()
    TObjectPtr<UMassEntitySubsystem> MassEntitySubsystem;

    UPROPERTY()
    TObjectPtr<UMassSpawnerSubsystem> MassSpawnerSubsystem;

    // Herd tracking
    UPROPERTY()
    TMap<EDinosaurHerdType, TArray<FEntityHandle>> ActiveHerds;

    UPROPERTY()
    TArray<FMigrationRoute> ActiveMigrations;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    TMap<EDinosaurHerdType, FHerdConfiguration> DefaultHerdConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    int32 MaxSimultaneousAgents = 50000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float PerformanceUpdateInterval = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float LODDistanceNear = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float LODDistanceFar = 5000.0f;

private:
    // Internal state management
    EHerdBehaviorState GlobalBehaviorState = EHerdBehaviorState::Grazing;
    float LastPerformanceCheck = 0.0f;
    int32 CurrentActiveAgents = 0;

    // Internal functions
    void UpdatePerformanceMetrics();
    void ProcessHerdBehaviors(float DeltaTime);
    void HandleLODSystem();
    void CleanupInactiveHerds();
};