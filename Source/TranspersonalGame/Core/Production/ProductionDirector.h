#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "ProductionDirector.generated.h"

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction   UMETA(DisplayName = "Pre-Production"),
    Prototyping     UMETA(DisplayName = "Prototyping"),
    Production      UMETA(DisplayName = "Production"), 
    Testing         UMETA(DisplayName = "Testing"),
    Polish          UMETA(DisplayName = "Polish"),
    Release         UMETA(DisplayName = "Release")
};

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Working         UMETA(DisplayName = "Working"),
    Blocked         UMETA(DisplayName = "Blocked"),
    Complete        UMETA(DisplayName = "Complete"),
    Error           UMETA(DisplayName = "Error")
};

USTRUCT(BlueprintType)
struct FDir_AgentInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    int32 AgentID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    EDir_AgentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    float ProgressPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FString CurrentTask;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    TArray<FString> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FDateTime LastUpdate;

    FDir_AgentInfo()
    {
        AgentName = TEXT("");
        AgentID = 0;
        Status = EDir_AgentStatus::Idle;
        ProgressPercentage = 0.0f;
        CurrentTask = TEXT("");
        LastUpdate = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float OverallProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CompletedSystems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalSystems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 ActiveAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 BlockedAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    FDateTime LastMetricsUpdate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float EstimatedTimeToCompletion;

    FDir_ProductionMetrics()
    {
        OverallProgress = 0.0f;
        CompletedSystems = 0;
        TotalSystems = 19;
        ActiveAgents = 0;
        BlockedAgents = 0;
        LastMetricsUpdate = FDateTime::Now();
        EstimatedTimeToCompletion = 0.0f;
    }
};

/**
 * Production Director - Master coordinator for the 19-agent development pipeline
 * Manages task distribution, dependency tracking, and production metrics
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AProductionDirector : public AActor
{
    GENERATED_BODY()

public:
    AProductionDirector();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    // Production State
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    EDir_ProductionPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FDir_ProductionMetrics ProductionMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_AgentInfo> AgentPipeline;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    bool bAutoUpdateMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float MetricsUpdateInterval;

    // Production Control Functions
    UFUNCTION(BlueprintCallable, Category = "Production")
    void InitializeProductionPipeline();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus, float Progress);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void SetAgentTask(int32 AgentID, const FString& TaskDescription);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void AdvanceProductionPhase();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void CalculateProductionMetrics();

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool ValidateAgentDependencies(int32 AgentID);

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<int32> GetBlockedAgents();

    UFUNCTION(BlueprintCallable, Category = "Production")
    FString GetProductionSummary();

    // Editor Tools
    UFUNCTION(CallInEditor, Category = "Production")
    void EditorRefreshMetrics();

    UFUNCTION(CallInEditor, Category = "Production")
    void EditorResetPipeline();

    UFUNCTION(CallInEditor, Category = "Production")
    void EditorGenerateReport();

    // Blueprint Events
    UFUNCTION(BlueprintImplementableEvent, Category = "Production")
    void OnPhaseChanged(EDir_ProductionPhase NewPhase);

    UFUNCTION(BlueprintImplementableEvent, Category = "Production")
    void OnAgentStatusChanged(int32 AgentID, EDir_AgentStatus NewStatus);

    UFUNCTION(BlueprintImplementableEvent, Category = "Production")
    void OnProductionMilestone(float ProgressPercentage);

private:
    // Internal state
    float LastMetricsUpdate;
    bool bPipelineInitialized;

    // Helper functions
    void CreateDefaultAgentPipeline();
    void UpdateProductionProgress();
    float CalculateOverallProgress();
    int32 CountAgentsByStatus(EDir_AgentStatus Status);
};