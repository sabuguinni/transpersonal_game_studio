#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "Dir_ProductionManager.generated.h"

/**
 * Production milestone tracking for development cycles
 */
UENUM(BlueprintType)
enum class EDir_ProductionMilestone : uint8
{
    None                    UMETA(DisplayName = "None"),
    CharacterMovement       UMETA(DisplayName = "Character Movement"),
    DinosaurSpawning        UMETA(DisplayName = "Dinosaur Spawning"),
    SurvivalHUD            UMETA(DisplayName = "Survival HUD"),
    TerrainGeneration      UMETA(DisplayName = "Terrain Generation"),
    BiomeSystem            UMETA(DisplayName = "Biome System"),
    CombatSystem           UMETA(DisplayName = "Combat System"),
    AudioSystem            UMETA(DisplayName = "Audio System"),
    PlayablePrototype      UMETA(DisplayName = "Playable Prototype")
};

/**
 * Agent coordination status for production pipeline
 */
UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle                    UMETA(DisplayName = "Idle"),
    Active                  UMETA(DisplayName = "Active"),
    Blocked                 UMETA(DisplayName = "Blocked"),
    Complete                UMETA(DisplayName = "Complete"),
    Error                   UMETA(DisplayName = "Error")
};

/**
 * Production metrics data structure
 */
USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 TotalCppFiles = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 TotalHeaderFiles = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 ImplementedModules = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 ActiveAgents = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 CompletedMilestones = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float ProductionEfficiency = 0.0f;

    FDir_ProductionMetrics()
    {
        TotalCppFiles = 0;
        TotalHeaderFiles = 0;
        ImplementedModules = 0;
        ActiveAgents = 0;
        CompletedMilestones = 0;
        ProductionEfficiency = 0.0f;
    }
};

/**
 * Agent task definition for production coordination
 */
USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    int32 Priority = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FString CurrentTask;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FString ExpectedDeliverable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    EDir_AgentStatus Status = EDir_AgentStatus::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    TArray<FString> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    float EstimatedCompletionTime = 0.0f;

    FDir_AgentTask()
    {
        AgentName = TEXT("");
        Priority = 0;
        CurrentTask = TEXT("");
        ExpectedDeliverable = TEXT("");
        Status = EDir_AgentStatus::Idle;
        EstimatedCompletionTime = 0.0f;
    }
};

/**
 * Production Manager Component - Coordinates agent tasks and tracks development milestones
 * Attached to GameMode to provide centralized production oversight
 */
UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDir_ProductionManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UDir_ProductionManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Production Metrics
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    FDir_ProductionMetrics CurrentMetrics;

    // Agent Coordination
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Coordination")
    TArray<FDir_AgentTask> AgentTasks;

    // Milestone Tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestones")
    TArray<EDir_ProductionMilestone> CompletedMilestones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestones")
    EDir_ProductionMilestone CurrentTargetMilestone = EDir_ProductionMilestone::CharacterMovement;

    // Production Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float MetricsUpdateInterval = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bEnableDebugLogging = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bAutoTrackProgress = true;

    // Public Functions
    UFUNCTION(BlueprintCallable, Category = "Production Management")
    void UpdateProductionMetrics();

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    void RegisterAgentTask(const FDir_AgentTask& NewTask);

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    void UpdateAgentStatus(const FString& AgentName, EDir_AgentStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    void CompleteMilestone(EDir_ProductionMilestone Milestone);

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    bool IsMilestoneComplete(EDir_ProductionMilestone Milestone) const;

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    float GetProductionEfficiency() const;

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    TArray<FDir_AgentTask> GetActiveAgentTasks() const;

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    void GenerateProductionReport();

    UFUNCTION(BlueprintCallable, Category = "Production Management", CallInEditor)
    void ScanLevelForAssets();

    UFUNCTION(BlueprintCallable, Category = "Production Management", CallInEditor)
    void ValidateModuleImplementations();

private:
    // Internal tracking
    float LastMetricsUpdate = 0.0f;
    int32 CycleCount = 0;

    // Helper functions
    void InitializeAgentTasks();
    void CalculateProductionEfficiency();
    void LogProductionStatus();
    bool ValidateAgentDependencies(const FDir_AgentTask& Task) const;
};