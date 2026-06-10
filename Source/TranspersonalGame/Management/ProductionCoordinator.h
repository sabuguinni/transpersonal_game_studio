#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "ProductionCoordinator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDir_MilestoneStatus
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString MilestoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    bool bIsComplete;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    float CompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString AssignedAgent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString LastUpdate;

    FDir_MilestoneStatus()
    {
        MilestoneName = TEXT("");
        bIsComplete = false;
        CompletionPercentage = 0.0f;
        AssignedAgent = TEXT("");
        LastUpdate = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    int32 Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString Status;

    FDir_AgentTask()
    {
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Priority = 0;
        bIsActive = false;
        Status = TEXT("Pending");
    }
};

/**
 * Production Coordinator - Studio Director's main coordination system
 * Tracks milestones, manages agent tasks, and monitors production progress
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AProductionCoordinator : public AActor
{
    GENERATED_BODY()

public:
    AProductionCoordinator();

protected:
    virtual void BeginPlay() override;

    // Milestone tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_MilestoneStatus> Milestones;

    // Agent task management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_AgentTask> AgentTasks;

    // Production metrics
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 CurrentCycle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float OverallProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString CurrentPhase;

    // Visual debug components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debug")
    class UStaticMeshComponent* DebugMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debug")
    class UTextRenderComponent* StatusText;

public:
    virtual void Tick(float DeltaTime) override;

    // Milestone management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateMilestone(const FString& MilestoneName, bool bComplete, float Percentage);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void AddMilestone(const FString& MilestoneName, const FString& AssignedAgent);

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool IsMilestoneComplete(const FString& MilestoneName);

    // Agent task management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void AssignTask(const FString& AgentName, const FString& TaskDescription, int32 Priority);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void CompleteTask(const FString& AgentName, const FString& TaskDescription);

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_AgentTask> GetActiveTasksForAgent(const FString& AgentName);

    // Production metrics
    UFUNCTION(BlueprintCallable, Category = "Production")
    void AdvanceCycle();

    UFUNCTION(BlueprintCallable, Category = "Production")
    float CalculateOverallProgress();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void GenerateProductionReport();

    // Debug and visualization
    UFUNCTION(CallInEditor, Category = "Debug")
    void InitializeMilestone1Tasks();

    UFUNCTION(CallInEditor, Category = "Debug")
    void RefreshStatusDisplay();

private:
    void UpdateStatusText();
    void InitializeDefaultMilestones();
};

#include "ProductionCoordinator.generated.h"