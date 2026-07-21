#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "UObject/NoExportTypes.h"
#include "Engine/World.h"
#include "../SharedTypes.h"
#include "Dir_AgentCoordinator.generated.h"

UENUM(BlueprintType)
enum class EDir_AgentType : uint8
{
    StudioDirector = 1,
    EngineArchitect = 2,
    CoreSystems = 3,
    Performance = 4,
    WorldGenerator = 5,
    EnvironmentArtist = 6,
    Architecture = 7,
    Lighting = 8,
    CharacterArtist = 9,
    Animation = 10,
    NPCBehavior = 11,
    CombatAI = 12,
    CrowdSimulation = 13,
    QuestDesigner = 14,
    Narrative = 15,
    Audio = 16,
    VFX = 17,
    QA = 18,
    Integration = 19
};

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction,
    PrototypeDevelopment,
    ProductionSprint,
    PolishPhase,
    Release
};

UENUM(BlueprintType)
enum class EDir_TaskPriority : uint8
{
    Critical,
    High,
    Medium,
    Low
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_AgentType AssignedAgent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_TaskPriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    TArray<FString> RequiredDeliverables;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    TArray<EDir_AgentType> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float EstimatedCycles;

    FDir_AgentTask()
    {
        AssignedAgent = EDir_AgentType::StudioDirector;
        TaskDescription = TEXT("");
        Priority = EDir_TaskPriority::Medium;
        bIsCompleted = false;
        EstimatedCycles = 1.0f;
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
    int32 CharacterCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TerrainActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CompletedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 PendingTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float OverallProgress;

    FDir_ProductionMetrics()
    {
        TotalActorsInLevel = 0;
        DinosaurCount = 0;
        CharacterCount = 0;
        TerrainActorCount = 0;
        CompletedTasks = 0;
        PendingTasks = 0;
        OverallProgress = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDir_AgentCoordinator : public UObject
{
    GENERATED_BODY()

public:
    UDir_AgentCoordinator();

    // Core coordination functions
    UFUNCTION(BlueprintCallable, Category = "Director")
    void InitializeProductionCycle();

    UFUNCTION(BlueprintCallable, Category = "Director")
    void AssignTaskToAgent(EDir_AgentType Agent, const FString& TaskDescription, EDir_TaskPriority Priority);

    UFUNCTION(BlueprintCallable, Category = "Director")
    void MarkTaskCompleted(EDir_AgentType Agent, const FString& TaskDescription);

    UFUNCTION(BlueprintCallable, Category = "Director")
    FDir_ProductionMetrics GetCurrentMetrics();

    UFUNCTION(BlueprintCallable, Category = "Director")
    TArray<FDir_AgentTask> GetPendingTasksForAgent(EDir_AgentType Agent);

    UFUNCTION(BlueprintCallable, Category = "Director")
    void SetProductionPhase(EDir_ProductionPhase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "Director")
    bool ValidateAgentDependencies(EDir_AgentType Agent);

    UFUNCTION(BlueprintCallable, Category = "Director")
    void GenerateProductionReport();

    // Critical path management
    UFUNCTION(BlueprintCallable, Category = "Director")
    TArray<EDir_AgentType> GetCriticalPathAgents();

    UFUNCTION(BlueprintCallable, Category = "Director")
    void UpdateProductionMetrics();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    EDir_ProductionPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_AgentTask> ActiveTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FDir_ProductionMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TMap<EDir_AgentType, bool> AgentReadyStatus;

private:
    void InitializeAgentDependencies();
    void CalculateOverallProgress();
    bool CheckAgentPrerequisites(EDir_AgentType Agent);
};