#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "ProductionCoordinator.generated.h"

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction   UMETA(DisplayName = "Pre-Production"),
    Prototype      UMETA(DisplayName = "Prototype"),
    Alpha          UMETA(DisplayName = "Alpha"),
    Beta           UMETA(DisplayName = "Beta"),
    Gold           UMETA(DisplayName = "Gold")
};

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Pending        UMETA(DisplayName = "Pending"),
    Active         UMETA(DisplayName = "Active"),
    Completed      UMETA(DisplayName = "Completed"),
    Blocked        UMETA(DisplayName = "Blocked")
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
    int32 Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float CompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FDateTime LastUpdate;

    FDir_AgentTask()
    {
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Status = EDir_AgentStatus::Pending;
        Priority = 0;
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
    int32 TerrainActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CharacterActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 DinosaurActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 EnvironmentActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 LightingActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float PrototypeCompletion;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    FDateTime LastAnalysis;

    FDir_ProductionMetrics()
    {
        TotalActors = 0;
        TerrainActors = 0;
        CharacterActors = 0;
        DinosaurActors = 0;
        EnvironmentActors = 0;
        LightingActors = 0;
        PrototypeCompletion = 0.0f;
        LastAnalysis = FDateTime::Now();
    }
};

/**
 * Production Coordinator - Studio Director's central coordination system
 * Manages agent tasks, tracks production metrics, and coordinates the 19-agent pipeline
 */
UCLASS(BlueprintType, Blueprintable)
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
    float CycleCompletionThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production", meta = (AllowPrivateAccess = "true"))
    int32 CurrentCycleNumber;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Production")
    void InitializeAgentTasks();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateAgentTask(const FString& AgentName, EDir_AgentStatus NewStatus, float CompletionPercentage);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void AnalyzeProductionState();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void AdvanceProductionPhase();

    UFUNCTION(BlueprintCallable, Category = "Production")
    FDir_ProductionMetrics GetCurrentMetrics() const { return CurrentMetrics; }

    UFUNCTION(BlueprintCallable, Category = "Production")
    EDir_ProductionPhase GetCurrentPhase() const { return CurrentPhase; }

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_AgentTask> GetAgentTasks() const { return AgentTasks; }

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool IsPrototypeComplete() const;

    UFUNCTION(BlueprintCallable, Category = "Production", CallInEditor)
    void ForceProductionAnalysis();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void LogProductionStatus();

private:
    void CountActorsByCategory();
    void UpdatePrototypeCompletion();
    void CheckPhaseTransition();
};