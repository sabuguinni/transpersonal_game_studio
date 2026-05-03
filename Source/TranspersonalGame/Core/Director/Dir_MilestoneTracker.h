#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/TextRenderComponent.h"
#include "Engine/Engine.h"
#include "Dir_MilestoneTracker.generated.h"

UENUM(BlueprintType)
enum class EDir_MilestoneStatus : uint8
{
    NotStarted      UMETA(DisplayName = "Not Started"),
    InProgress      UMETA(DisplayName = "In Progress"),
    Completed       UMETA(DisplayName = "Completed"),
    Blocked         UMETA(DisplayName = "Blocked")
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

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

    FDir_AgentTask()
    {
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Status = EDir_MilestoneStatus::NotStarted;
        Priority = 1.0f;
        Dependencies = TEXT("");
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
    TArray<FDir_AgentTask> Tasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    EDir_MilestoneStatus OverallStatus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    float CompletionPercentage;

    FDir_MilestoneData()
    {
        MilestoneName = TEXT("");
        Description = TEXT("");
        OverallStatus = EDir_MilestoneStatus::NotStarted;
        CompletionPercentage = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADir_MilestoneTracker : public AActor
{
    GENERATED_BODY()

public:
    ADir_MilestoneTracker();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UTextRenderComponent* ProgressDisplay;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* TrackerMesh;

    // Milestone Data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone Tracking")
    FDir_MilestoneData CurrentMilestone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone Tracking")
    TArray<FDir_MilestoneData> MilestoneHistory;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display")
    bool bShowProgressInViewport;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display")
    float UpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display")
    FLinearColor CompletedTaskColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display")
    FLinearColor InProgressTaskColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display")
    FLinearColor BlockedTaskColor;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Milestone Tracking")
    void InitializeMilestone1();

    UFUNCTION(BlueprintCallable, Category = "Milestone Tracking")
    void UpdateTaskStatus(const FString& AgentName, EDir_MilestoneStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Milestone Tracking")
    float CalculateCompletionPercentage();

    UFUNCTION(BlueprintCallable, Category = "Milestone Tracking")
    void UpdateProgressDisplay();

    UFUNCTION(BlueprintCallable, Category = "Milestone Tracking")
    void LogMilestoneProgress();

    UFUNCTION(BlueprintCallable, Category = "Milestone Tracking")
    bool IsMilestoneComplete();

    UFUNCTION(BlueprintCallable, Category = "Milestone Tracking")
    TArray<FString> GetBlockedTasks();

    UFUNCTION(BlueprintCallable, Category = "Milestone Tracking")
    TArray<FString> GetNextPriorityTasks();

private:
    float LastUpdateTime;
    
    void SetupComponents();
    void UpdateDisplayText();
    FString GenerateProgressReport();
};

#include "Dir_MilestoneTracker.generated.h"