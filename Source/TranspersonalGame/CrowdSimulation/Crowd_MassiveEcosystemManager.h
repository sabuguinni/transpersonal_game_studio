#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "MassEntitySubsystem.h"
#include "MassProcessingTypes.h"
#include "MassEntityTypes.h"
#include "MassCommonTypes.h"
#include "Crowd_SharedTypes.h"
#include "Crowd_MassiveEcosystemManager.generated.h"

class UCrowd_MassEntityManager;
class UCrowd_BiomeCrowdManager;
class UCrowd_MassHerdSystem;

/**
 * Massive ecosystem manager that coordinates 50,000+ entities across all biomes
 * Handles population dynamics, migration patterns, and ecosystem balance
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCrowd_MassiveEcosystemManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCrowd_MassiveEcosystemManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Ecosystem management
    UFUNCTION(BlueprintCallable, Category = "Ecosystem")
    void InitializeMassiveEcosystem();

    UFUNCTION(BlueprintCallable, Category = "Ecosystem")
    void SpawnBiomePopulations();

    UFUNCTION(BlueprintCallable, Category = "Ecosystem")
    void UpdateEcosystemBalance(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Ecosystem")
    void TriggerMigrationEvent(ECrowd_BiomeType FromBiome, ECrowd_BiomeType ToBiome, int32 EntityCount);

    // Population control
    UFUNCTION(BlueprintCallable, Category = "Population")
    void SetBiomePopulationTarget(ECrowd_BiomeType BiomeType, int32 TargetPopulation);

    UFUNCTION(BlueprintCallable, Category = "Population")
    int32 GetCurrentBiomePopulation(ECrowd_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Population")
    void BalancePopulations();

    // Performance management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetLODDistances(float Near, float Medium, float Far);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePerformanceSettings(int32 MaxVisibleEntities, float UpdateFrequency);

    // Debug and monitoring
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void GetEcosystemStats(int32& TotalEntities, int32& ActiveEntities, float& AverageFrameTime);

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogBiomePopulations();

protected:
    // Biome population targets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Population")
    TMap<ECrowd_BiomeType, int32> BiomePopulationTargets;

    // Current populations
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Population")
    TMap<ECrowd_BiomeType, int32> CurrentBiomePopulations;

    // Ecosystem managers
    UPROPERTY()
    TObjectPtr<UCrowd_MassEntityManager> MassEntityManager;

    UPROPERTY()
    TObjectPtr<UCrowd_BiomeCrowdManager> BiomeCrowdManager;

    UPROPERTY()
    TObjectPtr<UCrowd_MassHerdSystem> HerdSystem;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxTotalEntities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxVisibleEntities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float EcosystemUpdateFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODNearDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODMediumDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODFarDistance;

    // Migration system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    float MigrationCheckInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    float MigrationTriggerThreshold;

    UPROPERTY()
    FTimerHandle EcosystemUpdateTimer;

    UPROPERTY()
    FTimerHandle MigrationCheckTimer;

private:
    // Internal methods
    void InitializeBiomeTargets();
    void StartEcosystemTimers();
    void CheckMigrationTriggers();
    void ProcessMigration(ECrowd_BiomeType FromBiome, ECrowd_BiomeType ToBiome, int32 EntityCount);
    void UpdateLODSystem();
    void MonitorPerformance();

    // Performance tracking
    float LastFrameTime;
    int32 FrameTimeHistory[60]; // 1 second of frame times at 60fps
    int32 FrameTimeIndex;
    bool bPerformanceMonitoringEnabled;
};