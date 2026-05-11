#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Dir_ProductionManager.generated.h"

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction,
    PrototypeDevelopment,
    VerticalSlice,
    Production,
    Polish,
    Release
};

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle,
    Working,
    Completed,
    Blocked,
    Failed
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
    float Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FDateTime StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FDateTime EstimatedCompletion;

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
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalCppFiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalHeaderFiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CompilationErrors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 ActorsInLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float FrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    FDateTime LastBuildTime;

    FDir_ProductionMetrics()
    {
        TotalCppFiles = 0;
        TotalHeaderFiles = 0;
        CompilationErrors = 0;
        ActorsInLevel = 0;
        FrameRate = 60.0f;
        LastBuildTime = FDateTime::Now();
    }
};

/**
 * Studio Director Production Manager
 * Coordinates the 18-agent development pipeline
 * Tracks progress, manages dependencies, and ensures quality
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDir_ProductionManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UDir_ProductionManager();

    // Subsystem overrides
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Production Management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void StartProductionCycle(int32 CycleNumber);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void CompleteProductionCycle();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void AssignTaskToAgent(const FString& AgentName, const FString& TaskDescription, float Priority);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateAgentStatus(const FString& AgentName, EDir_AgentStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void BlockAgent(const FString& AgentName, const FString& Reason);

    // Metrics and Monitoring
    UFUNCTION(BlueprintCallable, Category = "Metrics")
    FDir_ProductionMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Metrics")
    void UpdateMetrics();

    UFUNCTION(BlueprintCallable, Category = "Metrics")
    TArray<FDir_AgentTask> GetAllAgentTasks() const;

    UFUNCTION(BlueprintCallable, Category = "Metrics")
    TArray<FDir_AgentTask> GetActiveAgentTasks() const;

    // Quality Assurance
    UFUNCTION(BlueprintCallable, Category = "QA")
    bool ValidateMinimumViablePrototype() const;

    UFUNCTION(BlueprintCallable, Category = "QA")
    bool CheckCompilationStatus() const;

    UFUNCTION(BlueprintCallable, Category = "QA")
    int32 CountActorsInLevel() const;

    // Production Phase Management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void SetProductionPhase(EDir_ProductionPhase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "Production")
    EDir_ProductionPhase GetCurrentProductionPhase() const;

    // Emergency Controls
    UFUNCTION(BlueprintCallable, Category = "Emergency", CallInEditor)
    void EmergencyStopProduction();

    UFUNCTION(BlueprintCallable, Category = "Emergency", CallInEditor)
    void ForceCompilationCheck();

    UFUNCTION(BlueprintCallable, Category = "Emergency", CallInEditor)
    void GenerateProductionReport();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Production")
    EDir_ProductionPhase CurrentPhase;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Production")
    int32 CurrentCycleNumber;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Production")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Metrics")
    FDir_ProductionMetrics CurrentMetrics;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Production")
    bool bProductionActive;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Production")
    FDateTime CycleStartTime;

private:
    void InitializeAgentTasks();
    void LogProductionStatus() const;
    void ValidateAgentDependencies();
};