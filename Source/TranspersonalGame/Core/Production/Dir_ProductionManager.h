#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Dir_ProductionManager.generated.h"

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction,
    Prototype,
    VerticalSlice,
    Alpha,
    Beta,
    Gold
};

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle,
    Working,
    Blocked,
    Complete,
    Failed
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    int32 AgentID = 0;

    UPROPERTY(BlueprintReadOnly)
    FString AgentName;

    UPROPERTY(BlueprintReadOnly)
    FString CurrentTask;

    UPROPERTY(BlueprintReadOnly)
    EDir_AgentStatus Status = EDir_AgentStatus::Idle;

    UPROPERTY(BlueprintReadOnly)
    float CompletionPercentage = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    int32 FilesCreated = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 UE5CommandsExecuted = 0;

    UPROPERTY(BlueprintReadOnly)
    FDateTime LastUpdate;

    FDir_AgentTask()
    {
        LastUpdate = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    int32 TotalActors = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 DinosaurCount = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 TerrainTiles = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 CompletedSystems = 0;

    UPROPERTY(BlueprintReadOnly)
    float OverallProgress = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    EDir_ProductionPhase CurrentPhase = EDir_ProductionPhase::PreProduction;

    UPROPERTY(BlueprintReadOnly)
    FDateTime LastBuild;

    FDir_ProductionMetrics()
    {
        LastBuild = FDateTime::Now();
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDir_ProductionManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Agent Management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void RegisterAgent(int32 AgentID, const FString& AgentName);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateAgentTask(int32 AgentID, const FString& TaskDescription, EDir_AgentStatus Status, float Completion);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void IncrementAgentFiles(int32 AgentID, int32 FileCount = 1);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void IncrementAgentCommands(int32 AgentID, int32 CommandCount = 1);

    UFUNCTION(BlueprintCallable, Category = "Production", BlueprintPure)
    TArray<FDir_AgentTask> GetAllAgentTasks() const;

    UFUNCTION(BlueprintCallable, Category = "Production", BlueprintPure)
    FDir_AgentTask GetAgentTask(int32 AgentID) const;

    // Production Metrics
    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateProductionMetrics();

    UFUNCTION(BlueprintCallable, Category = "Production", BlueprintPure)
    FDir_ProductionMetrics GetProductionMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    void SetProductionPhase(EDir_ProductionPhase NewPhase);

    // Milestone Tracking
    UFUNCTION(BlueprintCallable, Category = "Production")
    bool CheckMilestone1Complete() const; // Walk Around milestone

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool CheckMilestone2Complete() const; // Basic Survival milestone

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool CheckMilestone3Complete() const; // Dinosaur Interaction milestone

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Production")
    void LogPerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool IsWithinActorLimits() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    void EnforceActorLimits();

private:
    UPROPERTY()
    TMap<int32, FDir_AgentTask> AgentTasks;

    UPROPERTY()
    FDir_ProductionMetrics CurrentMetrics;

    void InitializeAgents();
    void CalculateOverallProgress();
    int32 CountActorsInLevel() const;
    int32 CountDinosaursInLevel() const;
};

#include "Dir_ProductionManager.generated.h"