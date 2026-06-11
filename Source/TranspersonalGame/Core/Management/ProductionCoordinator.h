#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "ProductionCoordinator.generated.h"

/**
 * Studio Director Agent #01 - Production Coordination System
 * Tracks agent deliverables, coordinates development pipeline, manages production metrics
 */

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Working UMETA(DisplayName = "Working"), 
    Completed UMETA(DisplayName = "Completed"),
    Blocked UMETA(DisplayName = "Blocked"),
    Failed UMETA(DisplayName = "Failed")
};

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction UMETA(DisplayName = "Pre-Production"),
    PrototypePhase UMETA(DisplayName = "Prototype Phase"),
    CoreDevelopment UMETA(DisplayName = "Core Development"),
    ContentCreation UMETA(DisplayName = "Content Creation"),
    Polish UMETA(DisplayName = "Polish"),
    Release UMETA(DisplayName = "Release")
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    int32 AgentNumber = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString AgentName = "";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString CurrentTask = "";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    EDir_AgentStatus Status = EDir_AgentStatus::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    float CompletionPercentage = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString LastDeliverable = "";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FDateTime LastUpdate;

    FDir_AgentTask()
    {
        LastUpdate = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 TotalCppFiles = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 TotalHeaderFiles = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 TotalBlueprints = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 TotalAssets = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 CompilationErrors = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    float BuildTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    EDir_ProductionPhase CurrentPhase = EDir_ProductionPhase::PreProduction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    FDateTime LastBuild;

    FDir_ProductionMetrics()
    {
        LastBuild = FDateTime::Now();
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AProductionCoordinator : public AActor
{
    GENERATED_BODY()

public:
    AProductionCoordinator();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    // Production tracking data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Coordination")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Coordination")
    FDir_ProductionMetrics ProductionMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Coordination")
    FString CurrentCycle = "CYCLE_020";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Coordination")
    int32 CycleNumber = 20;

public:
    virtual void Tick(float DeltaTime) override;

    // Agent coordination functions
    UFUNCTION(BlueprintCallable, Category = "Production Coordination")
    void InitializeAgentTasks();

    UFUNCTION(BlueprintCallable, Category = "Production Coordination")
    void UpdateAgentStatus(int32 AgentNumber, EDir_AgentStatus NewStatus, const FString& TaskDescription);

    UFUNCTION(BlueprintCallable, Category = "Production Coordination")
    void SetAgentDeliverable(int32 AgentNumber, const FString& DeliverablePath);

    UFUNCTION(BlueprintCallable, Category = "Production Coordination")
    FDir_AgentTask GetAgentTask(int32 AgentNumber) const;

    UFUNCTION(BlueprintCallable, Category = "Production Coordination")
    TArray<FDir_AgentTask> GetBlockedAgents() const;

    // Production metrics functions
    UFUNCTION(BlueprintCallable, Category = "Production Metrics")
    void UpdateProductionMetrics();

    UFUNCTION(BlueprintCallable, Category = "Production Metrics")
    void IncrementFileCount(bool bIsCppFile);

    UFUNCTION(BlueprintCallable, Category = "Production Metrics")
    void ReportCompilationError(const FString& ErrorMessage);

    UFUNCTION(BlueprintCallable, Category = "Production Metrics")
    void SetBuildTime(float BuildTimeSeconds);

    UFUNCTION(BlueprintCallable, Category = "Production Metrics")
    FDir_ProductionMetrics GetProductionMetrics() const { return ProductionMetrics; }

    // Coordination workflow functions
    UFUNCTION(BlueprintCallable, Category = "Workflow Coordination")
    void StartProductionCycle(int32 NewCycleNumber);

    UFUNCTION(BlueprintCallable, Category = "Workflow Coordination")
    bool CanAgentProceed(int32 AgentNumber) const;

    UFUNCTION(BlueprintCallable, Category = "Workflow Coordination")
    void BlockAgent(int32 AgentNumber, const FString& BlockReason);

    UFUNCTION(BlueprintCallable, Category = "Workflow Coordination")
    void UnblockAgent(int32 AgentNumber);

    // Priority task assignment
    UFUNCTION(BlueprintCallable, Category = "Task Assignment")
    void AssignPriorityTask(int32 AgentNumber, const FString& TaskDescription, float Priority);

    UFUNCTION(BlueprintCallable, Category = "Task Assignment")
    TArray<FDir_AgentTask> GetHighPriorityTasks() const;

    // Development phase management
    UFUNCTION(BlueprintCallable, Category = "Phase Management")
    void AdvanceProductionPhase();

    UFUNCTION(BlueprintCallable, Category = "Phase Management")
    bool IsPhaseComplete(EDir_ProductionPhase Phase) const;

    UFUNCTION(BlueprintCallable, Category = "Phase Management")
    FString GetPhaseDescription(EDir_ProductionPhase Phase) const;

    // Debug and logging
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void LogProductionStatus();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void ValidateAgentDependencies();

private:
    void InitializeDefaultAgents();
    bool ValidateAgentChain() const;
    void LogAgentStatus(const FDir_AgentTask& Task) const;
};