#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "MassEntityTypes.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerTypes.h"
#include "MassCommonTypes.h"
#include "Crowd_SharedTypes.h"
#include "Crowd_MassiveEcosystemIntegration.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_EcosystemZoneData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    FString ZoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    FVector ZoneCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    float ZoneRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    int32 MaxPopulation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    TArray<FString> DominantSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    float BiomeDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    float ThreatLevel;

    FCrowd_EcosystemZoneData()
    {
        ZoneName = TEXT("Unknown");
        ZoneCenter = FVector::ZeroVector;
        ZoneRadius = 1000.0f;
        MaxPopulation = 1000;
        BiomeDensity = 0.5f;
        ThreatLevel = 0.3f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_MassiveSimulationMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 TotalEntitiesSimulated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 ActiveLODLevels;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float UpdateFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MemoryUsageMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CPUUsagePercent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float GPUUsagePercent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bPerformanceOptimized;

    FCrowd_MassiveSimulationMetrics()
    {
        TotalEntitiesSimulated = 0;
        ActiveLODLevels = 4;
        UpdateFrequency = 60.0f;
        MemoryUsageMB = 1024.0f;
        CPUUsagePercent = 50.0f;
        GPUUsagePercent = 40.0f;
        bPerformanceOptimized = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_BehavioralPattern
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    FString PatternName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    FString TriggerCondition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    FString BehaviorType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    int32 GroupSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float CoordinationLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bIsActive;

    FCrowd_BehavioralPattern()
    {
        PatternName = TEXT("Default");
        TriggerCondition = TEXT("None");
        BehaviorType = TEXT("Idle");
        GroupSize = 10;
        CoordinationLevel = 0.5f;
        Duration = 60.0f;
        bIsActive = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowd_MassiveEcosystemIntegration : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_MassiveEcosystemIntegration();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Ecosystem zone management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    TArray<FCrowd_EcosystemZoneData> EcosystemZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    int32 MaxEcosystemZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    float EcosystemUpdateInterval;

    // Massive simulation metrics
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FCrowd_MassiveSimulationMetrics SimulationMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnablePerformanceMonitoring;

    // Behavioral patterns
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    TArray<FCrowd_BehavioralPattern> BehavioralPatterns;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    int32 MaxActiveBehaviors;

    // Integration systems
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    bool bMemorySystemReady;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    bool bTerritorialSystemReady;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    bool bMassiveSimulationReady;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Ecosystem")
    void InitializeEcosystemZones();

    UFUNCTION(BlueprintCallable, Category = "Ecosystem")
    void AddEcosystemZone(const FCrowd_EcosystemZoneData& ZoneData);

    UFUNCTION(BlueprintCallable, Category = "Ecosystem")
    void RemoveEcosystemZone(const FString& ZoneName);

    UFUNCTION(BlueprintCallable, Category = "Ecosystem")
    FCrowd_EcosystemZoneData GetEcosystemZoneByName(const FString& ZoneName);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateSimulationMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceOptimal();

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void ActivateBehavioralPattern(const FString& PatternName);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void DeactivateBehavioralPattern(const FString& PatternName);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    TArray<FCrowd_BehavioralPattern> GetActiveBehavioralPatterns();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateSystemIntegration();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void InitializeMassiveSimulation();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ShutdownMassiveSimulation();

protected:
    // Internal functions
    void UpdateEcosystemZones(float DeltaTime);
    void MonitorPerformance(float DeltaTime);
    void ProcessBehavioralPatterns(float DeltaTime);
    void ValidateIntegrationSystems();

    // Timers
    float EcosystemUpdateTimer;
    float PerformanceMonitorTimer;
    float BehaviorUpdateTimer;

    // State tracking
    bool bSystemsInitialized;
    bool bSimulationActive;
    int32 CurrentActiveZones;
    int32 CurrentActiveBehaviors;
};