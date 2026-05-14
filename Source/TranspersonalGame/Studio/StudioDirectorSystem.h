#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "StudioDirectorSystem.generated.h"

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Working     UMETA(DisplayName = "Working"),
    Completed   UMETA(DisplayName = "Completed"),
    Error       UMETA(DisplayName = "Error"),
    Blocked     UMETA(DisplayName = "Blocked")
};

USTRUCT(BlueprintType)
struct FDir_AgentInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Agent")
    int32 AgentID = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Agent")
    FString AgentName;

    UPROPERTY(BlueprintReadOnly, Category = "Agent")
    EDir_AgentStatus Status = EDir_AgentStatus::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Agent")
    FString CurrentTask;

    UPROPERTY(BlueprintReadOnly, Category = "Agent")
    float ProgressPercentage = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Agent")
    FString LastOutput;

    FDir_AgentInfo()
    {
        AgentID = 0;
        AgentName = TEXT("");
        Status = EDir_AgentStatus::Idle;
        CurrentTask = TEXT("");
        ProgressPercentage = 0.0f;
        LastOutput = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionCycle
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    FString CycleID;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    FDateTime StartTime;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    FDateTime EndTime;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    TArray<FDir_AgentInfo> AgentResults;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    int32 FilesCreated = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    int32 UE5CommandsExecuted = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    bool bCycleCompleted = false;

    FDir_ProductionCycle()
    {
        CycleID = TEXT("");
        StartTime = FDateTime::Now();
        EndTime = FDateTime::Now();
        FilesCreated = 0;
        UE5CommandsExecuted = 0;
        bCycleCompleted = false;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAgentStatusChanged, const FDir_AgentInfo&, AgentInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCycleCompleted, const FDir_ProductionCycle&, CycleInfo);

/**
 * Studio Director System - Coordinates all 19 agents in the production pipeline
 * Tracks agent status, manages production cycles, and ensures quality delivery
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UStudioDirectorSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Agent Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void InitializeAgents();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus, const FString& TaskDescription);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FDir_AgentInfo GetAgentInfo(int32 AgentID) const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_AgentInfo> GetAllAgents() const;

    // Production Cycle Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void StartProductionCycle(const FString& CycleID);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CompleteProductionCycle();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FDir_ProductionCycle GetCurrentCycle() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_ProductionCycle> GetCycleHistory() const;

    // Quality Assurance
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool ValidateMinPlayableMap();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void GenerateProductionReport();

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Studio Director")
    FOnAgentStatusChanged OnAgentStatusChanged;

    UPROPERTY(BlueprintAssignable, Category = "Studio Director")
    FOnCycleCompleted OnCycleCompleted;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Studio Director")
    TArray<FDir_AgentInfo> Agents;

    UPROPERTY(BlueprintReadOnly, Category = "Studio Director")
    FDir_ProductionCycle CurrentCycle;

    UPROPERTY(BlueprintReadOnly, Category = "Studio Director")
    TArray<FDir_ProductionCycle> CycleHistory;

    UPROPERTY(BlueprintReadOnly, Category = "Studio Director")
    bool bSystemInitialized = false;

private:
    void SetupAgentDefinitions();
    void LogAgentActivity(const FDir_AgentInfo& AgentInfo);
    bool CheckAgentDependencies(int32 AgentID);
};