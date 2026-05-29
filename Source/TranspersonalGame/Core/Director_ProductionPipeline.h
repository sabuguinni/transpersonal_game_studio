#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Director_ProductionPipeline.generated.h"

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction   UMETA(DisplayName = "Pre-Production"),
    CoreSystems     UMETA(DisplayName = "Core Systems"),
    WorldBuilding   UMETA(DisplayName = "World Building"),
    CharacterDev    UMETA(DisplayName = "Character Development"),
    GameplayLoop    UMETA(DisplayName = "Gameplay Loop"),
    Polish          UMETA(DisplayName = "Polish & Testing"),
    Release         UMETA(DisplayName = "Release Ready")
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_AgentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    TArray<FString> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString ExpectedOutput;

    FDir_AgentTask()
    {
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Status = EDir_AgentStatus::Idle;
        Priority = 1.0f;
        ExpectedOutput = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct FDir_MilestoneData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString MilestoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    EDir_ProductionPhase Phase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FDir_AgentTask> RequiredTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    bool bIsComplete;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    float CompletionPercentage;

    FDir_MilestoneData()
    {
        MilestoneName = TEXT("");
        Phase = EDir_ProductionPhase::PreProduction;
        bIsComplete = false;
        CompletionPercentage = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDir_ProductionPipeline : public UActorComponent
{
    GENERATED_BODY()

public:
    UDir_ProductionPipeline();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Pipeline")
    EDir_ProductionPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Pipeline")
    TArray<FDir_MilestoneData> ProductionMilestones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Pipeline")
    TArray<FDir_AgentTask> ActiveTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Pipeline")
    int32 CurrentCycleNumber;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Pipeline")
    float CycleBudgetUsed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Pipeline")
    float CycleBudgetLimit;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Production Pipeline")
    void InitializeProductionPipeline();

    UFUNCTION(BlueprintCallable, Category = "Production Pipeline")
    void StartNewCycle();

    UFUNCTION(BlueprintCallable, Category = "Production Pipeline")
    void AssignTaskToAgent(const FString& AgentName, const FString& TaskDescription, float Priority);

    UFUNCTION(BlueprintCallable, Category = "Production Pipeline")
    void CompleteAgentTask(const FString& AgentName, const FString& Output);

    UFUNCTION(BlueprintCallable, Category = "Production Pipeline")
    void BlockAgentTask(const FString& AgentName, const FString& Reason);

    UFUNCTION(BlueprintCallable, Category = "Production Pipeline")
    bool CheckMilestoneCompletion(const FString& MilestoneName);

    UFUNCTION(BlueprintCallable, Category = "Production Pipeline")
    void AdvanceToNextPhase();

    UFUNCTION(BlueprintCallable, Category = "Production Pipeline")
    TArray<FDir_AgentTask> GetCriticalPathTasks();

    UFUNCTION(BlueprintCallable, Category = "Production Pipeline")
    float CalculateOverallProgress();

    UFUNCTION(BlueprintCallable, Category = "Production Pipeline")
    void GenerateProductionReport();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Production Pipeline")
    void CreateMinimalPlayablePrototype();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Production Pipeline")
    void ValidateCurrentBuild();

private:
    void SetupMilestone1WalkAround();
    void SetupCoreSystemsMilestones();
    void ValidateAgentDependencies();
    void UpdateCriticalPath();
    void LogProductionMetrics();
};

#include "Director_ProductionPipeline.generated.h"