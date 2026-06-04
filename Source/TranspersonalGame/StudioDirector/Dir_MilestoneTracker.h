#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
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
struct FDir_MilestoneData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString MilestoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    EDir_MilestoneStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    float CompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString AssignedAgent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    int32 CycleStarted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    int32 CycleCompleted;

    FDir_MilestoneData()
    {
        MilestoneName = TEXT("");
        Status = EDir_MilestoneStatus::NotStarted;
        CompletionPercentage = 0.0f;
        AssignedAgent = TEXT("");
        CycleStarted = -1;
        CycleCompleted = -1;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDir_MilestoneTracker : public UActorComponent
{
    GENERATED_BODY()

public:
    UDir_MilestoneTracker();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Milestone management
    UFUNCTION(BlueprintCallable, Category = "Milestone Tracker")
    void AddMilestone(const FString& MilestoneName, const FString& AssignedAgent);

    UFUNCTION(BlueprintCallable, Category = "Milestone Tracker")
    void UpdateMilestoneStatus(const FString& MilestoneName, EDir_MilestoneStatus NewStatus, float CompletionPercentage = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Milestone Tracker")
    void CompleteMilestone(const FString& MilestoneName);

    UFUNCTION(BlueprintCallable, Category = "Milestone Tracker")
    void BlockMilestone(const FString& MilestoneName, const FString& BlockReason);

    // Query functions
    UFUNCTION(BlueprintCallable, Category = "Milestone Tracker")
    TArray<FDir_MilestoneData> GetAllMilestones() const;

    UFUNCTION(BlueprintCallable, Category = "Milestone Tracker")
    FDir_MilestoneData GetMilestone(const FString& MilestoneName) const;

    UFUNCTION(BlueprintCallable, Category = "Milestone Tracker")
    TArray<FDir_MilestoneData> GetMilestonesByAgent(const FString& AgentName) const;

    UFUNCTION(BlueprintCallable, Category = "Milestone Tracker")
    TArray<FDir_MilestoneData> GetMilestonesByStatus(EDir_MilestoneStatus Status) const;

    // Production metrics
    UFUNCTION(BlueprintCallable, Category = "Milestone Tracker")
    float GetOverallCompletionPercentage() const;

    UFUNCTION(BlueprintCallable, Category = "Milestone Tracker")
    int32 GetCompletedMilestonesCount() const;

    UFUNCTION(BlueprintCallable, Category = "Milestone Tracker")
    int32 GetBlockedMilestonesCount() const;

    // Debug and logging
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Milestone Tracker")
    void LogMilestoneStatus();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Milestone Tracker")
    void InitializeWalkAroundMilestones();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone Tracker")
    TArray<FDir_MilestoneData> Milestones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone Tracker")
    int32 CurrentCycle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone Tracker")
    FString CurrentPhase;

private:
    void UpdateCurrentCycle();
    int32 FindMilestoneIndex(const FString& MilestoneName) const;
};