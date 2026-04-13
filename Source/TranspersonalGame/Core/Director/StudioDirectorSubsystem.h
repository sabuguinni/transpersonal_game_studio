#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/Engine.h"
#include "StudioDirectorSubsystem.generated.h"

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction   UMETA(DisplayName = "Pre-Production"),
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
    int32 AgentID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FString AgentName = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    EDir_AgentStatus Status = EDir_AgentStatus::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FString CurrentTask = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    float Progress = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FDateTime LastUpdate;

    FDir_AgentInfo()
    {
        AgentID = 0;
        AgentName = TEXT("");
        Status = EDir_AgentStatus::Idle;
        CurrentTask = TEXT("");
        Progress = 0.0f;
        LastUpdate = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalAgents = 19;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 ActiveAgents = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CompletedTasks = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 BlockedAgents = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float OverallProgress = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    FDateTime CycleStartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    FDateTime LastUpdateTime;

    FDir_ProductionMetrics()
    {
        TotalAgents = 19;
        ActiveAgents = 0;
        CompletedTasks = 0;
        BlockedAgents = 0;
        OverallProgress = 0.0f;
        CycleStartTime = FDateTime::Now();
        LastUpdateTime = FDateTime::Now();
    }
};

/**
 * Studio Director Subsystem - Central coordination hub for the 19-agent production pipeline
 * Manages agent states, production metrics, and ensures proper execution order
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UStudioDirectorSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Production Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void StartProductionCycle(const FString& CycleID);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus, const FString& TaskDescription, float Progress);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CompleteAgentTask(int32 AgentID, const FString& Output);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void BlockAgent(int32 AgentID, const FString& BlockReason);

    // Metrics and Monitoring
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Studio Director")
    FDir_ProductionMetrics GetProductionMetrics() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Studio Director")
    TArray<FDir_AgentInfo> GetAllAgentStates() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Studio Director")
    FDir_AgentInfo GetAgentInfo(int32 AgentID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Studio Director")
    bool CanAgentProceed(int32 AgentID) const;

    // Production Flow Control
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void TriggerNextAgent();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void EmergencyStop(const FString& Reason);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void ResetProductionPipeline();

    // Debug and Development
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Studio Director")
    void DebugLogAllAgentStates();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Studio Director")
    void SimulateProductionCycle();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Studio Director")
    EDir_ProductionPhase CurrentPhase = EDir_ProductionPhase::PreProduction;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Studio Director")
    FString CurrentCycleID = TEXT("");

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Studio Director")
    TArray<FDir_AgentInfo> AgentStates;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Studio Director")
    FDir_ProductionMetrics Metrics;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Studio Director")
    int32 CurrentActiveAgent = 1;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Studio Director")
    bool bProductionInProgress = false;

private:
    void InitializeAgentStates();
    void UpdateMetrics();
    bool ValidateAgentDependencies(int32 AgentID) const;
    void LogProductionEvent(const FString& Event, const FString& Details = TEXT(""));
};