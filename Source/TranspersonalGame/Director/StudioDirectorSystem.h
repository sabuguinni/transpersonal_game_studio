#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "../SharedTypes.h"
#include "StudioDirectorSystem.generated.h"

/**
 * Studio Director System - Coordinates all game agents and manages production pipeline
 * Oversees the 19-agent development chain and ensures milestone delivery
 */

USTRUCT(BlueprintType)
struct FDir_AgentStatus
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Status")
    int32 AgentNumber = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Status")
    FString AgentName = "";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Status")
    FString CurrentTask = "";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Status")
    EDir_ProductionPhase Phase = EDir_ProductionPhase::Planning;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Status")
    float CompletionPercentage = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Status")
    bool bIsBlocked = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Status")
    FString BlockingReason = "";

    FDir_AgentStatus()
    {
        AgentNumber = 0;
        AgentName = "";
        CurrentTask = "";
        Phase = EDir_ProductionPhase::Planning;
        CompletionPercentage = 0.0f;
        bIsBlocked = false;
        BlockingReason = "";
    }
};

USTRUCT(BlueprintType)
struct FDir_MilestoneTracker
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString MilestoneName = "";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<int32> RequiredAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FString> Deliverables;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    float Progress = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    bool bIsComplete = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FDateTime TargetDate;

    FDir_MilestoneTracker()
    {
        MilestoneName = "";
        RequiredAgents.Empty();
        Deliverables.Empty();
        Progress = 0.0f;
        bIsComplete = false;
        TargetDate = FDateTime::Now();
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UStudioDirectorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UStudioDirectorComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Agent Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    TArray<FDir_AgentStatus> AgentChain;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    int32 CurrentActiveAgent = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    FString CurrentCycleID = "";

    // Milestone Tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestones")
    TArray<FDir_MilestoneTracker> ActiveMilestones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestones")
    FDir_MilestoneTracker WalkAroundMilestone;

    // Production Metrics
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 TotalCppFiles = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 TotalHeaderFiles = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 PlayableActors = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 DinosaurCount = 0;

    // Public Functions
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void InitializeAgentChain();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateAgentStatus(int32 AgentNumber, const FString& TaskDescription, float Progress);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void BlockAgent(int32 AgentNumber, const FString& Reason);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UnblockAgent(int32 AgentNumber);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool CanAgentProceed(int32 AgentNumber);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void AdvanceToNextAgent();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void InitializeWalkAroundMilestone();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateMilestoneProgress(const FString& MilestoneName, float Progress);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void ValidateMinPlayableMap();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FString GenerateProductionReport();

private:
    void SetupAgentDependencies();
    void CheckMilestoneCompletion();
    bool ValidateAgentOutput(int32 AgentNumber);
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AStudioDirectorActor : public AActor
{
    GENERATED_BODY()

public:
    AStudioDirectorActor();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStudioDirectorComponent* DirectorComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* CommandCenterMesh;

    // Director Dashboard Functions
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void DisplayAgentStatus();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void TriggerEmergencyStop();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void ResetProductionCycle();
};