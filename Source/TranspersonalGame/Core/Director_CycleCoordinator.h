#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Components/ActorComponent.h"
#include "Director_CycleCoordinator.generated.h"

/**
 * Studio Director Cycle Coordinator - Manages production cycles and agent task distribution
 * Ensures each cycle produces concrete playable elements rather than just documentation
 */

UENUM(BlueprintType)
enum class EDir_CyclePhase : uint8
{
    Planning        UMETA(DisplayName = "Planning"),
    CoreSystems     UMETA(DisplayName = "Core Systems"),
    WorldGeneration UMETA(DisplayName = "World Generation"),
    CharacterSetup  UMETA(DisplayName = "Character Setup"),
    AIImplementation UMETA(DisplayName = "AI Implementation"),
    Integration     UMETA(DisplayName = "Integration"),
    Testing         UMETA(DisplayName = "Testing"),
    Completion      UMETA(DisplayName = "Completion")
};

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Working         UMETA(DisplayName = "Working"),
    Completed       UMETA(DisplayName = "Completed"),
    Blocked         UMETA(DisplayName = "Blocked"),
    Failed          UMETA(DisplayName = "Failed")
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    int32 AgentID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_AgentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    TArray<FString> RequiredDeliverables;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    TArray<FString> CompletedDeliverables;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    bool bIsBlocking;

    FDir_AgentTask()
    {
        AgentID = 0;
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Status = EDir_AgentStatus::Idle;
        Priority = 1.0f;
        bIsBlocking = false;
    }
};

USTRUCT(BlueprintType)
struct FDir_CycleMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CycleNumber;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    EDir_CyclePhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CompletedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 FilesCreated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 UE5CommandsExecuted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float OverallProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    bool bPlayablePrototypeReady;

    FDir_CycleMetrics()
    {
        CycleNumber = 0;
        CurrentPhase = EDir_CyclePhase::Planning;
        CompletedTasks = 0;
        TotalTasks = 0;
        FilesCreated = 0;
        UE5CommandsExecuted = 0;
        OverallProgress = 0.0f;
        bPlayablePrototypeReady = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDir_CycleCoordinator : public UActorComponent
{
    GENERATED_BODY()

public:
    UDir_CycleCoordinator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core coordination functions
    UFUNCTION(BlueprintCallable, Category = "Cycle Management")
    void InitializeCycle(int32 CycleNumber);

    UFUNCTION(BlueprintCallable, Category = "Cycle Management")
    void AdvanceToNextPhase();

    UFUNCTION(BlueprintCallable, Category = "Cycle Management")
    bool ValidatePhaseCompletion();

    UFUNCTION(BlueprintCallable, Category = "Agent Management")
    void AssignTaskToAgent(int32 AgentID, const FString& TaskDescription, const TArray<FString>& Deliverables, float Priority = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Agent Management")
    void UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Agent Management")
    void MarkDeliverableComplete(int32 AgentID, const FString& Deliverable);

    // Critical path management
    UFUNCTION(BlueprintCallable, Category = "Critical Path")
    TArray<int32> GetCriticalPathAgents();

    UFUNCTION(BlueprintCallable, Category = "Critical Path")
    bool IsAgentBlocking(int32 AgentID);

    UFUNCTION(BlueprintCallable, Category = "Critical Path")
    void ResolveBlockingIssue(int32 AgentID, const FString& Resolution);

    // Playable prototype validation
    UFUNCTION(BlueprintCallable, Category = "Prototype Validation")
    bool ValidatePlayablePrototype();

    UFUNCTION(BlueprintCallable, Category = "Prototype Validation")
    TArray<FString> GetMissingPrototypeElements();

    UFUNCTION(BlueprintCallable, Category = "Prototype Validation")
    float CalculatePrototypeCompleteness();

    // Metrics and reporting
    UFUNCTION(BlueprintCallable, Category = "Metrics")
    FDir_CycleMetrics GetCurrentCycleMetrics();

    UFUNCTION(BlueprintCallable, Category = "Metrics")
    void GenerateCycleReport();

    UFUNCTION(BlueprintCallable, Category = "Metrics")
    void ExportProductionDashboard();

    // Emergency protocols
    UFUNCTION(BlueprintCallable, Category = "Emergency")
    void TriggerEmergencyReset();

    UFUNCTION(BlueprintCallable, Category = "Emergency")
    void ForcePhaseAdvancement();

    UFUNCTION(BlueprintCallable, Category = "Emergency")
    void PrioritizePlayableElements();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cycle State")
    FDir_CycleMetrics CurrentMetrics;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Agent Management")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    TArray<int32> CriticalPathAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    TArray<FString> PlayablePrototypeRequirements;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    bool bCycleActive;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    float CycleStartTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    int32 CurrentCycleNumber;

private:
    void InitializeAgentTasks();
    void UpdateCycleMetrics();
    void ValidateCriticalPath();
    void CheckPlayablePrototypeStatus();
    void LogCycleProgress();
};