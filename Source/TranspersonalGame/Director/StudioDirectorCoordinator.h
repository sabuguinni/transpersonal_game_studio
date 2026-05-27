#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "StudioDirectorCoordinator.generated.h"

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Working         UMETA(DisplayName = "Working"), 
    Completed       UMETA(DisplayName = "Completed"),
    Blocked         UMETA(DisplayName = "Blocked"),
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
    TArray<FString> Deliverables;

    FDir_AgentTask()
    {
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Status = EDir_AgentStatus::Idle;
        Priority = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionCycle
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cycle")
    FString CycleID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cycle")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cycle")
    float CycleStartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cycle")
    bool bIsCompleted;

    FDir_ProductionCycle()
    {
        CycleID = TEXT("");
        CycleStartTime = 0.0f;
        bIsCompleted = false;
    }
};

/**
 * Studio Director Coordinator - Manages the 19-agent production pipeline
 * Coordinates task distribution, dependency tracking, and milestone validation
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UStudioDirectorCoordinator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UStudioDirectorCoordinator();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Production Cycle Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void StartProductionCycle(const FString& CycleID);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CompleteProductionCycle();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FDir_ProductionCycle GetCurrentCycle() const { return CurrentCycle; }

    // Agent Task Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void AssignTaskToAgent(const FString& AgentName, const FString& TaskDescription, float Priority = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateAgentStatus(const FString& AgentName, EDir_AgentStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void AddAgentDeliverable(const FString& AgentName, const FString& Deliverable);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_AgentTask> GetAgentTasks() const { return CurrentCycle.AgentTasks; }

    // Milestone Validation
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool ValidatePlayablePrototype();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void GenerateProductionReport();

    // Priority Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void SetMilestone1Priorities();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Studio Director")
    FDir_ProductionCycle CurrentCycle;

    UPROPERTY(BlueprintReadOnly, Category = "Studio Director")
    TArray<FString> AgentNames;

    UPROPERTY(BlueprintReadOnly, Category = "Studio Director")
    bool bMilestone1Completed;

private:
    void InitializeAgentList();
    void ValidateAgentDependencies();
    FDir_AgentTask* FindAgentTask(const FString& AgentName);
};