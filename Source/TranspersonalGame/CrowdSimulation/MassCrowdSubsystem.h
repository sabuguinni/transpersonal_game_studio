#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MassEntityTypes.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerTypes.h"
#include "Engine/World.h"
#include "MassCrowdSubsystem.generated.h"

class UMassEntitySubsystem;
struct FMassEntityHandle;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCrowdDensityChanged, FVector, Location, float, NewDensity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnMigrationStarted, int32, GroupID, FVector, StartLocation, FVector, TargetLocation);

/**
 * Prehistoric Crowd Simulation Subsystem
 * Manages large-scale crowd behavior for dinosaur herds, human tribes, and predator packs
 * Uses Mass Entity Framework for performance with up to 50,000 simultaneous agents
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UMassCrowdSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Core crowd management
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void InitializeCrowdSystem();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnDinosaurHerd(const FVector& Location, int32 HerdSize, TSubclassOf<class ADinosaurAgent> DinosaurClass);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnHumanTribe(const FVector& Location, int32 TribeSize, float TerritoryRadius);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnPredatorPack(const FVector& Location, int32 PackSize, TSubclassOf<class APredatorAgent> PredatorClass);

    // Migration and movement patterns
    UFUNCTION(BlueprintCallable, Category = "Migration")
    void TriggerSeasonalMigration(const FVector& FromRegion, const FVector& ToRegion, float MigrationSpeed = 100.0f);

    UFUNCTION(BlueprintCallable, Category = "Migration")
    void SetWaterSourceLocations(const TArray<FVector>& WaterSources);

    UFUNCTION(BlueprintCallable, Category = "Migration")
    void SetFoodSourceDensity(const FVector& Location, float Radius, float FoodDensity);

    // Crowd density and flow analysis
    UFUNCTION(BlueprintCallable, Category = "Crowd Analysis")
    float GetCrowdDensityAtLocation(const FVector& Location, float Radius = 1000.0f) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Analysis")
    TArray<FVector> GetHighTrafficAreas(float MinDensity = 5.0f) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Analysis")
    FVector GetNearestSafeZone(const FVector& FromLocation, float SearchRadius = 5000.0f) const;

    // Predator-prey dynamics
    UFUNCTION(BlueprintCallable, Category = "Ecosystem")
    void RegisterPredatorThreat(const FVector& ThreatLocation, float ThreatRadius, float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Ecosystem")
    void TriggerHerdScatter(const FVector& ThreatLocation, float ScatterRadius);

    // Player interaction with crowds
    UFUNCTION(BlueprintCallable, Category = "Player Interaction")
    void SetPlayerStealthLevel(float StealthLevel); // 0.0 = fully visible, 1.0 = invisible

    UFUNCTION(BlueprintCallable, Category = "Player Interaction")
    bool IsPlayerDetectedByCrowd(const FVector& PlayerLocation) const;

    // Events
    UPROPERTY(BlueprintAssignable)
    FOnCrowdDensityChanged OnCrowdDensityChanged;

    UPROPERTY(BlueprintAssignable)
    FOnMigrationStarted OnMigrationStarted;

protected:
    // Mass Entity management
    UPROPERTY()
    TObjectPtr<UMassEntitySubsystem> MassEntitySubsystem;

    // Crowd configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    int32 MaxSimultaneousAgents = 50000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float CrowdUpdateFrequency = 0.1f; // Update every 100ms

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float MaxViewDistance = 10000.0f; // LOD distance for crowd rendering

    // Migration patterns
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    TArray<FVector> SeasonalMigrationRoutes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    TArray<FVector> WaterSourceLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    float MigrationTriggerDistance = 2000.0f;

    // Crowd density tracking
    UPROPERTY()
    TMap<FVector, float> CrowdDensityGrid;

    UPROPERTY()
    float GridCellSize = 500.0f;

    // Player stealth system
    UPROPERTY()
    float CurrentPlayerStealthLevel = 0.0f;

    UPROPERTY()
    float PlayerDetectionRadius = 1500.0f;

    // Internal methods
    void UpdateCrowdDensityGrid();
    void ProcessMigrationLogic();
    void HandlePredatorPreyInteractions();
    FVector GetGridKey(const FVector& WorldLocation) const;
    void SpawnMassAgents(const FVector& Location, int32 Count, const FString& AgentType);

    // Timers
    FTimerHandle CrowdUpdateTimer;
    FTimerHandle MigrationUpdateTimer;
    FTimerHandle DensityAnalysisTimer;
};