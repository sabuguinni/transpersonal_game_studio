#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "StudioDirectorSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDir_AgentTaskData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    int32 AgentID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString TaskDescription = "";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString Priority = "Normal";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    bool bCompleted = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    float EstimatedHours = 1.0f;

    FDir_AgentTaskData()
    {
        AgentID = 0;
        TaskDescription = "";
        Priority = "Normal";
        bCompleted = false;
        EstimatedHours = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 TotalActorsInMap = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 DinosaurCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 EnvironmentPropsCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float FrameRate = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float MemoryUsageMB = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    bool bBuildCompiling = false;

    FDir_ProductionMetrics()
    {
        TotalActorsInMap = 0;
        DinosaurCount = 0;
        EnvironmentPropsCount = 0;
        FrameRate = 0.0f;
        MemoryUsageMB = 0.0f;
        bBuildCompiling = false;
    }
};

UENUM(BlueprintType)
enum class EDir_AgentSpecialization : uint8
{
    EngineArchitect = 2,
    CoreSystems = 3,
    PerformanceOptimizer = 4,
    ProceduralWorldGenerator = 5,
    EnvironmentArtist = 6,
    ArchitectureInterior = 7,
    LightingAtmosphere = 8,
    CharacterArtist = 9,
    Animation = 10,
    NPCBehavior = 11,
    CombatEnemyAI = 12,
    CrowdTrafficSimulation = 13,
    QuestMissionDesigner = 14,
    NarrativeDialogue = 15,
    Audio = 16,
    VFX = 17,
    QATesting = 18,
    IntegrationBuild = 19
};

/**
 * Studio Director System - Central coordination hub for all 18 specialized agents
 * Manages task distribution, production metrics, and agent workflow coordination
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AStudioDirectorSystem : public AActor
{
    GENERATED_BODY()

public:
    AStudioDirectorSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    // Agent Task Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Management")
    TArray<FDir_AgentTaskData> ActiveTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Management")
    TArray<FDir_AgentTaskData> CompletedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Management")
    int32 CurrentCycleID = 0;

    // Production Metrics
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    FDir_ProductionMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    float MetricsUpdateInterval = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    float LastMetricsUpdate = 0.0f;

    // Agent Coordination
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coordination")
    bool bAgentChainActive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coordination")
    int32 CurrentActiveAgent = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coordination")
    float AgentTimeoutThreshold = 300.0f; // 5 minutes

public:
    // Task Management Functions
    UFUNCTION(BlueprintCallable, Category = "Agent Management")
    void AssignTaskToAgent(int32 AgentID, const FString& TaskDescription, const FString& Priority = "Normal", float EstimatedHours = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Agent Management")
    void CompleteAgentTask(int32 AgentID, const FString& TaskDescription);

    UFUNCTION(BlueprintCallable, Category = "Agent Management")
    TArray<FDir_AgentTaskData> GetTasksForAgent(int32 AgentID);

    UFUNCTION(BlueprintCallable, Category = "Agent Management")
    int32 GetTotalActiveTasks();

    // Production Metrics Functions
    UFUNCTION(BlueprintCallable, Category = "Production Metrics")
    void UpdateProductionMetrics();

    UFUNCTION(BlueprintCallable, Category = "Production Metrics")
    FDir_ProductionMetrics GetCurrentMetrics();

    UFUNCTION(BlueprintCallable, Category = "Production Metrics")
    bool IsProductionOnTrack();

    // Agent Coordination Functions
    UFUNCTION(BlueprintCallable, Category = "Coordination")
    void StartAgentChain();

    UFUNCTION(BlueprintCallable, Category = "Coordination")
    void AdvanceToNextAgent();

    UFUNCTION(BlueprintCallable, Category = "Coordination")
    void ReportAgentCompletion(int32 AgentID, bool bSuccess);

    UFUNCTION(BlueprintCallable, Category = "Coordination")
    bool IsAgentChainComplete();

    // Emergency Controls
    UFUNCTION(BlueprintCallable, Category = "Emergency")
    void EmergencyStopAllAgents();

    UFUNCTION(BlueprintCallable, Category = "Emergency")
    void RestartAgentChain();

    UFUNCTION(BlueprintCallable, Category = "Emergency")
    void ForceAgentTimeout(int32 AgentID);

    // Debug and Monitoring
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogCurrentStatus();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void ValidateAgentChainIntegrity();

private:
    void UpdateAgentTimeouts();
    void CheckProductionLimits();
    void ValidateMapState();
    
    float LastAgentStartTime = 0.0f;
    bool bEmergencyMode = false;
};