#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Dir_ProductionCoordinator.generated.h"

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle,
    Working,
    Completed,
    Blocked,
    Failed
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDir_AgentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ProgressPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString LastOutput;

    FDir_AgentTask()
    {
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Status = EDir_AgentStatus::Idle;
        ProgressPercentage = 0.0f;
        LastOutput = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMilestone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString MilestoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> RequiredAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime Deadline;

    FDir_ProductionMilestone()
    {
        MilestoneName = TEXT("");
        bIsCompleted = false;
        Deadline = FDateTime::Now();
    }
};

UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
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
    void RegisterAgent(const FString& AgentName, const FString& InitialTask);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateAgentStatus(const FString& AgentName, EDir_AgentStatus NewStatus, float Progress = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void AssignTaskToAgent(const FString& AgentName, const FString& TaskDescription);

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_AgentTask> GetAllAgentTasks() const;

    // Milestone Management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void CreateMilestone(const FString& MilestoneName, const TArray<FString>& RequiredAgents, const FDateTime& Deadline);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void CheckMilestoneCompletion(const FString& MilestoneName);

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_ProductionMilestone> GetActiveMilestones() const;

    // Production Metrics
    UFUNCTION(BlueprintCallable, Category = "Production")
    float GetOverallProgress() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    int32 GetActiveAgentCount() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    FString GenerateProductionReport() const;

    // Critical Path Analysis
    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FString> GetBlockedAgents() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    void ResolveAgentBlockage(const FString& AgentName, const FString& Resolution);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Production")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Production")
    TArray<FDir_ProductionMilestone> ProductionMilestones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float ProductionUpdateInterval;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Production")
    FDateTime LastProductionUpdate;

private:
    void InitializeDefaultAgents();
    void UpdateProductionMetrics();
    bool ValidateAgentDependencies(const FString& AgentName);
    void LogProductionEvent(const FString& EventDescription);
};