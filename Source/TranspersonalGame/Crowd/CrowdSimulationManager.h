#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/Engine.h"
#include "../SharedTypes.h"
#include "CrowdSimulationManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCrowdDensityChanged, float, NewDensity, FVector, Location);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCrowdEvent, ECrowd_EventType, EventType, FVector, Location, float, Intensity);

/**
 * Main manager for crowd simulation using Mass Entity system
 * Handles spawn points, LOD management, and crowd behavior coordination
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowdSimulationManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowdSimulationManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core crowd management
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd System")
    class USphereComponent* CrowdBounds;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd System")
    class UStaticMeshComponent* VisualizationMesh;

    // Crowd configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config", meta = (ClampMin = "10", ClampMax = "50000"))
    int32 MaxCrowdAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float CrowdDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config")
    ECrowd_BehaviorType DefaultBehaviorType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config")
    ECrowd_ActivityType DefaultActivityType;

    // LOD system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD System", meta = (ClampMin = "100.0", ClampMax = "5000.0"))
    float HighDetailRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD System", meta = (ClampMin = "500.0", ClampMax = "10000.0"))
    float MediumDetailRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD System", meta = (ClampMin = "1000.0", ClampMax = "20000.0"))
    float LowDetailRadius;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "10", ClampMax = "120"))
    int32 TargetFPS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnablePerformanceScaling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableLODSystem;

    // Spawn point management
    UPROPERTY(BlueprintReadOnly, Category = "Spawn System")
    TArray<class ACrowdSpawnPoint*> SpawnPoints;

    UPROPERTY(BlueprintReadOnly, Category = "Spawn System")
    TArray<class ACrowdAgent*> ActiveAgents;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnCrowdDensityChanged OnCrowdDensityChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnCrowdEvent OnCrowdEvent;

public:
    // Core crowd management functions
    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void InitializeCrowdSystem();

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void ShutdownCrowdSystem();

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void UpdateCrowdDensity(float NewDensity);

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void SetCrowdBehavior(ECrowd_BehaviorType NewBehavior);

    // Spawn point management
    UFUNCTION(BlueprintCallable, Category = "Spawn Management")
    void RegisterSpawnPoint(class ACrowdSpawnPoint* SpawnPoint);

    UFUNCTION(BlueprintCallable, Category = "Spawn Management")
    void UnregisterSpawnPoint(class ACrowdSpawnPoint* SpawnPoint);

    UFUNCTION(BlueprintCallable, Category = "Spawn Management")
    void SpawnCrowdAtPoint(class ACrowdSpawnPoint* SpawnPoint, int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Spawn Management")
    void DespawnCrowdAtPoint(class ACrowdSpawnPoint* SpawnPoint, int32 Count);

    // LOD management
    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    ECrowd_LODLevel GetLODLevelForDistance(float Distance) const;

    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    void UpdateAgentLOD(class ACrowdAgent* Agent, ECrowd_LODLevel NewLOD);

    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    void RefreshAllAgentLODs();

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCurrentFPS() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetActiveAgentCount() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void AdjustPerformanceSettings();

    // Event system
    UFUNCTION(BlueprintCallable, Category = "Events")
    void TriggerCrowdEvent(ECrowd_EventType EventType, FVector Location, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Events")
    void HandlePlayerProximity(FVector PlayerLocation, float Radius);

    // Debug and visualization
    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void DebugDrawCrowdInfo();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void ToggleCrowdVisualization();

    UFUNCTION(BlueprintPure, Category = "Debug")
    FString GetCrowdSystemStatus() const;

private:
    // Internal management
    void UpdateLODSystem(float DeltaTime);
    void UpdatePerformanceScaling(float DeltaTime);
    void ProcessCrowdEvents(float DeltaTime);
    void CleanupInactiveAgents();

    // Performance tracking
    float LastFPSCheck;
    float CurrentFPS;
    int32 FrameCounter;
    
    // LOD update timing
    float LODUpdateTimer;
    static constexpr float LODUpdateInterval = 0.5f;

    // Event processing
    TArray<FCrowd_EventData> PendingEvents;
    float EventProcessTimer;
};