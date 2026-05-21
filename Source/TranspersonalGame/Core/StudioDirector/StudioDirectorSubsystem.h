#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "StudioDirectorSubsystem.generated.h"

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle,
    Working,
    Completed,
    Failed,
    Blocked
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Agent Task")
    FString AgentName;

    UPROPERTY(BlueprintReadWrite, Category = "Agent Task")
    FString TaskDescription;

    UPROPERTY(BlueprintReadWrite, Category = "Agent Task")
    EDir_AgentStatus Status;

    UPROPERTY(BlueprintReadWrite, Category = "Agent Task")
    float Priority;

    UPROPERTY(BlueprintReadWrite, Category = "Agent Task")
    FDateTime AssignedTime;

    UPROPERTY(BlueprintReadWrite, Category = "Agent Task")
    FDateTime CompletedTime;

    FDir_AgentTask()
    {
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Status = EDir_AgentStatus::Idle;
        Priority = 1.0f;
        AssignedTime = FDateTime::Now();
        CompletedTime = FDateTime::MinValue();
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Production")
    int32 TotalActorsSpawned;

    UPROPERTY(BlueprintReadWrite, Category = "Production")
    int32 DinosaurCount;

    UPROPERTY(BlueprintReadWrite, Category = "Production")
    int32 EnvironmentAssetsCount;

    UPROPERTY(BlueprintReadWrite, Category = "Production")
    float WorldCompletionPercentage;

    UPROPERTY(BlueprintReadWrite, Category = "Production")
    FString CurrentMilestone;

    FDir_ProductionMetrics()
    {
        TotalActorsSpawned = 0;
        DinosaurCount = 0;
        EnvironmentAssetsCount = 0;
        WorldCompletionPercentage = 0.0f;
        CurrentMilestone = TEXT("Milestone 1 - Walk Around");
    }
};

/**
 * Studio Director Subsystem - Coordinates all agent activities and tracks production progress
 * Manages the 19-agent pipeline and ensures milestone completion
 */
UCLASS()
class TRANSPERSONALGAME_API UStudioDirectorSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Agent coordination
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void AssignTaskToAgent(const FString& AgentName, const FString& TaskDescription, float Priority = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateAgentStatus(const FString& AgentName, EDir_AgentStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_AgentTask> GetActiveAgentTasks() const;

    // Production tracking
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FDir_ProductionMetrics GetProductionMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateProductionMetrics();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool IsMilestone1Complete() const;

    // World state validation
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool ValidateMinPlayableMap() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    int32 CountDinosaursInWorld() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool HasPlayableCharacter() const;

    // Emergency coordination
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void TriggerEmergencyProtocol(const FString& Reason);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void ResetProductionPipeline();

protected:
    UPROPERTY()
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY()
    FDir_ProductionMetrics CurrentMetrics;

    UPROPERTY()
    bool bEmergencyMode;

    UPROPERTY()
    FString EmergencyReason;

private:
    void InitializeAgentPipeline();
    void ValidateAgentDependencies();
    void UpdateMilestoneProgress();
    void LogProductionStatus() const;
};