#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "ProductionCoordinator.generated.h"

UENUM(BlueprintType)
enum class EDir_MilestoneStatus : uint8
{
    NotStarted      UMETA(DisplayName = "Not Started"),
    InProgress      UMETA(DisplayName = "In Progress"),
    Completed       UMETA(DisplayName = "Completed"),
    Blocked         UMETA(DisplayName = "Blocked"),
    Failed          UMETA(DisplayName = "Failed")
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    int32 AgentID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_MilestoneStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    TArray<FString> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    TArray<FString> Deliverables;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FDateTime LastUpdate;

    FDir_AgentTask()
    {
        AgentID = 0;
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Status = EDir_MilestoneStatus::NotStarted;
        Priority = 1.0f;
        LastUpdate = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct FDir_MilestoneData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString MilestoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    EDir_MilestoneStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FDir_AgentTask> Tasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    float CompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FDateTime TargetDate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FDateTime LastUpdate;

    FDir_MilestoneData()
    {
        MilestoneName = TEXT("");
        Description = TEXT("");
        Status = EDir_MilestoneStatus::NotStarted;
        CompletionPercentage = 0.0f;
        TargetDate = FDateTime::Now();
        LastUpdate = FDateTime::Now();
    }
};

/**
 * Production Coordinator - Sistema central de coordenação de produção
 * Monitoriza o progresso dos 18 agentes especializados e coordena o Milestone 1
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UProductionCoordinator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UProductionCoordinator();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Milestone management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void InitializeMilestone1();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateAgentTask(int32 AgentID, const FString& TaskDescription, EDir_MilestoneStatus Status, const TArray<FString>& Deliverables);

    UFUNCTION(BlueprintCallable, Category = "Production")
    float GetMilestone1Progress() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_AgentTask> GetBlockedTasks() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_AgentTask> GetHighPriorityTasks() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    FString GenerateProductionReport() const;

    // Agent coordination
    UFUNCTION(BlueprintCallable, Category = "Production")
    bool IsAgentReady(int32 AgentID) const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<int32> GetNextAgentsToExecute() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    void MarkAgentCompleted(int32 AgentID, const TArray<FString>& OutputFiles);

    // Critical path analysis
    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FString> GetCriticalPath() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool HasCriticalBlockers() const;

    // Development validation
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Production")
    void ValidateCurrentState();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Production")
    void GenerateAgentInstructions();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Production")
    FDir_MilestoneData Milestone1Data;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Production")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Production")
    TArray<int32> CompletedAgents;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Production")
    TArray<FString> CriticalBlockers;

private:
    void SetupAgentTasks();
    void CalculateMilestoneProgress();
    void UpdateCriticalPath();
    FString GetAgentName(int32 AgentID) const;
    bool CheckAgentDependencies(int32 AgentID) const;
};