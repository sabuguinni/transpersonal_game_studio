#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "NavigationSystem.h"
#include "AI/NavigationSystemBase.h"
#include "SharedTypes.h"
#include "CrowdSimulationManager.generated.h"

// Forward declarations
class ACrowdAgent;
class UCrowdBehaviorComponent;

UENUM(BlueprintType)
enum class ECrowd_BehaviorPattern : uint8
{
    Wandering       UMETA(DisplayName = "Wandering"),
    Gathering       UMETA(DisplayName = "Gathering"),
    Fleeing         UMETA(DisplayName = "Fleeing"),
    Following       UMETA(DisplayName = "Following"),
    Working         UMETA(DisplayName = "Working"),
    Resting         UMETA(DisplayName = "Resting"),
    Hunting         UMETA(DisplayName = "Hunting"),
    Ritual          UMETA(DisplayName = "Ritual")
};

UENUM(BlueprintType)
enum class ECrowd_LODLevel : uint8
{
    LOD0_Full       UMETA(DisplayName = "LOD0 - Full Detail"),
    LOD1_Reduced    UMETA(DisplayName = "LOD1 - Reduced Detail"),
    LOD2_Minimal    UMETA(DisplayName = "LOD2 - Minimal Detail"),
    LOD3_Culled     UMETA(DisplayName = "LOD3 - Culled")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_AgentData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FString AgentID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FVector Position;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    ECrowd_BehaviorPattern BehaviorPattern;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    ECrowd_LODLevel CurrentLOD;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    float MovementSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    float AvoidanceRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    float LastUpdateTime;

    FCrowd_AgentData()
    {
        AgentID = "";
        Position = FVector::ZeroVector;
        Velocity = FVector::ZeroVector;
        BehaviorPattern = ECrowd_BehaviorPattern::Wandering;
        CurrentLOD = ECrowd_LODLevel::LOD0_Full;
        MovementSpeed = 150.0f;
        AvoidanceRadius = 50.0f;
        TargetLocation = FVector::ZeroVector;
        bIsActive = true;
        LastUpdateTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_SimulationArea
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation")
    FString AreaID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation")
    int32 MaxAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation")
    int32 CurrentAgentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation")
    float SpawnRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation")
    ECrowd_BehaviorPattern DefaultBehavior;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation")
    bool bIsActive;

    FCrowd_SimulationArea()
    {
        AreaID = "";
        CenterLocation = FVector::ZeroVector;
        Radius = 1000.0f;
        MaxAgents = 1000;
        CurrentAgentCount = 0;
        SpawnRate = 10.0f;
        DefaultBehavior = ECrowd_BehaviorPattern::Wandering;
        bIsActive = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 TotalAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 VisibleAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float UpdateTimeMS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PathfindingTimeMS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODUpdateTimeMS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CurrentFPS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 LOD0_Count;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 LOD1_Count;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 LOD2_Count;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 LOD3_Count;

    FCrowd_PerformanceMetrics()
    {
        TotalAgents = 0;
        VisibleAgents = 0;
        UpdateTimeMS = 0.0f;
        PathfindingTimeMS = 0.0f;
        LODUpdateTimeMS = 0.0f;
        CurrentFPS = 60.0f;
        LOD0_Count = 0;
        LOD1_Count = 0;
        LOD2_Count = 0;
        LOD3_Count = 0;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCrowdSimulationManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UCrowdSimulationManager();

    // Subsystem overrides
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core crowd simulation functions
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void InitializeCrowdSimulation();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateCrowdSimulation(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void ShutdownCrowdSimulation();

    // Agent management
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    FString SpawnCrowdAgent(const FVector& Location, ECrowd_BehaviorPattern BehaviorPattern);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void DespawnCrowdAgent(const FString& AgentID);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetAgentBehavior(const FString& AgentID, ECrowd_BehaviorPattern NewBehavior);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetAgentTarget(const FString& AgentID, const FVector& TargetLocation);

    // Area management
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    FString CreateSimulationArea(const FVector& Center, float Radius, int32 MaxAgents, ECrowd_BehaviorPattern DefaultBehavior);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void RemoveSimulationArea(const FString& AreaID);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetAreaActive(const FString& AreaID, bool bActive);

    // LOD system
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateLODSystem(const FVector& ViewerLocation);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetLODDistances(float LOD0Distance, float LOD1Distance, float LOD2Distance, float LOD3Distance);

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    FCrowd_PerformanceMetrics GetPerformanceMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetPerformanceTarget(float TargetFPS, int32 MaxVisibleAgents);

    // Event system
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void TriggerCrowdEvent(const FString& EventName, const FVector& Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetGlobalBehaviorModifier(ECrowd_BehaviorPattern Behavior, float Intensity);

    // Debug and visualization
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation", CallInEditor)
    void DebugDrawCrowdSimulation();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetDebugVisualization(bool bEnabled);

    // Getters
    UFUNCTION(BlueprintPure, Category = "Crowd Simulation")
    int32 GetTotalAgentCount() const { return CrowdAgents.Num(); }

    UFUNCTION(BlueprintPure, Category = "Crowd Simulation")
    int32 GetActiveAgentCount() const;

    UFUNCTION(BlueprintPure, Category = "Crowd Simulation")
    TArray<FCrowd_AgentData> GetAllAgents() const;

    UFUNCTION(BlueprintPure, Category = "Crowd Simulation")
    TArray<FCrowd_SimulationArea> GetAllAreas() const;

protected:
    // Core data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd Simulation")
    TMap<FString, FCrowd_AgentData> CrowdAgents;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd Simulation")
    TMap<FString, FCrowd_SimulationArea> SimulationAreas;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd Simulation")
    FCrowd_PerformanceMetrics PerformanceMetrics;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    int32 MaxConcurrentAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    int32 MaxVisibleAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float TargetFPS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float UpdateFrequency;

    // LOD distances
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD System")
    float LOD0_Distance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD System")
    float LOD1_Distance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD System")
    float LOD2_Distance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD System")
    float LOD3_Distance;

    // Performance budgets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODUpdateBudgetMS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PathfindingBudgetMS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float BehaviorUpdateBudgetMS;

    // Debug
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bDebugVisualization;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowPerformanceStats;

private:
    // Internal update functions
    void UpdateAgentBehaviors(float DeltaTime);
    void UpdateAgentMovement(float DeltaTime);
    void UpdateLODLevels(const FVector& ViewerLocation);
    void UpdatePerformanceMetrics(float DeltaTime);
    void ProcessCrowdEvents(float DeltaTime);

    // Helper functions
    ECrowd_LODLevel CalculateLODLevel(float Distance) const;
    FVector CalculateAvoidanceVector(const FCrowd_AgentData& Agent) const;
    bool IsLocationNavigable(const FVector& Location) const;
    FString GenerateUniqueAgentID() const;

    // Internal state
    float LastUpdateTime;
    int32 NextAgentID;
    TArray<FVector> CrowdEventLocations;
    TArray<float> CrowdEventRadii;
    TArray<float> CrowdEventTimes;
};