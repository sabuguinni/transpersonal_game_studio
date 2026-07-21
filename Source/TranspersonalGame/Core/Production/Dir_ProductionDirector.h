#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Dir_ProductionDirector.generated.h"

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction UMETA(DisplayName = "Pre-Production"),
    Prototype UMETA(DisplayName = "Prototype"),
    Alpha UMETA(DisplayName = "Alpha"),
    Beta UMETA(DisplayName = "Beta"),
    Gold UMETA(DisplayName = "Gold Master"),
    PostLaunch UMETA(DisplayName = "Post-Launch")
};

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Working UMETA(DisplayName = "Working"),
    Blocked UMETA(DisplayName = "Blocked"),
    Complete UMETA(DisplayName = "Complete"),
    Failed UMETA(DisplayName = "Failed")
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_AgentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float CompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FDateTime StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FDateTime EstimatedCompletion;

    FDir_AgentTask()
    {
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Status = EDir_AgentStatus::Idle;
        Priority = 1.0f;
        CompletionPercentage = 0.0f;
        StartTime = FDateTime::Now();
        EstimatedCompletion = FDateTime::Now();
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
    int32 TerrainActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float OverallProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float FrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float MemoryUsage;

    FDir_ProductionMetrics()
    {
        TotalActors = 0;
        DinosaurCount = 0;
        CharacterCount = 0;
        TerrainActors = 0;
        OverallProgress = 0.0f;
        FrameRate = 0.0f;
        MemoryUsage = 0.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDir_ProductionDirector : public UActorComponent
{
    GENERATED_BODY()

public:
    UDir_ProductionDirector();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Production Phase Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    EDir_ProductionPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FDir_ProductionMetrics CurrentMetrics;

    // Agent Management Functions
    UFUNCTION(BlueprintCallable, Category = "Production")
    void AssignTaskToAgent(const FString& AgentName, const FString& TaskDescription, float Priority);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateAgentStatus(const FString& AgentName, EDir_AgentStatus NewStatus, float CompletionPercentage);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void AdvanceProductionPhase();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateProductionMetrics();

    UFUNCTION(BlueprintCallable, Category = "Production")
    FDir_ProductionMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_AgentTask> GetAgentTasks() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    float GetOverallProgress() const;

    // Critical Path Analysis
    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FString> GetBlockedAgents() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    void ResolveBlockedAgent(const FString& AgentName);

    // Production Coordination
    UFUNCTION(BlueprintCallable, Category = "Production")
    void CoordinateAgentChain();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void ValidateProductionState();

private:
    void InitializeAgentTasks();
    void CalculateOverallProgress();
    void MonitorPerformanceMetrics();
    void EnforceProductionRules();

    UPROPERTY()
    float LastMetricsUpdate;

    UPROPERTY()
    bool bProductionActive;
};