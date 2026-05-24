#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "UObject/UObjectGlobals.h"
#include "../SharedTypes.h"
#include "Quest_MissionProgressTracker.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMissionProgressUpdated, FString, MissionID, float, Progress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMissionCompleted, FString, MissionID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMissionFailed, FString, MissionID);

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_MissionProgress
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Progress")
    FString MissionID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Progress")
    FString MissionName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Progress")
    float CurrentProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Progress")
    float TargetProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Progress")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Progress")
    bool bIsFailed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Progress")
    FDateTime StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Progress")
    FDateTime CompletionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Progress")
    TArray<FString> ObjectiveIDs;

    FQuest_MissionProgress()
    {
        MissionID = TEXT("");
        MissionName = TEXT("");
        CurrentProgress = 0.0f;
        TargetProgress = 100.0f;
        bIsCompleted = false;
        bIsFailed = false;
        StartTime = FDateTime::Now();
        CompletionTime = FDateTime::MinValue();
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_ObjectiveProgress
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective Progress")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective Progress")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective Progress")
    float CurrentValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective Progress")
    float TargetValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective Progress")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective Progress")
    bool bIsOptional;

    FQuest_ObjectiveProgress()
    {
        ObjectiveID = TEXT("");
        Description = TEXT("");
        CurrentValue = 0.0f;
        TargetValue = 1.0f;
        bIsCompleted = false;
        bIsOptional = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuest_MissionProgressTracker : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_MissionProgressTracker();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Mission Management
    UFUNCTION(BlueprintCallable, Category = "Mission Progress")
    void StartMission(const FString& MissionID, const FString& MissionName, float TargetProgress = 100.0f);

    UFUNCTION(BlueprintCallable, Category = "Mission Progress")
    void UpdateMissionProgress(const FString& MissionID, float ProgressDelta);

    UFUNCTION(BlueprintCallable, Category = "Mission Progress")
    void CompleteMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Mission Progress")
    void FailMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Mission Progress")
    bool IsMissionActive(const FString& MissionID) const;

    UFUNCTION(BlueprintCallable, Category = "Mission Progress")
    float GetMissionProgress(const FString& MissionID) const;

    // Objective Management
    UFUNCTION(BlueprintCallable, Category = "Mission Progress")
    void AddObjective(const FString& MissionID, const FString& ObjectiveID, const FString& Description, float TargetValue = 1.0f, bool bIsOptional = false);

    UFUNCTION(BlueprintCallable, Category = "Mission Progress")
    void UpdateObjectiveProgress(const FString& ObjectiveID, float ProgressDelta);

    UFUNCTION(BlueprintCallable, Category = "Mission Progress")
    void CompleteObjective(const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Mission Progress")
    bool IsObjectiveCompleted(const FString& ObjectiveID) const;

    // Progress Queries
    UFUNCTION(BlueprintCallable, Category = "Mission Progress")
    TArray<FQuest_MissionProgress> GetActiveMissions() const;

    UFUNCTION(BlueprintCallable, Category = "Mission Progress")
    TArray<FQuest_ObjectiveProgress> GetMissionObjectives(const FString& MissionID) const;

    UFUNCTION(BlueprintCallable, Category = "Mission Progress")
    int32 GetCompletedMissionCount() const;

    UFUNCTION(BlueprintCallable, Category = "Mission Progress")
    float GetOverallProgressPercentage() const;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Mission Events")
    FOnMissionProgressUpdated OnMissionProgressUpdated;

    UPROPERTY(BlueprintAssignable, Category = "Mission Events")
    FOnMissionCompleted OnMissionCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Mission Events")
    FOnMissionFailed OnMissionFailed;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission Data", meta = (AllowPrivateAccess = "true"))
    TArray<FQuest_MissionProgress> ActiveMissions;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission Data", meta = (AllowPrivateAccess = "true"))
    TArray<FQuest_MissionProgress> CompletedMissions;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission Data", meta = (AllowPrivateAccess = "true"))
    TArray<FQuest_ObjectiveProgress> AllObjectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Settings")
    float ProgressUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Settings")
    bool bAutoCompleteOnTargetReached;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Settings")
    bool bLogProgressUpdates;

private:
    void CheckMissionCompletion(const FString& MissionID);
    void UpdateMissionFromObjectives(const FString& MissionID);
    FQuest_MissionProgress* FindMissionProgress(const FString& MissionID);
    FQuest_ObjectiveProgress* FindObjectiveProgress(const FString& ObjectiveID);
    void LogMissionEvent(const FString& Message);

    float LastUpdateTime;
};