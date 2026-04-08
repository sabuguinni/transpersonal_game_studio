#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MassEntitySubsystem.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "Engine/World.h"
#include "CrowdSimulationManager.generated.h"

USTRUCT(BlueprintType)
struct FHerdBehaviorData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MinHerdSize = 5;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxHerdSize = 25;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CohesionRadius = 1000.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SeparationRadius = 300.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AlignmentRadius = 800.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GrazingTime = 300.0f; // seconds
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MigrationSpeed = 200.0f; // cm/s
};

USTRUCT(BlueprintType)
struct FTerritoryData
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector CenterLocation;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TerritoryRadius = 5000.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxOccupants = 50;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ResourceDensity = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsWaterSource = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsShelter = false;
};

USTRUCT(BlueprintType)
struct FMigrationRoute
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> Waypoints;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SeasonalTrigger = 0.0f; // 0-1 representing time of year
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ExpectedHerdCount = 10;
};

/**
 * Manages crowd simulation for dinosaur herds using Mass Entity Framework
 * Handles up to 50,000 simultaneous agents with realistic herd behaviors
 */
UCLASS()
class TRANSPERSONALGAME_API ACrowdSimulationManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowdSimulationManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // === CORE SYSTEMS ===
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mass Entity")
    class UMassEntitySubsystem* MassEntitySubsystem;
    
    // === HERD BEHAVIOR CONFIGURATION ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Behavior")
    TMap<FString, FHerdBehaviorData> SpeciesHerdBehaviors;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory Management")
    TArray<FTerritoryData> AvailableTerritories;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    TArray<FMigrationRoute> MigrationRoutes;
    
    // === SIMULATION PARAMETERS ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation", meta = (ClampMin = "1000", ClampMax = "50000"))
    int32 MaxSimulatedEntities = 25000;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation")
    float SimulationRadius = 10000.0f; // Distance from player to simulate
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation")
    float LODDistance1 = 2000.0f; // Full behavior
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation")
    float LODDistance2 = 5000.0f; // Simplified behavior
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation")
    float LODDistance3 = 10000.0f; // Position-only updates
    
    // === ENVIRONMENTAL FACTORS ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float CurrentSeasonalProgress = 0.0f; // 0-1 representing year progress
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float WaterAvailability = 1.0f; // 0-1 representing drought conditions
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float FoodAvailability = 1.0f; // 0-1 representing vegetation density
    
    // === PLAYER INTERACTION ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Response")
    float PlayerThreatRadius = 500.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Response")
    float PlayerMemoryDuration = 600.0f; // seconds
    
public:
    // === PUBLIC INTERFACE ===
    
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnHerdAtLocation(const FString& SpeciesName, const FVector& Location, int32 HerdSize);
    
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void TriggerMigration(int32 RouteIndex);
    
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetEnvironmentalConditions(float SeasonProgress, float Water, float Food);
    
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void RegisterPlayerDisturbance(const FVector& Location, float Intensity, float Radius);
    
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    TArray<FVector> GetActiveHerdLocations(const FString& SpeciesName) const;
    
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    int32 GetTotalActiveEntities() const;

protected:
    // === INTERNAL SYSTEMS ===
    
    UFUNCTION()
    void InitializeMassEntity();
    
    UFUNCTION()
    void UpdateHerdBehaviors(float DeltaTime);
    
    UFUNCTION()
    void ProcessMigrationLogic(float DeltaTime);
    
    UFUNCTION()
    void UpdateLODLevels();
    
    UFUNCTION()
    void HandlePlayerProximity();
    
    UFUNCTION()
    void ManageTerritoryOccupancy();

private:
    // === INTERNAL DATA ===
    
    TArray<FMassEntityHandle> ActiveHerds;
    TMap<FMassEntityHandle, FString> EntityToSpeciesMap;
    TMap<FVector, float> PlayerDisturbanceMap; // Location -> Timestamp
    
    float LastMigrationCheck = 0.0f;
    float LastLODUpdate = 0.0f;
    float LastTerritoryUpdate = 0.0f;
    
    // === PERFORMANCE TRACKING ===
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debug", meta = (AllowPrivateAccess = "true"))
    int32 CurrentEntityCount = 0;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debug", meta = (AllowPrivateAccess = "true"))
    float LastFrameProcessingTime = 0.0f;
};