#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MassEntityTypes.h"
#include "MassCommonTypes.h"
#include "Engine/World.h"
#include "CrowdSimulationSubsystem.generated.h"

class UMassEntitySubsystem;
class UMassSpawnerSubsystem;
struct FMassEntityHandle;

DECLARE_LOG_CATEGORY_EXTERN(LogCrowdSimulation, Log, All);

/**
 * Types of dinosaur crowd behaviors
 */
UENUM(BlueprintType)
enum class EDinosaurCrowdType : uint8
{
    None = 0,
    HerbivorePack,      // Peaceful herbivore groups (10-50 individuals)
    CarnivorePack,      // Hunting carnivore groups (2-8 individuals)  
    MigrationHerd,      // Large seasonal migration groups (100-500 individuals)
    PanicFlock,         // Emergency scatter behavior (any size)
    NestingColony       // Breeding/nesting groups (5-20 individuals)
};

/**
 * Crowd simulation parameters for different dinosaur types
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurCrowdParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDinosaurCrowdType CrowdType = EDinosaurCrowdType::HerbivorePack;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1", ClampMax = "500"))
    int32 MinGroupSize = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1", ClampMax = "500"))
    int32 MaxGroupSize = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "100", ClampMax = "10000"))
    float CohesionRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "50", ClampMax = "2000"))
    float SeparationRadius = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "500", ClampMax = "20000"))
    float AlignmentRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float CohesionWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float SeparationWeight = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float AlignmentWeight = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "50", ClampMax = "2000"))
    float MovementSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "100", ClampMax = "5000"))
    float PanicSpeed = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1000", ClampMax = "50000"))
    float TerritoryRadius = 5000.0f;

    FDinosaurCrowdParams()
    {
        CrowdType = EDinosaurCrowdType::HerbivorePack;
        MinGroupSize = 8;
        MaxGroupSize = 25;
        CohesionRadius = 800.0f;
        SeparationRadius = 150.0f;
        AlignmentRadius = 1200.0f;
        CohesionWeight = 1.2f;
        SeparationWeight = 2.5f;
        AlignmentWeight = 1.0f;
        MovementSpeed = 250.0f;
        PanicSpeed = 600.0f;
        TerritoryRadius = 3000.0f;
    }
};

/**
 * Migration route definition for seasonal movements
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FMigrationRoute
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> Waypoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RouteWidth = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SeasonStartDay = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SeasonEndDay = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsActive = false;
};

/**
 * Main subsystem for managing dinosaur crowd simulation
 * Integrates with Mass Entity framework for high-performance simulation
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

    // Crowd management
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnDinosaurCrowd(const FVector& Location, const FDinosaurCrowdParams& Params, int32 GroupSize = 0);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void TriggerPanicBehavior(const FVector& ThreatLocation, float PanicRadius = 2000.0f);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void StartMigrationEvent(int32 RouteIndex);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void StopMigrationEvent(int32 RouteIndex);

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetGlobalCrowdDensity(float DensityMultiplier);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void RegisterMigrationRoute(const FMigrationRoute& Route);

    // Query functions
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    int32 GetActiveCrowdCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    TArray<FVector> GetNearbyGroupCenters(const FVector& Location, float Radius) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    bool IsLocationInMigrationPath(const FVector& Location) const;

protected:
    // Mass Entity integration
    UPROPERTY()
    TObjectPtr<UMassEntitySubsystem> MassEntitySubsystem;

    UPROPERTY()
    TObjectPtr<UMassSpawnerSubsystem> MassSpawnerSubsystem;

    // Crowd tracking
    UPROPERTY()
    TArray<FMassEntityHandle> ActiveCrowds;

    UPROPERTY()
    TMap<FMassEntityHandle, FDinosaurCrowdParams> CrowdParameters;

    // Migration system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    TArray<FMigrationRoute> MigrationRoutes;

    UPROPERTY()
    TArray<int32> ActiveMigrations;

    // Global settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float GlobalDensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    int32 MaxSimultaneousCrowds = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float CrowdUpdateFrequency = 0.1f;

    // Internal functions
    void UpdateCrowdBehaviors(float DeltaTime);
    void ProcessMigrations(float DeltaTime);
    void CleanupInactiveCrowds();
    
    FVector CalculateFlockingForce(const FMassEntityHandle& Entity, const FDinosaurCrowdParams& Params);
    FVector CalculateCohesion(const FMassEntityHandle& Entity, const TArray<FMassEntityHandle>& Neighbors);
    FVector CalculateSeparation(const FMassEntityHandle& Entity, const TArray<FMassEntityHandle>& Neighbors, float SeparationRadius);
    FVector CalculateAlignment(const FMassEntityHandle& Entity, const TArray<FMassEntityHandle>& Neighbors);

private:
    FTimerHandle CrowdUpdateTimer;
    float LastUpdateTime = 0.0f;
};