#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "ProductionCoordinator.generated.h"

/**
 * Studio Director's Production Coordination System
 * Tracks development progress, agent deliverables, and milestone completion
 * Used for coordinating the 19-agent development pipeline
 */

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
    bool bIsCompleted = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString DeliverableType; // "CODE", "ASSET", "UE5_COMMAND", "BLUEPRINT"

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float Priority = 1.0f; // 1.0 = normal, 2.0 = high, 0.5 = low

    FDir_AgentTask()
    {
        AgentNumber = 0;
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        bIsCompleted = false;
        DeliverableType = TEXT("CODE");
        Priority = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct FDir_MilestoneStatus
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString MilestoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    bool bPlayableCharacter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    bool bTerrainExists = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    bool bDinosaursPlaced = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    bool bLightingSetup = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    bool bSurvivalHUD = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    float CompletionPercentage = 0.0f;

    FDir_MilestoneStatus()
    {
        MilestoneName = TEXT("Milestone 1 - Walk Around");
        bPlayableCharacter = false;
        bTerrainExists = false;
        bDinosaursPlaced = false;
        bLightingSetup = false;
        bSurvivalHUD = false;
        CompletionPercentage = 0.0f;
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

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    // Production tracking data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FDir_MilestoneStatus CurrentMilestone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 CurrentCycle = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString ProductionPhase = TEXT("MILESTONE_1_WALKABOUT");

public:
    virtual void Tick(float DeltaTime) override;

    // Production coordination functions
    UFUNCTION(BlueprintCallable, Category = "Production")
    void InitializeAgentTasks();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateMilestoneStatus();

    UFUNCTION(BlueprintCallable, Category = "Production")
    float GetOverallProgress() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    void MarkTaskCompleted(int32 AgentNumber, const FString& DeliverableType);

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_AgentTask> GetPendingTasks() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool IsMilestoneComplete() const;

    // Debug and testing functions
    UFUNCTION(CallInEditor, Category = "Debug")
    void DebugPrintProductionStatus();

    UFUNCTION(CallInEditor, Category = "Debug")
    void ResetProductionState();

private:
    void CreateAgentTaskList();
    void AssessLevelState();
    int32 CountActorsByType(const FString& ActorType) const;
};