#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassSimulationSubsystem.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "SharedTypes.h"
#include "Crowd_MassEntityManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_AgentConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Agent")
    float MovementSpeed = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Agent")
    float PersonalSpace = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Agent")
    float ViewRange = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Agent")
    ECrowd_BehaviorType BehaviorType = ECrowd_BehaviorType::Wandering;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Agent")
    int32 LODLevel = 0;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_SpawnZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Zone")
    FVector Center = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Zone")
    float Radius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Zone")
    int32 MaxAgents = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Zone")
    ECrowd_DensityLevel DensityLevel = ECrowd_DensityLevel::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Zone")
    TArray<FCrowd_AgentConfig> AgentConfigs;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowd_MassEntityManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_MassEntityManager();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core Mass Entity System
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mass Entity")
    class UMassEntitySubsystem* MassEntitySubsystem;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mass Entity")
    class UMassSpawnerSubsystem* MassSpawnerSubsystem;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mass Entity")
    class UMassSimulationSubsystem* MassSimulationSubsystem;

    // Crowd Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config")
    TArray<FCrowd_SpawnZone> SpawnZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config")
    int32 MaxTotalAgents = 50000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config")
    float TickInterval = 0.016f; // 60 FPS

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config")
    float LODUpdateInterval = 0.5f;

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CullingDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxVisibleAgents = 2000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableOcclusion = true;

    // Runtime State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    int32 ActiveAgentCount = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    int32 VisibleAgentCount = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    float CurrentFrameTime = 0.0f;

    // Core Functions
    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void InitializeMassEntitySystem();

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void SpawnCrowdInZone(const FCrowd_SpawnZone& Zone);

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void DespawnAllAgents();

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void UpdateLODSystem();

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void SetDensityLevel(ECrowd_DensityLevel NewDensity);

    // Pathfinding Integration
    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    void RegisterWaypoint(const FVector& Location, const FString& WaypointName);

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    void CreatePathNetwork();

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    TArray<FVector> GetOptimalPath(const FVector& Start, const FVector& End);

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCurrentPerformanceMetric() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizePerformance();

private:
    // Internal Systems
    void UpdateAgentBehaviors(float DeltaTime);
    void ProcessCulling();
    void HandleCollisionAvoidance();
    void UpdatePathfinding();
    
    // Performance Optimization
    void AdaptiveLOD();
    void DynamicCulling();
    void BatchProcessing();

    // Internal State
    TArray<FVector> WaypointNetwork;
    TMap<FString, FVector> NamedWaypoints;
    float LastLODUpdate = 0.0f;
    float LastPerformanceCheck = 0.0f;
};