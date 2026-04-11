#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "CrowdSimulationTypes.h"
#include "CrowdAgent.h"
#include "CrowdSpawnPoint.h"
#include "CrowdSimulationManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCrowdDensityChanged, float, NewDensity, float, MaxDensity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCrowdEmergencyEvent, ECrowdEmergencyType, EventType);

/**
 * Central manager for crowd simulation using Mass Entity framework
 * Handles up to 50,000 simultaneous agents with LOD system
 * Implements Jane Jacobs urban planning principles
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowdSimulationManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowdSimulationManager();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    // Crowd Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings", meta = (ClampMin = "100", ClampMax = "50000"))
    int32 MaxCrowdAgents = 10000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings", meta = (ClampMin = "0.1", ClampMax = "5.0"))
    float CrowdDensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    bool bEnableDynamicDensity = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    bool bEnableEmergencyBehavior = true;

    // LOD System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings", meta = (ClampMin = "500", ClampMax = "5000"))
    float HighDetailDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings", meta = (ClampMin = "1000", ClampMax = "10000"))
    float MediumDetailDistance = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings", meta = (ClampMin = "2000", ClampMax = "20000"))
    float LowDetailDistance = 8000.0f;

    // Spawn Points
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Settings")
    TArray<ACrowdSpawnPoint*> SpawnPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Settings", meta = (ClampMin = "1.0", ClampMax = "60.0"))
    float SpawnInterval = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Settings", meta = (ClampMin = "1", ClampMax = "100"))
    int32 AgentsPerSpawn = 10;

    // Performance Monitoring
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 CurrentActiveAgents;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    float CurrentFrameTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 HighDetailAgents;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 MediumDetailAgents;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 LowDetailAgents;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnCrowdDensityChanged OnCrowdDensityChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnCrowdEmergencyEvent OnCrowdEmergencyEvent;

private:
    // Internal state
    TArray<FCrowdAgent> ActiveAgents;
    TArray<FCrowdAgent> AgentPool;
    
    float LastSpawnTime;
    float LastPerformanceCheck;
    float PerformanceCheckInterval = 1.0f;
    
    // Emergency system
    bool bEmergencyActive;
    ECrowdEmergencyType CurrentEmergencyType;
    FVector EmergencyLocation;
    float EmergencyRadius;
    float EmergencyStartTime;

    // Spatial partitioning for performance
    TMap<FIntVector, TArray<int32>> SpatialGrid;
    float GridCellSize = 1000.0f;

public:
    // Blueprint Interface
    UFUNCTION(BlueprintCallable, Category = "Crowd Control")
    void StartCrowdSimulation();

    UFUNCTION(BlueprintCallable, Category = "Crowd Control")
    void StopCrowdSimulation();

    UFUNCTION(BlueprintCallable, Category = "Crowd Control")
    void PauseCrowdSimulation();

    UFUNCTION(BlueprintCallable, Category = "Crowd Control")
    void SetCrowdDensity(float NewDensity);

    UFUNCTION(BlueprintCallable, Category = "Crowd Control")
    void TriggerEmergencyEvent(ECrowdEmergencyType EventType, FVector Location, float Radius = 2000.0f);

    UFUNCTION(BlueprintCallable, Category = "Crowd Control")
    void ClearEmergencyEvent();

    UFUNCTION(BlueprintCallable, Category = "Crowd Query")
    int32 GetAgentsInRadius(FVector Center, float Radius) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Query")
    float GetCrowdDensityAtLocation(FVector Location, float SampleRadius = 500.0f) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Query")
    TArray<FVector> GetHighTrafficAreas() const;

    // C++ Interface
    void SpawnAgentsAtPoint(ACrowdSpawnPoint* SpawnPoint, int32 Count);
    void UpdateAgentLOD(FCrowdAgent& Agent, float DistanceToPlayer);
    void UpdateSpatialGrid();
    void ProcessEmergencyBehavior(float DeltaTime);
    void OptimizePerformance();
    
    // Getters
    FORCEINLINE int32 GetActiveAgentCount() const { return CurrentActiveAgents; }
    FORCEINLINE float GetCurrentDensity() const { return CurrentActiveAgents / (float)MaxCrowdAgents; }
    FORCEINLINE bool IsEmergencyActive() const { return bEmergencyActive; }
};