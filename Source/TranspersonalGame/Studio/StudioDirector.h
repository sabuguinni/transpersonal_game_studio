#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "StudioDirector.generated.h"

/**
 * Studio Director - Central coordination system for the Transpersonal Game Studio
 * Manages agent task distribution, monitors project state, and ensures playable prototype delivery
 * 
 * This actor serves as the executive oversight system that tracks:
 * - Agent task completion and coordination
 * - Playable prototype readiness metrics
 * - Critical path dependencies between systems
 * - Performance and quality benchmarks
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    int32 AgentID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    int32 Priority; // 1 = Critical, 2 = High, 3 = Medium, 4 = Low

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    bool bCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    float EstimatedHours;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString Dependencies; // Comma-separated agent IDs that must complete first

    FDir_AgentTask()
    {
        AgentID = 0;
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Priority = 3;
        bCompleted = false;
        EstimatedHours = 1.0f;
        Dependencies = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDir_PlayableMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Playable Metrics")
    bool bCharacterMovementWorking;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Playable Metrics")
    bool bTerrainNavigable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Playable Metrics")
    bool bDinosaurAIActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Playable Metrics")
    bool bSurvivalHUDVisible;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Playable Metrics")
    bool bAudioFunctional;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Playable Metrics")
    float FrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Playable Metrics")
    int32 ActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Playable Metrics")
    float PlayableScore; // 0-100%

    FDir_PlayableMetrics()
    {
        bCharacterMovementWorking = false;
        bTerrainNavigable = false;
        bDinosaurAIActive = false;
        bSurvivalHUDVisible = false;
        bAudioFunctional = false;
        FrameRate = 0.0f;
        ActorCount = 0;
        PlayableScore = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AStudioDirector : public AActor
{
    GENERATED_BODY()

public:
    AStudioDirector();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    // Agent coordination system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Coordination")
    TArray<FDir_AgentTask> CurrentCycleTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Coordination")
    int32 CurrentCycleID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Coordination")
    FString CurrentCycleName;

    // Playable prototype tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prototype Metrics")
    FDir_PlayableMetrics PlayableState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prototype Metrics")
    float PlayableThreshold; // Minimum score to consider prototype ready

    // Performance monitoring
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bPerformanceWarning;

public:
    virtual void Tick(float DeltaTime) override;

    // Agent coordination functions
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void InitializeCycle(int32 CycleID, const FString& CycleName);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void AddAgentTask(int32 AgentID, const FString& AgentName, const FString& TaskDescription, 
                     int32 Priority, float EstimatedHours, const FString& Dependencies = TEXT(""));

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CompleteAgentTask(int32 AgentID);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool AreTaskDependenciesMet(int32 AgentID);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    float GetCycleCompletionPercentage();

    // Playable prototype assessment
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void AssessPlayableState();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool IsPrototypeReady();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FString GetPlayableStatusReport();

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool IsPerformanceAcceptable();

    // Critical path analysis
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_AgentTask> GetCriticalPathTasks();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void GenerateTaskReport();

    // Emergency coordination
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void TriggerEmergencyMode(const FString& Reason);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void ResetCycle();

protected:
    // Internal coordination logic
    void UpdateTaskPriorities();
    void CheckCriticalDependencies();
    void MonitorAgentProgress();
    float CalculatePlayableScore();
    void LogCycleStatus();

private:
    // Cycle timing
    float CycleStartTime;
    float LastAssessmentTime;
    
    // Emergency state
    bool bEmergencyMode;
    FString EmergencyReason;
    
    // Performance tracking
    TArray<float> FrameRateHistory;
    TArray<int32> ActorCountHistory;
};