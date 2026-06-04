#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "Dir_ProductionCoordinator.generated.h"

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Working UMETA(DisplayName = "Working"),
    Completed UMETA(DisplayName = "Completed"),
    Blocked UMETA(DisplayName = "Blocked"),
    Failed UMETA(DisplayName = "Failed")
};

UENUM(BlueprintType)
enum class EDir_MilestoneType : uint8
{
    WalkAround UMETA(DisplayName = "Walk Around Prototype"),
    BasicSurvival UMETA(DisplayName = "Basic Survival"),
    DinosaurAI UMETA(DisplayName = "Dinosaur AI"),
    WorldGeneration UMETA(DisplayName = "World Generation"),
    FullPrototype UMETA(DisplayName = "Full Prototype")
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    int32 AgentNumber;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString CurrentTask;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_AgentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float ProgressPercent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FDateTime LastUpdate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    TArray<FString> Deliverables;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    TArray<FString> Dependencies;

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
struct FDir_MilestoneProgress
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    EDir_MilestoneType MilestoneType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString MilestoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    float CompletionPercent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<int32> RequiredAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FString> RequiredDeliverables;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    bool bIsBlocked;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString BlockingReason;

    FDir_MilestoneProgress()
    {
        MilestoneType = EDir_MilestoneType::WalkAround;
        MilestoneName = TEXT("");
        CompletionPercent = 0.0f;
        bIsBlocked = false;
        BlockingReason = TEXT("");
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDir_ProductionCoordinator : public UActorComponent
{
    GENERATED_BODY()

public:
    UDir_ProductionCoordinator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Agent Management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void InitializeAgents();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateAgentStatus(int32 AgentNumber, EDir_AgentStatus NewStatus, const FString& TaskDescription);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void AssignTaskToAgent(int32 AgentNumber, const FString& TaskDescription, const TArray<FString>& Dependencies);

    UFUNCTION(BlueprintCallable, Category = "Production")
    FDir_AgentTask GetAgentStatus(int32 AgentNumber);

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_AgentTask> GetAllAgentStatuses();

    // Milestone Management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void InitializeMilestones();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateMilestoneProgress(EDir_MilestoneType MilestoneType, float NewProgress);

    UFUNCTION(BlueprintCallable, Category = "Production")
    FDir_MilestoneProgress GetMilestoneProgress(EDir_MilestoneType MilestoneType);

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_MilestoneProgress> GetAllMilestones();

    // Production Flow Control
    UFUNCTION(BlueprintCallable, Category = "Production")
    bool CanAgentProceed(int32 AgentNumber);

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<int32> GetBlockedAgents();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void ResolveAgentBlock(int32 AgentNumber, const FString& Resolution);

    // Critical Path Analysis
    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<int32> GetCriticalPathAgents();

    UFUNCTION(BlueprintCallable, Category = "Production")
    float GetOverallProjectProgress();

    // Emergency Controls
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Production")
    void EmergencyResetAllAgents();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Production")
    void ForceUnblockAgent(int32 AgentNumber);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Production")
    void GenerateProductionReport();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production", meta = (AllowPrivateAccess = "true"))
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production", meta = (AllowPrivateAccess = "true"))
    TArray<FDir_MilestoneProgress> Milestones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production", meta = (AllowPrivateAccess = "true"))
    float ProductionUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production", meta = (AllowPrivateAccess = "true"))
    bool bAutoResolveBlocks;

private:
    float LastUpdateTime;
    
    void ValidateAgentDependencies();
    void CheckMilestoneCompletion();
    void LogProductionMetrics();
};