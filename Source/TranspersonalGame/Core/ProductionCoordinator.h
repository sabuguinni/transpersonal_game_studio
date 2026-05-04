#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "SharedTypes.h"
#include "ProductionCoordinator.generated.h"

UENUM(BlueprintType)
enum class EDir_MilestoneStatus : uint8
{
    NotStarted      UMETA(DisplayName = "Not Started"),
    InProgress      UMETA(DisplayName = "In Progress"),
    Testing         UMETA(DisplayName = "Testing"),
    Complete        UMETA(DisplayName = "Complete"),
    Blocked         UMETA(DisplayName = "Blocked")
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    int32 AgentID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_MilestoneStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString Deliverables;

    FDir_AgentTask()
    {
        AgentID = 0;
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Status = EDir_MilestoneStatus::NotStarted;
        Priority = 1.0f;
        Dependencies = TEXT("");
        Deliverables = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct FDir_MilestoneData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString MilestoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    EDir_MilestoneStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FDir_AgentTask> RequiredTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    float CompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FDateTime StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FDateTime TargetTime;

    FDir_MilestoneData()
    {
        MilestoneName = TEXT("");
        Description = TEXT("");
        Status = EDir_MilestoneStatus::NotStarted;
        CompletionPercentage = 0.0f;
        StartTime = FDateTime::Now();
        TargetTime = FDateTime::Now();
    }
};

/**
 * Studio Director's Production Coordinator
 * Manages milestone tracking and agent task coordination for the prehistoric survival game
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AProductionCoordinator : public AActor
{
    GENERATED_BODY()

public:
    AProductionCoordinator();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* CoordinatorMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UTextRenderComponent* StatusDisplay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FDir_MilestoneData CurrentMilestone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_AgentTask> ActiveTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_MilestoneData> AllMilestones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    bool bAutoUpdateStatus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float UpdateInterval;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Production")
    void InitializeMilestone1();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateTaskStatus(int32 AgentID, EDir_MilestoneStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void AddAgentTask(const FDir_AgentTask& NewTask);

    UFUNCTION(BlueprintCallable, Category = "Production")
    float CalculateMilestoneProgress();

    UFUNCTION(BlueprintCallable, Category = "Production")
    FString GetProductionStatusReport();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void ValidateAgentDeliverables();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Production")
    void RefreshProductionStatus();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Production")
    void GenerateTaskAssignments();

private:
    void UpdateStatusDisplay();
    void CheckMilestoneCompletion();
    FString FormatStatusText();

    float LastUpdateTime;
    bool bMilestone1Initialized;
};