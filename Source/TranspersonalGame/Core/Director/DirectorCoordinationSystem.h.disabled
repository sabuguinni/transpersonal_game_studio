#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "../../SharedTypes.h"
#include "DirectorCoordinationSystem.generated.h"

// Forward declarations
class UDirectorSystemManager;
class UEngineArchitectSystem;

/**
 * Coordination states for the 19-agent production pipeline
 */
UENUM(BlueprintType)
enum class EDir_AgentCoordinationState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Waiting         UMETA(DisplayName = "Waiting for Dependencies"),
    Active          UMETA(DisplayName = "Active Production"),
    Completed       UMETA(DisplayName = "Completed"),
    Blocked         UMETA(DisplayName = "Blocked"),
    RequiresApproval UMETA(DisplayName = "Requires Human Approval")
};

/**
 * Production phase tracking for the studio pipeline
 */
UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction   UMETA(DisplayName = "Pre-Production"),
    TechnicalSetup  UMETA(DisplayName = "Technical Setup"),
    WorldBuilding   UMETA(DisplayName = "World Building"),
    CharacterDev    UMETA(DisplayName = "Character Development"),
    GameplayCore    UMETA(DisplayName = "Gameplay Core"),
    Polish          UMETA(DisplayName = "Polish & QA"),
    Integration     UMETA(DisplayName = "Final Integration")
};

/**
 * Agent coordination data structure
 */
USTRUCT(BlueprintType)
struct FDir_AgentCoordinationData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Coordination")
    int32 AgentID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Coordination")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Coordination")
    EDir_AgentCoordinationState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Coordination")
    EDir_ProductionPhase RequiredPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Coordination")
    TArray<int32> DependencyAgentIDs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Coordination")
    float ProgressPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Coordination")
    FString CurrentTask;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Coordination")
    bool bRequiresHumanApproval;

    FDir_AgentCoordinationData()
    {
        AgentID = 0;
        AgentName = TEXT("");
        CurrentState = EDir_AgentCoordinationState::Idle;
        RequiredPhase = EDir_ProductionPhase::PreProduction;
        ProgressPercentage = 0.0f;
        CurrentTask = TEXT("");
        bRequiresHumanApproval = false;
    }
};

/**
 * Production milestone tracking
 */
USTRUCT(BlueprintType)
struct FDir_ProductionMilestone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString MilestoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    EDir_ProductionPhase Phase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<int32> RequiredAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FDateTime CompletionDate;

    FDir_ProductionMilestone()
    {
        MilestoneName = TEXT("");
        Phase = EDir_ProductionPhase::PreProduction;
        bIsCompleted = false;
        CompletionDate = FDateTime::Now();
    }
};

/**
 * DirectorCoordinationSystem
 * 
 * Core coordination system that manages the 19-agent production pipeline.
 * Ensures proper sequencing, dependency management, and creative vision preservation.
 * This is the Studio Director's primary tool for orchestrating the entire development process.
 */
UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDirectorCoordinationSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UDirectorCoordinationSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === AGENT COORDINATION ===
    
    /**
     * Initialize the complete 19-agent production pipeline
     */
    UFUNCTION(BlueprintCallable, Category = "Director Coordination")
    void InitializeProductionPipeline();

    /**
     * Update the state of a specific agent in the pipeline
     */
    UFUNCTION(BlueprintCallable, Category = "Director Coordination")
    void UpdateAgentState(int32 AgentID, EDir_AgentCoordinationState NewState, float Progress = 0.0f);

    /**
     * Check if an agent can proceed based on dependencies
     */
    UFUNCTION(BlueprintCallable, Category = "Director Coordination")
    bool CanAgentProceed(int32 AgentID) const;

    /**
     * Get the next agent that should be activated
     */
    UFUNCTION(BlueprintCallable, Category = "Director Coordination")
    int32 GetNextActiveAgent() const;

    /**
     * Force approve an agent that requires human approval
     */
    UFUNCTION(BlueprintCallable, Category = "Director Coordination", CallInEditor)
    void ApproveAgent(int32 AgentID);

    /**
     * Block an agent due to issues
     */
    UFUNCTION(BlueprintCallable, Category = "Director Coordination")
    void BlockAgent(int32 AgentID, const FString& Reason);

    // === PRODUCTION PHASE MANAGEMENT ===

    /**
     * Advance to the next production phase
     */
    UFUNCTION(BlueprintCallable, Category = "Director Coordination")
    void AdvanceProductionPhase();

    /**
     * Check if current phase is complete
     */
    UFUNCTION(BlueprintCallable, Category = "Director Coordination")
    bool IsCurrentPhaseComplete() const;

    /**
     * Get overall production progress (0.0 to 1.0)
     */
    UFUNCTION(BlueprintCallable, Category = "Director Coordination")
    float GetOverallProgress() const;

    // === MILESTONE TRACKING ===

    /**
     * Mark a milestone as completed
     */
    UFUNCTION(BlueprintCallable, Category = "Director Coordination")
    void CompleteMilestone(const FString& MilestoneName);

    /**
     * Get all pending milestones
     */
    UFUNCTION(BlueprintCallable, Category = "Director Coordination")
    TArray<FDir_ProductionMilestone> GetPendingMilestones() const;

    // === CREATIVE VISION PRESERVATION ===

    /**
     * Validate that current production aligns with Miguel's vision
     */
    UFUNCTION(BlueprintCallable, Category = "Director Coordination")
    bool ValidateCreativeVision() const;

    /**
     * Generate production report for Miguel
     */
    UFUNCTION(BlueprintCallable, Category = "Director Coordination", CallInEditor)
    FString GenerateProductionReport() const;

    // === DEBUG & MONITORING ===

    /**
     * Get detailed status of all agents
     */
    UFUNCTION(BlueprintCallable, Category = "Director Coordination")
    TArray<FDir_AgentCoordinationData> GetAllAgentStates() const;

    /**
     * Reset the entire production pipeline
     */
    UFUNCTION(BlueprintCallable, Category = "Director Coordination", CallInEditor)
    void ResetProductionPipeline();

protected:
    // === CORE DATA ===

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Director Coordination")
    TArray<FDir_AgentCoordinationData> AgentStates;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Director Coordination")
    EDir_ProductionPhase CurrentPhase;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Director Coordination")
    TArray<FDir_ProductionMilestone> ProductionMilestones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Director Coordination")
    bool bAutoAdvancePhases;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Director Coordination")
    float CoordinationTickInterval;

    // === INTERNAL STATE ===

    UPROPERTY()
    float LastCoordinationTick;

    UPROPERTY()
    int32 CurrentActiveAgent;

    UPROPERTY()
    TMap<int32, FString> AgentBlockReasons;

private:
    // === INTERNAL METHODS ===

    void SetupAgentDependencies();
    void SetupProductionMilestones();
    void ProcessAgentCoordination();
    bool CheckAgentDependencies(int32 AgentID) const;
    void NotifyAgentStateChange(int32 AgentID, EDir_AgentCoordinationState OldState, EDir_AgentCoordinationState NewState);
    void ValidateProductionIntegrity();
};