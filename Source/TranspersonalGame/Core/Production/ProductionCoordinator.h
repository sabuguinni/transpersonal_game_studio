#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "ProductionCoordinator.generated.h"

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction,
    Prototype,
    Alpha,
    Beta,
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

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    FString AgentName;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    FString TaskDescription;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    EDir_AgentStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    float Priority;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    FDateTime StartTime;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
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

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 TotalActors;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 DinosaurCount;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 CharacterCount;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 TerrainActors;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float CompletionPercentage;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    EDir_ProductionPhase CurrentPhase;

    FDir_ProductionMetrics()
    {
        TotalActors = 0;
        DinosaurCount = 0;
        CharacterCount = 0;
        TerrainActors = 0;
        CompletionPercentage = 0.0f;
        CurrentPhase = EDir_ProductionPhase::PreProduction;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UProductionCoordinator : public UActorComponent
{
    GENERATED_BODY()

public:
    UProductionCoordinator();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    FDir_ProductionMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadWrite, Category = "Production")
    float ProductionUpdateInterval;

    FTimerHandle ProductionTimerHandle;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Production")
    void AssignTaskToAgent(const FString& AgentName, const FString& TaskDescription, float Priority = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateAgentStatus(const FString& AgentName, EDir_AgentStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateProductionMetrics();

    UFUNCTION(BlueprintCallable, Category = "Production")
    FDir_ProductionMetrics GetProductionMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_AgentTask> GetAgentTasks() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    void SetProductionPhase(EDir_ProductionPhase NewPhase);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Production")
    void InitializeProductionPipeline();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Production")
    void GenerateProductionReport();

private:
    void UpdateProductionTimer();
    void AnalyzeWorldState();
    void CoordinateAgentPriorities();
};

#include "ProductionCoordinator.generated.h"