#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Dir_StudioDirector.generated.h"

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction   UMETA(DisplayName = "Pre-Production"),
    Prototype       UMETA(DisplayName = "Prototype"),
    Production      UMETA(DisplayName = "Production"),
    Polish          UMETA(DisplayName = "Polish"),
    Release         UMETA(DisplayName = "Release")
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    int32 Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    float CompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    bool bIsBlocked;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString BlockingReason;

    FDir_AgentTask()
    {
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Priority = 0;
        CompletionPercentage = 0.0f;
        bIsBlocked = false;
        BlockingReason = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 TotalActorsInLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 DinosaurCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 CompletedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 BlockedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    float OverallProgress;

    FDir_ProductionMetrics()
    {
        TotalActorsInLevel = 0;
        DinosaurCount = 0;
        CompletedTasks = 0;
        BlockedTasks = 0;
        OverallProgress = 0.0f;
    }
};

/**
 * Studio Director - Coordinates all 19 AI agents and manages production pipeline
 * Translates creative vision into technical tasks and ensures milestone delivery
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADir_StudioDirector : public AActor
{
    GENERATED_BODY()

public:
    ADir_StudioDirector();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    // Production Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    EDir_ProductionPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FDir_ProductionMetrics ProductionMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float CycleUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString CurrentCycleID;

public:
    virtual void Tick(float DeltaTime) override;

    // Agent Management Functions
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void InitializeAgentTasks();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateProductionMetrics();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void AssignTaskToAgent(const FString& AgentName, const FString& TaskDescription, int32 Priority);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void MarkTaskCompleted(const FString& AgentName, float CompletionPercentage);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void BlockTask(const FString& AgentName, const FString& Reason);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UnblockTask(const FString& AgentName);

    // Production Phase Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void AdvanceProductionPhase();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool CanAdvancePhase() const;

    // Milestone Tracking
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool IsMilestone1Complete() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void ValidatePlayablePrototype();

    // Agent Coordination
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CoordinateAgentChain();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FString GetNextAgentInChain(const FString& CurrentAgent) const;

    // Debug and Monitoring
    UFUNCTION(BlueprintCallable, Category = "Studio Director", CallInEditor)
    void LogProductionStatus();

    UFUNCTION(BlueprintCallable, Category = "Studio Director", CallInEditor)
    void GenerateProductionReport();

protected:
    // Internal tracking
    UPROPERTY()
    float LastUpdateTime;

    UPROPERTY()
    TMap<FString, int32> AgentPriorities;

    // Agent chain definition (19 agents)
    UPROPERTY()
    TArray<FString> AgentChain;

    void SetupAgentChain();
    void ValidateAgentOutputs();
    void EnforceProductionRules();
};