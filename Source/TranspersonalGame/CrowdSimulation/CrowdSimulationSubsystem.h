#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassSimulationSubsystem.h"
#include "Engine/World.h"
#include "CrowdSimulationSubsystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogCrowdSimulation, Log, All);

/**
 * Ecosystem Types for different crowd behaviors
 */
UENUM(BlueprintType)
enum class EEcosystemType : uint8
{
    Herbivore_Grazing,      // Peaceful grazing herds
    Herbivore_Migrating,    // Large migration groups
    Carnivore_Hunting,      // Predator packs
    Carnivore_Scavenging,   // Opportunistic feeders
    Mixed_WateringHole,     // Mixed species at water sources
    Territorial_Nesting     // Defensive territorial behavior
};

/**
 * Crowd Density Levels
 */
UENUM(BlueprintType)
enum class ECrowdDensity : uint8
{
    Sparse,     // 1-5 individuals
    Small,      // 5-15 individuals  
    Medium,     // 15-50 individuals
    Large,      // 50-200 individuals
    Massive     // 200+ individuals (migrations)
};

/**
 * Time of Day Behavior Modifiers
 */
UENUM(BlueprintType)
enum class ETimeOfDayBehavior : uint8
{
    Dawn_Active,        // Most active at dawn
    Day_Active,         // Active during day
    Dusk_Active,        // Most active at dusk
    Night_Active,       // Nocturnal
    Continuous_Active   // Active throughout day/night cycle
};

/**
 * Main subsystem for managing prehistoric ecosystem crowd simulation
 * Handles up to 50,000 simultaneous agents using UE5 Mass Entity Framework
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

    // Core crowd management
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnEcosystemGroup(EEcosystemType EcosystemType, FVector Location, ECrowdDensity Density, float Radius = 1000.0f);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void DespawnEcosystemGroup(int32 GroupID);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateEcosystemBehavior(ETimeOfDayBehavior TimeOfDay, float Temperature, float Humidity);

    // Player interaction effects
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void TriggerPlayerDisturbance(FVector PlayerLocation, float DisturbanceRadius, float IntensityLevel);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetPlayerAsHuntTarget(FVector PlayerLocation, bool bIsBeingHunted);

    // Environmental triggers
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void TriggerMigrationEvent(FVector StartLocation, FVector EndLocation, EEcosystemType MigratingSpecies);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void CreateWateringHoleActivity(FVector WaterLocation, float ActivityRadius);

    // Performance and LOD management
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetCrowdLODDistance(float NearDistance, float MidDistance, float FarDistance);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetMaxActiveAgents(int32 MaxAgents);

    // Debug and monitoring
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    int32 GetActiveAgentCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    TArray<FVector> GetActiveGroupLocations() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void EnableDebugVisualization(bool bEnable);

protected:
    // Mass Entity references
    UPROPERTY()
    class UMassEntitySubsystem* MassEntitySubsystem;

    UPROPERTY()
    class UMassSpawnerSubsystem* MassSpawnerSubsystem;

    UPROPERTY()
    class UMassSimulationSubsystem* MassSimulationSubsystem;

    // Active ecosystem groups
    UPROPERTY()
    TMap<int32, struct FEcosystemGroupData> ActiveGroups;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveAgents = 50000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODNearDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODMidDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODFarDistance = 10000.0f;

    // Environmental state
    UPROPERTY()
    ETimeOfDayBehavior CurrentTimeOfDay;

    UPROPERTY()
    float CurrentTemperature = 25.0f;

    UPROPERTY()
    float CurrentHumidity = 0.6f;

    // Player tracking
    UPROPERTY()
    FVector LastPlayerLocation;

    UPROPERTY()
    bool bPlayerIsBeingHunted = false;

    UPROPERTY()
    float PlayerDisturbanceRadius = 500.0f;

private:
    // Internal group management
    int32 NextGroupID = 1;
    bool bDebugVisualizationEnabled = false;

    // Helper functions
    void InitializeMassEntitySystem();
    void CreateEcosystemBehaviorProfile(EEcosystemType Type, struct FEcosystemBehaviorProfile& OutProfile);
    void UpdateGroupBehaviorBasedOnEnvironment(struct FEcosystemGroupData& GroupData);
    void HandlePlayerProximityEffects();
    void ProcessLODUpdates();
};

/**
 * Data structure for tracking ecosystem groups
 */
USTRUCT(BlueprintType)
struct FEcosystemGroupData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    int32 GroupID = 0;

    UPROPERTY(BlueprintReadOnly)
    EEcosystemType EcosystemType = EEcosystemType::Herbivore_Grazing;

    UPROPERTY(BlueprintReadOnly)
    ECrowdDensity Density = ECrowdDensity::Small;

    UPROPERTY(BlueprintReadOnly)
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly)
    float ActivityRadius = 1000.0f;

    UPROPERTY(BlueprintReadOnly)
    int32 AgentCount = 0;

    UPROPERTY(BlueprintReadOnly)
    TArray<FMassEntityHandle> EntityHandles;

    UPROPERTY(BlueprintReadOnly)
    float LastPlayerDisturbanceTime = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    bool bIsActive = true;

    UPROPERTY(BlueprintReadOnly)
    bool bIsMigrating = false;

    UPROPERTY(BlueprintReadOnly)
    FVector MigrationTarget = FVector::ZeroVector;
};

/**
 * Behavior profile for different ecosystem types
 */
USTRUCT(BlueprintType)
struct FEcosystemBehaviorProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MovementSpeed = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GroupCohesion = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SeparationDistance = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PlayerAvoidanceDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PredatorAvoidanceDistance = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ETimeOfDayBehavior PreferredTimeOfDay = ETimeOfDayBehavior::Day_Active;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanBeDomesticated = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AggressionLevel = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CuriosityLevel = 0.3f;
};