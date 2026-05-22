#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "../SharedTypes.h"
#include "ProductionCoordinator.generated.h"

/**
 * Studio Director Production Coordinator
 * Manages the 19-agent production pipeline for Transpersonal Game Studio
 * Tracks agent progress, validates deliverables, and ensures milestone completion
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
    Development     UMETA(DisplayName = "Development"),
    Testing         UMETA(DisplayName = "Testing"),
    Integration     UMETA(DisplayName = "Integration"),
    Complete        UMETA(DisplayName = "Complete")
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    int32 AgentNumber;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString CurrentTask;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    EDir_AgentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    float ProgressPercent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FDateTime LastUpdate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    TArray<FString> Deliverables;

    FDir_AgentTask()
    {
        AgentNumber = 0;
        AgentName = TEXT("");
        CurrentTask = TEXT("");
        Status = EDir_AgentStatus::Idle;
        ProgressPercent = 0.0f;
        LastUpdate = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMilestone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString MilestoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    EDir_MilestonePhase Phase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<int32> RequiredAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FString> Criteria;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    bool bIsComplete;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FDateTime TargetDate;

    FDir_ProductionMilestone()
    {
        MilestoneName = TEXT("");
        Phase = EDir_MilestonePhase::Planning;
        bIsComplete = false;
        TargetDate = FDateTime::Now();
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AProductionCoordinator : public AActor
{
    GENERATED_BODY()

public:
    AProductionCoordinator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Agent Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_ProductionMilestone> Milestones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString CurrentCycleID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 TotalCyclesCompleted;

    // Core Production Functions
    UFUNCTION(BlueprintCallable, Category = "Production")
    void InitializeProductionPipeline();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateAgentStatus(int32 AgentNumber, EDir_AgentStatus NewStatus, float Progress);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void AddAgentDeliverable(int32 AgentNumber, const FString& Deliverable);

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool ValidateMilestoneCompletion(const FString& MilestoneName);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void DispatchTaskToNextAgent();

    // Milestone Management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void CreateWalkAroundMilestone();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void CreatePlayablePrototypeMilestone();

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool CheckMinimumViablePrototype();

    // Quality Assurance
    UFUNCTION(BlueprintCallable, Category = "Production")
    bool ValidateCompilation();

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool ValidateGameplayElements();

    UFUNCTION(BlueprintCallable, Category = "Production")
    int32 CountVisibleActorsInLevel();

    // Reporting
    UFUNCTION(BlueprintCallable, Category = "Production")
    FString GenerateProductionReport();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void LogCycleCompletion();

private:
    // Internal tracking
    float LastUpdateTime;
    bool bPipelineInitialized;
    
    void SetupAgentHierarchy();
    void ValidateAgentDependencies();
    FDir_AgentTask* FindAgentTask(int32 AgentNumber);
};