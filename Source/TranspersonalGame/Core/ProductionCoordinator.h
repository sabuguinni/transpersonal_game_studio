#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "ProductionCoordinator.generated.h"

/**
 * Studio Director's Production Coordinator
 * Manages agent task distribution and milestone tracking
 * Ensures proper execution order and dependency resolution
 */

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Working     UMETA(DisplayName = "Working"), 
    Completed   UMETA(DisplayName = "Completed"),
    Blocked     UMETA(DisplayName = "Blocked"),
    Failed      UMETA(DisplayName = "Failed")
};

UENUM(BlueprintType)
enum class EDir_MilestonePhase : uint8
{
    Planning        UMETA(DisplayName = "Planning"),
    Foundation      UMETA(DisplayName = "Foundation"),
    Implementation  UMETA(DisplayName = "Implementation"),
    Integration     UMETA(DisplayName = "Integration"),
    Testing         UMETA(DisplayName = "Testing"),
    Complete        UMETA(DisplayName = "Complete")
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    int32 AgentID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    EDir_AgentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    TArray<int32> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    float Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FDateTime StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FDateTime CompletionTime;

    FDir_AgentTask()
    {
        AgentID = 0;
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Status = EDir_AgentStatus::Idle;
        Priority = 1.0f;
        StartTime = FDateTime::Now();
        CompletionTime = FDateTime::MinValue();
    }
};

USTRUCT(BlueprintType)
struct FDir_MilestoneProgress
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString MilestoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    EDir_MilestonePhase Phase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FDir_AgentTask> Tasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    float CompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FString> BlockingIssues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FDateTime TargetDate;

    FDir_MilestoneProgress()
    {
        MilestoneName = TEXT("");
        Phase = EDir_MilestonePhase::Planning;
        CompletionPercentage = 0.0f;
        TargetDate = FDateTime::Now() + FTimespan::FromDays(7);
    }
};

/**
 * Production Coordinator - Studio Director's central command system
 * Coordinates all 18 agents for efficient game development
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDir_ProductionCoordinator : public UObject
{
    GENERATED_BODY()

public:
    UDir_ProductionCoordinator();

    // Core coordination functions
    UFUNCTION(BlueprintCallable, Category = "Production Coordinator")
    void InitializeProductionPipeline();

    UFUNCTION(BlueprintCallable, Category = "Production Coordinator")
    void UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Production Coordinator")
    bool CanAgentStart(int32 AgentID);

    UFUNCTION(BlueprintCallable, Category = "Production Coordinator")
    TArray<FDir_AgentTask> GetNextAvailableTasks();

    // Milestone management
    UFUNCTION(BlueprintCallable, Category = "Production Coordinator")
    void SetCurrentMilestone(const FString& MilestoneName, EDir_MilestonePhase Phase);

    UFUNCTION(BlueprintCallable, Category = "Production Coordinator")
    FDir_MilestoneProgress GetCurrentMilestone() const;

    UFUNCTION(BlueprintCallable, Category = "Production Coordinator")
    void AddBlockingIssue(const FString& Issue);

    UFUNCTION(BlueprintCallable, Category = "Production Coordinator")
    void ResolveBlockingIssue(const FString& Issue);

    // Critical production checks
    UFUNCTION(BlueprintCallable, Category = "Production Coordinator")
    bool IsCompilationGreen();

    UFUNCTION(BlueprintCallable, Category = "Production Coordinator")
    int32 GetHeadersWithoutCPP();

    UFUNCTION(BlueprintCallable, Category = "Production Coordinator")
    TArray<FString> GetDuplicateActors();

    // Agent chain management
    UFUNCTION(BlueprintCallable, Category = "Production Coordinator")
    void StartAgentChain();

    UFUNCTION(BlueprintCallable, Category = "Production Coordinator")
    void CompleteAgentTask(int32 AgentID, const FString& Output);

    UFUNCTION(BlueprintCallable, Category = "Production Coordinator")
    FString GetProductionReport();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production State")
    FDir_MilestoneProgress CurrentMilestone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production State")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production State")
    TMap<int32, FString> AgentOutputs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production State")
    bool bCompilationStatus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production State")
    int32 HeadersWithoutCPPCount;

private:
    void InitializeMilestone1();
    void SetupAgentDependencies();
    void ValidateProductionState();
    float CalculateMilestoneProgress();
};