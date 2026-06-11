#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "ProductionCoordinator.generated.h"

/**
 * Production milestone tracking structure
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDir_ProductionMilestone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString MilestoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    bool bIsComplete;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    float CompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FString> Requirements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FString> CompletedRequirements;

    FDir_ProductionMilestone()
    {
        MilestoneName = TEXT("Unnamed Milestone");
        bIsComplete = false;
        CompletionPercentage = 0.0f;
    }
};

/**
 * Agent task tracking structure
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    int32 AgentID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FString CurrentTask;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    float TaskProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    bool bTaskComplete;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FString LastOutput;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    int32 CycleCount;

    FDir_AgentTask()
    {
        AgentName = TEXT("Unknown Agent");
        AgentID = 0;
        CurrentTask = TEXT("No Task Assigned");
        TaskProgress = 0.0f;
        bTaskComplete = false;
        LastOutput = TEXT("");
        CycleCount = 0;
    }
};

/**
 * Production metrics structure
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalActorsInLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 DinosaurCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CharacterCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 EnvironmentActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float OverallCompletion;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 SystemsImplemented;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalSystems;

    FDir_ProductionMetrics()
    {
        TotalActorsInLevel = 0;
        DinosaurCount = 0;
        CharacterCount = 0;
        EnvironmentActorCount = 0;
        OverallCompletion = 0.0f;
        SystemsImplemented = 0;
        TotalSystems = 19;
    }
};

/**
 * Studio Director Production Coordinator
 * Manages the entire production pipeline and coordinates all 19 agents
 */
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

    // Production Milestones
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_ProductionMilestone> ProductionMilestones;

    // Agent Tasks
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_AgentTask> AgentTasks;

    // Current Production Metrics
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FDir_ProductionMetrics CurrentMetrics;

    // Current Cycle Information
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 CurrentCycle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString CurrentCycleID;

    // Production State
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    bool bProductionActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float ProductionStartTime;

    // Blueprint callable functions
    UFUNCTION(BlueprintCallable, Category = "Production", CallInEditor)
    void InitializeProduction();

    UFUNCTION(BlueprintCallable, Category = "Production", CallInEditor)
    void UpdateProductionMetrics();

    UFUNCTION(BlueprintCallable, Category = "Production", CallInEditor)
    void ValidateMilestone1();

    UFUNCTION(BlueprintCallable, Category = "Production", CallInEditor)
    void AssignAgentTasks();

    UFUNCTION(BlueprintCallable, Category = "Production", CallInEditor)
    void GenerateProductionReport();

    UFUNCTION(BlueprintCallable, Category = "Production")
    float GetOverallProgress() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool IsMilestone1Complete() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    FString GetCurrentProductionStatus() const;

    // Agent coordination functions
    UFUNCTION(BlueprintCallable, Category = "Agents")
    void UpdateAgentTask(int32 AgentID, const FString& TaskDescription, float Progress);

    UFUNCTION(BlueprintCallable, Category = "Agents")
    FDir_AgentTask GetAgentTask(int32 AgentID) const;

    UFUNCTION(BlueprintCallable, Category = "Agents")
    TArray<FDir_AgentTask> GetIncompleteAgentTasks() const;

private:
    // Internal helper functions
    void SetupMilestone1Requirements();
    void CountLevelActors();
    void ValidateGameSystems();
    void CalculateCompletionPercentage();

    // Milestone validation helpers
    bool ValidateCharacterMovement() const;
    bool ValidateTerrainSystem() const;
    bool ValidateDinosaurPlaceholders() const;
    bool ValidateLightingSystem() const;
    bool ValidateGameMode() const;
};