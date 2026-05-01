#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "SharedTypes.h"
#include "ProductionCoordinator.generated.h"

// Production milestone states
UENUM(BlueprintType)
enum class EDir_ProductionMilestone : uint8
{
    NotStarted      UMETA(DisplayName = "Not Started"),
    InProgress      UMETA(DisplayName = "In Progress"),
    Completed       UMETA(DisplayName = "Completed"),
    Blocked         UMETA(DisplayName = "Blocked"),
    Failed          UMETA(DisplayName = "Failed")
};

// Agent status tracking
UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Working         UMETA(DisplayName = "Working"),
    Completed       UMETA(DisplayName = "Completed"),
    Error           UMETA(DisplayName = "Error"),
    Waiting         UMETA(DisplayName = "Waiting for Dependencies")
};

// Production milestone data
USTRUCT(BlueprintType)
struct FDir_ProductionMilestoneData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString MilestoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    EDir_ProductionMilestone Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    int32 RequiredAgentID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<int32> DependentAgentIDs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    float Priority;

    FDir_ProductionMilestoneData()
    {
        MilestoneName = TEXT("");
        Status = EDir_ProductionMilestone::NotStarted;
        RequiredAgentID = 0;
        Description = TEXT("");
        Priority = 1.0f;
    }
};

// Agent coordination data
USTRUCT(BlueprintType)
struct FDir_AgentCoordinationData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    int32 AgentID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    EDir_AgentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FString CurrentTask;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    float TaskProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FDateTime LastUpdateTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    TArray<FString> CompletedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    TArray<FString> BlockedTasks;

    FDir_AgentCoordinationData()
    {
        AgentID = 0;
        AgentName = TEXT("");
        Status = EDir_AgentStatus::Idle;
        CurrentTask = TEXT("");
        TaskProgress = 0.0f;
        LastUpdateTime = FDateTime::Now();
    }
};

/**
 * Production Coordinator - Studio Director's main coordination system
 * Manages the 19-agent production pipeline and milestone tracking
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADir_ProductionCoordinator : public AActor
{
    GENERATED_BODY()

public:
    ADir_ProductionCoordinator();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core coordination components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    // Production milestone tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_ProductionMilestoneData> ProductionMilestones;

    // Agent coordination
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agents")
    TArray<FDir_AgentCoordinationData> AgentCoordination;

    // Production settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float CoordinationUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bAutoProgressMilestones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bEnableDebugLogging;

private:
    // Internal timing
    float LastCoordinationUpdate;

public:
    // Milestone management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void InitializeProductionMilestones();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateMilestoneStatus(const FString& MilestoneName, EDir_ProductionMilestone NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Production")
    EDir_ProductionMilestone GetMilestoneStatus(const FString& MilestoneName) const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FString> GetBlockedMilestones() const;

    // Agent coordination
    UFUNCTION(BlueprintCallable, Category = "Agents")
    void InitializeAgentCoordination();

    UFUNCTION(BlueprintCallable, Category = "Agents")
    void UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus, const FString& CurrentTask);

    UFUNCTION(BlueprintCallable, Category = "Agents")
    EDir_AgentStatus GetAgentStatus(int32 AgentID) const;

    UFUNCTION(BlueprintCallable, Category = "Agents")
    TArray<int32> GetIdleAgents() const;

    UFUNCTION(BlueprintCallable, Category = "Agents")
    TArray<int32> GetBlockedAgents() const;

    // Production flow control
    UFUNCTION(BlueprintCallable, Category = "Production")
    void ProcessProductionPipeline();

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool CanAgentProceed(int32 AgentID) const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    void ReportAgentCompletion(int32 AgentID, const FString& CompletedTask);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void ReportAgentBlocked(int32 AgentID, const FString& BlockReason);

    // Debug and monitoring
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogProductionStatus() const;

    UFUNCTION(BlueprintCallable, Category = "Debug")
    FString GetProductionSummary() const;

    // Editor utilities
    UFUNCTION(CallInEditor, Category = "Editor")
    void EditorInitializeProduction();

    UFUNCTION(CallInEditor, Category = "Editor")
    void EditorResetProduction();

    UFUNCTION(CallInEditor, Category = "Editor")
    void EditorLogStatus();
};