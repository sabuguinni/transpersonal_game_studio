#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "DirectorCoordinator.generated.h"

// Production stage tracking for 19-agent pipeline
UENUM(BlueprintType)
enum class EDir_ProductionStage : uint8
{
    PreProduction       UMETA(DisplayName = "Pre-Production"),
    EngineArchitecture  UMETA(DisplayName = "Engine Architecture"),
    CoreSystems         UMETA(DisplayName = "Core Systems"),
    Performance         UMETA(DisplayName = "Performance Optimization"),
    WorldGeneration     UMETA(DisplayName = "World Generation"),
    EnvironmentArt      UMETA(DisplayName = "Environment Art"),
    Architecture        UMETA(DisplayName = "Architecture & Interiors"),
    LightingAtmosphere  UMETA(DisplayName = "Lighting & Atmosphere"),
    CharacterArt        UMETA(DisplayName = "Character Art"),
    Animation           UMETA(DisplayName = "Animation Systems"),
    NPCBehavior         UMETA(DisplayName = "NPC Behavior"),
    CombatAI            UMETA(DisplayName = "Combat & Enemy AI"),
    CrowdSimulation     UMETA(DisplayName = "Crowd & Traffic"),
    QuestDesign         UMETA(DisplayName = "Quest & Mission Design"),
    Narrative           UMETA(DisplayName = "Narrative & Dialogue"),
    Audio               UMETA(DisplayName = "Audio Systems"),
    VFX                 UMETA(DisplayName = "Visual Effects"),
    QATesting           UMETA(DisplayName = "QA & Testing"),
    Integration         UMETA(DisplayName = "Integration & Build")
};

// Agent status tracking
UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle                UMETA(DisplayName = "Idle"),
    Working             UMETA(DisplayName = "Working"),
    Completed           UMETA(DisplayName = "Completed"),
    Blocked             UMETA(DisplayName = "Blocked"),
    Error               UMETA(DisplayName = "Error"),
    WaitingApproval     UMETA(DisplayName = "Waiting Approval")
};

// Biome population status for Criterion 1
UENUM(BlueprintType)
enum class EDir_BiomeType : uint8
{
    Savana              UMETA(DisplayName = "Savana"),
    Pantano             UMETA(DisplayName = "Pantano"),
    Floresta            UMETA(DisplayName = "Floresta"),
    Deserto             UMETA(DisplayName = "Deserto"),
    Montanha            UMETA(DisplayName = "Montanha")
};

// Individual agent task tracking
USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Agent Task")
    int32 AgentNumber = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Agent Task")
    FString AgentName;

    UPROPERTY(BlueprintReadWrite, Category = "Agent Task")
    EDir_ProductionStage CurrentStage = EDir_ProductionStage::PreProduction;

    UPROPERTY(BlueprintReadWrite, Category = "Agent Task")
    EDir_AgentStatus Status = EDir_AgentStatus::Idle;

    UPROPERTY(BlueprintReadWrite, Category = "Agent Task")
    FString CurrentTask;

    UPROPERTY(BlueprintReadWrite, Category = "Agent Task")
    float ProgressPercent = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Agent Task")
    FDateTime LastUpdate;

    UPROPERTY(BlueprintReadWrite, Category = "Agent Task")
    TArray<FString> CompletedFiles;

    UPROPERTY(BlueprintReadWrite, Category = "Agent Task")
    TArray<FString> PendingDependencies;

    FDir_AgentTask()
    {
        LastUpdate = FDateTime::Now();
    }
};

// Biome population tracking for asset purchase criteria
USTRUCT(BlueprintType)
struct FDir_BiomeStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Biome")
    EDir_BiomeType BiomeType = EDir_BiomeType::Savana;

    UPROPERTY(BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Biome")
    int32 ActorCount = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Biome")
    int32 TargetActorCount = 500;

    UPROPERTY(BlueprintReadWrite, Category = "Biome")
    bool bIsPopulated = false;

    UPROPERTY(BlueprintReadWrite, Category = "Biome")
    TArray<FString> AssetTypes;

    UPROPERTY(BlueprintReadWrite, Category = "Biome")
    FDateTime LastPopulationUpdate;

    FDir_BiomeStatus()
    {
        LastPopulationUpdate = FDateTime::Now();
    }
};

// Production milestone tracking
USTRUCT(BlueprintType)
struct FDir_ProductionMilestone
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Milestone")
    FString MilestoneName;

    UPROPERTY(BlueprintReadWrite, Category = "Milestone")
    FString Description;

    UPROPERTY(BlueprintReadWrite, Category = "Milestone")
    TArray<int32> RequiredAgents;

    UPROPERTY(BlueprintReadWrite, Category = "Milestone")
    bool bIsCompleted = false;

    UPROPERTY(BlueprintReadWrite, Category = "Milestone")
    FDateTime TargetDate;

    UPROPERTY(BlueprintReadWrite, Category = "Milestone")
    FDateTime CompletionDate;
};

/**
 * DirectorCoordinator - Studio Director coordination system
 * Manages the 19-agent production pipeline, tracks biome population,
 * and coordinates milestone achievement for asset purchase criteria.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDirectorCoordinator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UDirectorCoordinator();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Agent management
    UFUNCTION(BlueprintCallable, Category = "Director")
    void InitializeAgentPipeline();

    UFUNCTION(BlueprintCallable, Category = "Director")
    void UpdateAgentStatus(int32 AgentNumber, EDir_AgentStatus NewStatus, const FString& TaskDescription);

    UFUNCTION(BlueprintCallable, Category = "Director")
    void AddCompletedFile(int32 AgentNumber, const FString& FilePath);

    UFUNCTION(BlueprintCallable, Category = "Director")
    FDir_AgentTask GetAgentStatus(int32 AgentNumber) const;

    UFUNCTION(BlueprintCallable, Category = "Director")
    TArray<FDir_AgentTask> GetAllAgentStatuses() const;

    // Biome management for Criterion 1
    UFUNCTION(BlueprintCallable, Category = "Director")
    void InitializeBiomes();

    UFUNCTION(BlueprintCallable, Category = "Director")
    void UpdateBiomePopulation(EDir_BiomeType BiomeType, int32 NewActorCount);

    UFUNCTION(BlueprintCallable, Category = "Director")
    FDir_BiomeStatus GetBiomeStatus(EDir_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Director")
    bool AreAllBiomesPopulated() const;

    UFUNCTION(BlueprintCallable, Category = "Director")
    float GetOverallBiomeProgress() const;

    // Asset purchase criteria tracking
    UFUNCTION(BlueprintCallable, Category = "Director")
    bool CheckCriterion1_BiomePopulation() const;

    UFUNCTION(BlueprintCallable, Category = "Director")
    bool CheckCriterion2_AtmosphereStability() const;

    UFUNCTION(BlueprintCallable, Category = "Director")
    bool CheckCriterion3_FBXPipeline() const;

    UFUNCTION(BlueprintCallable, Category = "Director")
    bool AreAllCriteriaComplete() const;

    // Production milestone management
    UFUNCTION(BlueprintCallable, Category = "Director")
    void SetupProductionMilestones();

    UFUNCTION(BlueprintCallable, Category = "Director")
    void CompleteMilestone(const FString& MilestoneName);

    UFUNCTION(BlueprintCallable, Category = "Director")
    TArray<FDir_ProductionMilestone> GetActiveMilestones() const;

    // Emergency coordination
    UFUNCTION(BlueprintCallable, Category = "Director")
    void TriggerEmergencyCoordination(const FString& Reason);

    UFUNCTION(BlueprintCallable, Category = "Director")
    void ResolveAgentConflict(int32 Agent1, int32 Agent2, const FString& Resolution);

protected:
    // Agent tracking
    UPROPERTY(BlueprintReadOnly, Category = "Director")
    TArray<FDir_AgentTask> AgentTasks;

    // Biome tracking
    UPROPERTY(BlueprintReadOnly, Category = "Director")
    TArray<FDir_BiomeStatus> BiomeStatuses;

    // Milestone tracking
    UPROPERTY(BlueprintReadOnly, Category = "Director")
    TArray<FDir_ProductionMilestone> ProductionMilestones;

    // Criteria tracking
    UPROPERTY(BlueprintReadOnly, Category = "Director")
    bool bCriterion1Complete = false;

    UPROPERTY(BlueprintReadOnly, Category = "Director")
    bool bCriterion2Complete = false;

    UPROPERTY(BlueprintReadOnly, Category = "Director")
    bool bCriterion3Complete = false;

    UPROPERTY(BlueprintReadOnly, Category = "Director")
    int32 AtmosphereStabilityCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Director")
    bool bFBXPipelineTested = false;

private:
    void SetupDefaultAgents();
    void SetupDefaultBiomes();
    FDir_AgentTask* FindAgentTask(int32 AgentNumber);
    FDir_BiomeStatus* FindBiomeStatus(EDir_BiomeType BiomeType);
};