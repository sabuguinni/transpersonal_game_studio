#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Subsystems/WorldSubsystem.h"
#include "StudioDirectorSystem.generated.h"

USTRUCT(BlueprintType)
struct FDir_AgentStatus
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Status")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Status")
    FString CurrentTask;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Status")
    FString Status; // Active, Waiting, Completed, Error

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Status")
    float Progress; // 0.0 to 1.0

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Status")
    FVector TrackerLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Status")
    FDateTime LastUpdate;

    FDir_AgentStatus()
    {
        AgentName = TEXT("");
        CurrentTask = TEXT("");
        Status = TEXT("Waiting");
        Progress = 0.0f;
        TrackerLocation = FVector::ZeroVector;
        LastUpdate = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct FDir_BiomeMilestone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector BiomeCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 ActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 RequiredActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> AssignedAgents;

    FDir_BiomeMilestone()
    {
        BiomeName = TEXT("");
        BiomeCenter = FVector::ZeroVector;
        ActorCount = 0;
        RequiredActorCount = 500;
        bIsCompleted = false;
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalCycles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CompletedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 ActiveAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float OverallProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    FDateTime CycleStartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    FString CurrentMilestone;

    FDir_ProductionMetrics()
    {
        TotalCycles = 0;
        CompletedTasks = 0;
        ActiveAgents = 0;
        OverallProgress = 0.0f;
        CycleStartTime = FDateTime::Now();
        CurrentMilestone = TEXT("Milestone 1 - Walk Around");
    }
};

/**
 * Studio Director System - Central coordination for all 19 agents
 * Manages production pipeline, agent status, and milestone tracking
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UStudioDirectorSystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UStudioDirectorSystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Agent Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void RegisterAgent(const FString& AgentName, const FString& InitialTask, const FVector& TrackerLocation);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateAgentStatus(const FString& AgentName, const FString& NewTask, const FString& Status, float Progress);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FDir_AgentStatus GetAgentStatus(const FString& AgentName) const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_AgentStatus> GetAllAgentStatuses() const;

    // Biome Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void InitializeBiomeMilestones();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateBiomeProgress(const FString& BiomeName, int32 NewActorCount);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool IsBiomeCompleted(const FString& BiomeName) const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_BiomeMilestone> GetBiomeMilestones() const;

    // Production Metrics
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void StartNewCycle();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CompleteTask(const FString& TaskName);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FDir_ProductionMetrics GetProductionMetrics() const;

    // Pipeline Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void ValidatePipelineOrder();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FString GetNextAgentInPipeline(const FString& CurrentAgent) const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool CanAgentProceed(const FString& AgentName) const;

    // Asset Pipeline
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void TrackAssetRequest(const FString& AssetName, const FString& Category, const FString& BiomeName);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    int32 GetPendingAssetCount() const;

    // Milestone Validation
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool ValidateMilestone1() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FString> GetMilestone1Requirements() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void GenerateProductionReport();

protected:
    // Agent tracking
    UPROPERTY(BlueprintReadOnly, Category = "Studio Director")
    TMap<FString, FDir_AgentStatus> AgentStatuses;

    // Biome tracking
    UPROPERTY(BlueprintReadOnly, Category = "Studio Director")
    TArray<FDir_BiomeMilestone> BiomeMilestones;

    // Production metrics
    UPROPERTY(BlueprintReadOnly, Category = "Studio Director")
    FDir_ProductionMetrics ProductionMetrics;

    // Pipeline order (19 agents)
    UPROPERTY(BlueprintReadOnly, Category = "Studio Director")
    TArray<FString> PipelineOrder;

    // Asset tracking
    UPROPERTY(BlueprintReadOnly, Category = "Studio Director")
    TArray<FString> PendingAssets;

private:
    void InitializePipelineOrder();
    void CreateAgentTrackers();
    void CreateBiomeMarkers();
    void ValidateSystemIntegrity();
};