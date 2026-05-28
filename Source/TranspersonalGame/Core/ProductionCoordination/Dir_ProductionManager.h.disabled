#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Dir_ProductionManager.generated.h"

/**
 * Studio Director's Production Manager
 * Coordinates the 19-agent development pipeline and tracks milestone progress
 * Monitors the 3 critical criteria for asset purchase readiness
 */

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    Planning        UMETA(DisplayName = "Planning"),
    Development     UMETA(DisplayName = "Development"),
    Testing         UMETA(DisplayName = "Testing"),
    Integration     UMETA(DisplayName = "Integration"),
    Complete        UMETA(DisplayName = "Complete")
};

UENUM(BlueprintType)
enum class EDir_MilestoneStatus : uint8
{
    NotStarted      UMETA(DisplayName = "Not Started"),
    InProgress      UMETA(DisplayName = "In Progress"),
    NeedsReview     UMETA(DisplayName = "Needs Review"),
    Complete        UMETA(DisplayName = "Complete"),
    Blocked         UMETA(DisplayName = "Blocked")
};

USTRUCT(BlueprintType)
struct FDir_AgentTaskInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    int32 AgentNumber = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString AgentName = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString CurrentTask = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    EDir_ProductionPhase Phase = EDir_ProductionPhase::Planning;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    float ProgressPercent = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FDateTime LastUpdate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    TArray<FString> Deliverables;

    FDir_AgentTaskInfo()
    {
        LastUpdate = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct FDir_MilestoneInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString MilestoneName = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString Description = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    EDir_MilestoneStatus Status = EDir_MilestoneStatus::NotStarted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<int32> ResponsibleAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FDateTime TargetDate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FDateTime CompletionDate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FString> Requirements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FString> CompletionCriteria;

    FDir_MilestoneInfo()
    {
        TargetDate = FDateTime::Now() + FTimespan::FromDays(7);
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADir_ProductionManager : public AActor
{
    GENERATED_BODY()

public:
    ADir_ProductionManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    // Agent Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Management")
    TArray<FDir_AgentTaskInfo> AgentTasks;

    // Milestone Tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Management")
    TArray<FDir_MilestoneInfo> Milestones;

    // Critical Criteria (Hugo's 3 criteria for asset purchase)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset Purchase Criteria")
    bool bLandscapeExpanded = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset Purchase Criteria")
    bool bAtmosphereStable = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset Purchase Criteria")
    bool bFBXPipelineTested = false;

    // Production Metrics
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 TotalCyclesCompleted = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 SuccessfulBuilds = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 FailedBuilds = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    float AverageAgentExecutionTime = 0.0f;

public:
    virtual void Tick(float DeltaTime) override;

    // Agent Management Functions
    UFUNCTION(BlueprintCallable, Category = "Production Management")
    void UpdateAgentTask(int32 AgentNumber, const FString& TaskDescription, EDir_ProductionPhase Phase, float Progress);

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    void AddAgentDeliverable(int32 AgentNumber, const FString& Deliverable);

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    FDir_AgentTaskInfo GetAgentTaskInfo(int32 AgentNumber);

    // Milestone Management Functions
    UFUNCTION(BlueprintCallable, Category = "Production Management")
    void CreateMilestone(const FString& Name, const FString& Description, const TArray<int32>& ResponsibleAgents);

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    void UpdateMilestoneStatus(const FString& MilestoneName, EDir_MilestoneStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    bool IsMilestoneComplete(const FString& MilestoneName);

    // Critical Criteria Functions
    UFUNCTION(BlueprintCallable, Category = "Asset Purchase Criteria")
    void SetLandscapeExpanded(bool bExpanded);

    UFUNCTION(BlueprintCallable, Category = "Asset Purchase Criteria")
    void SetAtmosphereStable(bool bStable);

    UFUNCTION(BlueprintCallable, Category = "Asset Purchase Criteria")
    void SetFBXPipelineTested(bool bTested);

    UFUNCTION(BlueprintCallable, Category = "Asset Purchase Criteria")
    bool AreAllCriteriaMet();

    // Production Metrics Functions
    UFUNCTION(BlueprintCallable, Category = "Production Metrics")
    void IncrementCycleCount();

    UFUNCTION(BlueprintCallable, Category = "Production Metrics")
    void RecordBuildResult(bool bSuccess);

    UFUNCTION(BlueprintCallable, Category = "Production Metrics")
    void UpdateAverageExecutionTime(float NewTime);

    // Reporting Functions
    UFUNCTION(BlueprintCallable, Category = "Production Management")
    FString GenerateProductionReport();

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    FString GenerateMilestoneReport();

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    FString GenerateAgentStatusReport();

    // Validation Functions
    UFUNCTION(BlueprintCallable, Category = "Production Management")
    bool ValidateAgentDeliverables(int32 AgentNumber);

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    TArray<FString> GetBlockedTasks();

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    TArray<FString> GetOverdueMilestones();

private:
    void InitializeAgentTasks();
    void InitializeMilestones();
    void UpdateProductionMetrics();
    
    FDateTime LastMetricsUpdate;
    float ProductionStartTime;
};