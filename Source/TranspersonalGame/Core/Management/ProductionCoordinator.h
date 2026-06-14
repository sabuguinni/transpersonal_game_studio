#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "ProductionCoordinator.generated.h"

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction UMETA(DisplayName = "Pre-Production"),
    Prototype UMETA(DisplayName = "Prototype"),
    Alpha UMETA(DisplayName = "Alpha"),
    Beta UMETA(DisplayName = "Beta"),
    Release UMETA(DisplayName = "Release")
};

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Active UMETA(DisplayName = "Active"),
    Complete UMETA(DisplayName = "Complete"),
    Blocked UMETA(DisplayName = "Blocked"),
    Error UMETA(DisplayName = "Error")
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    EDir_AgentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float CompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FDateTime LastUpdate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FString> Dependencies;

    FDir_AgentTask()
    {
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Status = EDir_AgentStatus::Idle;
        CompletionPercentage = 0.0f;
        LastUpdate = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 DinosaurCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CharacterCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TerrainElements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float OverallProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    FDateTime LastMetricsUpdate;

    FDir_ProductionMetrics()
    {
        TotalActors = 0;
        DinosaurCount = 0;
        CharacterCount = 0;
        TerrainElements = 0;
        OverallProgress = 0.0f;
        LastMetricsUpdate = FDateTime::Now();
    }
};

/**
 * Production Coordinator - Manages the entire game development pipeline
 * Coordinates between 19 specialized AI agents and tracks production progress
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UProductionCoordinator : public UActorComponent
{
    GENERATED_BODY()

public:
    UProductionCoordinator();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Production Phase Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Phase")
    EDir_ProductionPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Phase")
    float PhaseProgress;

    // Agent Task Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Management")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Management")
    TMap<FString, EDir_AgentStatus> AgentStatusMap;

    // Production Metrics
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    FDir_ProductionMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    TArray<FDir_ProductionMetrics> MetricsHistory;

public:
    // Agent Coordination Functions
    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateAgentStatus(const FString& AgentName, EDir_AgentStatus NewStatus, float CompletionPercentage = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void AssignTaskToAgent(const FString& AgentName, const FString& TaskDescription, const TArray<FString>& Dependencies = TArray<FString>());

    UFUNCTION(BlueprintCallable, Category = "Production")
    EDir_AgentStatus GetAgentStatus(const FString& AgentName) const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FString> GetBlockedAgents() const;

    // Production Phase Management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void AdvanceToNextPhase();

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool CanAdvancePhase() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    FString GetCurrentPhaseDescription() const;

    // Metrics and Reporting
    UFUNCTION(BlueprintCallable, Category = "Metrics")
    void UpdateProductionMetrics();

    UFUNCTION(BlueprintCallable, Category = "Metrics")
    FDir_ProductionMetrics GetCurrentMetrics() const { return CurrentMetrics; }

    UFUNCTION(BlueprintCallable, Category = "Metrics")
    float GetOverallProgress() const;

    UFUNCTION(BlueprintCallable, Category = "Metrics")
    void GenerateProgressReport();

    // Debug and Development
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void InitializeAgentTasks();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void ResetProductionState();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void PrintProductionStatus();

private:
    // Internal helper functions
    void InitializeDefaultAgents();
    void CheckDependencies();
    void UpdatePhaseProgress();
    void LogProductionEvent(const FString& Event);

    // Timer handles
    FTimerHandle MetricsUpdateTimer;
    FTimerHandle DependencyCheckTimer;
};