#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "ProductionCoordinator.generated.h"

/**
 * Studio Director Production Coordinator
 * Manages the overall production pipeline and agent task coordination
 * Tracks milestone progress and ensures proper build integration
 */

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDir_ProductionMilestone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString MilestoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    bool bCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString ResponsibleAgent;

    FDir_ProductionMilestone()
    {
        MilestoneName = TEXT("");
        Description = TEXT("");
        bCompleted = false;
        Priority = 1;
        ResponsibleAgent = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDir_AgentTaskStatus
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString CurrentTask;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float CompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    bool bBlocked;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString BlockingReason;

    FDir_AgentTaskStatus()
    {
        AgentName = TEXT("");
        CurrentTask = TEXT("");
        CompletionPercentage = 0.0f;
        bBlocked = false;
        BlockingReason = TEXT("");
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

    // Production milestone tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production|Milestones")
    TArray<FDir_ProductionMilestone> ProductionMilestones;

    // Agent task coordination
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production|Agents")
    TArray<FDir_AgentTaskStatus> AgentTasks;

    // Visual components for editor visualization
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* CoordinatorMesh;

    // Production metrics
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production|Metrics")
    int32 CurrentCycle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production|Metrics")
    float OverallProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production|Metrics")
    int32 CompletedMilestones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production|Metrics")
    int32 TotalMilestones;

    // Production coordination functions
    UFUNCTION(BlueprintCallable, Category = "Production")
    void InitializeProductionPipeline();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateMilestoneStatus(const FString& MilestoneName, bool bCompleted);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateAgentTask(const FString& AgentName, const FString& TaskDescription, float Completion);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void SetAgentBlocked(const FString& AgentName, const FString& Reason);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void ClearAgentBlock(const FString& AgentName);

    UFUNCTION(BlueprintCallable, Category = "Production")
    float CalculateOverallProgress();

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FString> GetBlockedAgents();

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FString> GetCriticalTasks();

    // Editor-only functions for development
    UFUNCTION(CallInEditor, Category = "Production|Debug")
    void LogProductionStatus();

    UFUNCTION(CallInEditor, Category = "Production|Debug")
    void ResetAllMilestones();

    UFUNCTION(CallInEditor, Category = "Production|Debug")
    void ValidateProductionPipeline();

private:
    void SetupDefaultMilestones();
    void SetupDefaultAgentTasks();
    void UpdateVisualIndicators();
};