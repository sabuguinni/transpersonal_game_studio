#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "ProductionCoordinator.generated.h"

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Working UMETA(DisplayName = "Working"), 
    Completed UMETA(DisplayName = "Completed"),
    Blocked UMETA(DisplayName = "Blocked"),
    Failed UMETA(DisplayName = "Failed")
};

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction UMETA(DisplayName = "Pre-Production"),
    CoreSystems UMETA(DisplayName = "Core Systems"),
    ContentCreation UMETA(DisplayName = "Content Creation"),
    Integration UMETA(DisplayName = "Integration"),
    Testing UMETA(DisplayName = "Testing"),
    Polish UMETA(DisplayName = "Polish")
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    EDir_AgentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    float Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    int32 CycleAssigned;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    int32 CycleCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    TArray<FString> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    TArray<FString> Deliverables;

    FDir_AgentTask()
    {
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Status = EDir_AgentStatus::Idle;
        Priority = 1.0f;
        CycleAssigned = 0;
        CycleCompleted = 0;
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 TotalActorsInLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 PlayableCharacters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 DinosaurActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 EnvironmentActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    float TerrainCoverage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    bool bHasWorkingGameMode;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    bool bHasPlayerController;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    bool bHasLighting;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    float OverallCompletionPercentage;

    FDir_ProductionMetrics()
    {
        TotalActorsInLevel = 0;
        PlayableCharacters = 0;
        DinosaurActors = 0;
        EnvironmentActors = 0;
        TerrainCoverage = 0.0f;
        bHasWorkingGameMode = false;
        bHasPlayerController = false;
        bHasLighting = false;
        OverallCompletionPercentage = 0.0f;
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production State")
    EDir_ProductionPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production State")
    int32 CurrentCycle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production State")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production State")
    FDir_ProductionMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production State")
    TArray<FString> CompletedMilestones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production State")
    TArray<FString> BlockingIssues;

public:
    virtual void Tick(float DeltaTime) override;

    // Production Management Functions
    UFUNCTION(BlueprintCallable, Category = "Production Management")
    void InitializeProductionCycle(int32 CycleNumber);

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    void AssignTaskToAgent(const FString& AgentName, const FString& TaskDescription, float Priority = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    void CompleteAgentTask(const FString& AgentName, const TArray<FString>& Deliverables);

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    void BlockAgentTask(const FString& AgentName, const FString& BlockingReason);

    // Metrics and Analysis
    UFUNCTION(BlueprintCallable, Category = "Production Analysis")
    FDir_ProductionMetrics CalculateCurrentMetrics();

    UFUNCTION(BlueprintCallable, Category = "Production Analysis")
    float GetOverallProgress() const;

    UFUNCTION(BlueprintCallable, Category = "Production Analysis")
    TArray<FString> GetActiveAgents() const;

    UFUNCTION(BlueprintCallable, Category = "Production Analysis")
    TArray<FString> GetBlockedAgents() const;

    UFUNCTION(BlueprintCallable, Category = "Production Analysis")
    bool ValidateMinimumPlayableState() const;

    // Milestone Validation
    UFUNCTION(BlueprintCallable, Category = "Milestone Validation")
    bool CheckMilestone_WalkAround() const;

    UFUNCTION(BlueprintCallable, Category = "Milestone Validation")
    bool CheckMilestone_BasicSurvival() const;

    UFUNCTION(BlueprintCallable, Category = "Milestone Validation")
    bool CheckMilestone_DinosaurEncounters() const;

    // Agent Coordination
    UFUNCTION(BlueprintCallable, Category = "Agent Coordination")
    void CoordinateAgentSequence();

    UFUNCTION(BlueprintCallable, Category = "Agent Coordination")
    FString GetNextPriorityAgent() const;

    UFUNCTION(BlueprintCallable, Category = "Agent Coordination")
    void ReportProductionStatus() const;

    // Debug and Development
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Development")
    void DebugPrintProductionState();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Development")
    void ForceRecalculateMetrics();

private:
    void UpdateProductionPhase();
    void ValidateAgentDependencies();
    int32 CountActorsByClass(UClass* ActorClass) const;
    bool HasRequiredGameSystems() const;
};