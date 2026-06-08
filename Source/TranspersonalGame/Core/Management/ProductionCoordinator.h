#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "ProductionCoordinator.generated.h"

/**
 * Studio Director Production Coordination System
 * Manages the 19-agent production pipeline and milestone tracking
 * Ensures each agent delivers measurable outputs and maintains quality standards
 */

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Working         UMETA(DisplayName = "Working"), 
    Completed       UMETA(DisplayName = "Completed"),
    Blocked         UMETA(DisplayName = "Blocked"),
    Failed          UMETA(DisplayName = "Failed")
};

UENUM(BlueprintType)
enum class EDir_MilestonePhase : uint8
{
    Planning        UMETA(DisplayName = "Planning"),
    WalkAround      UMETA(DisplayName = "Walk Around Prototype"),
    BasicSurvival   UMETA(DisplayName = "Basic Survival"),
    DinosaurAI      UMETA(DisplayName = "Dinosaur AI"),
    FullGameplay    UMETA(DisplayName = "Full Gameplay"),
    Polish          UMETA(DisplayName = "Polish & Optimization")
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    int32 AgentNumber = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_AgentStatus Status = EDir_AgentStatus::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float Priority = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    TArray<FString> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    TArray<FString> ExpectedDeliverables;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FDateTime StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FDateTime DeadlineTime;

    FDir_AgentTask()
    {
        StartTime = FDateTime::Now();
        DeadlineTime = FDateTime::Now() + FTimespan::FromHours(2);
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CycleNumber = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalCppFiles = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalHeaderFiles = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CompilationErrors = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 ActiveActorsInLevel = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float FrameRate = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    bool bIsPlayable = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    EDir_MilestonePhase CurrentPhase = EDir_MilestonePhase::Planning;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UProductionCoordinator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Production Management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void StartProductionCycle(int32 CycleNumber);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void AssignTaskToAgent(int32 AgentNumber, const FString& TaskDescription, const TArray<FString>& Deliverables);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void CompleteAgentTask(int32 AgentNumber, const TArray<FString>& ActualDeliverables);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void BlockAgent(int32 AgentNumber, const FString& BlockReason);

    // Milestone Management
    UFUNCTION(BlueprintCallable, Category = "Milestones")
    void AdvanceToNextPhase();

    UFUNCTION(BlueprintCallable, Category = "Milestones")
    bool IsMilestoneComplete(EDir_MilestonePhase Phase) const;

    UFUNCTION(BlueprintCallable, Category = "Milestones")
    float GetMilestoneProgress(EDir_MilestonePhase Phase) const;

    // Quality Assurance
    UFUNCTION(BlueprintCallable, Category = "QA")
    bool ValidatePlayablePrototype() const;

    UFUNCTION(BlueprintCallable, Category = "QA")
    void RunCompilationCheck();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void UpdateProductionMetrics();

    // Agent Coordination
    UFUNCTION(BlueprintCallable, Category = "Coordination")
    TArray<FDir_AgentTask> GetPendingTasks() const;

    UFUNCTION(BlueprintCallable, Category = "Coordination")
    TArray<FDir_AgentTask> GetBlockedTasks() const;

    UFUNCTION(BlueprintCallable, Category = "Coordination")
    FDir_AgentTask GetNextTaskForAgent(int32 AgentNumber) const;

    // Reporting
    UFUNCTION(BlueprintCallable, Category = "Reporting")
    FDir_ProductionMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Reporting")
    FString GenerateStatusReport() const;

    UFUNCTION(BlueprintCallable, Category = "Reporting")
    void LogCycleCompletion(int32 CycleNumber, const FString& Summary);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production", meta = (AllowPrivateAccess = "true"))
    TArray<FDir_AgentTask> ActiveTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production", meta = (AllowPrivateAccess = "true"))
    FDir_ProductionMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production", meta = (AllowPrivateAccess = "true"))
    TMap<int32, FString> AgentNames;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production", meta = (AllowPrivateAccess = "true"))
    bool bProductionActive = false;

private:
    void InitializeAgentNames();
    void ValidateAgentDependencies();
    void CheckCriticalPath();
    void UpdateFrameRateMetrics();
};

#include "ProductionCoordinator.generated.h"