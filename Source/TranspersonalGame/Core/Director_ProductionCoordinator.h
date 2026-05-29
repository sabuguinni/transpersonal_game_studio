#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Director_ProductionCoordinator.generated.h"

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    Phase1_BasicMovement    UMETA(DisplayName = "Phase 1: Basic Movement"),
    Phase2_SurvivalSystems  UMETA(DisplayName = "Phase 2: Survival Systems"),
    Phase3_DinosaurAI       UMETA(DisplayName = "Phase 3: Dinosaur AI"),
    Phase4_WorldPopulation  UMETA(DisplayName = "Phase 4: World Population"),
    Phase5_Polish           UMETA(DisplayName = "Phase 5: Polish & Testing")
};

USTRUCT(BlueprintType)
struct FDir_ProductionMilestone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString MilestoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    EDir_ProductionPhase Phase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    bool bCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float CompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FString> RequiredComponents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FString> BlockingIssues;

    FDir_ProductionMilestone()
    {
        MilestoneName = TEXT("Unnamed Milestone");
        Phase = EDir_ProductionPhase::Phase1_BasicMovement;
        bCompleted = false;
        CompletionPercentage = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FDir_AgentTaskAssignment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Management")
    int32 AgentID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Management")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Management")
    FString CurrentTask;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Management")
    EDir_ProductionPhase AssignedPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Management")
    float TaskProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Management")
    bool bTaskBlocked;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Management")
    TArray<FString> Dependencies;

    FDir_AgentTaskAssignment()
    {
        AgentID = 0;
        AgentName = TEXT("Unassigned Agent");
        CurrentTask = TEXT("No Task");
        AssignedPhase = EDir_ProductionPhase::Phase1_BasicMovement;
        TaskProgress = 0.0f;
        bTaskBlocked = false;
    }
};

/**
 * Production Coordinator for Transpersonal Game Studio
 * Manages the 19-agent production pipeline and milestone tracking
 * Ensures proper sequencing and dependency management
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDir_ProductionCoordinator : public UActorComponent
{
    GENERATED_BODY()

public:
    UDir_ProductionCoordinator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Production Phase Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Management")
    EDir_ProductionPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Management")
    TArray<FDir_ProductionMilestone> ProductionMilestones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Management")
    TArray<FDir_AgentTaskAssignment> AgentAssignments;

    // Critical Path Tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Critical Path")
    bool bCharacterMovementComplete;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Critical Path")
    bool bSurvivalSystemsComplete;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Critical Path")
    bool bDinosaurAIComplete;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Critical Path")
    bool bTerrainSystemComplete;

    // Production Functions
    UFUNCTION(BlueprintCallable, Category = "Production Management")
    void InitializeProductionPipeline();

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    void AdvanceToNextPhase();

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    bool ValidatePhaseCompletion(EDir_ProductionPhase Phase);

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    void AssignTaskToAgent(int32 AgentID, const FString& TaskDescription, EDir_ProductionPhase Phase);

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    void ReportAgentProgress(int32 AgentID, float ProgressPercentage);

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    void BlockAgent(int32 AgentID, const FString& BlockingReason);

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    void UnblockAgent(int32 AgentID);

    // Milestone Management
    UFUNCTION(BlueprintCallable, Category = "Milestone Management")
    void CreateMilestone(const FString& Name, EDir_ProductionPhase Phase, const TArray<FString>& Requirements);

    UFUNCTION(BlueprintCallable, Category = "Milestone Management")
    void CompleteMilestone(const FString& MilestoneName);

    UFUNCTION(BlueprintCallable, Category = "Milestone Management")
    float GetOverallProgress() const;

    // Critical Path Functions
    UFUNCTION(BlueprintCallable, Category = "Critical Path")
    void ValidateCharacterMovement();

    UFUNCTION(BlueprintCallable, Category = "Critical Path")
    void ValidateSurvivalSystems();

    UFUNCTION(BlueprintCallable, Category = "Critical Path")
    void ValidateDinosaurAI();

    UFUNCTION(BlueprintCallable, Category = "Critical Path")
    void ValidateTerrainSystem();

    // Emergency Protocols
    UFUNCTION(BlueprintCallable, Category = "Emergency Protocols")
    void TriggerEmergencyReassignment();

    UFUNCTION(BlueprintCallable, Category = "Emergency Protocols")
    void ResetProductionPipeline();

    UFUNCTION(BlueprintCallable, Category = "Emergency Protocols")
    void GenerateProductionReport();

private:
    void UpdateMilestoneProgress();
    void CheckDependencies();
    void ValidateAgentSequencing();
    
    UPROPERTY()
    float LastProgressUpdate;

    UPROPERTY()
    TArray<FString> CriticalPathErrors;
};