#pragma once

#include "CoreMinimal.h"
#include "Engine/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/World.h"
#include "ProductionDirector.generated.h"

/**
 * Studio Director's Production Coordinator Actor
 * Tracks Milestone 1 progress in real-time within MinPlayableMap
 * Coordinates agent deliverables and ensures gameplay-first development
 */

UENUM(BlueprintType)
enum class EDir_MilestoneStatus : uint8
{
    NotStarted      UMETA(DisplayName = "Not Started"),
    InProgress      UMETA(DisplayName = "In Progress"),
    Completed       UMETA(DisplayName = "Completed"),
    Blocked         UMETA(DisplayName = "Blocked")
};

USTRUCT(BlueprintType)
struct FDir_MilestoneTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString TaskName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    EDir_MilestoneStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    int32 Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString AssignedAgent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    float CompletionPercentage;

    FDir_MilestoneTask()
    {
        TaskName = TEXT("");
        Status = EDir_MilestoneStatus::NotStarted;
        Priority = 1;
        AssignedAgent = TEXT("");
        CompletionPercentage = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADir_ProductionDirector : public AActor
{
    GENERATED_BODY()

public:
    ADir_ProductionDirector();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* DirectorMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UTextRenderComponent* StatusDisplay;

    // Milestone 1 Tasks
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone 1")
    TArray<FDir_MilestoneTask> Milestone1Tasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone 1")
    float OverallProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone 1")
    bool bMilestone1Complete;

    // Production Tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 CurrentCycle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FString> AgentDeliverables;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateTaskStatus(const FString& TaskName, EDir_MilestoneStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void RecalculateProgress();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void LogProductionStatus();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Production")
    void InitializeMilestone1Tasks();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Production")
    void CheckMilestone1Completion();

private:
    void UpdateStatusDisplay();
    void SetupDefaultTasks();
};