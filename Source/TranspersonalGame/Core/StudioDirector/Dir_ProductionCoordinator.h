#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/TextRenderComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "Dir_ProductionCoordinator.generated.h"

/**
 * Studio Director's Production Coordination System
 * Tracks development progress across all 19 agents in real-time
 * Provides visual dashboard and automated task delegation
 */

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Inactive        UMETA(DisplayName = "Inactive"),
    Active          UMETA(DisplayName = "Active"),
    Completed       UMETA(DisplayName = "Completed"),
    Blocked         UMETA(DisplayName = "Blocked"),
    Error           UMETA(DisplayName = "Error")
};

UENUM(BlueprintType)
enum class EDir_MilestonePhase : uint8
{
    Planning        UMETA(DisplayName = "Planning"),
    Architecture    UMETA(DisplayName = "Architecture"),
    CoreSystems     UMETA(DisplayName = "Core Systems"),
    WorldBuilding   UMETA(DisplayName = "World Building"),
    Characters      UMETA(DisplayName = "Characters"),
    Gameplay        UMETA(DisplayName = "Gameplay"),
    Polish          UMETA(DisplayName = "Polish"),
    Testing         UMETA(DisplayName = "Testing"),
    Integration     UMETA(DisplayName = "Integration")
};

USTRUCT(BlueprintType)
struct FDir_AgentProgress
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Progress")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Progress")
    int32 AgentNumber;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Progress")
    EDir_AgentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Progress")
    float CompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Progress")
    FString CurrentTask;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Progress")
    FString LastOutput;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Progress")
    float CycleTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Progress")
    int32 ToolCallsUsed;

    FDir_AgentProgress()
    {
        AgentName = TEXT("");
        AgentNumber = 0;
        Status = EDir_AgentStatus::Inactive;
        CompletionPercentage = 0.0f;
        CurrentTask = TEXT("");
        LastOutput = TEXT("");
        CycleTime = 0.0f;
        ToolCallsUsed = 0;
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 CurrentCycle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    EDir_MilestonePhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    float OverallProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 CompletedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 BlockedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    float BudgetUsed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    float EstimatedTimeToMilestone;

    FDir_ProductionMetrics()
    {
        CurrentCycle = 20;
        CurrentPhase = EDir_MilestonePhase::Architecture;
        OverallProgress = 15.0f;
        CompletedTasks = 0;
        BlockedTasks = 0;
        BudgetUsed = 9.39f;
        EstimatedTimeToMilestone = 72.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADir_ProductionCoordinator : public AActor
{
    GENERATED_BODY()

public:
    ADir_ProductionCoordinator();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UTextRenderComponent* DashboardDisplay;

    // Production Data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_AgentProgress> AgentProgressArray;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FDir_ProductionMetrics ProductionMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString CurrentMilestone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    bool bAutoUpdateDashboard;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float DashboardUpdateInterval;

public:
    // Production Management Functions
    UFUNCTION(BlueprintCallable, Category = "Production Management")
    void InitializeAgentTracking();

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    void UpdateAgentProgress(int32 AgentNumber, EDir_AgentStatus NewStatus, float Completion, const FString& Task);

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    void SetCurrentMilestone(const FString& MilestoneName, EDir_MilestonePhase Phase);

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    void UpdateProductionMetrics();

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    void RefreshDashboard();

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    FString GenerateDashboardText();

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    bool IsAgentBlocked(int32 AgentNumber) const;

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    TArray<int32> GetBlockedAgents() const;

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    float GetOverallProgress() const;

    UFUNCTION(BlueprintCallable, Category = "Production Management", CallInEditor = true)
    void ForceUpdateDashboard();

    // Critical Path Analysis
    UFUNCTION(BlueprintCallable, Category = "Critical Path")
    TArray<int32> GetCriticalPathAgents() const;

    UFUNCTION(BlueprintCallable, Category = "Critical Path")
    bool IsMilestoneOnTrack() const;

    UFUNCTION(BlueprintCallable, Category = "Critical Path")
    FString GetNextCriticalTask() const;

private:
    float LastDashboardUpdate;
    
    void SetupDashboardDisplay();
    void UpdateDashboardColors();
    FLinearColor GetStatusColor(EDir_AgentStatus Status) const;
};