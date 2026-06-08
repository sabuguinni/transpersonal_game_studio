#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "ProductionCoordinator.generated.h"

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    Planning,
    PreProduction,
    Production,
    Testing,
    Polish,
    Complete
};

USTRUCT(BlueprintType)
struct FDir_MilestoneData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString MilestoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    bool bIsComplete;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    float CompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FString> RequiredFeatures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FString> CompletedFeatures;

    FDir_MilestoneData()
    {
        MilestoneName = TEXT("Unnamed Milestone");
        bIsComplete = false;
        CompletionPercentage = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    int32 AgentNumber;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_ProductionPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    bool bIsBlocked;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString BlockingReason;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float Priority;

    FDir_AgentTask()
    {
        AgentNumber = 0;
        AgentName = TEXT("Unknown Agent");
        TaskDescription = TEXT("No task assigned");
        CurrentPhase = EDir_ProductionPhase::Planning;
        bIsBlocked = false;
        BlockingReason = TEXT("");
        Priority = 1.0f;
    }
};

/**
 * Production Coordinator - Studio Director's management system
 * Tracks milestone progress, agent coordination, and production metrics
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AProductionCoordinator : public AActor
{
    GENERATED_BODY()
    
public:    
    AProductionCoordinator();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production", meta = (AllowPrivateAccess = "true"))
    TArray<FDir_MilestoneData> Milestones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production", meta = (AllowPrivateAccess = "true"))
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production", meta = (AllowPrivateAccess = "true"))
    int32 CurrentCycle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production", meta = (AllowPrivateAccess = "true"))
    FString ProductionPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production", meta = (AllowPrivateAccess = "true"))
    float OverallProgress;

public:    
    virtual void Tick(float DeltaTime) override;

    // Milestone Management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void InitializeMilestones();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateMilestoneProgress(const FString& MilestoneName, float NewProgress);

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool IsMilestoneComplete(const FString& MilestoneName) const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    float GetMilestoneProgress(const FString& MilestoneName) const;

    // Agent Task Management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void AssignTaskToAgent(int32 AgentNumber, const FString& TaskDescription, float Priority = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void CompleteAgentTask(int32 AgentNumber);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void BlockAgent(int32 AgentNumber, const FString& Reason);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UnblockAgent(int32 AgentNumber);

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_AgentTask> GetBlockedAgents() const;

    // Production Metrics
    UFUNCTION(BlueprintCallable, Category = "Production")
    void IncrementCycle();

    UFUNCTION(BlueprintCallable, Category = "Production")
    float CalculateOverallProgress();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void LogProductionStatus();

    // Editor Functions
    UFUNCTION(CallInEditor, Category = "Production")
    void RefreshProductionData();

    UFUNCTION(CallInEditor, Category = "Production")
    void ResetAllTasks();

    UFUNCTION(CallInEditor, Category = "Production")
    void GenerateProductionReport();
};