#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "../../SharedTypes.h"
#include "ProductionCoordinator.generated.h"

/**
 * Studio Director's Production Coordination System
 * Manages the 18-agent development pipeline and ensures proper task handoff
 * between specialized agents during development cycles.
 */

UENUM(BlueprintType)
enum class EDir_AgentType : uint8
{
    StudioDirector = 1,
    EngineArchitect = 2,
    CoreSystemsProgrammer = 3,
    PerformanceOptimizer = 4,
    ProceduralWorldGenerator = 5,
    EnvironmentArtist = 6,
    ArchitectureInterior = 7,
    LightingAtmosphere = 8,
    CharacterArtist = 9,
    AnimationAgent = 10,
    NPCBehavior = 11,
    CombatEnemyAI = 12,
    CrowdTrafficSimulation = 13,
    QuestMissionDesigner = 14,
    NarrativeDialogue = 15,
    AudioAgent = 16,
    VFXAgent = 17,
    QATesting = 18,
    IntegrationBuild = 19
};

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    Architecture,
    CoreSystems,
    WorldGeneration,
    ContentCreation,
    Polish,
    Integration,
    Testing
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    EDir_AgentType AgentType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    EDir_ProductionPhase Phase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    bool bIsBlocked;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float Priority;

    FDir_AgentTask()
    {
        AgentType = EDir_AgentType::StudioDirector;
        TaskDescription = TEXT("");
        Phase = EDir_ProductionPhase::Architecture;
        bIsCompleted = false;
        bIsBlocked = false;
        Dependencies = TEXT("");
        Priority = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalCycles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CompletedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 BlockedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float AverageTaskTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CompilationErrors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 UE5BridgeFailures;

    FDir_ProductionMetrics()
    {
        TotalCycles = 0;
        CompletedTasks = 0;
        BlockedTasks = 0;
        AverageTaskTime = 0.0f;
        CompilationErrors = 0;
        UE5BridgeFailures = 0;
    }
};

/**
 * Production Coordinator Component
 * Manages agent task distribution and pipeline coordination
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDir_ProductionCoordinator : public UActorComponent
{
    GENERATED_BODY()

public:
    UDir_ProductionCoordinator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Production Pipeline Management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void InitializeProductionPipeline();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void StartProductionCycle();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void CompleteAgentTask(EDir_AgentType AgentType, bool bSuccess);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void BlockAgentTask(EDir_AgentType AgentType, const FString& Reason);

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_AgentTask> GetPendingTasks();

    UFUNCTION(BlueprintCallable, Category = "Production")
    FDir_AgentTask GetNextTaskForAgent(EDir_AgentType AgentType);

    // Milestone Management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void CheckMilestone1Progress();

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool IsMilestone1Complete();

    // Crisis Management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void HandleUE5BridgeFailure();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void HandleCompilationError(const FString& ErrorDetails);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void EmergencyFallback();

    // Metrics and Reporting
    UFUNCTION(BlueprintCallable, Category = "Production")
    FDir_ProductionMetrics GetProductionMetrics();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void GenerateProductionReport();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_AgentTask> CurrentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    EDir_ProductionPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FDir_ProductionMetrics Metrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    bool bIsInCrisisMode;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float CycleStartTime;

private:
    void SetupInitialTasks();
    void ValidateAgentDependencies();
    void UpdateProductionMetrics();
    bool CheckAgentReadiness(EDir_AgentType AgentType);
    void LogProductionEvent(const FString& Event);
};

/**
 * Production Hub Actor
 * Central coordination point for the development pipeline
 */
UCLASS()
class TRANSPERSONALGAME_API ADir_ProductionHub : public AActor
{
    GENERATED_BODY()

public:
    ADir_ProductionHub();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Hub Management
    UFUNCTION(BlueprintCallable, Category = "Production Hub")
    void InitializeHub();

    UFUNCTION(BlueprintCallable, Category = "Production Hub")
    void UpdateHubStatus();

    UFUNCTION(BlueprintCallable, Category = "Production Hub")
    void DisplayAgentStatus();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* HubMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UDir_ProductionCoordinator* ProductionCoordinator;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UTextRenderComponent* StatusDisplay;

private:
    void SetupHubVisualization();
    void UpdateStatusText();
};