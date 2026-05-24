#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "../SharedTypes.h"
#include "DirectorCoordinationSystem.generated.h"

// Agent task priority levels
UENUM(BlueprintType)
enum class EDir_AgentPriority : uint8
{
    Critical    UMETA(DisplayName = "Critical Path"),
    High        UMETA(DisplayName = "High Priority"),
    Normal      UMETA(DisplayName = "Normal Priority"),
    Low         UMETA(DisplayName = "Low Priority"),
    Blocked     UMETA(DisplayName = "Blocked")
};

// Agent specialization types
UENUM(BlueprintType)
enum class EDir_AgentType : uint8
{
    Director        UMETA(DisplayName = "Studio Director"),
    Architect       UMETA(DisplayName = "Engine Architect"),
    CoreSystems     UMETA(DisplayName = "Core Systems"),
    Performance     UMETA(DisplayName = "Performance"),
    WorldGen        UMETA(DisplayName = "World Generation"),
    Environment     UMETA(DisplayName = "Environment Art"),
    Architecture    UMETA(DisplayName = "Architecture"),
    Lighting        UMETA(DisplayName = "Lighting"),
    Characters      UMETA(DisplayName = "Character Art"),
    Animation       UMETA(DisplayName = "Animation"),
    NPCBehavior     UMETA(DisplayName = "NPC Behavior"),
    CombatAI        UMETA(DisplayName = "Combat AI"),
    CrowdSim        UMETA(DisplayName = "Crowd Simulation"),
    QuestDesign     UMETA(DisplayName = "Quest Design"),
    Narrative       UMETA(DisplayName = "Narrative"),
    Audio           UMETA(DisplayName = "Audio"),
    VFX             UMETA(DisplayName = "VFX"),
    QA              UMETA(DisplayName = "QA Testing"),
    Integration     UMETA(DisplayName = "Integration")
};

// Task assignment structure
USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_AgentPriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString ExpectedDeliverable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString CycleID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    bool bIsBlocking;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    TArray<int32> DependentAgents;

    FDir_AgentTask()
    {
        TaskDescription = TEXT("");
        Priority = EDir_AgentPriority::Normal;
        ExpectedDeliverable = TEXT("");
        CycleID = TEXT("");
        bIsBlocking = false;
    }
};

// Agent status tracking
USTRUCT(BlueprintType)
struct FDir_AgentStatus
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
    EDir_AgentType AgentType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
    FString LastCycleID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
    int32 DeliverablesCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
    FString CurrentTask;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
    float PerformanceScore;

    FDir_AgentStatus()
    {
        AgentType = EDir_AgentType::Director;
        LastCycleID = TEXT("");
        DeliverablesCompleted = 0;
        bIsActive = true;
        CurrentTask = TEXT("");
        PerformanceScore = 1.0f;
    }
};

// Biome coordination data
USTRUCT(BlueprintType)
struct FDir_BiomeStatus
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector Coordinates;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 ActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    bool bIsPopulated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> AssetTypes;

    FDir_BiomeStatus()
    {
        BiomeName = TEXT("");
        Coordinates = FVector::ZeroVector;
        ActorCount = 0;
        bIsPopulated = false;
    }
};

/**
 * Studio Director Coordination System
 * Manages the 19-agent production pipeline for Transpersonal Game Studio
 * Tracks agent status, task assignments, and critical path dependencies
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDirectorCoordinationSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UDirectorCoordinationSystem();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Agent management
    UFUNCTION(BlueprintCallable, Category = "Director")
    void AssignTaskToAgent(int32 AgentNumber, const FDir_AgentTask& Task);

    UFUNCTION(BlueprintCallable, Category = "Director")
    FDir_AgentStatus GetAgentStatus(int32 AgentNumber) const;

    UFUNCTION(BlueprintCallable, Category = "Director")
    void UpdateAgentStatus(int32 AgentNumber, const FDir_AgentStatus& Status);

    UFUNCTION(BlueprintCallable, Category = "Director")
    TArray<int32> GetCriticalPathAgents() const;

    // Biome coordination
    UFUNCTION(BlueprintCallable, Category = "Director")
    void UpdateBiomeStatus(const FString& BiomeName, int32 ActorCount, bool bPopulated);

    UFUNCTION(BlueprintCallable, Category = "Director")
    FDir_BiomeStatus GetBiomeStatus(const FString& BiomeName) const;

    UFUNCTION(BlueprintCallable, Category = "Director")
    TArray<FDir_BiomeStatus> GetAllBiomeStatuses() const;

    // Pipeline management
    UFUNCTION(BlueprintCallable, Category = "Director")
    void StartProductionCycle(const FString& CycleID);

    UFUNCTION(BlueprintCallable, Category = "Director")
    void CompleteProductionCycle(const FString& CycleID);

    UFUNCTION(BlueprintCallable, Category = "Director")
    bool IsPlayablePrototypeReady() const;

    UFUNCTION(BlueprintCallable, Category = "Director")
    float GetOverallProgress() const;

    // Critical criteria tracking
    UFUNCTION(BlueprintCallable, Category = "Director")
    bool CheckCriterion1_BiomePopulation() const;

    UFUNCTION(BlueprintCallable, Category = "Director")
    bool CheckCriterion2_AtmosphereStable() const;

    UFUNCTION(BlueprintCallable, Category = "Director")
    bool CheckCriterion3_FBXPipeline() const;

protected:
    // Agent tracking
    UPROPERTY(BlueprintReadOnly, Category = "Director")
    TMap<int32, FDir_AgentStatus> AgentStatuses;

    UPROPERTY(BlueprintReadOnly, Category = "Director")
    TMap<int32, FDir_AgentTask> CurrentTasks;

    // Biome tracking
    UPROPERTY(BlueprintReadOnly, Category = "Director")
    TMap<FString, FDir_BiomeStatus> BiomeStatuses;

    // Pipeline state
    UPROPERTY(BlueprintReadOnly, Category = "Director")
    FString CurrentCycleID;

    UPROPERTY(BlueprintReadOnly, Category = "Director")
    bool bFBXPipelineTested;

    UPROPERTY(BlueprintReadOnly, Category = "Director")
    int32 StableAtmosphereCycles;

    UPROPERTY(BlueprintReadOnly, Category = "Director")
    float BudgetUsed;

    UPROPERTY(BlueprintReadOnly, Category = "Director")
    float BudgetLimit;

private:
    void InitializeAgentStatuses();
    void InitializeBiomeStatuses();
    void ValidateAgentDependencies();
};