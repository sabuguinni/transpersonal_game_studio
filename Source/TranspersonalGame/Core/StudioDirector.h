#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "SharedTypes.h"
#include "StudioDirector.generated.h"

/**
 * Studio Director - Agent #1
 * Coordinates the entire production pipeline and manages agent task distribution.
 * Ensures all agents follow the gameplay-first directive and milestone targets.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AStudioDirector : public AActor
{
    GENERATED_BODY()

public:
    AStudioDirector();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core production management
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Production")
    class USceneComponent* RootSceneComponent;

    // Agent coordination system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Coordination")
    TArray<FString> ActiveAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Coordination")
    TArray<FString> CompletedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Coordination")
    TArray<FString> BlockedTasks;

    // Milestone tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestones")
    bool bMilestone1_WalkAround;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestones")
    bool bCharacterMovementReady;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestones")
    bool bLandscapeReady;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestones")
    bool bDinosaursPlaced;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestones")
    bool bLightingSetup;

    // Production metrics
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Metrics")
    int32 TotalHeaderFiles;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Metrics")
    int32 TotalCppFiles;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Metrics")
    int32 PhantomHeaders;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Metrics")
    float CompilationStatus;

public:
    // Agent task management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void AssignTaskToAgent(const FString& AgentName, const FString& TaskDescription);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void MarkTaskCompleted(const FString& AgentName, const FString& TaskDescription);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void BlockTask(const FString& AgentName, const FString& Reason);

    // Milestone validation
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool ValidateMilestone1();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CheckProductionHealth();

    // Agent coordination
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CoordinateAgentPipeline();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FString GetNextPriorityTask();

    // Production oversight
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void EnforceGameplayFirstDirective();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void ValidateAgentOutputs();

    // Critical system checks
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CheckForDuplicateActors();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void ValidateBiomeDistribution();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void AssessCompilationStatus();

private:
    // Internal production tracking
    float LastProductionCheck;
    TArray<FString> CriticalIssues;
    TMap<FString, float> AgentPerformanceMetrics;

    // Helper functions
    void LogProductionStatus();
    void UpdateMilestoneProgress();
    bool IsAgentOutputValid(const FString& AgentName);
    void EscalateCriticalIssue(const FString& Issue);
};