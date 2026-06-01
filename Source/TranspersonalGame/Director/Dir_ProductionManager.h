#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Dir_ProductionManager.generated.h"

/**
 * Production Manager - Studio Director's core system for coordinating game development
 * Tracks build state, agent coordination, and critical production metrics
 */

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction,
    Prototyping,
    VerticalSlice,
    Production,
    Polish,
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 AgentID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    EDir_AgentStatus Status = EDir_AgentStatus::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float Priority = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FDateTime StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FDateTime Deadline;

    FDir_AgentTask()
    {
        StartTime = FDateTime::Now();
        Deadline = FDateTime::Now() + FTimespan::FromHours(24);
    }
};

USTRUCT(BlueprintType)
struct FDir_BuildMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalActors = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 DinosaurCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CompilationErrors = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float FrameRate = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float MemoryUsageGB = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    bool bIsPlayable = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    FDateTime LastUpdate;

    FDir_BuildMetrics()
    {
        LastUpdate = FDateTime::Now();
    }
};

/**
 * Production Manager Subsystem
 * Core coordination system for the 19-agent development pipeline
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDir_ProductionManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UDir_ProductionManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Production coordination
    UFUNCTION(BlueprintCallable, Category = "Production", CallInEditor)
    void StartProductionCycle();

    UFUNCTION(BlueprintCallable, Category = "Production", CallInEditor)
    void AssignTaskToAgent(int32 AgentID, const FString& TaskDescription, float Priority = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Production", CallInEditor)
    void UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Production", CallInEditor)
    void GenerateProductionReport();

    // Build monitoring
    UFUNCTION(BlueprintCallable, Category = "Metrics", CallInEditor)
    void UpdateBuildMetrics();

    UFUNCTION(BlueprintCallable, Category = "Metrics", CallInEditor)
    bool ValidateActorLimits();

    UFUNCTION(BlueprintCallable, Category = "Metrics", CallInEditor)
    void EnforceActorLimits();

    // Getters
    UFUNCTION(BlueprintPure, Category = "Production")
    EDir_ProductionPhase GetCurrentPhase() const { return CurrentPhase; }

    UFUNCTION(BlueprintPure, Category = "Production")
    const TArray<FDir_AgentTask>& GetActiveTasks() const { return ActiveTasks; }

    UFUNCTION(BlueprintPure, Category = "Metrics")
    const FDir_BuildMetrics& GetCurrentMetrics() const { return CurrentMetrics; }

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    EDir_ProductionPhase CurrentPhase = EDir_ProductionPhase::Prototyping;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_AgentTask> ActiveTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    FDir_BuildMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Limits")
    int32 MaxTotalActors = 8000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Limits")
    int32 MaxDinosaurs = 150;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Limits")
    int32 MaxPropsPerBiome = 1000;

private:
    void InitializeAgentPipeline();
    void ValidateProjectStructure();
    void LogProductionStatus();
};