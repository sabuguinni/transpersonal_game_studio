#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/Engine.h"
#include "Templates/SharedPointer.h"
#include "ProductionPipelineController.generated.h"

class UStudioDirectorSubsystem;
class UEngineArchitectureValidator;

UENUM(BlueprintType)
enum class EProductionPhase : uint8
{
    PreProduction,
    TechnicalFoundation,
    WorldGeneration,
    CharacterSystems,
    GameplayCore,
    AudioVisual,
    Integration,
    QualityAssurance,
    Shipping
};

UENUM(BlueprintType)
enum class EAgentStatus : uint8
{
    Idle,
    Working,
    Blocked,
    Complete,
    Error
};

USTRUCT(BlueprintType)
struct FAgentWorkItem
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    int32 AgentID;

    UPROPERTY(BlueprintReadOnly)
    FString AgentName;

    UPROPERTY(BlueprintReadOnly)
    EAgentStatus Status;

    UPROPERTY(BlueprintReadOnly)
    FString CurrentTask;

    UPROPERTY(BlueprintReadOnly)
    float ProgressPercent;

    UPROPERTY(BlueprintReadOnly)
    TArray<FString> Dependencies;

    UPROPERTY(BlueprintReadOnly)
    TArray<FString> Outputs;

    FAgentWorkItem()
    {
        AgentID = 0;
        Status = EAgentStatus::Idle;
        ProgressPercent = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    int32 TotalAgents;

    UPROPERTY(BlueprintReadOnly)
    int32 ActiveAgents;

    UPROPERTY(BlueprintReadOnly)
    int32 CompletedAgents;

    UPROPERTY(BlueprintReadOnly)
    int32 BlockedAgents;

    UPROPERTY(BlueprintReadOnly)
    float OverallProgress;

    UPROPERTY(BlueprintReadOnly)
    EProductionPhase CurrentPhase;

    UPROPERTY(BlueprintReadOnly)
    FString LastUpdate;

    FProductionMetrics()
    {
        TotalAgents = 19;
        ActiveAgents = 0;
        CompletedAgents = 0;
        BlockedAgents = 0;
        OverallProgress = 0.0f;
        CurrentPhase = EProductionPhase::PreProduction;
    }
};

/**
 * Production Pipeline Controller
 * Manages the 19-agent production chain for Transpersonal Game Studio
 * Ensures proper sequencing, dependency management, and quality gates
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UProductionPipelineController : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UProductionPipelineController();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Production Control
    UFUNCTION(BlueprintCallable, Category = "Production Pipeline")
    void StartProductionCycle(const FString& CycleID, const FString& InitialDirective);

    UFUNCTION(BlueprintCallable, Category = "Production Pipeline")
    void AdvanceToNextAgent();

    UFUNCTION(BlueprintCallable, Category = "Production Pipeline")
    void BlockAgent(int32 AgentID, const FString& Reason);

    UFUNCTION(BlueprintCallable, Category = "Production Pipeline")
    void UnblockAgent(int32 AgentID);

    UFUNCTION(BlueprintCallable, Category = "Production Pipeline")
    void CompleteAgentWork(int32 AgentID, const TArray<FString>& Outputs);

    // Quality Gates
    UFUNCTION(BlueprintCallable, Category = "Production Pipeline")
    bool ValidatePhaseCompletion(EProductionPhase Phase);

    UFUNCTION(BlueprintCallable, Category = "Production Pipeline")
    bool CheckDependencies(int32 AgentID);

    // Metrics and Monitoring
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Production Pipeline")
    FProductionMetrics GetProductionMetrics() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Production Pipeline")
    TArray<FAgentWorkItem> GetAgentStatuses() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Production Pipeline")
    FAgentWorkItem GetAgentStatus(int32 AgentID) const;

    // Emergency Controls
    UFUNCTION(BlueprintCallable, Category = "Production Pipeline")
    void EmergencyStop(const FString& Reason);

    UFUNCTION(BlueprintCallable, Category = "Production Pipeline")
    void RollbackToLastStableState();

    UFUNCTION(BlueprintCallable, Category = "Production Pipeline")
    void ForceAgentSequence(const TArray<int32>& AgentOrder);

protected:
    // Internal state
    UPROPERTY()
    TArray<FAgentWorkItem> AgentWorkItems;

    UPROPERTY()
    FProductionMetrics CurrentMetrics;

    UPROPERTY()
    FString CurrentCycleID;

    UPROPERTY()
    int32 CurrentAgentIndex;

    UPROPERTY()
    bool bProductionActive;

    UPROPERTY()
    bool bEmergencyStop;

    // Agent definitions
    void InitializeAgentDefinitions();
    void UpdateMetrics();
    void LogProductionEvent(const FString& Event);
    bool ValidateAgentOutput(int32 AgentID, const TArray<FString>& Outputs);

    // Dependencies map (AgentID -> Required AgentIDs)
    TMap<int32, TArray<int32>> AgentDependencies;

    // Agent execution order
    TArray<int32> ProductionSequence;

private:
    // Studio Director integration
    UPROPERTY()
    UStudioDirectorSubsystem* StudioDirector;

    // Architecture validation
    UPROPERTY()
    UEngineArchitectureValidator* ArchitectureValidator;
};