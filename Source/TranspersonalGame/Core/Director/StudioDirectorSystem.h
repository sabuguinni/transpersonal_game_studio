#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "StudioDirectorSystem.generated.h"

// Studio Director Production States
UENUM(BlueprintType)
enum class EDir_ProductionState : uint8
{
    Planning        UMETA(DisplayName = "Planning"),
    InProgress      UMETA(DisplayName = "In Progress"),
    Review          UMETA(DisplayName = "Review"),
    Complete        UMETA(DisplayName = "Complete"),
    Blocked         UMETA(DisplayName = "Blocked")
};

// Agent Status Information
USTRUCT(BlueprintType)
struct FDir_AgentStatus
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    int32 AgentID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    EDir_ProductionState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FString CurrentTask;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    float ProgressPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FDateTime LastUpdate;

    FDir_AgentStatus()
    {
        AgentName = TEXT("Unknown");
        AgentID = 0;
        CurrentState = EDir_ProductionState::Planning;
        CurrentTask = TEXT("Idle");
        ProgressPercentage = 0.0f;
        LastUpdate = FDateTime::Now();
    }
};

// Production Cycle Information
USTRUCT(BlueprintType)
struct FDir_ProductionCycle
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cycle")
    FString CycleID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cycle")
    int32 CycleNumber;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cycle")
    FDateTime StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cycle")
    FDateTime EstimatedEndTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cycle")
    TArray<FDir_AgentStatus> AgentStatuses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cycle")
    EDir_ProductionState OverallState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cycle")
    FString CreativeDirectorNotes;

    FDir_ProductionCycle()
    {
        CycleID = TEXT("PROD_CYCLE_000");
        CycleNumber = 0;
        StartTime = FDateTime::Now();
        EstimatedEndTime = FDateTime::Now();
        OverallState = EDir_ProductionState::Planning;
        CreativeDirectorNotes = TEXT("");
    }
};

/**
 * Studio Director System - Central coordination and management component
 * Manages the 19-agent production chain and ensures Miguel's vision is preserved
 */
UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UStudioDirectorSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UStudioDirectorSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Current production cycle
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production", meta = (AllowPrivateAccess = "true"))
    FDir_ProductionCycle CurrentCycle;

    // All registered agents
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production", meta = (AllowPrivateAccess = "true"))
    TArray<FDir_AgentStatus> RegisteredAgents;

    // Production settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (AllowPrivateAccess = "true"))
    float CycleTimeoutHours;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (AllowPrivateAccess = "true"))
    bool bAutoAdvanceAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (AllowPrivateAccess = "true"))
    bool bStrictSequenceMode;

public:
    // Production cycle management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void StartNewCycle(const FString& CycleID, const FString& CreativeNotes);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void CompleteCycle();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void BlockCycle(const FString& Reason);

    // Agent management
    UFUNCTION(BlueprintCallable, Category = "Agents")
    void RegisterAgent(int32 AgentID, const FString& AgentName);

    UFUNCTION(BlueprintCallable, Category = "Agents")
    void UpdateAgentStatus(int32 AgentID, EDir_ProductionState NewState, const FString& CurrentTask, float Progress);

    UFUNCTION(BlueprintCallable, Category = "Agents")
    FDir_AgentStatus GetAgentStatus(int32 AgentID);

    UFUNCTION(BlueprintCallable, Category = "Agents")
    void AdvanceToNextAgent();

    // Production oversight
    UFUNCTION(BlueprintCallable, Category = "Production")
    bool ValidateProductionChain();

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FString> GetProductionWarnings();

    UFUNCTION(BlueprintCallable, Category = "Production")
    float GetOverallProgress();

    // Creative direction preservation
    UFUNCTION(BlueprintCallable, Category = "Creative")
    void SetCreativeVision(const FString& VisionStatement);

    UFUNCTION(BlueprintCallable, Category = "Creative")
    FString GetCreativeVision();

    UFUNCTION(BlueprintCallable, Category = "Creative")
    bool ValidateAgainstVision(const FString& ProposedChange);

    // Debug and monitoring
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void LogProductionStatus();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void ResetProductionChain();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void SimulateAgentProgress();

private:
    // Internal state
    FString CreativeVision;
    int32 CurrentActiveAgent;
    bool bCycleInProgress;
    
    // Agent chain definition
    TArray<FString> AgentChain;
    
    void InitializeAgentChain();
    void ValidateAgentSequence();
    bool IsAgentReadyToAdvance(int32 AgentID);
    void NotifyAgentActivation(int32 AgentID);
};