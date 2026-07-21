#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "StudioDirector.generated.h"

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction UMETA(DisplayName = "Pre-Production"),
    PrototypeDevelopment UMETA(DisplayName = "Prototype Development"),
    CoreSystemsImplementation UMETA(DisplayName = "Core Systems Implementation"),
    ContentCreation UMETA(DisplayName = "Content Creation"),
    PolishAndOptimization UMETA(DisplayName = "Polish and Optimization"),
    ReleasePreparation UMETA(DisplayName = "Release Preparation")
};

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Working UMETA(DisplayName = "Working"),
    Blocked UMETA(DisplayName = "Blocked"),
    Complete UMETA(DisplayName = "Complete"),
    Error UMETA(DisplayName = "Error")
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
    FDateTime StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FDateTime EstimatedCompletion;

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
        StartTime = FDateTime::Now();
        EstimatedCompletion = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMilestone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString MilestoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FString> RequiredDeliverables;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    bool bIsComplete;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    float CompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FDateTime TargetDate;

    FDir_ProductionMilestone()
    {
        MilestoneName = TEXT("");
        Description = TEXT("");
        bIsComplete = false;
        CompletionPercentage = 0.0f;
        TargetDate = FDateTime::Now();
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AStudioDirector : public AActor
{
    GENERATED_BODY()

public:
    AStudioDirector();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    // Production Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    EDir_ProductionPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_AgentTask> ActiveTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_ProductionMilestone> ProductionMilestones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float OverallProjectProgress;

    // Agent Coordination
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agents")
    TMap<FString, EDir_AgentStatus> AgentStatusMap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agents")
    TArray<FString> BlockedAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agents")
    TArray<FString> CriticalPathAgents;

public:
    virtual void Tick(float DeltaTime) override;

    // Production Management Functions
    UFUNCTION(BlueprintCallable, Category = "Production")
    void InitializeProductionPipeline();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateProductionPhase(EDir_ProductionPhase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "Production")
    float CalculateOverallProgress();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void CreateProductionReport();

    // Agent Management Functions
    UFUNCTION(BlueprintCallable, Category = "Agents")
    void AssignTaskToAgent(const FString& AgentName, const FString& TaskDescription, float Priority);

    UFUNCTION(BlueprintCallable, Category = "Agents")
    void UpdateAgentStatus(const FString& AgentName, EDir_AgentStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Agents")
    TArray<FString> GetAvailableAgents();

    UFUNCTION(BlueprintCallable, Category = "Agents")
    void ResolveAgentBlockage(const FString& AgentName);

    // Milestone Management
    UFUNCTION(BlueprintCallable, Category = "Milestones")
    void CreateMilestone(const FString& Name, const FString& Description, const TArray<FString>& Deliverables);

    UFUNCTION(BlueprintCallable, Category = "Milestones")
    void UpdateMilestoneProgress(const FString& MilestoneName, float Progress);

    UFUNCTION(BlueprintCallable, Category = "Milestones")
    bool CheckMilestoneCompletion(const FString& MilestoneName);

    // Quality Assurance Integration
    UFUNCTION(BlueprintCallable, Category = "QA")
    void TriggerQualityCheck();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void HandleQABlockage(const FString& BlockageReason);

    // Emergency Protocols
    UFUNCTION(BlueprintCallable, Category = "Emergency")
    void ActivateEmergencyProtocol();

    UFUNCTION(BlueprintCallable, Category = "Emergency")
    void RollbackToLastStableState();

protected:
    // Internal Management
    void UpdateAgentDependencies();
    void CheckCriticalPath();
    void GenerateProductionMetrics();
    
    UPROPERTY()
    FDateTime LastUpdateTime;
    
    UPROPERTY()
    TArray<FString> ProductionLog;
};