#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "Dir_ProductionCoordinator.generated.h"

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Working         UMETA(DisplayName = "Working"),
    Completed       UMETA(DisplayName = "Completed"),
    Blocked         UMETA(DisplayName = "Blocked"),
    Failed          UMETA(DisplayName = "Failed")
};

UENUM(BlueprintType)
enum class EDir_MilestoneType : uint8
{
    WalkAround      UMETA(DisplayName = "Walk Around"),
    BasicSurvival   UMETA(DisplayName = "Basic Survival"),
    DinosaurAI      UMETA(DisplayName = "Dinosaur AI"),
    FullGameplay    UMETA(DisplayName = "Full Gameplay")
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Task")
    int32 AgentID = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Task")
    FString AgentName;

    UPROPERTY(BlueprintReadOnly, Category = "Task")
    FString TaskDescription;

    UPROPERTY(BlueprintReadOnly, Category = "Task")
    EDir_AgentStatus Status = EDir_AgentStatus::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Task")
    float Priority = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Task")
    TArray<FString> Dependencies;

    UPROPERTY(BlueprintReadOnly, Category = "Task")
    TArray<FString> Deliverables;

    UPROPERTY(BlueprintReadOnly, Category = "Task")
    FDateTime StartTime;

    UPROPERTY(BlueprintReadOnly, Category = "Task")
    FDateTime CompletionTime;

    FDir_AgentTask()
    {
        StartTime = FDateTime::Now();
        CompletionTime = FDateTime::MinValue();
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 TotalCppFiles = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 TotalHeaderFiles = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 OrphanedHeaders = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 CompilationErrors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 ActorsInLevel = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 DinosaurActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    bool bCanPlayerMove = false;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    bool bHasPlayableCharacter = false;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    bool bHasBasicTerrain = false;
};

/**
 * Studio Director's Production Coordinator
 * Manages the 19-agent pipeline for the prehistoric survival game
 * Ensures gameplay-first development and milestone tracking
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDir_ProductionCoordinator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UDir_ProductionCoordinator();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Agent Management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void InitializeAgentPipeline();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void AssignTaskToAgent(int32 AgentID, const FString& TaskDescription, float Priority = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void SetAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Production")
    EDir_AgentStatus GetAgentStatus(int32 AgentID) const;

    // Milestone Management
    UFUNCTION(BlueprintCallable, Category = "Production")
    bool IsMilestoneComplete(EDir_MilestoneType Milestone) const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    void ValidateCurrentMilestone();

    UFUNCTION(BlueprintCallable, Category = "Production")
    FString GetCurrentMilestoneDescription() const;

    // Production Metrics
    UFUNCTION(BlueprintCallable, Category = "Production")
    FDir_ProductionMetrics GetProductionMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateMetrics();

    // Critical Production Rules
    UFUNCTION(BlueprintCallable, Category = "Production")
    bool CanAgentProceed(int32 AgentID) const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FString> GetBlockingIssues() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    void ReportCompilationError(const FString& ErrorMessage);

    // Biome Coordination
    UFUNCTION(BlueprintCallable, Category = "Production")
    FVector GetRandomSpawnLocationForBiome(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    void ValidateBiomeDistribution();

    // Debug and Monitoring
    UFUNCTION(BlueprintCallable, Category = "Production", CallInEditor)
    void PrintProductionStatus();

    UFUNCTION(BlueprintCallable, Category = "Production", CallInEditor)
    void ForceCleanDuplicateActors();

protected:
    // Agent tracking
    UPROPERTY(BlueprintReadOnly, Category = "Production")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    TMap<int32, FString> AgentNames;

    // Production state
    UPROPERTY(BlueprintReadOnly, Category = "Production")
    EDir_MilestoneType CurrentMilestone;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    FDir_ProductionMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    TArray<FString> CompilationErrors;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    bool bProductionBlocked;

private:
    void InitializeAgentNames();
    void CheckMilestone1Requirements();
    void CheckMilestone2Requirements();
    void CheckMilestone3Requirements();
    void ScanForOrphanedHeaders();
    void ValidateActorCounts();
    FVector GetBiomeCenterLocation(EBiomeType BiomeType) const;
    FVector GetBiomeRandomOffset() const;
};