#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "DirectorCoordinator.generated.h"

/**
 * Studio Director Coordinator - Central command system for managing agent outputs and production pipeline
 * Tracks compilation status, coordinates agent deliverables, and ensures milestone progression
 */

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDir_AgentStatus
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Status")
    int32 AgentID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Status")
    FString AgentName = "";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Status")
    bool bHasCompletedCycle = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Status")
    int32 FilesCreated = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Status")
    int32 UE5CommandsExecuted = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Status")
    FString LastError = "";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Status")
    float CycleCompletionTime = 0.0f;

    FDir_AgentStatus()
    {
        AgentID = 0;
        AgentName = "";
        bHasCompletedCycle = false;
        FilesCreated = 0;
        UE5CommandsExecuted = 0;
        LastError = "";
        CycleCompletionTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDir_CompilationStatus
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compilation")
    bool bCompilationSuccessful = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compilation")
    int32 ErrorCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compilation")
    int32 WarningCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compilation")
    TArray<FString> ErrorMessages;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compilation")
    TArray<FString> AffectedFiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compilation")
    float CompilationTime = 0.0f;

    FDir_CompilationStatus()
    {
        bCompilationSuccessful = false;
        ErrorCount = 0;
        WarningCount = 0;
        CompilationTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDir_MilestoneProgress
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString MilestoneName = "";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    bool bCompleted = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    float ProgressPercentage = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FString> RequiredDeliverables;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FString> CompletedDeliverables;

    FDir_MilestoneProgress()
    {
        MilestoneName = "";
        bCompleted = false;
        ProgressPercentage = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADirectorCoordinator : public AActor
{
    GENERATED_BODY()

public:
    ADirectorCoordinator();

protected:
    virtual void BeginPlay() override;

    // Agent Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Director | Agents")
    TArray<FDir_AgentStatus> AgentStatuses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Director | Compilation")
    FDir_CompilationStatus CurrentCompilationStatus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Director | Milestones")
    TArray<FDir_MilestoneProgress> Milestones;

    // Critical Production Metrics
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Director | Metrics")
    int32 TotalHeadersWithoutCpp = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Director | Metrics")
    int32 DuplicateActorsInLevel = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Director | Metrics")
    int32 ActorsAtOrigin = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Director | Metrics")
    bool bMinPlayableMapReady = false;

public:
    // Core Director Functions
    UFUNCTION(BlueprintCallable, Category = "Director | Coordination")
    void InitializeProductionPipeline();

    UFUNCTION(BlueprintCallable, Category = "Director | Coordination")
    void UpdateAgentStatus(int32 AgentID, const FString& AgentName, int32 FilesCreated, int32 UE5Commands);

    UFUNCTION(BlueprintCallable, Category = "Director | Compilation")
    void CheckCompilationStatus();

    UFUNCTION(BlueprintCallable, Category = "Director | Compilation")
    void TriggerCompilation();

    UFUNCTION(BlueprintCallable, Category = "Director | Milestones")
    void UpdateMilestoneProgress(const FString& MilestoneName, float Progress);

    UFUNCTION(BlueprintCallable, Category = "Director | Milestones")
    bool IsMilestoneComplete(const FString& MilestoneName);

    // Critical Issue Detection
    UFUNCTION(BlueprintCallable, Category = "Director | Issues")
    void ScanForGhostHeaders();

    UFUNCTION(BlueprintCallable, Category = "Director | Issues")
    void ScanForDuplicateActors();

    UFUNCTION(BlueprintCallable, Category = "Director | Issues")
    void ValidateBiomeDistribution();

    // Production Pipeline Control
    UFUNCTION(BlueprintCallable, Category = "Director | Pipeline")
    bool CanAgentProceed(int32 AgentID);

    UFUNCTION(BlueprintCallable, Category = "Director | Pipeline")
    void BlockProductionOnCompilationError();

    UFUNCTION(BlueprintCallable, Category = "Director | Pipeline")
    void ResumeProduction();

    // Milestone 1 Specific Tracking
    UFUNCTION(BlueprintCallable, Category = "Director | Milestone1")
    void CheckMilestone1Progress();

    UFUNCTION(BlueprintCallable, Category = "Director | Milestone1")
    bool IsCharacterMovementWorking();

    UFUNCTION(BlueprintCallable, Category = "Director | Milestone1")
    bool AreDinosaursVisible();

    UFUNCTION(BlueprintCallable, Category = "Director | Milestone1")
    bool IsTerrainPlayable();

    // Agent Task Distribution
    UFUNCTION(BlueprintCallable, Category = "Director | Tasks")
    void AssignCriticalTasks();

    UFUNCTION(BlueprintCallable, Category = "Director | Tasks")
    FString GetNextAgentPriority(int32 AgentID);

public:
    virtual void Tick(float DeltaTime) override;

private:
    void InitializeMilestones();
    void SetupAgentStatuses();
    void LogProductionStatus();
};