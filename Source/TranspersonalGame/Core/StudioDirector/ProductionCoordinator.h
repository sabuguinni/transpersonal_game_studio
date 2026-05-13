#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "../../SharedTypes.h"
#include "ProductionCoordinator.generated.h"

/**
 * Studio Director's Production Coordination System
 * Tracks agent progress, milestone completion, and production pipeline status
 * Used by Studio Director (Agent #1) to coordinate the 18-agent development chain
 */

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Pending         UMETA(DisplayName = "Pending"),
    InProgress      UMETA(DisplayName = "In Progress"),
    Completed       UMETA(DisplayName = "Completed"),
    Blocked         UMETA(DisplayName = "Blocked"),
    Failed          UMETA(DisplayName = "Failed")
};

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    Architecture    UMETA(DisplayName = "Architecture Phase"),
    CoreSystems     UMETA(DisplayName = "Core Systems Phase"),
    WorldBuilding   UMETA(DisplayName = "World Building Phase"),
    Characters      UMETA(DisplayName = "Characters Phase"),
    Gameplay        UMETA(DisplayName = "Gameplay Phase"),
    Polish          UMETA(DisplayName = "Polish Phase"),
    Integration     UMETA(DisplayName = "Integration Phase")
};

USTRUCT(BlueprintType)
struct FDir_AgentMilestone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    int32 AgentNumber = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    EDir_AgentStatus Status = EDir_AgentStatus::Pending;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FString CurrentTask;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    float CompletionPercentage = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FString LastUpdate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    TArray<FString> Deliverables;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    TArray<FString> Dependencies;

    FDir_AgentMilestone()
    {
        AgentNumber = 0;
        AgentName = TEXT("Unknown Agent");
        Status = EDir_AgentStatus::Pending;
        CurrentTask = TEXT("No task assigned");
        CompletionPercentage = 0.0f;
        LastUpdate = TEXT("Never");
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CycleNumber = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalAgents = 18;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 ActiveAgents = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CompletedAgents = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 BlockedAgents = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float OverallProgress = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    EDir_ProductionPhase CurrentPhase = EDir_ProductionPhase::Architecture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    FString CriticalPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    TArray<FString> RiskFactors;

    FDir_ProductionMetrics()
    {
        CycleNumber = 0;
        TotalAgents = 18;
        ActiveAgents = 0;
        CompletedAgents = 0;
        BlockedAgents = 0;
        OverallProgress = 0.0f;
        CurrentPhase = EDir_ProductionPhase::Architecture;
        CriticalPath = TEXT("Agent #2 Engine Architect");
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
    UStaticMeshComponent* CoordinatorMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UTextRenderComponent* StatusDisplay;

    // Production Data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_AgentMilestone> AgentMilestones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FDir_ProductionMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FString> ProductionLog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    bool bAutoUpdateDisplay = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float UpdateInterval = 5.0f;

public:
    // Agent Management Functions
    UFUNCTION(BlueprintCallable, Category = "Production")
    void InitializeAgentMilestones();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateAgentStatus(int32 AgentNumber, EDir_AgentStatus NewStatus, const FString& TaskDescription);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void SetAgentProgress(int32 AgentNumber, float ProgressPercentage);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void AddAgentDeliverable(int32 AgentNumber, const FString& Deliverable);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void LogProductionEvent(const FString& EventDescription);

    // Metrics and Analysis
    UFUNCTION(BlueprintCallable, Category = "Production")
    void CalculateProductionMetrics();

    UFUNCTION(BlueprintCallable, Category = "Production")
    FDir_AgentMilestone GetAgentMilestone(int32 AgentNumber);

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<int32> GetBlockedAgents();

    UFUNCTION(BlueprintCallable, Category = "Production")
    FString GetCriticalPath();

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool IsProductionOnTrack();

    // Display and Visualization
    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateStatusDisplay();

    UFUNCTION(BlueprintCallable, Category = "Production")
    FString GenerateStatusReport();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void SetProductionPhase(EDir_ProductionPhase NewPhase);

    // Cycle Management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void StartNewCycle(int32 CycleNumber);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void CompleteCycle();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void HandleAgentFailure(int32 AgentNumber, const FString& FailureReason);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void TriggerEmergencyRecovery();

private:
    float LastUpdateTime = 0.0f;
    bool bInitialized = false;

    void SetupDefaultAgents();
    void UpdateDisplayText();
    FString FormatAgentStatus(const FDir_AgentMilestone& Milestone);
};