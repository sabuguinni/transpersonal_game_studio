#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Components/SceneComponent.h"
#include "ProductionCoordinator.generated.h"

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction,
    CoreSystems,
    WorldBuilding,
    CharacterSystems,
    GameplayMechanics,
    Polish,
    Testing
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_AgentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    TArray<FString> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float EstimatedHours;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float CompletionPercentage;

    FDir_AgentTask()
    {
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Status = EDir_AgentStatus::Idle;
        Priority = 1.0f;
        EstimatedHours = 1.0f;
        CompletionPercentage = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float TerrainCompletion;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float CharacterSystemCompletion;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float DinosaurPopulationCompletion;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float OverallProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalActorsInLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CompletedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalTasks;

    FDir_ProductionMetrics()
    {
        TerrainCompletion = 0.0f;
        CharacterSystemCompletion = 0.0f;
        DinosaurPopulationCompletion = 0.0f;
        OverallProgress = 0.0f;
        TotalActorsInLevel = 0;
        CompletedTasks = 0;
        TotalTasks = 0;
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    EDir_ProductionPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FDir_ProductionMetrics ProductionMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float CycleStartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 CurrentCycleNumber;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Production")
    void InitializeProductionCycle(int32 CycleNumber);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateAgentTask(const FString& AgentName, EDir_AgentStatus NewStatus, float CompletionPercentage);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void AddAgentTask(const FDir_AgentTask& NewTask);

    UFUNCTION(BlueprintCallable, Category = "Production")
    FDir_ProductionMetrics CalculateProductionMetrics();

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_AgentTask> GetPriorityTasks();

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool CheckTaskDependencies(const FString& AgentName);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void AdvanceProductionPhase();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Production")
    void GenerateProductionReport();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Production")
    void ResetProductionState();

    UFUNCTION(BlueprintPure, Category = "Production")
    float GetOverallProgress() const;

    UFUNCTION(BlueprintPure, Category = "Production")
    int32 GetCompletedTaskCount() const;

    UFUNCTION(BlueprintPure, Category = "Production")
    FString GetCurrentPhaseString() const;

private:
    void UpdateProductionMetrics();
    void LogProductionState();
    void ValidateAgentDependencies();
};