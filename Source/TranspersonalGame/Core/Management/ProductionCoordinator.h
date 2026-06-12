#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "ProductionCoordinator.generated.h"

/**
 * Studio Director's Production Coordination System
 * Tracks development milestones, agent tasks, and game state metrics
 * Used for real-time production monitoring and agent task distribution
 */

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    float Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    bool bCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FDateTime AssignedTime;

    FDir_AgentTask()
    {
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Priority = 1.0f;
        bCompleted = false;
        AssignedTime = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 TotalActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 CharacterActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 DinosaurActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 TerrainActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    float OverallProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    FDateTime LastUpdated;

    FDir_ProductionMetrics()
    {
        TotalActors = 0;
        CharacterActors = 0;
        DinosaurActors = 0;
        TerrainActors = 0;
        OverallProgress = 0.0f;
        LastUpdated = FDateTime::Now();
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

    // Production Metrics
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FDir_ProductionMetrics CurrentMetrics;

    // Agent Task Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_AgentTask> AgentTasks;

    // Milestone Tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    bool bTerrainComplete;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    bool bCharacterFunctional;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    bool bDinosaursPlaced;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    bool bLightingSetup;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    bool bBasicGameplay;

    // Update frequency
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float UpdateInterval;

private:
    float TimeSinceLastUpdate;

public:
    // Core Functions
    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateProductionMetrics();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void AssignTaskToAgent(const FString& AgentName, const FString& TaskDescription, float Priority);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void CompleteAgentTask(const FString& AgentName);

    UFUNCTION(BlueprintCallable, Category = "Production")
    float CalculateOverallProgress();

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_AgentTask> GetPendingTasks();

    UFUNCTION(BlueprintCallable, Category = "Production")
    FDir_ProductionMetrics GetCurrentMetrics() const;

    // Milestone Validation
    UFUNCTION(BlueprintCallable, Category = "Production")
    void ValidateMilestones();

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool IsMilestone1Complete() const;

    // Debug Functions
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Production")
    void LogProductionState();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Production")
    void ResetAllTasks();
};