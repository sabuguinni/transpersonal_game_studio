#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "ProductionDirector.generated.h"

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction   UMETA(DisplayName = "Pre-Production"),
    Production      UMETA(DisplayName = "Production"),
    Polish          UMETA(DisplayName = "Polish"),
    Release         UMETA(DisplayName = "Release")
};

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Working         UMETA(DisplayName = "Working"),
    Blocked         UMETA(DisplayName = "Blocked"),
    Complete        UMETA(DisplayName = "Complete"),
    Failed          UMETA(DisplayName = "Failed")
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    EDir_AgentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    float Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FDateTime StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FDateTime EstimatedCompletion;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    TArray<FString> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString OutputPath;

    FDir_AgentTask()
    {
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Status = EDir_AgentStatus::Idle;
        Priority = 1.0f;
        StartTime = FDateTime::Now();
        EstimatedCompletion = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 TotalActorsInLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 TranspersonalActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 DinosaurActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 TerrainActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 LightingActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    float CompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    FDateTime LastUpdate;

    FDir_ProductionMetrics()
    {
        TotalActorsInLevel = 0;
        TranspersonalActors = 0;
        DinosaurActors = 0;
        TerrainActors = 0;
        LightingActors = 0;
        CompletionPercentage = 0.0f;
        LastUpdate = FDateTime::Now();
    }
};

/**
 * Production Director - Studio Director's coordination system for managing all 19 agents
 * Tracks production state, assigns tasks, monitors progress, and ensures milestone completion
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AProductionDirector : public AActor
{
    GENERATED_BODY()

public:
    AProductionDirector();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // PRODUCTION PHASE MANAGEMENT
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production", meta = (AllowPrivateAccess = "true"))
    EDir_ProductionPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production", meta = (AllowPrivateAccess = "true"))
    FString CurrentCycleID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production", meta = (AllowPrivateAccess = "true"))
    int32 CycleNumber;

    // AGENT TASK MANAGEMENT
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Management", meta = (AllowPrivateAccess = "true"))
    TArray<FDir_AgentTask> ActiveTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Management", meta = (AllowPrivateAccess = "true"))
    TArray<FString> AgentNames;

    // PRODUCTION METRICS
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics", meta = (AllowPrivateAccess = "true"))
    FDir_ProductionMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics", meta = (AllowPrivateAccess = "true"))
    float TargetFrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics", meta = (AllowPrivateAccess = "true"))
    float CurrentFrameRate;

    // MILESTONE TRACKING
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestones", meta = (AllowPrivateAccess = "true"))
    TArray<FString> CompletedMilestones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestones", meta = (AllowPrivateAccess = "true"))
    FString CurrentMilestone;

    // CORE FUNCTIONS
    UFUNCTION(BlueprintCallable, Category = "Production Director")
    void InitializeProductionCycle(const FString& CycleID);

    UFUNCTION(BlueprintCallable, Category = "Production Director")
    void AssignTaskToAgent(const FString& AgentName, const FString& TaskDescription, float Priority);

    UFUNCTION(BlueprintCallable, Category = "Production Director")
    void UpdateAgentTaskStatus(const FString& AgentName, EDir_AgentStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Production Director")
    void AnalyzeProductionState();

    UFUNCTION(BlueprintCallable, Category = "Production Director")
    void GenerateProductionReport();

    UFUNCTION(BlueprintCallable, Category = "Production Director")
    float CalculateOverallProgress();

    UFUNCTION(BlueprintCallable, Category = "Production Director")
    void ValidateMilestoneCompletion(const FString& MilestoneName);

    UFUNCTION(BlueprintCallable, Category = "Production Director")
    TArray<FString> GetBlockedAgents();

    UFUNCTION(BlueprintCallable, Category = "Production Director")
    void CoordinateAgentDependencies();

    // LEVEL ANALYSIS
    UFUNCTION(BlueprintCallable, Category = "Level Analysis")
    void CountLevelActors();

    UFUNCTION(BlueprintCallable, Category = "Level Analysis")
    void ValidatePlayableState();

    UFUNCTION(BlueprintCallable, Category = "Level Analysis")
    bool IsMinimumViablePrototypeReady();

    // DEBUGGING AND EDITOR TOOLS
    UFUNCTION(CallInEditor, Category = "Debug")
    void DebugPrintProductionState();

    UFUNCTION(CallInEditor, Category = "Debug")
    void ResetProductionCycle();

    UFUNCTION(CallInEditor, Category = "Debug")
    void ForceAgentTaskCompletion(const FString& AgentName);

private:
    // INTERNAL HELPERS
    void UpdateProductionMetrics();
    void CheckAgentDependencies();
    void LogProductionEvent(const FString& Event);
    FDir_AgentTask* FindAgentTask(const FString& AgentName);
    void InitializeAgentList();
    void CalculateFrameRate();
};