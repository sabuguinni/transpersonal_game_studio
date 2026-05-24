#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "StudioDirectorSystem.generated.h"

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Working     UMETA(DisplayName = "Working"),
    Completed   UMETA(DisplayName = "Completed"),
    Failed      UMETA(DisplayName = "Failed"),
    Blocked     UMETA(DisplayName = "Blocked")
};

USTRUCT(BlueprintType)
struct FDir_AgentInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    int32 AgentID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    EDir_AgentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FString CurrentTask;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    float CompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FDateTime LastUpdate;

    FDir_AgentInfo()
    {
        AgentName = TEXT("");
        AgentID = 0;
        Status = EDir_AgentStatus::Idle;
        CurrentTask = TEXT("");
        CompletionPercentage = 0.0f;
        LastUpdate = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct FDir_CycleMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CycleNumber;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 FilesCreated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 UE5CommandsExecuted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 ActorsSpawned;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float TotalExecutionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    bool bCycleSuccess;

    FDir_CycleMetrics()
    {
        CycleNumber = 0;
        FilesCreated = 0;
        UE5CommandsExecuted = 0;
        ActorsSpawned = 0;
        TotalExecutionTime = 0.0f;
        bCycleSuccess = false;
    }
};

/**
 * Studio Director System - Coordinates all 19 agents in the production pipeline
 * Tracks agent status, manages task distribution, and monitors cycle metrics
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UStudioDirectorSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UStudioDirectorSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Agent Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Director")
    TArray<FDir_AgentInfo> AgentChain;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Director")
    int32 CurrentActiveAgent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Director")
    FString CurrentCycleID;

    // Cycle Metrics
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    TArray<FDir_CycleMetrics> CycleHistory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    FDir_CycleMetrics CurrentCycleMetrics;

    // Production Status
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    bool bProductionActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float DailyBudgetUsed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float DailyBudgetLimit;

    // Agent Control Functions
    UFUNCTION(BlueprintCallable, Category = "Director")
    void InitializeAgentChain();

    UFUNCTION(BlueprintCallable, Category = "Director")
    void StartProductionCycle(const FString& CycleID);

    UFUNCTION(BlueprintCallable, Category = "Director")
    void UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus, const FString& Task, float Completion);

    UFUNCTION(BlueprintCallable, Category = "Director")
    void AdvanceToNextAgent();

    UFUNCTION(BlueprintCallable, Category = "Director")
    void CompleteCycle();

    // Metrics Functions
    UFUNCTION(BlueprintCallable, Category = "Metrics")
    void RecordFileCreation();

    UFUNCTION(BlueprintCallable, Category = "Metrics")
    void RecordUE5Command();

    UFUNCTION(BlueprintCallable, Category = "Metrics")
    void RecordActorSpawn(int32 Count = 1);

    // Query Functions
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Director")
    FDir_AgentInfo GetAgentInfo(int32 AgentID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Director")
    bool IsAgentActive(int32 AgentID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Director")
    float GetCycleProgress() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Director")
    int32 GetTotalActorsInScene() const;

    // Emergency Functions
    UFUNCTION(BlueprintCallable, Category = "Director")
    void EmergencyStopCycle(const FString& Reason);

    UFUNCTION(BlueprintCallable, Category = "Director")
    void ResetAgentChain();

private:
    void LogAgentActivity(const FString& Message);
    void ValidateAgentChain();
    void UpdateCycleMetrics();
};