#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "StudioDirectorSystem.generated.h"

UENUM(BlueprintType)
enum class EDir_AgentPriority : uint8
{
    LOW = 0,
    MEDIUM = 1,
    HIGH = 2,
    CRITICAL = 3
};

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    IDLE = 0,
    WORKING = 1,
    COMPLETED = 2,
    BLOCKED = 3,
    FAILED = 4
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString ExpectedDeliverable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    EDir_AgentPriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    EDir_AgentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    TArray<FString> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FDateTime AssignedTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FDateTime CompletedTime;

    FDir_AgentTask()
    {
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        ExpectedDeliverable = TEXT("");
        Priority = EDir_AgentPriority::MEDIUM;
        Status = EDir_AgentStatus::IDLE;
        AssignedTime = FDateTime::Now();
        CompletedTime = FDateTime(0);
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionCycle
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Cycle")
    FString CycleID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Cycle")
    FDateTime StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Cycle")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Cycle")
    TArray<FString> CriticalBlockers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Cycle")
    TArray<FString> CompletedDeliverables;

    FDir_ProductionCycle()
    {
        CycleID = TEXT("");
        StartTime = FDateTime::Now();
    }
};

/**
 * Studio Director System - Coordinates AI agent workflow and tracks production progress
 * Manages task assignment, dependency tracking, and deliverable validation
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

    // Current production cycle data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    FDir_ProductionCycle CurrentCycle;

    // Agent task management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void AssignTaskToAgent(const FString& AgentName, const FString& TaskDescription, 
                          const FString& ExpectedDeliverable, EDir_AgentPriority Priority,
                          const TArray<FString>& Dependencies);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateAgentStatus(const FString& AgentName, EDir_AgentStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CompleteAgentTask(const FString& AgentName, const FString& DeliverableDescription);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool AreAgentDependenciesMet(const FString& AgentName) const;

    // Critical blocker management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void AddCriticalBlocker(const FString& BlockerDescription);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void ResolveCriticalBlocker(const FString& BlockerDescription);

    // Production cycle management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void StartNewProductionCycle(const FString& CycleID);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void GenerateProductionReport() const;

    // Validation and quality control
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool ValidateMinimumViablePrototype() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FString> GetBlockingIssues() const;

    // Editor utilities
    UFUNCTION(CallInEditor, Category = "Studio Director")
    void InitializeCycle005Tasks();

    UFUNCTION(CallInEditor, Category = "Studio Director")
    void CheckCompilationStatus();

protected:
    // Internal task tracking
    UPROPERTY()
    TMap<FString, int32> AgentTaskIndices;

    // Performance tracking
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Studio Director", meta = (AllowPrivateAccess = "true"))
    float CycleStartTime;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Studio Director", meta = (AllowPrivateAccess = "true"))
    int32 TotalFilesCreated;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Studio Director", meta = (AllowPrivateAccess = "true"))
    int32 CompilationErrors;

private:
    void LogProductionMetrics() const;
    void ValidateAgentOutputs() const;
};