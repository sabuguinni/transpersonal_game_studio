#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Dir_ProductionCoordinator.generated.h"

/**
 * Production phases for the Transpersonal Game Studio development pipeline
 */
UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction   UMETA(DisplayName = "Pre-Production"),
    Prototype       UMETA(DisplayName = "Prototype"),
    Production      UMETA(DisplayName = "Production"),
    Alpha           UMETA(DisplayName = "Alpha"),
    Beta            UMETA(DisplayName = "Beta"),
    Release         UMETA(DisplayName = "Release")
};

/**
 * Agent specialization types in the 19-agent production pipeline
 */
UENUM(BlueprintType)
enum class EDir_AgentType : uint8
{
    StudioDirector      UMETA(DisplayName = "Studio Director"),
    EngineArchitect     UMETA(DisplayName = "Engine Architect"),
    CoreSystems         UMETA(DisplayName = "Core Systems"),
    Performance         UMETA(DisplayName = "Performance"),
    WorldGenerator      UMETA(DisplayName = "World Generator"),
    Environment         UMETA(DisplayName = "Environment"),
    Architecture        UMETA(DisplayName = "Architecture"),
    Lighting            UMETA(DisplayName = "Lighting"),
    Character           UMETA(DisplayName = "Character"),
    Animation           UMETA(DisplayName = "Animation"),
    NPCBehavior         UMETA(DisplayName = "NPC Behavior"),
    CombatAI            UMETA(DisplayName = "Combat AI"),
    CrowdSimulation     UMETA(DisplayName = "Crowd Simulation"),
    QuestDesign         UMETA(DisplayName = "Quest Design"),
    Narrative           UMETA(DisplayName = "Narrative"),
    Audio               UMETA(DisplayName = "Audio"),
    VFX                 UMETA(DisplayName = "VFX"),
    QA                  UMETA(DisplayName = "QA"),
    Integration         UMETA(DisplayName = "Integration")
};

/**
 * Production metrics tracking structure
 */
USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 TotalActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 DinosaurCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 CharacterCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 EnvironmentProps = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float ProductionProgress = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 CompletedTasks = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 PendingTasks = 0;

    FDir_ProductionMetrics()
    {
        TotalActors = 0;
        DinosaurCount = 0;
        CharacterCount = 0;
        EnvironmentProps = 0;
        ProductionProgress = 0.0f;
        CompletedTasks = 0;
        PendingTasks = 0;
    }
};

/**
 * Agent task definition for the production pipeline
 */
USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Task")
    EDir_AgentType AgentType;

    UPROPERTY(BlueprintReadOnly, Category = "Task")
    FString TaskDescription;

    UPROPERTY(BlueprintReadOnly, Category = "Task")
    int32 Priority = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Task")
    bool bIsCompleted = false;

    UPROPERTY(BlueprintReadOnly, Category = "Task")
    TArray<EDir_AgentType> Dependencies;

    FDir_AgentTask()
    {
        AgentType = EDir_AgentType::StudioDirector;
        TaskDescription = TEXT("Default Task");
        Priority = 0;
        bIsCompleted = false;
    }
};

/**
 * Studio Director Production Coordinator
 * Manages the 19-agent development pipeline for the prehistoric survival game
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADir_ProductionCoordinator : public AActor
{
    GENERATED_BODY()

public:
    ADir_ProductionCoordinator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Production Phase Management
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Production")
    EDir_ProductionPhase CurrentPhase;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    FDir_ProductionMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    TArray<FDir_AgentTask> TaskQueue;

    // Core Production Functions
    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateProductionMetrics();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void AssignTaskToAgent(EDir_AgentType AgentType, const FString& TaskDescription, int32 Priority);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void CompleteAgentTask(EDir_AgentType AgentType);

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool CanAgentProceed(EDir_AgentType AgentType) const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    float GetOverallProgress() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    void AdvanceProductionPhase();

    // Critical Path Management
    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<EDir_AgentType> GetCriticalPath() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    void ValidateAgentDependencies();

    // Milestone Tracking
    UFUNCTION(BlueprintCallable, Category = "Production")
    bool IsMilestone1Complete() const; // Walk Around milestone

    UFUNCTION(BlueprintCallable, Category = "Production")
    void ReportMilestoneProgress();

private:
    // Internal tracking
    UPROPERTY()
    float LastMetricsUpdate;

    UPROPERTY()
    TMap<EDir_AgentType, bool> AgentStatus;

    void InitializeAgentDependencies();
    void UpdateTaskPriorities();
    bool CheckDependenciesComplete(const TArray<EDir_AgentType>& Dependencies) const;
};