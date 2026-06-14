#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Dir_ProductionCoordinator.generated.h"

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction,
    CoreSystems,
    WorldBuilding,
    CharacterImplementation,
    GameplayIntegration,
    Polish,
    Release
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
    int32 Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float CompletionPercentage;

    FDir_AgentTask()
    {
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Priority = 1;
        bIsCompleted = false;
        CompletionPercentage = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalActorsInLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 DinosaurCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CompletedSystems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float OverallProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    FString CurrentBottleneck;

    FDir_ProductionMetrics()
    {
        TotalActorsInLevel = 0;
        DinosaurCount = 0;
        CompletedSystems = 0;
        OverallProgress = 0.0f;
        CurrentBottleneck = TEXT("Unknown");
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDir_ProductionCoordinator : public UActorComponent
{
    GENERATED_BODY()

public:
    UDir_ProductionCoordinator();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production", meta = (AllowPrivateAccess = "true"))
    EDir_ProductionPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production", meta = (AllowPrivateAccess = "true"))
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production", meta = (AllowPrivateAccess = "true"))
    FDir_ProductionMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production", meta = (AllowPrivateAccess = "true"))
    float CycleStartTime;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Production")
    void InitializeProductionCycle();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateAgentTask(const FString& AgentName, float CompletionPercentage, bool bCompleted = false);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void AnalyzeCurrentState();

    UFUNCTION(BlueprintCallable, Category = "Production")
    FDir_ProductionMetrics GetProductionMetrics() const { return CurrentMetrics; }

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_AgentTask> GetCriticalTasks() const;

    UFUNCTION(BlueprintCallable, Category = "Production", CallInEditor)
    void GenerateProductionReport();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void SetProductionPhase(EDir_ProductionPhase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool ValidateMinimumPlayableState() const;

private:
    void UpdateMetrics();
    void IdentifyBottlenecks();
    void LogProductionState();
};

#include "Dir_ProductionCoordinator.generated.h"